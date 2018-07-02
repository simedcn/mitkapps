#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
//#include <QSettings>

#include <mitkDataStorage.h>
#include <mitkImage.h>

#include <berryIPreferences.h>

#include <list>

using namespace std;

class DataManager : public QObject
{
  Q_OBJECT
public:
  explicit DataManager(mitk::DataStorage* datastorage, QObject* parent = nullptr);

//signals:

public slots:
	int on_LoadImageSet(const QString& filenameAndDirectory);
	int on_LoadImageFolder(const QString& directory);

protected:
	int AskAboutNewPatient();

	mitk::DataNode::Pointer AddImage(const string& name, mitk::Image::Pointer image);

	void SaveDataOfCurrentPatient();
	vector<string> LoadDataOfCurrentPatient();

	//vector<string> GetDICOMFilesInSameDirectory(const string& filePath);

protected:
	mitk::DataStorage* m_DataStorage;
	QString m_PatientId;


protected:
	//static QSettings m_Settings;
	static berry::IPreferences::Pointer m_PreferencesNode;

protected:
	static berry::IPreferences::Pointer getPreferencesNode();
public:
	static QString GetWorkDirectory();
	static void SetWorkDirectory(const QString& dir_path);
};

#endif // DATAMANAGER_H
