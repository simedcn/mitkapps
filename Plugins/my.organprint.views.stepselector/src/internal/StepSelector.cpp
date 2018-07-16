
#include "StepSelector.h"

#include <berryISelectionService.h>
#include "berryUIException.h"
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

#include "mitkLabel.h"
#include "mitkToolManagerProvider.h"
#include "mitkOtsuSegmentationFilter.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkIOUtil.h"

// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usModuleResource.h"

#include "itkImageRegionIterator.h"
#include "mitkImageCast.h"
#include <mitkITKImageImport.h>
#include <mitkPaintbrushTool.h>
#include <QFile>
#include <itkBinaryThresholdImageFilter.h>
#include "my_awesomeproject_stepselector_PluginActivator.h"



// Don't forget to initialize the VIEW_ID.
const string StepSelector::VIEW_ID = "my.organprint.views.stepselector";


StepDescriptor::StepDescriptor(const QString& pluginId, QPushButton* button)
    : pluginId(pluginId), button(button)
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

    ui.pushButton_3->hide();

    // Wire up the UI widgets with our functionality.
    m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    assert(m_ToolManager);

    // Set steps
    m_steps =
    {
        { "my.organprint.views.importpanel",		ui.pushButton_1 },
        { "org.mitk.views.segmentation",			ui.pushButton_2 },
        { "", 	ui.pushButton_3 },
        { "my.organprint.views.exportpanel", 				ui.pushButton_4 }
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

    on_pushButton_clicked(0);

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

}

StepSelector::StepSelector()
    : m_currentStep(0)
{
}

StepSelector::~StepSelector()
{
}

void StepSelector::selectView(int n)
{
    cout << "Selecting view " << n << endl;
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

    m_currentStep = n;

    for (int i = 0; i < (int) m_steps.size(); i++)
    {
        const auto& step = m_steps[i];
        berry::IViewPart::Pointer view = page->FindView(step.pluginId);
        try
        {

            cout << "currentStep = " << m_currentStep << " | i = " << i << endl;



            if (i == m_currentStep && view == nullptr)
            {   // Open
                //step.button->checked(true);

                page->ShowView(step.pluginId);
                //step.button->setStyleSheet("background-color: #3399cc");

                // step.button->SetDisabled(false);
            }
            else if (i != m_currentStep && view != nullptr)
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
}

void StepSelector::SetFocus()
{
}

void StepSelector::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& dataNodes)
{

    cout << &dataNodes << endl;

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
