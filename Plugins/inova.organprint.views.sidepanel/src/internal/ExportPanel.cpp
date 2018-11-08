
#include "ExportPanel.h"


#include "STLExportService.h"
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <usModuleRegistry.h>

#include <QMessageBox>

#include <mitkWorkbenchUtil.h>

#include <QmitkIOUtil.h>

#include <berryIPreferences.h>

#include <QFileDialog>
#include <QmitkExtFileSaveProjectAction.h>
#include <QTextStream>
#include <mitkStatusBar.h>
#include <mitkProgressBar.h>

#include <ExternalProgramOpener.h>
#include <sstream>
#include <mitkNodePredicateProperty.h>
#include <QMessageBox>

// Don't forget to initialize the VIEW_ID.
const std::string orgpnt::ExportPanel::VIEW_ID = "inova.organprint.views.exportpanel";

orgpnt::ExportPanel::ExportPanel():
    listener(StorageListener(this, &ExportPanel::OnNodeChanged))
{

}

void orgpnt::ExportPanel::CreateQtPartControl(QWidget* parent)
{
    // Setting up the UI is a true pleasure when using .ui files, isn't it?
    m_Controls.setupUi(parent);

    // Wire up the UI widgets with our functionality.
    connect(m_Controls.exportSTLButton, SIGNAL(clicked()), this, SLOT(ExportInSTL()));
    connect(m_Controls.saveButton, SIGNAL(clicked()), this, SLOT(SaveProject()));
    connect(m_Controls.slicerButton,SIGNAL(clicked()),this,SLOT(OpenWith3DSlicer()));

    mitk::DataStorage * storage = GetDataStorage();

    storage->ChangedNodeEvent.AddListener(listener);

    mitk::DataStorage::SetOfObjects::ConstPointer nodes = storage->GetSubset(nullptr);

    for(mitk::DataStorage::SetOfObjects::ConstIterator it = nodes->Begin(); it!=nodes->End(); it++) {
        mitk::DataNode::Pointer node = it->Value();
        OnNodeChanged(node);
    }

}

orgpnt::ExportPanel::~ExportPanel() {
    MITK_DEBUG << "Destroying tissue panel" << endl;

    mitk::DataStorage * storage = GetDataStorage();

    storage->ChangedNodeEvent.RemoveListener(listener);
}

void orgpnt::ExportPanel::SetFocus()
{
    //m_Controls.openImageButton->setFocus();
}



void orgpnt::ExportPanel::ExportInSTL() {

    STLExportService * service = new STLExportService();
    QString dir = QFileDialog::getExistingDirectory(nullptr, tr("Open Directory"),
                  QString(), // standard path
                  QFileDialog::ShowDirsOnly
                  | QFileDialog::DontResolveSymlinks);

    if(dir != nullptr) {
        service->exportTo(dir,GetDataStorage());
    }
}

void orgpnt::ExportPanel::SaveProject() {

    QmitkExtFileSaveProjectAction * action = new QmitkExtFileSaveProjectAction(this->GetSite()->GetWorkbenchWindow());
    action->Run();

}
void orgpnt::ExportPanel::OpenWith3DSlicer() {


    STLExportService * exportService = new STLExportService();

    mitk::ProgressBar * progress = mitk::ProgressBar::GetInstance();
    progress->Reset();

    progress->AddStepsToDo(4);

    progress->Progress(2);

    progress->SetPercentageVisible(true);

    mitk::DataStorage * storage = GetDataStorage();

    ExternalProgramOpener opener("3D Slicer","Slicer");

    SetOfObjects::ConstPointer selected = exportService->GetSelectedNodes(storage);
    SetOfObjects::ConstIterator iterator = selected->Begin();

    while(iterator != selected->End()) {

        const mitk::DataNode * node = iterator->Value();

        bool isSegmentation = false;

        if(!node) continue;

        node->GetBoolProperty("segmentation",isSegmentation);

        if(!isSegmentation) {

            std::stringstream sstr;

            sstr << "Only segmented node can be exported.\nIgnoring ";
            sstr << node->GetName();

            QMessageBox msgBox;
            msgBox.setText(sstr.str().c_str());
            msgBox.exec();
            ++iterator;
            continue;
        }


        mitk::StatusBar::GetInstance()->DisplayText("Opening in 3D Slicer...");

        // creating file name
        std::string tmpFile(std::tmpnam(nullptr));
        QString finalPath((tmpFile  + std::string("_") + node->GetName() + std::string(".stl")).c_str());
        opener.addArgument(finalPath);
        MITK_INFO << "Creating temporay file and adding it to the list of arguments" << finalPath.toStdString();

        // getting the smoothing out of it
        bool smoothing = m_Controls.smoothingCheckBox->checkState() == Qt::Checked;


        // exporting in STL
        exportService->exportToSTL(finalPath,node,smoothing);
        ++iterator;
    }
// running the 3DSlicer
    opener.run();
}

void orgpnt::ExportPanel::OnNodeChanged(const mitk::DataNode *node) {

    bool selected = false;
    if(node) {
        node->GetBoolProperty("selected",selected);
    }
    else {
        return;
    }

    if(selected) {

        m_Controls.selectedNodeLabel->setText(QString::fromStdString(node->GetName()));

    }
}

