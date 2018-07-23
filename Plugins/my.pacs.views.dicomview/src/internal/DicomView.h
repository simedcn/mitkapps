/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef DICOMVIEW_H_
#define DICOMVIEW_H_

#include <QmitkAbstractView.h>
#include <berryQtViewPart.h>
#include <berryIPreferences.h>

#include "dcmtk/dcmnet/scu.h"

#include <QProgressDialog>
#include <QAbstractButton>

// ctkDICOMCore includes
#include <ctkDICOMDatabase.h>
#include <ctkDICOMModel.h>
#include <ctkDICOMQuery.h>
#include <ctkDICOMRetrieve.h>
#include <service/event/ctkEventAdmin.h>

#include <memory>

using namespace std;

class ctkDICOMTableManager;
namespace Ui
{
	class DicomViewControls;
}
enum RetrieveProtocol;

/**
 * \brief A view class suited for the DicomPerspective within the custom viewer plug-in.
 *
 * This view class contributes dicom import functionality to the DicomPerspective.
 * The view controls are provided within CreatePartControl() by the QmitkDicomExternalDataWidget
 * class. A DicomView instance is part of the DicomPerspective for Dicom import functionality.
 */
// //! [DicomViewDecl]
class DicomView : public QWidget
// //! [DicomViewDecl]
{
	Q_OBJECT;
	//Q_PROPERTY(ctkDICOMTableManager* dicomTableManager READ dicomTableManager)

public:
	/// String based view identifier.
	static const string VIEW_ID;

	/// Standard constructor.
	DicomView();

	/// Standard destructor.
	virtual ~DicomView();

	/// Creates the view control widgets provided by the QmitkDicomExternalDataWidget class.
	/// Widgets associated with unused functionality are being removed and DICOM import and data storage transfer funcionality being connected to the appropriate slots.
	void CreateQtPartControl(QWidget *parent);

protected:
	void updateSettings();
	void updateProtocolInUI();
	void updateSettingsVisibility();
	void updateButtons();
	//void updateUI();
	QString getDestinationAE(RetrieveProtocol protocol);
	void addSeriesToDataManager(const QStringList& listOfFilesForSeries, const shared_ptr<QString> modality);

protected:
	void SetFocus();

private:
	void OnPreferencesChanged(const berry::IBerryPreferences*);

signals:
	void OpenDICOMSeries(const ctkDictionary&);

protected slots:
	void on_pushButton_Query_clicked();
	void on_pushButton_Retrieve_clicked();
	void on_canceled();
	void on_queryProgress_changed(int value);
	void on_retrieveProgress_changed(int value);
	void on_retrieveProgress_changed(QLabel* progressLabel, QString value);
	void on_pushButton_AddToDataManager_clicked();
	void on_pushButton_ShowSettings_clicked();
	//void on_UploadToPACSAction_run(const string&);
	// Settings
	void on_lineEdit_IP_textChanged(QString);
	void on_spinBox_Port_valueChanged(int);
	void on_lineEdit_HostAETitle_textChanged(QString);
	void on_pushButton_PublicPACS_MedicalConnections_clicked();
	void on_pushButton_PublicPACS_PixelMed_clicked();
	//void on_comboBox_Protocol_currentIndexChanged(int);
	void on_buttonGroup_Protocol_buttonToggled(QAbstractButton*, bool);
	void on_buttonGroup_Destination_buttonToggled(QAbstractButton*, bool);
	void on_lineEdit_DestinationAETitle_textChanged(QString);
	void on_lineEdit_LocalFolderPath_textChanged(QString);
	void on_pushButton_LocalFolderPath_clicked();
	void on_pushButton_SetTemporaryLocalFolderPath_clicked();
	void on_lineEdit_StorageAETitle_textChanged(QString);
	void on_lineEdit_LocalStorageIP_textChanged(QString);
	void on_spinBox_LocalStoragePort_valueChanged(int);

	// Tables
	void on_table_Studies_selectionChanged(const QStringList&);
	void on_table_Series_selectionChanged(const QStringList&);

protected:
	Ui::DicomViewControls& ui;
	QWidget *m_Parent;
	berry::IPreferences::Pointer m_DicomViewPreferencesNode;
	QString m_StoreSCUExecutable;
	bool is_savedDestination_LocalFolder = false;
	bool are_settings_visible = false;
	bool is_retrieved = false;

	ctkDICOMDatabase queryResultDatabase;
	ctkDICOMQuery* query;
	QMap<QString, ctkDICOMQuery*> queriesByStudyUID;
	QMap<QString, ctkDICOMRetrieve*> retrievalsByStudyUID;
	ctkDICOMModel model;
	QProgressDialog* progressDialog = nullptr;
	QSharedPointer<ctkDICOMDatabase> retrieveDatabase;
	shared_ptr<QTemporaryDir> temp_dir = nullptr;
	QString retrieveFolder;
	list<QString> retrievedFiles;

	//QmitkDicomDataEventPublisher* m_Publisher;
};

enum RetrieveProtocol
{
	PROTOCOL_CGET = 0,
	PROTOCOL_CMOVE = 1
};

enum RetrieveDestination
{
	DESTINATION_LOCALFOLDER = 0,
	DESTINATION_PACS = 1
};
static const QString NO_DIRECTORY_SPECIFIED = "<TEMP>";

class LocalSCU : public QObject, public DcmSCU
{
	Q_OBJECT
protected:
	void notifyRECEIVEProgress(const unsigned long byteCount) override;
	void notifyInstanceStored(const OFString &filename, const OFString &sopClassUID, const OFString &sopInstanceUID) const override;

signals:
	void progress_text(QString);
	void progress(int) const;

protected slots:
	void cancel();

protected:
	int num_received = 0;
	unsigned long prevByteCount = 0;
	unsigned long totalByteCount = 0;
};

#endif /*DICOMVIEW_H_*/
