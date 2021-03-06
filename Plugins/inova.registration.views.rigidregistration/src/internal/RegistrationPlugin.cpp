#include "RegistrationPlugin.h"
#include "inova_registration_views_rigidregistration_PluginActivator.h"
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
#include <QmitkPreferencesDialog.h>

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
const string RegistrationPlugin::VIEW_ID = "inova.registration.views.rigidregistration";

//int RegistrationPlugin::name_counter = 1;

RegistrationPlugin::RegistrationPlugin()
{}
RegistrationPlugin::~RegistrationPlugin()
{
	//if (m_CalculationThread.isRunning())
	//{
	//	m_CalculationThread.Abort();
	//}
	while (this->m_CalculationThread.isRunning())
	{
		itksys::SystemTools::Delay(100);
	}
	//ctkDictionary properties;
	//properties["id"] = QString::fromStdString(VIEW_ID);
	//emit PluginIsIdle(properties);
}

void RegistrationPlugin::CreateQtPartControl(QWidget* parent)
{
	/// Setting up the UI is a true pleasure when using .ui files, isn't it?
	ui.setupUi(parent);
	this->GetRenderWindowPart(OPEN);
	this->RequestRenderWindowUpdate();

	/// Set preferences node.
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	this->m_RegistrationPluginPreferencesNode = prefService->GetSystemPreferences()->Node("/inova.registration.views.rigidregistration");

	// Connect Signals and Slots of the Plugin UI
	connect(this->ui.pushButton_Settings, SIGNAL(clicked()), this, SLOT(on_pushButton_Settings_clicked()));
	connect(this->ui.pushButton_Registration, SIGNAL(clicked()), this, SLOT(on_pushButton_Registration_clicked()));
	connect((QObject*)&this->m_CalculationThread, SIGNAL(finished()), this, SLOT(on_ThreadedRegistrationCalculation_finished()), Qt::QueuedConnection);

	/// CTK signals.
	auto pluginContext = inova_registration_views_rigidregistration_PluginActivator::GetPluginContext();
	ctkDictionary propsForSlot;
	ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		eventAdmin->publishSignal(this, SIGNAL(GonnaAddNewDataNode(const ctkDictionary&)), "data/GONNAADDNEWDATANODE", Qt::DirectConnection);
		eventAdmin->publishSignal(this, SIGNAL(NewDataNodeAdded(const ctkDictionary&)), "data/NEWDATANODEADDED", Qt::DirectConnection);
		eventAdmin->publishSignal(this, SIGNAL(PluginIsBusy(const ctkDictionary&)), "registration/PLUGINISBUSY", Qt::DirectConnection);
		eventAdmin->publishSignal(this, SIGNAL(PluginIsIdle(const ctkDictionary&)), "registration/PLUGINISIDLE", Qt::DirectConnection);
	}

	CheckInputs();
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
string RegistrationPlugin::updateNewImageName(mitk::DataNode::Pointer datanode)
{
	if (datanode != nullptr)
	{
		string moving_image_name = datanode->GetName();
		auto dataStorage = GetDataStorage();
		string name = !moving_image_name.empty() ?
			Elements::get_next_name(moving_image_name, dataStorage) :
			"MappedImage";
		this->ui.label_NewImageName->setText(QString::fromStdString(name));
		return name;
	}
	else
	{
		this->ui.label_NewImageName->setText("MappedImage");
		return "MappedImage";
	}
}
void RegistrationPlugin::AddImage(const string& name, mitk::Image::Pointer image)
{
	mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();
	auto allNodes = dataStorage->GetAll();
	auto iterator = allNodes->Begin();
	while (iterator != allNodes->End())
	{
		mitk::DataNode::Pointer datanode = iterator->Value();
		++iterator;
		mitk::Image* image_i = dynamic_cast<mitk::Image*>(datanode->GetData());
		if (!image_i)
			continue;
		if (datanode->GetName().compare(name) == 0)
			return;
	}
	mitk::DataNode::Pointer dn = mitk::DataNode::New();
	dn->SetName(name);
	dn->SetData(image);
	ctkDictionary properties;
	emit GonnaAddNewDataNode(properties);//?? queued?
	dataStorage->Add(dn);
	emit NewDataNodeAdded(properties);//??
}

void RegistrationPlugin::CheckInputs()
{
	if (m_CalculationThread.isRunning())
		return;

	/// Update selected images.
	mitk::DataNode::Pointer moving_image = nullptr;
	mitk::DataNode::Pointer target_image = nullptr;
	QList<mitk::DataNode::Pointer> selectedDataNodes = this->GetDataManagerSelection();
	for (auto datanode : selectedDataNodes)
	{
		mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
		if (!image)
			continue;
		// Reverse direction
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
		QString short_name = Elements::get_short_name_for_image(name);
		ui.label_MovingImage->setText("<b>Moving image:</b> " + short_name);
		ui.label_MovingImage->setToolTip(QString::fromStdString(name));
		ui.label_MovingImage->setStyleSheet("");
		ui.label_MovingImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		ui.label_MovingImage->setText("<b>Moving image:</b> Please select in Data Manager.");
		ui.label_MovingImage->setToolTip("");
		ui.label_MovingImage->setStyleSheet("color: #E02000;\nbackground-color: #efef95;");
		ui.label_MovingImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}

	/// Set target image.
	if (target_image != nullptr)
	{
		string name = target_image->GetName();
		QString short_name = Elements::get_short_name_for_image(name);
		ui.label_TargetImage->setText("<b>Target image:</b> " + short_name);
		ui.label_TargetImage->setToolTip(QString::fromStdString(name));
		ui.label_TargetImage->setStyleSheet("");
		ui.label_TargetImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		ui.label_TargetImage->setText("<b>Target image:</b> Please select in Data Manager.");
		ui.label_TargetImage->setToolTip("");
		ui.label_TargetImage->setStyleSheet("color: #E02000;\nbackground-color: #efef95;");
		ui.label_TargetImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}

	updateNewImageName(moving_image);
	bool enabled = (moving_image != nullptr && target_image != nullptr);
	updateButton(enabled);
}
void RegistrationPlugin::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&)
{
	CheckInputs(); //selectedDataNodes
}
void RegistrationPlugin::NodeRemoved(const mitk::DataNode*)
{
	this->CheckInputs();
}
void RegistrationPlugin::OnPreferencesChanged(const berry::IBerryPreferences*)
{
}

template<typename TPixel, unsigned int VImageDimension>
static void GetCenter(const itk::Image<TPixel, VImageDimension>* inputImage, shared_ptr<vector<double>> center)
{
	using ImageType = itk::Image<TPixel, VImageDimension>;
	using FixedImageCalculatorType = itk::ImageMomentsCalculator<ImageType>;
	typename FixedImageCalculatorType::Pointer fixedCalculator = FixedImageCalculatorType::New();
	fixedCalculator->SetImage(inputImage);
	fixedCalculator->Compute();
	typename FixedImageCalculatorType::VectorType fixedCenter = fixedCalculator->GetCenterOfGravity();
	center->resize(fixedCenter.GetVectorDimension());
	for (size_t i = 0; i < fixedCenter.GetVectorDimension(); i++)
	{
		center->operator[](i) = static_cast<double>(fixedCenter[i]);
	}
}

template<typename TPixel, unsigned int VImageDimension>
static void Translate(const itk::Image<TPixel, VImageDimension>* inputImage, shared_ptr<vector<double>> center, mitk::Image::Pointer outputImage)
{
	using ImageType = itk::Image<TPixel, VImageDimension>;
	using FilterType = itk::ShiftScaleImageFilter<ImageType, ImageType>;
	using FixedImageCalculatorType = itk::ImageMomentsCalculator<ImageType>;

	typename FixedImageCalculatorType::Pointer calculator = FixedImageCalculatorType::New();
	calculator->SetImage(inputImage);
	calculator->Compute();
	typename FixedImageCalculatorType::VectorType imageCenter = calculator->GetCenterOfGravity();

	//const unsigned int dimension = VImageDimension; // 3; // imageCenter.GetVectorDimension();
	assert(VImageDimension == imageCenter.GetVectorDimension());
	using TranslationTransformType = itk::TranslationTransform<double, VImageDimension>;
	typename TranslationTransformType::Pointer transform = TranslationTransformType::New();
	typename TranslationTransformType::OutputVectorType translation;
	for (size_t i = 0; i < VImageDimension; i++)
	{
		translation[i] = imageCenter[i] - center->operator[](i);
	}
	transform->Translate(translation);
	MITK_INFO << "Translation: " << translation;

	using ResampleImageFilterType = itk::ResampleImageFilter<ImageType, ImageType>;
	typename ResampleImageFilterType::Pointer resampleFilter = ResampleImageFilterType::New();
	resampleFilter->SetTransform(transform.GetPointer());
	resampleFilter->SetInput(inputImage);
	// Without this, the program crashes
	typename ImageType::SizeType size = inputImage->GetLargestPossibleRegion().GetSize();
	resampleFilter->SetSize(size);
	resampleFilter->Update();

	// This is the tricky part that is done wrong very often. As the image data
	// of ITK images and MITK images are binary compatible, we don't need to
	// cast or copy the ITK output image. Instead, we just want to reference
	// the image data and tell ITK that we took the ownership.
	mitk::GrabItkImageMemory(resampleFilter->GetOutput(), outputImage);
}

void RegistrationPlugin::on_pushButton_Settings_clicked()
{
	QmitkPreferencesDialog preferencesDialog(QApplication::activeWindow());
	preferencesDialog.SetSelectedPage("inova.registration.views.rigidregistrationPreferencePage");
	preferencesDialog.exec();
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
	mitk::DataNode::Pointer moving_node;
	for (auto datanode : dataNodes)
	{
		mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
		if (!image)
			continue;
		if (moving_image == nullptr)
		{
			moving_image = image;
			moving_node = datanode;
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

	/// Set the name of the new image.
	m_moving_image_name = updateNewImageName(moving_node);

	/// Move the moving image to the center of gravity of the fixed image.
	mitk::Image::Pointer translated_image = nullptr;
	translated_image = moving_image;
	/*try
	{
		translated_image = mitk::Image::New();
		auto center = make_shared<vector<double>>();
		AccessIntegralPixelTypeByItk_n(target_image, GetCenter, (center));
		AccessIntegralPixelTypeByItk_n(moving_image, Translate, (center, translated_image));
		AddImage("translated_image", translated_image);
		//return;
		auto filter = PopeImageFilter::New();
		filter->SetInput(0, moving_image);
		//filter->SetInput(0, moving_image);
		//filter->SetInput(1, target_image);
		filter->Update();
		translated_image = filter->GetOutput();
	}
	catch (const mitk::AccessByItkException& e)
	{
		MITK_ERROR << "Unsupported pixel type or image dimension: " << e.what();
		translated_image = moving_image;
	}
	catch (...)
	{
		translated_image = moving_image;
	}*/

	ctkDictionary properties;
	properties["id"] = QString::fromStdString(VIEW_ID);
	emit PluginIsBusy(properties);

	/// Start the processing in a separate thread.
	m_CalculationThread.Initialize(translated_image, target_image);
	m_CalculationThread.start();
	updateButton();
}
void RegistrationPlugin::on_ThreadedRegistrationCalculation_finished()
{
	ctkDictionary properties;
	properties["id"] = QString::fromStdString(VIEW_ID);
	emit PluginIsIdle(properties);
	bool is_ok = m_CalculationThread.GetRegistrationUpdateSuccessFlag();
	if (!is_ok)
	{
		updateButton(ui.pushButton_Registration->isEnabled());
		CheckInputs();
		return;
	}
	auto result_image = m_CalculationThread.GetResultImage();
	auto pluginContext = inova_registration_views_rigidregistration_PluginActivator::GetPluginContext();
	ctkServiceReference serviceReference = pluginContext->getServiceReference<mitk::IDataStorageService>();
	//mitk::IDataStorageService* storageService = pluginContext->getService<mitk::IDataStorageService>(serviceReference);
	//mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

	auto new_properties = m_CalculationThread.GetMovingImage()->GetPropertyList();
	result_image->SetPropertyList(new_properties);

	auto dataStorage = GetDataStorage();
	// Set an appropriate name
	auto all_nodes = dataStorage->GetAll();
	/*stringstream ss;
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
	}*/
	mitk::DataNode::Pointer datanode = mitk::DataNode::New();
	//datanode->SetName(ss.str());
	datanode->SetName(m_moving_image_name);
	datanode->SetData(result_image);
	dataStorage->Add(datanode);
	CheckInputs();
}
