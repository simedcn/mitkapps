//#pragma once
#ifndef DICOMVIEWPREFERENCEPAGE_H
#define DICOMVIEWPREFERENCEPAGE_H

#include "ui_DicomViewPreferences.h"

#include <berryIPreferences.h>
#include <berryIQtPreferencePage.h>

using namespace std;

class QWidget;
class QCheckBox;
class QRadioButton;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QPushButton;


class DicomViewPreferencePage : public QObject, public berry::IQtPreferencePage
{
	Q_OBJECT
	Q_INTERFACES(berry::IPreferencePage)

public:
	/// Build UI in constructor
	DicomViewPreferencePage(QWidget* parent = 0, Qt::WindowFlags f = 0);
	/// Invoked when the OK button was clicked in the preferences dialog
	virtual bool PerformOk() override;
	/// Invoked when the Cancel button was clicked in the preferences dialog
	virtual void PerformCancel() override;
	/// \see IPreferencePage::Update()
	virtual void Update() override;

	void Init(berry::IWorkbench::Pointer workbench) override;
	void CreateQtControl(QWidget* widget) override;
	QWidget* GetQtControl() const override;

protected:
	void updateProtocolInUI();

protected slots:
	void on_pushButton_PublicPACS_MedicalConnections_clicked();
	void on_pushButton_PublicPACS_PixelMed_clicked();
	void on_lineEdit_LocalFolderPath_textChanged(QString);
	void on_pushButton_LocalFolderPath_clicked();
	void on_pushButton_SetTemporaryLocalFolderPath_clicked();
	void on_buttonGroup_Protocol_buttonToggled(QAbstractButton*, bool);

protected:
	QWidget* m_Control = nullptr;
	berry::IPreferences::Pointer m_DicomViewPreferencesNode;
	bool is_savedDestination_LocalFolder = false;

	Ui::DicomViewPreferences ui;
};
#endif // DICOMVIEWPREFERENCEPAGE_H