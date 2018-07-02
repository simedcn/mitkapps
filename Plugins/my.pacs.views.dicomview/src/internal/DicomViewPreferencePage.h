//#pragma once
#ifndef DICOMVIEWPREFERENCEPAGE_H
#define DICOMVIEWPREFERENCEPAGE_H

#include <berryIPreferences.h>
#include "berryIQtPreferencePage.h"

using namespace std;

class QWidget;
class QCheckBox;
class QRadioButton;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;
class QComboBox;


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

protected slots:
	//void OnVolumeRenderingCheckboxChecked(int);
	//void OnSmoothingCheckboxChecked(int);

protected:
	QWidget* m_MainControl = nullptr;
	bool m_Initializing = false;
	berry::IPreferences::Pointer m_DicomViewPreferencesNode;

	QLineEdit *m_lineEdit_IP;
	QSpinBox *m_spinBox_Port;
	QLineEdit *m_lineEdit_AETitle;
	QComboBox *m_comboBox_Protocol;
	QLineEdit *m_lineEdit_StorageAETitle;
};
#endif // DICOMVIEWPREFERENCEPAGE_H