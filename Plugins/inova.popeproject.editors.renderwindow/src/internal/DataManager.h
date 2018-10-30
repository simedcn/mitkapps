#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
//#include <QSettings>

#include <mitkDataStorage.h>
#include <mitkImage.h>

#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEventConstants.h>

#include <berryIPreferences.h>

#include <list>
#include <algorithm>
#include <iterator>

using namespace std;


class DataManager : public QObject
{
	Q_OBJECT
public:
	explicit DataManager(mitk::DataStorage* datastorage, QObject* parent = nullptr);

signals:
	void GonnaAddNewDataNode();
	void NewDataNodeAdded();

public slots:
	int on_LoadImageSet(const QString& filenameAndDirectory);
	int on_LoadImageFolder(const QString& directory);
	int on_LoadImageSeries(const QStringList& files, const QStringList& series);
	//int on_LoadImage(const string& name, mitk::Image::Pointer image);
	void on_Action_OpenDICOMdataset_clicked(const ctkEvent& event);
	void on_Action_OpenFolder_clicked(const ctkEvent& event);
	void on_Action_OpenDICOMSeries_clicked(const ctkEvent& event);
	void On_ToolsPlugin_AllNodesRemoved(const ctkEvent& event);
	//void on_Action_LoadImage(const ctkEvent& event);

protected:
	int AskAboutNewPatient();

	bool IsTimeGeometryOK(mitk::Image::Pointer image);
	bool IsCentralRegionOK(mitk::Image::Pointer image);
	void CheckImageSpacing(mitk::Image::Pointer image, const string& image_name);
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
