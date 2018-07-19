
#ifndef StepSelector_h
#define StepSelector_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <QPushButton>

#include <memory>
#include <vector>
#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include "my_awesomeproject_stepselector_PluginActivator.h"
#include <mitkDataStorage.h>
using namespace std;


// There's an item "StepSelectorControls.ui" in the UI_FILES list in
// files.cmake. The Qt UI Compiler will parse this file and generate a
// header file prefixed with "ui_", which is located in the build directory.
// Use Qt Creator to view and edit .ui files. The generated header file
// provides a class that contains all of the UI widgets.
#include <ui_StepSelectorControls.h>
#include <QButtonGroup>


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


// All views in MITK derive from QmitkAbstractView. You have to override
// at least the two methods CreateQtPartControl() and SetFocus().
class StepSelector : public QmitkAbstractView
{
    Q_OBJECT


public:
    // This is a tricky one and will give you some headache later on in your debug sessions if it has been forgotten.
    // Also, don't forget to initialize it in the implementation file.
    static const std::string VIEW_ID;

    typedef mitk::DataStorage::SetOfObjects SetOfObjects;

    StepSelector();
    ~StepSelector();

    // In this method we initialize the GUI components and connect the associated signals and slots.
    void CreateQtPartControl(QWidget* parent) override;


private:
    // Typically a one-liner. Set the focus to the default widget.
    void SetFocus() override;

    // This method is conveniently called whenever the selection of Data Manager items changes.
    void OnSelectionChanged(berry::IWorkbenchPart::Pointer source, const QList<mitk::DataNode::Pointer>& dataNodes) override;

    void selectView(int n);

    // Generated from the associated UI file, it encapsulates all the widgets of our view.
    Ui::StepSelectorControls ui;
    mitk::ToolManager *m_ToolManager;
    int m_currentStep;
    vector<StepDescriptor> m_steps;

    QButtonGroup * group;
public:
    void onNodeListChanged(const mitk::DataNode*);
private slots:
    void on_pushButton_clicked(int step);
    void initListeners();


public slots:
    void onChangeStepEvent(const ctkEvent&);

};
struct StepDescriptor
{
    QString pluginId;
    QPushButton* button;
    bool requireData;
    bool hideEditor;
    StepDescriptor(const QString& pluginId, QPushButton* button, bool requireData,bool hideEditor);

};

#endif
