
#include "StepSelector.h"

#include <berryISelectionService.h>
#include <berryUIException.h>
#include <berryWorkbenchPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>

#include <usModuleRegistry.h>
#include <QmitkDataStorageComboBox.h>

#include <QMessageBox>
#include <QPainter>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>

#include <mitkLabel.h>
#include <mitkToolManagerProvider.h>
#include <mitkOtsuSegmentationFilter.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkIOUtil.h>
#include <mitkIDataStorageService.h>
#include <QmitkDataNodeSelectionProvider.h>

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <itkImageRegionIterator.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkPaintbrushTool.h>
#include <QFile>
#include <itkBinaryThresholdImageFilter.h>
#include "my_awesomeproject_stepselector_PluginActivator.h"
#include <mitkImage.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <QPixmap>
#include <QMessageDialogWithToggle.h>

// Don't forget to initialize the VIEW_ID.
const string StepSelector::VIEW_ID = "my.organprint.views.stepselector";


StepDescriptor::StepDescriptor(const QString& pluginId, QPushButton* button,bool requireData = false,bool hideEditor = false)
    : pluginId(pluginId), button(button),requireData(requireData),hideEditor(hideEditor)
{}



void StepSelector::CreateQtPartControl(QWidget* parent)
{
    ui.setupUi(parent);
    this->GetRenderWindowPart(OPEN);
    this->RequestRenderWindowUpdate();


    QFile file(":/my.organprint.views.stepselector/Styles.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    parent->setStyleSheet(styleSheet);

    // ui.pushButton_3->hide();

    // Wire up the UI widgets with our functionality.
    m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    assert(m_ToolManager);

    // Set steps
    m_steps =
    {
        { "my.organprint.views.importpanel",		ui.pushButton_1 },
        { "org.mitk.views.segmentation",			ui.pushButton_2,true},
        { "my.organprint.views.tissupanel",          ui.pushButton_3,true},
        { "my.organprint.views.exportpanel",        ui.pushButton_4,true }
    };

    group = new QButtonGroup();
    // Connect Signals and Slots of the Plugin UI
    for (unsigned long i = 0; i < m_steps.size(); i++)
    {
        //connect(m_steps[i].button, &QPushButton::clicked, this, [this, i] { on_pushButton_clicked(i); });

        group->addButton(m_steps[i].button,i);


    }

    group->setExclusive(true);

    connect(group,SIGNAL(buttonClicked(int)),this,SLOT(on_pushButton_clicked(int)));



    // Initialize the first step
    m_steps[1].button->toggled(true);


    // registering event

    ctkDictionary propsForSlot;
    propsForSlot[ctkEventConstants::EVENT_TOPIC] = "my/organprint/stepselector";
    ctkPluginContext * pluginContext = my_awesomeproject_stepselector_PluginActivator::GetPluginContext();
    ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
    if (ref)
    {
        ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
        eventAdmin->subscribeSlot(this, SLOT(onChangeStepEvent(ctkEvent)), propsForSlot);
    }

    onNodeListChanged(nullptr);
    initListeners();
    on_pushButton_clicked(0);


    QPixmap logo(":/images/logo-color.png");

    ui.logoLabel->setPixmap(logo.scaled(130,130,Qt::KeepAspectRatio));


}

StepSelector::StepSelector()
    : m_currentStep(-1)
{
}

StepSelector::~StepSelector()
{
}

void StepSelector::selectView(int n)
{
    cout << "Selecting view " << n << endl;

    const std::string editorArea = "org.mitk.editors.stdmultiwidget";

    if (n < 0 || n >= (int)m_steps.size())
        return;
    auto site = this->GetSite();
    if (site == nullptr)
        return;
    auto workbenchWindow = site->GetWorkbenchWindow();
    if (workbenchWindow == nullptr)
        return;
    auto page = workbenchWindow->GetActivePage();
    if (page == nullptr)
        return;

    cout << "Settting step to " << n << endl;
    const auto& current = m_steps[m_currentStep];

    if(m_currentStep >= 0) {
        berry::IViewPart::Pointer view = page->FindView(current.pluginId);

        if(view !=nullptr) {
            page->HideView(view);
        }
    }

    if(n == 1) {
        displayHelp("my.organpring.views.stepselector","skipHelpStep2",":/my.organprint.views.stepselector/step2.html");
    }
    page->ShowView(m_steps[n].pluginId);
    m_currentStep = n;
    /*
    for (int i = 0; i < (int) m_steps.size(); i++)
    {

        cout << "requested step = " << n << " | i = " << i << endl;
        const auto& step = m_steps[i];
        berry::IViewPart::Pointer view = page->FindView(step.pluginId);



        try
        {

            cout << "requested step = " << n << " | i = " << i << endl;
            //cout << "view :" << view << endl;
            if (i == n)
            {   // Open
                //step.button->checked(true);

                page->ShowView(step.pluginId);


                //step.button->setStyleSheet("background-color: #3399cc");

                // step.button->SetDisabled(false);
            }
            else if (i != n && view != nullptr)
            {   // Close
                page->HideView(view);
                // step.button->setStyleSheet("");
            }



        }
        catch (const berry::PartInitException& e)
        {
            BERRY_ERROR << "Error: " << e.what();
        }
    }
    */

}

void StepSelector::SetFocus()
{

}

void StepSelector::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&)
{

    /*
    /// Make invisible all the nodes but the selected ones.
    auto pluginContext = my_awesomeproject_stepselector_PluginActivator::GetPluginContext();
    ctkServiceReference serviceReference = pluginContext->getServiceReference<mitk::IDataStorageService>();
    mitk::IDataStorageService* storageService = pluginContext->getService<mitk::IDataStorageService>(serviceReference);
    mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();
    auto all_nodes = dataStorage->GetAll();
    for (auto datanode : *all_nodes)
    {
        mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
        if (!image)
            continue;

        bool is_selected_node = false;
        for (auto selected_node : selectedDataNodes)
        {
            if (datanode.GetPointer() == selected_node.GetPointer())
            {
                is_selected_node = true;
                break;
            }
        }
        datanode->SetBoolProperty("visible", is_selected_node);
    }
    this->RequestRenderWindowUpdate();


    cout << &selectedDataNodes << endl;
    */

}

void StepSelector::on_pushButton_clicked(int step)
{
    cout << "on_pushButton_clicked " << step << endl;
    selectView(step);
}
void StepSelector::onChangeStepEvent(const ctkEvent & event) {
    cout << "onChangeStepEvent " << event.getProperty("step").toInt() << endl;
    int id = event.getProperty("step").toInt();
    //group->buttonClicked(id);
    group->button(id)->setChecked(true);
    selectView(id);

}

void StepSelector::onNodeListChanged(const mitk::DataNode*) {

    cout << "The list has changed ! " << endl;

    SetOfObjects::ConstPointer nodes = GetDataStorage()->GetSubset(mitk::NodePredicateProperty::New("visible",mitk::BoolProperty::New(true)));

    SetOfObjects::ConstIterator it = nodes->Begin();

    int count = 0;

    for(mitk::DataNode::Pointer node = it->Value(); it!= nodes->End(); it++) {

        node = it->Value();

        mitk::Image * image = dynamic_cast<mitk::Image*>(node->GetData());



        std::string path = "";
        bool exists = node->GetStringProperty("path",path);
        if(image!=nullptr) {
            count++;
        }

        else if(exists) {
            count++;
        }

    }

    cout << "Number of nodes : " << nodes->Size() << endl;

    for (int i = 0; i < (int) m_steps.size(); i++) {


        StepDescriptor step = m_steps[i];

        step.button->setEnabled(!step.requireData || count > 0);

    }
}

void StepSelector::initListeners() {

    mitk::DataStorage * storage = GetDataStorage();

    storage->AddNodeEvent.AddListener(
        mitk::MessageDelegate1<StepSelector, const mitk::DataNode *>(this, &StepSelector::onNodeListChanged));

    storage->RemoveNodeEvent.AddListener(
        mitk::MessageDelegate1<StepSelector, const mitk::DataNode *>(this, &StepSelector::onNodeListChanged));

}

void StepSelector::displayHelp(const char *group, const char * settingKey, const char *helpPath) {

    bool skip = QMessageDialogWithToggle::getSettingValue(group,settingKey);

    if(!skip) {

        QMessageDialogWithToggle dialog(nullptr);
        dialog.setMessageFromResource(helpPath);
        dialog.setImage(":/images/logo-color.png");
        dialog.setCheckBoxText("Don't remind me next time.");
        dialog.setToggled(false);
        bool skip = dialog.exec();

        if(skip) {
            QMessageDialogWithToggle::setSettingValue(group,settingKey,true);
        }

    }

}
