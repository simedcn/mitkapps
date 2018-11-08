#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QSettings>
#include <memory>
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEventConstants.h>


class DataManager : public QObject
{
  Q_OBJECT
public:
  explicit DataManager(mitk::DataStorage* datastorage, QObject *parent = nullptr);
signals:

public slots:
	void OpenDICOMSeries(const ctkEvent& event);

protected:

  void SaveDataOfCurrentPatient();

  std::vector<std::string> LoadDataOfCurrentPatient();

  bool ProbeDicomFile(const std::string& filename, std::string& patientId);

  void AddImage(const std::string &name, mitk::Image::Pointer image);

  int LoadImageSet(const QString &filenameAndDirectory);

  int AskAboutNewPatient();
  int LoadImageDataSet(const QString& filenameAndDirectory);
  int LoadImageFolder(const QString& directory);

  mitk::DataStorage* m_DataStorage;
  QString m_PatientId;
  QString m_CurrentPath;
  QSettings m_Settings;
};

#endif // DATAMANAGER_H
