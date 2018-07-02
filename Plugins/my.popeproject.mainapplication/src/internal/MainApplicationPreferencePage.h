//#pragma once
#ifndef MAINAPPLICATIONPREFERENCEPAGE_H
#define MAINAPPLICATIONPREFERENCEPAGE_H

#include <PopeElements.h>

#include <berryIPreferences.h>
#include "berryIQtPreferencePage.h"

using namespace std;

class QWidget;
class QCheckBox;
class QRadioButton;
class QDoubleSpinBox;
class QComboBox;


class MainApplicationPreferencePage : public QObject, public berry::IQtPreferencePage
{
	Q_OBJECT
	Q_INTERFACES(berry::IPreferencePage)

protected:
	struct ViewDescriptor;

public:
	/// Build UI in constructor
	MainApplicationPreferencePage(QWidget* parent = 0, Qt::WindowFlags f = 0);
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
	berry::IPreferences::Pointer m_MainApplicationPreferencesNode;

	vector<ViewDescriptor> m_Views;

protected:
	struct ViewDescriptor
	{
		QCheckBox* checkBox = nullptr;
		const Elements::PluginDescriptor* plugin = nullptr;

		ViewDescriptor(QCheckBox* checkBox, const Elements::PluginDescriptor* plugin);
	};
};

#endif // MAINAPPLICATIONPREFERENCEPAGE_H