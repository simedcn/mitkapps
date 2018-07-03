#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QSettings>
#include <memory>
#include <mitkDataStorage.h>
#include <mitkImage.h>

class DataManager : public QObject
{
  Q_OBJECT
public:
  explicit DataManager(mitk::DataStorage* datastorage, QObject *parent = nullptr);
signals:

public slots:

  int LoadImageSet(const QString& filenameAndDirectory);

protected:

  void SaveDataOfCurrentPatient();

  std::vector<std::string> LoadDataOfCurrentPatient();

  bool ProbeDicomFile(const std::string& filename, std::string& patientId);

  void AddImage(const std::string &name, mitk::Image::Pointer image);

  std::vector<std::string> GetDICOMFilesInSameDirectory(const std::string &filePath);

  mitk::DataStorage* m_DataStorage;
  QString m_PatientId;
  QString m_CurrentPath;
  QSettings m_Settings;
};

#endif // DATAMANAGER_H
