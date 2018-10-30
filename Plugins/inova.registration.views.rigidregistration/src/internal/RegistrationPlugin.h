#ifndef RegistrationPlugin_h
#define RegistrationPlugin_h

#include "ImageRegistrationCalculationThread.h"

#include <mitkILifecycleAwarePart.h>
#include <berryIPartListener.h>

#include <berryIPreferences.h>
#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>

#include <ctkPluginContext.h>

//#include <QTimer>
#include <memory>

// There's an item "RegistrationPluginControls.ui" in the UI_FILES list in
// files.cmake. The Qt UI Compiler will parse this file and generate a
// header file prefixed with "ui_", which is located in the build directory.
// Use Qt Creator to view and edit .ui files. The generated header file
// provides a class that contains all of the UI widgets.
#include <ui_RegistrationPluginControls.h>

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

// All views in MITK derive from QmitkAbstractView. You have to override
// at least the two methods CreateQtPartControl() and SetFocus().
class RegistrationPlugin : public QmitkAbstractView//, public mitk::ILifecycleAwarePart//, public berry::IPartListener
{
	// As QmitkAbstractView derives from QObject and we want to use the Qt
	// signal and slot mechanism, we must not forget the Q_OBJECT macro.
	// This header file also has to be listed in MOC_H_FILES in files.cmake,
	// in order that the Qt Meta-Object Compiler can find and process this
	// class declaration.
	Q_OBJECT

public:
	// This is a tricky one and will give you some headache later on in your debug sessions if it has been forgotten.
	// Also, don't forget to initialize it in the implementation file.
	static const std::string VIEW_ID;

	RegistrationPlugin();
	~RegistrationPlugin();

	// In this method we initialize the GUI components and connect the associated signals and slots.
	void CreateQtPartControl(QWidget* parent) override;

protected:
	// Typically a one-liner. Set the focus to the default widget.
	void SetFocus() override;
	// This method is conveniently called whenever the selection of Data Manager items changes.
	void OnSelectionChanged(berry::IWorkbenchPart::Pointer source, const QList<mitk::DataNode::Pointer>& dataNodes) override;
	void NodeRemoved(const mitk::DataNode* node) override;
	void OnPreferencesChanged(const berry::IBerryPreferences*) override;

protected:
	void CheckInputs();
protected:
	void updateButton(bool enabled = true);
	std::string updateNewImageName(mitk::DataNode::Pointer datanode);
	void AddImage(const string& name, mitk::Image::Pointer image);

protected:
	/// Method called when itkModifiedEvent is called by selected data.
	void SelectedDataModified();

private slots:
	void on_pushButton_Settings_clicked();
	void on_pushButton_Registration_clicked();
protected slots:
	void on_ThreadedRegistrationCalculation_finished();

signals:
	void GonnaAddNewDataNode(const ctkDictionary&);
	void NewDataNodeAdded(const ctkDictionary&);
	void PluginIsBusy(const ctkDictionary&);
	void PluginIsIdle(const ctkDictionary&);

private:
	// Generated from the associated UI file, it encapsulates all the widgets of our view.
	Ui::RegistrationPluginControls ui;
	berry::IPreferences::Pointer m_RegistrationPluginPreferencesNode;
	//mitk::DataNode::Pointer m_SelectedMovingNode;
	std::string m_moving_image_name;
	//static int name_counter;
protected:
	ImageRegistrationCalculationThread m_CalculationThread;
};

#endif
