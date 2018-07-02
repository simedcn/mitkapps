#include "DataManager.h"
#include "itkGDCMImageIO.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <mitkIOUtil.h>
#include <mitkRenderingManager.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

DataManager::DataManager(mitk::DataStorage *datastorage, QObject *parent) :
  QObject(parent),
  m_DataStorage(datastorage),
  m_Settings("savedFiles.ini",QSettings::IniFormat)
{
}

bool DataManager::ProbeDicomFile(const std::string& filename, std::string& patientId)
{
  itk::GDCMImageIO::Pointer reader = itk::GDCMImageIO::New();
  if(!reader->CanReadFile(filename.c_str()))
    return false;
  reader->SetFileName(filename);
  reader->ReadImageInformation();
  char id[512];
  reader->GetPatientID(id);
  patientId = id;
  return true;
}

void DataManager::AddImage(const std::string& name, mitk::Image::Pointer image)
{
  bool isInDS = false;//is the datanode already loaded?
  auto allNodes = this->m_DataStorage->GetAll();
  auto iterator = allNodes->Begin();
  while(iterator != allNodes->End())
  {
    mitk::DataNode::Pointer datanode = iterator->Value();
    ++iterator;
    if(datanode->GetName().compare(name) == 0)
    {
      isInDS = true;
      iterator = allNodes->End();
    }
  }

  mitk::DataNode::Pointer dn = mitk::DataNode::New();
  if(!isInDS)
  {
  dn->SetName(name);
  dn->SetData(image);
  this->m_DataStorage->Add(dn);
  }
}

int DataManager::LoadImageSet(const QString &filenameAndDirectory)
{
  std::string patientId;
  bool success = this->ProbeDicomFile(filenameAndDirectory.toStdString(), patientId);
  if(!success) return 0;

  bool addImage = m_PatientId.isEmpty() || (QString(patientId.c_str()) == m_PatientId);
  if(!addImage)
  {
    QMessageBox msgBox;
    msgBox.setText("The file you chose contains data of a different patient");
    msgBox.setInformativeText("Do you want to save your changes and clear the datastorage?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Save:
      this->SaveDataOfCurrentPatient();
      break;
    case QMessageBox::Cancel:
      return 0;
      break;
    default:
      break;
    }
    m_DataStorage->Remove(this->m_DataStorage->GetSubset(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)))));
  }

  m_PatientId = patientId.c_str();
  QFileInfo fi(filenameAndDirectory);
  this->m_CurrentPath = fi.dir().absolutePath();

  std::vector<std::string> loadedFiles = this->LoadDataOfCurrentPatient();
  auto loaded = mitk::IOUtil::Load(filenameAndDirectory.toStdString());

  for (auto baseData : loaded)
  {
    mitk::DataNode::Pointer dn = mitk::DataNode::New();
    std::string curPatID;
    if (baseData->GetProperty("dicom.patient.PatientID"))
      curPatID = baseData->GetProperty("dicom.patient.PatientID")->GetValueAsString();
    else if (baseData->GetProperty("DICOM.0010.0020"))
      curPatID = baseData->GetProperty("DICOM.0010.0020")->GetValueAsString();


    std::string imageName = "image";
    if(baseData->GetProperty("dicom.series.SeriesInstanceUID"))
      imageName = baseData->GetProperty("dicom.series.SeriesInstanceUID")->GetValueAsString();
    if(baseData->GetProperty("dicom.series.Modality"))
    {
      imageName.append("_");
      imageName.append(baseData->GetProperty("dicom.series.Modality")->GetValueAsString());
    }
    if(patientId.compare(curPatID) == 0)
    {
      bool isInDS = false;//is the datanode already loaded?
      isInDS = !loadedFiles.empty() && std::find(loadedFiles.begin(),loadedFiles.end(), imageName) == loadedFiles.end();

      mitk::Image* img = dynamic_cast<mitk::Image*>(baseData.GetPointer());
      if(!isInDS && img)
      {
        this->AddImage(imageName, img);
      }
    }
  }
  // Initialize views as axial, sagittal, coronar (from
  // top-left to bottom)
  mitk::TimeGeometry::ConstPointer geo = this->m_DataStorage->ComputeBoundingGeometry3D(this->m_DataStorage->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(geo);
  //this->InitializeView();

  return 0;
}

void DataManager::SaveDataOfCurrentPatient()
{
  auto allNodes = this->m_DataStorage->GetAll();
  auto iterator = allNodes->Begin();
  while(iterator != allNodes->End())
  {
    mitk::DataNode::Pointer datanode = iterator->Value();
    mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
    if(image)
    {

      QString filename = datanode->GetName().c_str();
      filename.append(".nrrd");
      QString path = this->m_CurrentPath+"/"+filename;
      mitk::IOUtil::Save(image,path.toStdString());
      this->m_Settings.beginGroup(this->m_PatientId);

      this->m_Settings.setValue(datanode->GetName().c_str(),path);
      this->m_Settings.endGroup();
    }
    ++iterator;
  }
}

std::vector<std::string> DataManager::LoadDataOfCurrentPatient()
{
  std::vector<std::string> loadedFiles;
  if(this->m_CurrentPath.isEmpty() || this->m_PatientId.isEmpty()) return loadedFiles;
  this->m_Settings.beginGroup(this->m_PatientId);
  auto keys = this->m_Settings.childKeys();
  for(auto key : keys)
  { /*dicom.series.SeriesInstanceUID*/
    QString segFile = this->m_Settings.value(key).toString();
    mitk::Image::Pointer image = LoadImage(segFile.toStdString());
    this->AddImage(key.toStdString(),image);
    loadedFiles.push_back(key.toStdString());
  }
  this->m_Settings.endGroup(); //patientId
  return loadedFiles;
}

mitk::Image::Pointer DataManager::LoadImage(const std::string &path)
{
    mitk::Image::Pointer image
        =  mitk::IOUtil::Load<mitk::Image>(path);

    //mitk::Image::Pointer image = dynamic_cast<mitk::Image::Pointer>(baseData);

    return image;
}
