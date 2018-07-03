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

#include "ui_DicomViewControls.h"

#include <QmitkAbstractView.h>
#include <berryQtViewPart.h>
#include <berryIPreferences.h>

#include <QProgressDialog>

// ctkDICOMCore includes
#include <ctkDICOMDatabase.h>
#include <ctkDICOMModel.h>
#include <ctkDICOMQuery.h>
#include <ctkDICOMRetrieve.h>

#include <memory>

using namespace std;

class ctkDICOMTableManager;

/**
 * \brief A view class suited for the DicomPerspective within the custom viewer plug-in.
 *
 * This view class contributes dicom import functionality to the DicomPerspective.
 * The view controls are provided within CreatePartControl() by the QmitkDicomExternalDataWidget
 * class. A DicomView instance is part of the DicomPerspective for Dicom import functionality.
 */
// //! [DicomViewDecl]
class DicomView : public QmitkAbstractView
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
  virtual void CreateQtPartControl(QWidget *parent) override;

protected:
	void updateSettings();
	void addSeriesToDataManager(const QStringList& listOfFilesForSeries, const shared_ptr<QString> modality);

protected:
  void SetFocus() override;

private:
  virtual void OnPreferencesChanged(const berry::IBerryPreferences*) override;

protected slots:
	void on_pushButton_Query_clicked();
	void on_pushButton_Retrieve_clicked();
	void on_canceled();
	void on_queryProgress_changed(int value);
	void on_retrieveProgress_changed(int value);
	void on_pushButton_AddToDataManager_clicked();
	//void on_UploadToPACSAction_run(const string&);
	// Settings
	void on_lineEdit_IP_textChanged(QString);
	void on_spinBox_Port_valueChanged(int);
	void on_lineEdit_AETitle_textChanged(QString);
	void on_comboBox_Protocol_currentIndexChanged(int);
	void on_lineEdit_StorageAETitle_textChanged(QString);

protected:
  Ui::DicomViewControls ui;
  QWidget *m_Parent;
  berry::IPreferences::Pointer m_DicomViewPreferencesNode;
  QString m_StoreSCUExecutable;

  ctkDICOMDatabase queryResultDatabase;
  ctkDICOMQuery* query;
  QMap<QString, ctkDICOMQuery*> queriesByStudyUID;
  QMap<QString, ctkDICOMRetrieve*> retrievalsByStudyUID;
  ctkDICOMModel model;
  QProgressDialog* progressDialog = nullptr;
  QSharedPointer<ctkDICOMDatabase> retrieveDatabase;

  //QmitkDicomDataEventPublisher* m_Publisher;
};

enum RetrieveProtocol
{
	PROTOCOL_CMOVE = 0,
	PROTOCOL_CGET = 1
};

#endif /*DICOMVIEW_H_*/
