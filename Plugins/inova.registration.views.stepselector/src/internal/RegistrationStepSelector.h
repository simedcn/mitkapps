#ifndef RegistrationStepSelector_h
#define RegistrationStepSelector_h

#include <mitkILifecycleAwarePart.h>
#include <berryIPartListener.h>

#include <berryIPreferences.h>
#include <berryISelectionListener.h>
#include <berryIPartListener.h>
#include <QmitkAbstractView.h>
//#include <berryQtViewPart.h>

#include <ctkPluginContext.h>

//#include <QTimer>
#include <memory>
#include <climits>

// There's an item "RegistrationStepSelectorControls.ui" in the UI_FILES list in
// files.cmake. The Qt UI Compiler will parse this file and generate a
// header file prefixed with "ui_", which is located in the build directory.
// Use Qt Creator to view and edit .ui files. The generated header file
// provides a class that contains all of the UI widgets.
#include <ui_RegistrationStepSelectorControls.h>

#include <service/event/ctkEventAdmin.h>

using namespace std;

namespace mitk
{
	class LabelSetImage;
	class LabelSet;
	class Label;
	class DataStorage;
	class ToolManager;
	class DataNode;
}

struct StepDescriptor;


class RegistrationStepSelector : public QmitkAbstractView //berry::QtViewPart, public berry::IPartListener, public mitk::ILifecycleAwarePart
{
	Q_OBJECT

public:
	static const std::string VIEW_ID;
	static const QString PLUGIN_ID;

	RegistrationStepSelector();
	~RegistrationStepSelector();

	void CreateQtPartControl(QWidget* parent) override;

protected:
	void selectItem(size_t n = ULLONG_MAX);
	void updateButtons();

protected:
	void SetFocus() override;
	//void OnSelectionChanged(berry::IWorkbenchPart::Pointer source, const QList<mitk::DataNode::Pointer>& dataNodes) override;
	//void OnPreferencesChanged(const berry::IBerryPreferences*) override;

protected slots:
	void on_pushButton_clicked(int step);
	void on_Plugin_visible(ctkEvent event);
	void on_Plugin_hidden(ctkEvent event);
	void on_Plugin_isBusy(ctkEvent event);
	void on_Plugin_isIdle(ctkEvent event);

protected:
	Ui::RegistrationStepSelectorControls ui;
	vector<StepDescriptor> item_IDs;
	size_t current_item;
};


struct StepDescriptor
{
	QString pluginId;
	QPushButton* button;
	bool is_busy;

	StepDescriptor(const QString& pluginId, QPushButton* button);
};

#endif
