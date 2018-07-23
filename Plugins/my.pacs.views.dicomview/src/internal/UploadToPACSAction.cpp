#include "UploadToPACSAction.h"

#include <PopeElements.h>

// MITK
#include <mitkProgressBar.h>
#include <mitkStatusBar.h>

#include <mitkIRenderWindowPart.h>
#include <mitkIRenderingManager.h>
#include <mitkImage.h>

// Blueberry
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryPlatform.h>
//#include <berryIWorkbenchPage.h>

#include <QDir>
#include <QProcess>
#include <QCoreApplication>
#include <QMessageBox>

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

using namespace berry;
using namespace mitk;
using namespace std;


UploadToPACSAction::UploadToPACSAction()
{}

UploadToPACSAction::~UploadToPACSAction()
{}

bool UploadToPACSAction::findStoreSCU()
{
    QString filepath = QCoreApplication::applicationFilePath();
    auto exe_path = QFileInfo(filepath).absolutePath();
    QString storeSCU_filePath = "bin/storescu";
#if defined(_WIN32) || defined(_WIN64)
    storeSCU_filePath += ".exe";
//elif defined(__APPLE__) ||  defined(__MACH__) || defined(__linux__)
    // no file extension
#endif
    m_StoreSCUExecutable = QDir(exe_path).filePath(storeSCU_filePath);
    auto storeSCUfile = QFileInfo(m_StoreSCUExecutable);
    if (!storeSCUfile.exists() || !storeSCUfile.isExecutable())
    {
        MITK_WARN << "Wrong storeSCU executable: " << qPrintable(m_StoreSCUExecutable);
        return false;
    }
    return true;
}
void UploadToPACSAction::Run(const QList<DataNode::Pointer> &selectedNodes)
{
    DataNode::Pointer selectedNode = selectedNodes[0];
    Image::Pointer image = dynamic_cast<mitk::Image*>(selectedNode->GetData());

    if (image.IsNull())
        return;

    auto baseData = selectedNode->GetData();
    if (baseData == nullptr)
        return;

    QStringList files;
    vector<int> nums;
    string prop_files = baseData->GetProperty("files")->GetValueAsString();
    bool OK = Elements::split_properties(prop_files, &files, &nums);

    cout << "Was it OK ? " << OK << endl;
    /*if (!OK)
    {
      string path = baseData->GetProperty("path")->GetValueAsString();
      QFileInfo fi(QString::fromStdString(path));
      QString folderpath = fi.dir().absolutePath();
      QDir dir = folderpath;
      dir.setFilter(QDir::Files);
      files = dir.entryList();
    }*/

    int num_files = files.size();
    if (num_files == 0)
        return;

    findStoreSCU();

    berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
    auto dicomViewPreferencesNode = prefService->GetSystemPreferences()->Node("/my.pacs.views.dicomview");

    QProcess storeSCU(this);
    // Usage of storescu:
    // storescu -aec CTK_AE -aet CTK_AE localhost 11112 ./CMakeExternals/Source/CTKData/Data/DICOM/MRHEAD/*.IMA
    QString host = dicomViewPreferencesNode->Get("PACS storage local IP", "127.0.0.1");
    int port = dicomViewPreferencesNode->GetInt("PACS storage local port", 11112);
    QString storageAETitle = dicomViewPreferencesNode->Get("PACS storage local AETitle", "SERVERAE");
// QString AETitle = dicomViewPreferencesNode->Get("PACS host AETitle", "SERVERAE");
    QStringList storescuArgs;
    storescuArgs << "-aec" << storageAETitle;
    storescuArgs << "-aet" << "STORESCU";
    storescuArgs << host;
    storescuArgs << QString::number(port);
    storescuArgs << files;

    storeSCU.start(m_StoreSCUExecutable, storescuArgs);
    bool res = storeSCU.waitForFinished(-1);

    QTextStream out(stdout);
    out << "Uploading to PACS is finished.\n";

    QByteArray standardOutput = storeSCU.readAllStandardOutput();
    if (standardOutput.count())
    {
        out << "Standard Output:\n";
        out << standardOutput;
    }
    QByteArray standardError = storeSCU.readAllStandardError();
    if (standardError.count())
    {
        out << "Standard Error:\n";
        out << standardError;
    }
    out.flush();

    if (!res)
    {
        stringstream str_dataset;
        if (files.size() == 2)
        {
            str_dataset << "(and one more image) ";
        }
        else if (files.size() > 1)
        {
            str_dataset << "(and " << (files.size() - 1) << " more images) ";
        }
        MITK_ERROR << "Failed to store data \"" << qPrintable(files.front()) << "\" " << str_dataset.str()
                   << " via StoreSCU: " << qPrintable(m_StoreSCUExecutable);
        stringstream ss;
        ss << "Unable to upload the data you selected to PACS (IP/Host: " << host << "; Port: " << port << "; AE-Title: " << storageAETitle
           << "), using the local AE-Title \"" << "STORESCU" << "\".";
        QMessageBox::warning(nullptr, "Cannot upload data", QString::fromStdString(ss.str()));
    }
    storeSCU.kill();
    res = storeSCU.waitForFinished(-1);
    //if (!res) // Is OK?
    //	MITK_INFO << "Failed to stop storeSCU: " << qPrintable(m_StoreSCUExecutable);
}

void UploadToPACSAction::OnSurfaceCalculationDone()
{
    StatusBar::GetInstance()->Clear();
}

void UploadToPACSAction::SetDataStorage(DataStorage *dataStorage)
{
    m_DataStorage = dataStorage;
}

void UploadToPACSAction::SetSmoothed(bool smoothed)
{
    m_IsSmoothed = smoothed;
}

void UploadToPACSAction::SetDecimated(bool decimated)
{
    m_IsDecimated = decimated;
}

void UploadToPACSAction::SetFunctionality(QtViewPart *)
{
}
