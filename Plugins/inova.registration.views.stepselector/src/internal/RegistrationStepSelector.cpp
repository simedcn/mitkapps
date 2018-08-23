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
	: pluginId(pluginId), button(button)
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
		{ "inova.registration.views.comparison", ui.pushButton_JuxtaposeResults },
		{ "inova.registration.views.visualizer", ui.pushButton_VisualizeResults },
		//"inova.registration.views.rigidregistration", },
		//"inova.registration.views.mapper", },
	};
	current_item = 0;

	/// Connect Signals and Slots of the Plugin UI
	for (int i = 0; i < item_IDs.size(); i++)
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

	if (n >= 0 && n < item_IDs.size())
		this->current_item = n;

	for (int i = 0; i < item_IDs.size(); i++)
	{
		const auto& item = item_IDs[i];
		berry::IViewPart::Pointer view = page->FindView(item.pluginId);
		try
		{
			if (i == n)
			{// Open
				if (view == nullptr)
					view = page->ShowView(item.pluginId);
				item.button->setStyleSheet("background-color: #3399cc");
			}
			else
			{// Close
				if (view != nullptr)
					page->HideView(view);
				item.button->setStyleSheet("");
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
void RegistrationStepSelector::OnPreferencesChanged(const berry::IBerryPreferences*)
{
}*/

void RegistrationStepSelector::on_pushButton_clicked(int item)
{
	selectItem(item);
}