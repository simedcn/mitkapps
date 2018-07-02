//#pragma once
#ifndef POPEPREFERENCEPAGE_H
#define POPEPREFERENCEPAGE_H

#include <berryIPreferences.h>
#include <berryIQtPreferencePage.h>

using namespace std;

class QWidget;
class QCheckBox;
class QRadioButton;
class QDoubleSpinBox;
class QComboBox;
class QLineEdit;


class PopePreferencePage : public QObject, public berry::IQtPreferencePage
{
	Q_OBJECT
	Q_INTERFACES(berry::IPreferencePage)
	//berryObjectMacro(PopePreferencePage)

public:
	/// Build UI in constructor
	PopePreferencePage(QWidget* parent = 0, Qt::WindowFlags f = 0);
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
	berry::IPreferences::Pointer m_PopePreferencesNode;

	QCheckBox* m_CheckBox_SaveSession;
	QComboBox* m_ComboBox_CloseablePlugins;
	QComboBox* m_ComboBox_MoveablePlugins;
	QCheckBox* m_CheckBox_LoadRNND;
	QLineEdit* m_LineEdit_DefaultPath;
	//QDoubleSpinBox* m_SmoothingSpinBox;
	//QDoubleSpinBox* m_DecimationSpinBox;
	//QDoubleSpinBox* m_ClosingSpinBox;
	//QCheckBox* m_SelectionModeCheckBox;
};

enum CloseableMoveablePlugins : int
{
	NotForAll = 0,
	YesForAllButMain = 1,
	YesForAll = 2
};
#endif // POPEPREFERENCEPAGE_H