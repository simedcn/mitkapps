#include "RegistrationPlugin.h"
#include "my_popeproject_Registrationplugin_PluginActivator.h"
#include <PopeElements.h>
#include <PopeImageFilter.h>

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

#include <QmitkDataNodeSelectionProvider.h>
#include <QmitkDataManagerView.h>
#include <QmitkRenderWindow.h>

#include <berryWorkbenchPlugin.h>
#include <berryQtPreferences.h>

#include <algorithm>
#include <iterator>

using namespace std;

// Don't forget to initialize the VIEW_ID.
const string RegistrationPlugin::VIEW_ID = "my.popeproject.views.registrationplugin";

int RegistrationPlugin::name_counter = 1;

QString get_short_name_for_image(const string& name)
{
	QString str_name = QString::fromStdString(name);
	QString short_name = str_name;
	const string str_center_replacement = "...";
	const int max_length = 28 + str_center_replacement.length();
	if (name.length() > max_length)
	{
		stringstream ss;
		const int half = (max_length - str_center_replacement.length()) / 2;
		ss << name.substr(0, half) << str_center_replacement << name.substr(name.length() - half, half);
		short_name = QString::fromStdString(ss.str());
	}
	return short_name;
}

RegistrationPlugin::RegistrationPlugin()
{}
RegistrationPlugin::~RegistrationPlugin()
{
	while (this->m_CalculationThread.isRunning())
	{
		itksys::SystemTools::Delay(100);
	}
}

void RegistrationPlugin::CreateQtPartControl(QWidget* parent)
{
	/// Setting up the UI is a true pleasure when using .ui files, isn't it?
	ui.setupUi(parent);
	this->GetRenderWindowPart(OPEN);
	this->RequestRenderWindowUpdate();

	/// Set preferences node.
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	this->m_RegistrationPluginPreferencesNode = prefService->GetSystemPreferences()->Node("/my.popeproject.views.registrationplugin");

	// Connect Signals and Slots of the Plugin UI
	connect(this->ui.pushButton_Registration, SIGNAL(clicked()), this, SLOT(on_pushButton_Registration_clicked()));
	connect((QObject*)&this->m_CalculationThread, SIGNAL(finished()), this, SLOT(on_ThreadedRegistrationCalculation_finished()), Qt::QueuedConnection);

	/// CTK signals.
	/// Creating an Event Publisher.
}

void RegistrationPlugin::SetFocus()
{
	ui.pushButton_Registration->setFocus();
}

void RegistrationPlugin::updateButton(bool enabled)
{
	ui.pushButton_Registration->setText(m_CalculationThread.isRunning() ? "Stop Registration" : "Start Registration");
	ui.pushButton_Registration->setEnabled(m_CalculationThread.isRunning() || enabled);
}

void RegistrationPlugin::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& selectedDataNodes)
{
	/// Update selected images.
	mitk::DataNode::Pointer moving_image = nullptr;
	mitk::DataNode::Pointer target_image = nullptr;
	for (auto datanode : selectedDataNodes)
	{
		mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
		if (!image)
			continue;
		if (moving_image == nullptr)
		{
			moving_image = datanode;
		}
		else
		{
			target_image = datanode;
			break;
		}
	}

	/// Set moving image.
	if (moving_image != nullptr)
	{
		string name = moving_image->GetName();
		if (name.empty())
		{
			ui.label_MovingImageNotSelected->setText("");//("<b>Profile</b>");
		}
		else
		{
			QString short_name = get_short_name_for_image(name);
			ui.label_MovingImageNotSelected->setText(short_name);
			ui.label_MovingImageNotSelected->setToolTip(QString::fromStdString(name));
		}
		ui.label_MovingImageNotSelected->setStyleSheet("");
		ui.label_MovingImageNotSelected->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		ui.label_MovingImageNotSelected->setText("Please select a moving image in Data Manager.");
		ui.label_MovingImageNotSelected->setStyleSheet("color: #E02000;\nbackground-color: #efef95;");
		ui.label_MovingImageNotSelected->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	}
	ui.label_SelectedMovingImage->setVisible(moving_image != nullptr);

	/// Set target image.
	if (target_image != nullptr)
	{
		string name = target_image->GetName();
		if (name.empty())
		{
			ui.label_TargetImageNotSelected->setText("");//("<b>Profile</b>");
		}
		else
		{
			QString short_name = get_short_name_for_image(name);
			ui.label_TargetImageNotSelected->setText(short_name);
			ui.label_TargetImageNotSelected->setToolTip(QString::fromStdString(name));
		}
		ui.label_TargetImageNotSelected->setStyleSheet("");
		ui.label_TargetImageNotSelected->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		ui.label_TargetImageNotSelected->setText("Please select a moving image in Data Manager.");
		ui.label_TargetImageNotSelected->setStyleSheet("color: #E02000;\nbackground-color: #efef95;");
		ui.label_TargetImageNotSelected->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	}
	ui.label_SelectedTargetImage->setVisible(target_image != nullptr);

	bool enabled = (moving_image != nullptr && target_image != nullptr);
	updateButton(enabled);
}

void RegistrationPlugin::OnPreferencesChanged(const berry::IBerryPreferences*)
{
}

void RegistrationPlugin::on_pushButton_Registration_clicked()
{
	/// Start / Stop.
	if (m_CalculationThread.isRunning())
	{
		m_CalculationThread.Abort();
		// updateButton()
		return;
	}
	/// Get selected images.
	auto selection = GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager");
	if (selection == nullptr)
	{
		MITK_INFO << "No selection.";
		return;
	}
	auto node_selection = selection.Cast<const mitk::DataNodeSelection>();
	if (node_selection == nullptr)
	{
		MITK_INFO << "No Data Node Selection.";
		return;
	}
	auto dataNodes = node_selection->GetSelectedDataNodes();
	mitk::Image* moving_image = nullptr;
	mitk::Image* target_image = nullptr;
	for (auto datanode : dataNodes)
	{
		mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
		if (!image)
			continue;
		if (moving_image == nullptr)
		{
			moving_image = image;
		}
		else
		{
			target_image = image;
			break;
		}
	}
	if (target_image == nullptr)
	{
		MITK_INFO << "2 images should be selected.";
		return;
	}

	/// Move the moving image to the center of gravity of the fixed image.
	auto filter = PopeImageFilter::New();
	filter->SetInput(0, moving_image);
	filter->SetInput(1, target_image);
	//filter->SetFixedImage(target_image);
	//filter->SetOffset(10);
	filter->Update();
	auto translated_image = filter->GetOutput();

	/// Start the processing in a separate thread.
	m_CalculationThread.Initialize(translated_image, target_image);
	m_CalculationThread.start();
	updateButton();
}

void RegistrationPlugin::on_ThreadedRegistrationCalculation_finished()
{
	bool is_ok = m_CalculationThread.GetRegistrationUpdateSuccessFlag();
	if (!is_ok)
	{
		updateButton(ui.pushButton_Registration->isEnabled());
		return;
	}
	auto result_image = m_CalculationThread.GetResultImage();
	auto pluginContext = my_popeproject_registrationplugin_PluginActivator::GetPluginContext();
	ctkServiceReference serviceReference = pluginContext->getServiceReference<mitk::IDataStorageService>();
	mitk::IDataStorageService* storageService = pluginContext->getService<mitk::IDataStorageService>(serviceReference);
	mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();
	// Set an appropriate name
	auto all_nodes = dataStorage->GetAll();
	stringstream ss;
	for (int i = name_counter; i < name_counter + 1000; i++)
	{
		ss.str("");
		ss << "Registration #" << i;
		bool is_found = false;
		for (auto datanode : *all_nodes)
		{
			if (datanode->GetName() == ss.str())
			{
				is_found = true;
				break;
			}
		}
		if (is_found)
			continue;
		else
		{
			name_counter = i + 1;
			break;
		}
	}
	mitk::DataNode::Pointer datanode = mitk::DataNode::New();

	datanode->SetName(ss.str());
	datanode->SetData(result_image);
	dataStorage->Add(datanode);
}