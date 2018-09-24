#include "RegistrationStepSelector.h"
#include "inova_registration_views_stepselector_PluginActivator.h"
#include <PopeElements.h>

#include <berryIWorkbenchPage.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryQtWorkbenchAdvisor.h>

#include <usModuleRegistry.h>
#include <QmitkDataStorageComboBox.h>

#include <QMessageBox>
#include <QPainter>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include <Qt>
#include <QScrollBar>

//#include <PopeImageFilter.h>
//#include <PopeImageInteractor.h>

#include <vtkSTLWriter.h>

#include <ctkPluginActivator.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEventConstants.h>

#include <mitkLabel.h>
#include <mitkIOUtil.h>
#include <mitkIRenderWindowPart.h>
#include <mitkImage.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkIDataStorageService.h>
#include <mitkProgressBar.h>

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <itkImageRegionIterator.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include <itkBinaryThresholdImageFilter.h>
#include <itksys/SystemTools.hxx>

//#include <QmitkDataNodeSelectionProvider.h>
//#include <QmitkDataManagerView.h>
#include <QmitkRenderWindow.h>

#include <berryWorkbenchPlugin.h>
#include <berryQtPreferences.h>

#include <algorithm>
#include <iterator>

#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>
#include <itkShiftScaleImageFilter.h>
#include <itkImageMomentsCalculator.h>
#include <itkTranslationTransform.h>
#include <itkResampleImageFilter.h>

using namespace std;

// Don't forget to initialize the VIEW_ID.
const string RegistrationStepSelector::VIEW_ID = "inova.registration.views.stepselector";
const QString RegistrationStepSelector::PLUGIN_ID = QString::fromStdString(VIEW_ID);


StepDescriptor::StepDescriptor(const QString& pluginId, QPushButton* button)
    : pluginId(pluginId)
    , button(button)
    , is_busy(false)
{}


RegistrationStepSelector::RegistrationStepSelector()
{
    /// CTK slots.
    auto pluginContext = inova_registration_views_stepselector_PluginActivator::GetPluginContext();
    ctkDictionary propsForSlot;
    ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
    assert(ref);
    if (ref)
    {
        ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
        propsForSlot[ctkEventConstants::EVENT_TOPIC] = "plugin/VISIBLE";
        eventAdmin->subscribeSlot(this, SLOT(on_Plugin_visible(ctkEvent)), propsForSlot);
        propsForSlot[ctkEventConstants::EVENT_TOPIC] = "plugin/HIDDEN";
        eventAdmin->subscribeSlot(this, SLOT(on_Plugin_hidden(ctkEvent)), propsForSlot);
        propsForSlot[ctkEventConstants::EVENT_TOPIC] = "registration/PLUGINISBUSY";
        eventAdmin->subscribeSlot(this, SLOT(on_Plugin_isBusy(ctkEvent)), propsForSlot);
        propsForSlot[ctkEventConstants::EVENT_TOPIC] = "registration/PLUGINISIDLE";
        eventAdmin->subscribeSlot(this, SLOT(on_Plugin_isIdle(ctkEvent)), propsForSlot);
    }
}
RegistrationStepSelector::~RegistrationStepSelector()
{
    //selectItem();
}

void RegistrationStepSelector::CreateQtPartControl(QWidget* parent)
{
    /// Setting up the UI is a true pleasure when using .ui files, isn't it?
    ui.setupUi(parent);
    //this->GetRenderWindowPart(OPEN);
    //this->RequestRenderWindowUpdate();

    /// Create a list of items.
    item_IDs =
    {
        { "inova.registration.views.manualregistration", ui.pushButton_ManualRegistration },
        { "inova.registration.views.registrationalgorithms", ui.pushButton_AutomaticRegistration },
        { "inova.registration.views.frameregistration", ui.pushButton_TimeFrameRegistration },
        {"inova.registration.views.rigidregistration", ui.pushButton_RigidRegistration },
        { "inova.registration.views.comparison", ui.pushButton_JuxtaposeResults },
        { "inova.registration.views.visualizer", ui.pushButton_VisualizeResults },
        //{"inova.registration.views.mapper", },
    };
    current_item = 0;

    /// Connect Signals and Slots of the Plugin UI
    for (uint i = 0; i < item_IDs.size(); i++)
    {
        connect(item_IDs[i].button, &QPushButton::clicked, this, [this, i] { on_pushButton_clicked(i); });
    }
    //selectItem();
}

void RegistrationStepSelector::on_Plugin_visible(ctkEvent event)
{
    QString plugin_id = event.getProperty("id").toString();
    if (plugin_id == PLUGIN_ID)
        selectItem(current_item);
}
void RegistrationStepSelector::on_Plugin_hidden(ctkEvent event)
{
    QString plugin_id = event.getProperty("id").toString();
    if (plugin_id == PLUGIN_ID)
        selectItem();
}
void RegistrationStepSelector::on_Plugin_isBusy(ctkEvent event)
{
    QString plugin_id = event.getProperty("id").toString();
    for (auto& plugin : item_IDs)
    {
        if (plugin.pluginId == plugin_id)
        {
            plugin.is_busy = true;
            break;
        }
    }
    updateButtons();
}
void RegistrationStepSelector::on_Plugin_isIdle(ctkEvent event)
{
    QString plugin_id = event.getProperty("id").toString();
    for (auto& plugin : item_IDs)
    {
        if (plugin.pluginId == plugin_id)
        {
            plugin.is_busy = false;
            break;
        }
    }
    updateButtons();
}

void RegistrationStepSelector::updateButtons()
{
    bool is_busy_plugin = false;
    for (auto& plugin : item_IDs)
    {
        if (plugin.is_busy)
        {
            is_busy_plugin = true;
            break;
        }
    }
    for (auto& plugin : item_IDs)
    {
        QString styleSheet = plugin.button->styleSheet();
        const QString sColor = "color: #E0B000;\n";
        bool is_colored = styleSheet.contains(sColor);
        if (plugin.is_busy && !is_colored)
        {
            plugin.button->setStyleSheet(sColor + styleSheet);
        }
        else if (!plugin.is_busy && is_colored)
        {
            styleSheet.remove(sColor);
            plugin.button->setStyleSheet(styleSheet);
        }
        plugin.button->setEnabled(!is_busy_plugin);
    }
}

void RegistrationStepSelector::SetFocus()
{
    ui.pushButton_ManualRegistration->setFocus();
}

void RegistrationStepSelector::selectItem(size_t n)
{
    auto site = this->GetSite();
    if (site == nullptr)
        return;
    auto workbenchWindow = site->GetWorkbenchWindow();
    if (workbenchWindow == nullptr)
        return;
    auto page = workbenchWindow->GetActivePage();
    if (page == nullptr)
        return;

    if (n < item_IDs.size())
        this->current_item = n;

    for (uint i = 0; i < item_IDs.size(); i++)
    {
        const auto& item = item_IDs[i];
        berry::IViewPart::Pointer view = page->FindView(item.pluginId);
        try
        {
            const QString bkgColor = "background-color: #3399cc;\n";
            QString styleSheet = item.button->styleSheet();
            if (i == n)
            {   // Open
                if (view == nullptr)
                    view = page->ShowView(item.pluginId);
                if (!styleSheet.contains(bkgColor))
                    item.button->setStyleSheet(bkgColor + styleSheet);
            }
            else
            {   // Close
                if (view != nullptr)
                    page->HideView(view);
                if (styleSheet.contains(bkgColor))
                {
                    styleSheet.remove(bkgColor);
                    item.button->setStyleSheet(styleSheet);
                }
            }
        }
        catch (const berry::PartInitException& e)
        {
            BERRY_ERROR << "Error: " << e.what();
        }
    }
}

/*void RegistrationStepSelector::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& selectedDataNodes)
{
}
void RegistrationStepSelector::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
	if (prefs == nullptr)
		return;
	//MITK_INFO << prefs->
}*/

void RegistrationStepSelector::on_pushButton_clicked(int item)
{
    selectItem(item);
}
