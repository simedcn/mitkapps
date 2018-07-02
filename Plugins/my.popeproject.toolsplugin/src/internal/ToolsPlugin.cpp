#include "ToolsPlugin.h"
#include "TagTree.h"
#include "my_popeproject_toolsplugin_PluginActivator.h"
#include <PopeElements.h>

#include <berryIWorkbenchPage.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

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

#include "mitkLabel.h"
#include "mitkToolManagerProvider.h"
#include "mitkOtsuSegmentationFilter.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkIOUtil.h"
#include "mitkIRenderWindowPart.h"
#include "mitkImage.h"
#include "mitkImageStatisticsHolder.h"

// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usModuleResource.h"

#include "itkImageRegionIterator.h"
#include "mitkImageCast.h"
#include <mitkITKImageImport.h>
#include <mitkPaintbrushTool.h>

#include <itkBinaryThresholdImageFilter.h>
#include "itksys/SystemTools.hxx"

#include <QmitkDataNodeSelectionProvider.h>
#include <QmitkDataManagerView.h>
#include <QmitkChartWidget.h>
#include <QmitkRenderWindow.h>

#include <berryWorkbenchPlugin.h>
#include <berryQtPreferences.h>

#include <algorithm>
#include <iterator>

using namespace std;

// Don't forget to initialize the VIEW_ID.
const string ToolsPlugin::VIEW_ID = "my.popeproject.views.toolsplugin";
const int ToolsPlugin::STAT_TABLE_BASE_HEIGHT = 180;


void set_childTreeWidgetItems_editable(QTreeWidgetItem* item, bool isEditable)
{
	for (int j = 0; j < item->childCount(); j++)
	{
		auto child_item = item->child(j);

		if (isEditable)//!! move
			child_item->setFlags(item->flags() | Qt::ItemIsEditable);
		else
			child_item->setFlags(item->flags() & (~Qt::ItemIsEditable));

		set_childTreeWidgetItems_editable(child_item, isEditable);
	}
}

ToolsPlugin::ToolsPlugin()
	: m_TimeObserverTag(0)
{
	this->m_CalculationThread = new ImageStatisticsCalculationThread;
}
ToolsPlugin::~ToolsPlugin()
{
  this->m_ServiceRegistration.Unregister();
  // wait until thread has finished
  while (this->m_CalculationThread->isRunning())
  {
	  itksys::SystemTools::Delay(100);
  }
  delete this->m_CalculationThread;
}

#include <berryPlatformUI.h>
#include <berryQtWorkbenchAdvisor.h>
void ToolsPlugin::CreateQtPartControl(QWidget* parent)
{
	/// Setting up the UI is a true pleasure when using .ui files, isn't it?
	ui.setupUi(parent);
	this->GetRenderWindowPart(OPEN);
	this->RequestRenderWindowUpdate();

	/// Set preferences node.
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	this->m_ToolsPluginPreferencesNode = prefService->GetSystemPreferences()->Node("/my.popeproject.views.toolsplugin");

	/// Wire up the UI widgets with our functionality.
	m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
	assert(m_ToolManager);

	/// Set statistics table.
	int num_rows = ui.tableWidget_Statistics->rowCount();
	ui.tableWidget_Statistics->setRowCount(num_rows + num_percentiles);
	for (int i = 0; i < num_percentiles; i++)
	{
		QTableWidgetItem* tableWidgetItem = new QTableWidgetItem();
		ui.tableWidget_Statistics->setVerticalHeaderItem(num_rows + i, tableWidgetItem);
		stringstream ss;
		ss << i;
		int r = i - (i / 10) * 10;
		if (r == 1)
			ss << "st";
		else if (r == 2)
			ss << "nd";
		else if (r == 3)
			ss << "rd";
		else
			ss << "th";
		ss << " percentile";
		tableWidgetItem->setText(QString::fromStdString(ss.str()));
	}

	/// Settings.
	// before the signals are connected:
	updateEditableControls(false);
	updateAfterSelectionChanged();
	updateShowPatientData();
	updateTagRepresentation();
	updateShowStatistics();
	updateShowHistogram();

	// Connect Signals and Slots of the Plugin UI
	connect(this->ui.checkbox_EnableVolumeRendering, SIGNAL(toggled(bool)), this, SLOT(on_checkbox_EnableVolumeRendering_toggled(bool)));
	connect(this->ui.checkBox_ShowPatientData, SIGNAL(toggled(bool)), this, SLOT(on_checkBox_ShowPatientData_toggled(bool)));
	connect(this->ui.checkBox_GroupTags, SIGNAL(toggled(bool)), this, SLOT(on_checkBox_GroupTags_toggled(bool)));
	connect(this->ui.checkBox_ShowStatistics, SIGNAL(toggled(bool)), this, SLOT(on_checkBox_ShowStatistics_toggled(bool)));
	connect(this->ui.checkBox_ShowHistogram, SIGNAL(toggled(bool)), this, SLOT(on_checkBox_ShowHistogram_toggled(bool)));
	connect((QObject*)(this->ui.JSHistogram), SIGNAL(PageSuccessfullyLoaded()), (QObject*)this, SLOT(on_histogram_PageSuccessfullyLoaded()));

	connect((QObject*) this->m_CalculationThread, SIGNAL(finished()), this, SLOT(on_ThreadedStatisticsCalculation_ends()), Qt::QueuedConnection);

	/// CTK signals.
	auto pluginContext = my_popeproject_toolsplugin_PluginActivator::GetPluginContext();
	ctkDictionary propsForSlot;
	ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "pope/representation/VIEWCHANGED";
		eventAdmin->subscribeSlot(this, SLOT(on_MainWindow_Representation3D_changed(ctkEvent)), propsForSlot);
	}
	/// Creating an Event Publisher.
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		//ctkDictionary properties;
		//properties["title"] = report.getTitle();
		//properties["path"] = report.getAbsolutePath();
		//properties["time"] = QTime::currentTime();
		//ctkEvent reportGeneratedEvent("pope/representation/START3D", properties);
		//eventAdmin->sendEvent(reportGeneratedEvent);

		// Using Qt::DirectConnection is equivalent to ctkEventAdmin::sendEvent()
		eventAdmin->publishSignal(this, SIGNAL(Representation3DHasToBeInitiated(const ctkDictionary&)), "pope/representation/START3D", Qt::DirectConnection);
		eventAdmin->publishSignal(this, SIGNAL(NodeHasManyImages(const ctkDictionary&)), "pope/representation/ENABLED3D", Qt::DirectConnection);
		eventAdmin->publishSignal(this, SIGNAL(SetLevelWindowRange(const ctkDictionary&)), "pope/representation/SETRANGE", Qt::DirectConnection);
	}
  
	/// Create the DisplayCoordinate Supplier and provide it with the function name it should call.
	m_DisplayCoordinateSupplier = make_unique<DisplayCoordinateSupplier>();
	QString functionName = "on_pixel_selected";
	m_DisplayCoordinateSupplier->RegisterForUpdates(this, functionName);

	/// Register the coordinate supplier as MicroService.
	this->m_ServiceRegistration = us::GetModuleContext()->RegisterService<mitk::InteractionEventObserver>(this->m_DisplayCoordinateSupplier.get());
}

void ToolsPlugin::updateAfterSelectionChanged()
{
	auto dataNode = getFirstSelectedNode();
	bool is_selected_node = (dataNode != nullptr);

	//ui.label_ImageNotSelected->setVisible(!is_selected_node);
	if (is_selected_node)
	{
		string name = dataNode->GetName();
		if (name.empty())
		{
			ui.label_ImageNotSelected->setText("");//("<b>Profile</b>");
		}
		else
		{
			QString str_name = QString::fromStdString(name);
			ui.label_ImageNotSelected->setText(str_name);
			ui.label_ImageNotSelected->setToolTip(str_name);
		}
		ui.label_ImageNotSelected->setStyleSheet("");
		ui.label_ImageNotSelected->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		ui.label_ImageNotSelected->setText("Please load and select a dataset in Data Manager.");
		ui.label_ImageNotSelected->setStyleSheet("color: #E02000;\nbackground-color: #efef95;");
		ui.label_ImageNotSelected->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	}
	ui.label_SelectedImage->setVisible(is_selected_node);

	/// Read main properties
	string patientName = Elements::get_patientName(dataNode, "-");
	string patientBirthdate = Elements::get_patientBirthdate(dataNode, "-");
	string patientGender = Elements::get_patientGender(dataNode, "-");
	string patientID = Elements::get_patientId(dataNode, "-");
	patientName = Elements::recognize_name(patientName);
	patientBirthdate = Elements::recognize_birthday(patientBirthdate);
	patientGender = Elements::recognize_gender(patientGender);
	auto row_Patient_Name = ui.tableWidget_PatientData->item(0, 0);
	auto row_Patient_Gender = ui.tableWidget_PatientData->item(1, 0);
	auto row_Patient_Birthday = ui.tableWidget_PatientData->item(2, 0);
	auto row_Patient_ID = ui.tableWidget_PatientData->item(3, 0);
	row_Patient_Name->setText(QString::fromStdString(patientName));
	row_Patient_Gender->setText(QString::fromStdString(patientGender));
	row_Patient_Birthday->setText(QString::fromStdString(patientBirthdate));
	row_Patient_ID->setText(QString::fromStdString(patientID));

	updateTags();
}
void ToolsPlugin::updateTags()
{
	ui.treeWidget_Tags->clear();
	ui.tableWidget_Tags->setRowCount(0);

	auto dataNode = getFirstSelectedNode();
	if (dataNode == nullptr)
		return;

	const auto data = dataNode->GetData();
	if (data == nullptr)
		return;

	const auto properties = data->GetPropertyList();
	if (properties->IsEmpty())
		return;
	const auto property_map = properties->GetMap();

	// Settings
	bool editable_text = m_ToolsPluginPreferencesNode->GetBool("editable text", false);

	/// Fill out the list of tags.
	int num_tags = property_map->size();
	ui.tableWidget_Tags->setRowCount(num_tags);
	int i = 0;
	for (const auto tag : *property_map)
	{
		QTableWidgetItem* qtablewidgetitem_1 = new QTableWidgetItem(QString::fromStdString(tag.first));
		ui.tableWidget_Tags->setItem(i, 0, qtablewidgetitem_1);
		QString value = QString::fromStdString(tag.second->GetValueAsString());
		QTableWidgetItem* qtablewidgetitem_2 = new QTableWidgetItem(value);
		qtablewidgetitem_2->setToolTip(value);
		ui.tableWidget_Tags->setItem(i, 1, qtablewidgetitem_2);
		i++;
	}

	/// Fill out the groups of tags.
	auto tag_tree = TagTree::Create(*property_map);
	auto items = createTagItems(tag_tree);
	ui.treeWidget_Tags->insertTopLevelItems(0, *items);
	for (int i = 0; i < ui.treeWidget_Tags->topLevelItemCount(); i++)
	{
		auto item = ui.treeWidget_Tags->topLevelItem(i);
		if (editable_text)
			item->setFlags(item->flags() | Qt::ItemIsEditable);
		else
			item->setFlags(item->flags() & (~Qt::ItemIsEditable));
		setTagCounts(item);
		set_childTreeWidgetItems_editable(item, editable_text);
	}
	ui.treeWidget_Tags->expandAll();
}
shared_ptr<QList<QTreeWidgetItem*>> ToolsPlugin::createTagItems(TagNode tree, const string& prefix)
{
	const auto& groups = tree->tagGroups;
	const auto& tags = tree->tags;

	auto items = shared_ptr<QList<QTreeWidgetItem*>>(new QList<QTreeWidgetItem*>);
	/// Add groups
	for (const auto& group : groups)
	{
		const string& name = group.first;
		QString qname = QString::fromStdString(name);
		const TagNode subtree = group.second;
		QTreeWidgetItem* item_group = new QTreeWidgetItem(QStringList(qname));
		stringstream pre;
		pre << prefix << name << '.';
		auto subitems = createTagItems(subtree, pre.str());
		item_group->addChildren(*subitems);
		//item_group->setData(NAME_TAG, Qt::UserRole, i);
		stringstream full_name;
		full_name << prefix << name;
		item_group->setToolTip(NAME_TAG, QString::fromStdString(full_name.str()));
		items->append(item_group);

		stringstream value;
		value << subitems->size();
	}
	/// Add tags
	for (const auto& tag : tags)
	{
		QString name = QString::fromStdString(tag->name);
		QString value = QString::fromStdString(tag->value);
		QTreeWidgetItem* item_tag = new QTreeWidgetItem(QStringList(name));
		//item_tag->setData(0, Qt::UserRole, j);
		stringstream full_name;
		full_name << prefix << tag->name;
		item_tag->setToolTip(NAME_TAG, QString::fromStdString(full_name.str()));
		items->append(item_tag);
		//int ii = item_tag->data(NAME_TAG, Qt::UserRole).toInt();
		item_tag->setText(VALUE_TAG, value);
		item_tag->setToolTip(VALUE_TAG, value);
	}
	return items;
}
int ToolsPlugin::setTagCounts(QTreeWidgetItem* item)
{
	/// Update the number of tags inside the groups
	int num = 0;// item->childCount();
	if (item->text(VALUE_TAG).isEmpty())
	{
		for (int j = 0; j < item->childCount(); j++)
		{
			auto child_item = item->child(j);
			int num_children = setTagCounts(child_item);
			num += num_children;
		}
		if (num == 0)
			return 1; // a tag, not a group -- just empty string (value)
		stringstream name;
		if (num == 1)
			name << "... 1 tag";
		else
			name << "... " << to_string(num) << " tags";
		QLabel *label = new QLabel(QString::fromStdString(name.str()), ui.treeWidget_Tags);
		label->setStyleSheet(R"style(color: #595B5C; 
								background-color: transparent; 
								font: bold;)style");
		ui.treeWidget_Tags->setItemWidget(item, VALUE_TAG, label);
	}
	else
	{
		assert(item->childCount() == 0);
		return 1; // a tag, not a group
	}
	return num;
}
void ToolsPlugin::updateShowPatientData()
{
	bool showPatientData = m_ToolsPluginPreferencesNode->GetBool("show patient data", true);
	ui.checkBox_ShowPatientData->blockSignals(true);
	ui.checkBox_ShowPatientData->setChecked(showPatientData);
	ui.checkBox_ShowPatientData->blockSignals(false);
	ui.tableWidget_PatientData->setVisible(showPatientData);
}
void ToolsPlugin::updateTagRepresentation()
{
	bool isGrouped = m_ToolsPluginPreferencesNode->GetBool("group tags", true);
	ui.checkBox_GroupTags->blockSignals(true);
	ui.checkBox_GroupTags->setChecked(isGrouped);
	ui.checkBox_GroupTags->blockSignals(false);
	ui.treeWidget_Tags->setVisible(isGrouped);
	ui.tableWidget_Tags->setVisible(!isGrouped);
}
void ToolsPlugin::updateShowStatistics()
{
	bool showStatistics = m_ToolsPluginPreferencesNode->GetBool("show statistics", false);
	ui.checkBox_ShowStatistics->blockSignals(true);
	ui.checkBox_ShowStatistics->setChecked(showStatistics);
	ui.checkBox_ShowStatistics->blockSignals(false);
	ui.tableWidget_Statistics->setVisible(showStatistics);
}
void ToolsPlugin::updateShowHistogram()
{
	bool showHistogram = m_ToolsPluginPreferencesNode->GetBool("show histogram", false);
	ui.checkBox_ShowHistogram->blockSignals(true);
	ui.checkBox_ShowHistogram->setChecked(showHistogram);
	ui.checkBox_ShowHistogram->blockSignals(false);
	ui.JSHistogram->setVisible(showHistogram);
	ui.StatisticsWidgetStack->setVisible(showHistogram);
	//if (!showHistogram)
	//{
	//	bool showStatistics = m_ToolsPluginPreferencesNode->GetBool("show statistics", false);
	//	if (!showStatistics)
	//		...
	//}
}
void ToolsPlugin::updateEditableControls(bool update_tags)
{
	bool editable_text = m_ToolsPluginPreferencesNode->GetBool("editable text", false);
	QAbstractItemView::EditTriggers triggers = editable_text ? (QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed) : QAbstractItemView::NoEditTriggers;
	ui.tableWidget_PatientData->setEditTriggers(triggers);
	ui.tableWidget_Tags->setEditTriggers(triggers);
	ui.treeWidget_Tags->setEditTriggers(triggers);

	if (update_tags)
		updateTags();
}

std::map<double, double> ConvertHistogramToMap(itk::Statistics::Histogram<double>::ConstPointer histogram)
{
	std::map<double, double> histogramMap;

	auto endIt = histogram->End();
	auto it = histogram->Begin();

	// generating Lists of measurement and frequencies
	for (; it != endIt; ++it)
	{
		double frequency = it.GetFrequency();
		double measurement = it.GetMeasurementVector()[0];
		histogramMap.emplace(measurement, frequency);
	}

	return histogramMap;
}
void ToolsPlugin::displayHistogram(int time_step)
{
	// display histogram for selected timestep
	ui.JSHistogram->Clear();
	auto histogram = (ImageStatisticsCalculationThread::HistogramType::ConstPointer) this->m_CalculationThread->GetTimeStepHistogram(time_step);
	//auto histogram = (ImageStatisticsCalculationThread::HistogramType::ConstPointer)hist;
	if (histogram.IsNull())
		return;

	bool is_ok = this->m_CalculationThread->GetStatisticsUpdateSuccessFlag();
	if (!is_ok)
		return;

	auto hist_map = ConvertHistogramToMap(histogram);
	string imageNameLabel = "Histogram";
	ui.JSHistogram->AddData2D(hist_map, imageNameLabel);
	QmitkChartWidget::ChartType chart_type = QmitkChartWidget::ChartType::bar; //QmitkChartWidget::ChartType::line
	ui.JSHistogram->SetChartType(imageNameLabel, chart_type);
	ui.JSHistogram->SetXAxisLabel("Intensity");
	ui.JSHistogram->SetYAxisLabel("Frequency");
	ui.JSHistogram->Show(false);
}

void ToolsPlugin::NodeAdded(const mitk::DataNode* node)
{
	//berry::IWorkbenchWindow::Pointer workbenchWindow = this->GetSite()->GetWorkbenchWindow();
	//berry::ISelectionService* workbenchWindowSelectionService = workbenchWindow->GetSelectionService();
	//berry::ISelection::ConstPointer selection = workbenchWindowSelectionService->GetSelection("org.mitk.views.datamanager");
	//berry::IViewPart::Pointer dataManagerView = workbenchWindow->GetActivePage()->FindView("org.mitk.views.datamanager");
	//QmitkDataNodeSelectionProvider::Pointer dataManagerSelectionProvider = dataManagerView->GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>();
	//auto qs = dataManagerSelectionProvider->GetQItemSelection();
	//QItemSelectionRange qisr;
	//qs.push_back();
	//dataManagerSelectionProvider->SetSelection(selection);
	//dataManagerSelectionProvider->SetQItemSelection(qs);
	//auto s = dataManagerSelectionProvider->GetQItemSelection();
	//int is = s.size();


	//mitk::DataNode::Pointer pt = mitk::DataNode::New();
	//pt->SetData(node->GetData());
	//pt->SetName(node->GetName());
	//FireNodeSelected(pt);
	//this->RequestRenderWindowUpdate();
	//mitk::RenderingManager::GetInstance()->RequestUpdateAll();


	//berry::SmartPointer<berry::IEditorInput> nul;
	//auto list_rve = this->GetSite()->GetPage()->FindEditors(nul, "my.popeproject.editors.renderwindow", berry::IWorkbenchPage::MATCH_ID);
	//auto p_rve = list_rve.front().GetPointer();

	//PopeRenderWindowEditor* rve = dynamic_cast<PopeRenderWindowEditor*>(p_rve);
	//auto multiWidget = rve->GetStdMultiWidget();

	//auto vv = this->GetSite()->GetPage()->GetViews();
	//for (auto v : vv)
	//{
	//	MITK_INFO << v->GetPartName().toStdString();
	//}

	//berry::IWorkbenchWindow::Pointer window;
	//window = this->GetSite()->GetWorkbenchWindow();
	//MainWindow* mainWindow = static_cast<MainWindow*>(window->GetShell()->GetControl());

	//// get access to StdMultiWidget by using RenderWindowPart
	//auto part = GetRenderWindowPart();
	//auto window = part->GetActiveQmitkRenderWindow();
	//auto w = part->GetQmitkRenderWindow("my.popeproject.editors.renderwindow");
	//auto manager = part->GetRenderingManager();
	////auto multiWidget = qRWE->GetStdMultiWidget();

	/*/// Chcek if it contains more than one-two images.
	string tag = get_property("DICOM.0020.0013", "dicom.image.0020.0013", node);
	bool has_many_images = tag.length() > 30;

	/// If so, enable volume rendering if it was selected for first time.
	if (has_many_images)
	{
		// Find the node to be selected
		berry::IViewPart::Pointer datamanagerView = this->GetSite()->GetWorkbenchWindow()->GetActivePage()->FindView("org.mitk.views.datamanager");
		//berry::ISelection::ConstPointer selection(this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
		//datamanagerView->GetSite()->GetSelectionProvider().Cast<berry::QtSelectionProvider>()->SetSelection(selection, QItemSelectionModel::Select);
		//mitk::DataNodeSelection::ConstPointer currentSelection = selection.Cast<const mitk::DataNodeSelection>();
		auto dView = dynamic_cast<QmitkDataManagerView*>(datamanagerView.GetPointer());
		//QList<mitk::DataNode::Pointer> all_nodes = dView->GetNodeSet();
		//mitk::DataNode::Pointer node_to_select = nullptr;
		//for (auto n : all_nodes)
		//{
		//	if (n->GetName() == node->GetName())
		//	{
		//		node_to_select = n;
		//		break;
		//	}
		//}
		//if (node_to_select != nullptr)
		//{
		// Select datanode
		//dView->SelectNode(node_to_select);
		dView->SelectNode(node);
	}*/

}
void ToolsPlugin::NodeRemoved(const mitk::DataNode* node)
{
	while (this->m_CalculationThread->isRunning())
	{ // wait until thread has finished
		itksys::SystemTools::Delay(30);
	}
}

void ToolsPlugin::Activated()
{}
void ToolsPlugin::Deactivated()
{}
void ToolsPlugin::Visible()
{
	/// Initialize the timeChanged event of imageNavigator.
	mitk::IRenderWindowPart* renderWindow = GetRenderWindowPart();
	if (!renderWindow)
		return;
	auto cmdTimeEvent =	itk::ReceptorMemberCommand<ToolsPlugin>::New();
	cmdTimeEvent->SetCallbackFunction(this, &ToolsPlugin::on_imageNavigator_timeChanged);
	// It is sufficient to add the observer to the axial render window since the GeometryTimeEvent is always triggered by all views.
	auto event = mitk::SliceNavigationController::GeometryTimeEvent(nullptr, 0);
	m_TimeObserverTag = renderWindow->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->AddObserver(event, cmdTimeEvent);
}
void ToolsPlugin::Hidden()
{
	// The slice navigation controller observer is removed here instead of in the destructor.
	// If it was called in the destructor, the application would freeze because the view's destructor gets called after the render windows have been destructed.
	if (m_TimeObserverTag == 0)
		return;

	mitk::IRenderWindowPart* renderWindow = GetRenderWindowPart();
	if (renderWindow)
	{
		renderWindow->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->RemoveObserver(m_TimeObserverTag);
	}
	m_TimeObserverTag = 0;
}
void ToolsPlugin::SetFocus()
{
	// default focus --> one line to set focus for the default widget
}

void ToolsPlugin::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& dataNodes)
{
	auto nodes = shared_ptr<DataNodeList>(new DataNodeList);
	*nodes = dataNodes.toStdList();
	auto dataNode = getFirstSelectedNode(nodes);

	if ((m_SelectedNode != dataNode) && m_SelectedNode != nullptr)
	{// Disable autorotation for the current node
		bool property_autorotation = false; // default value - is used when the property doesn't exist
		m_SelectedNode->GetBoolProperty("autorotation", property_autorotation);

		if (property_autorotation)
		{
			// Disable 3D representation
			enable3DRepresentation(false);
		}
	}

	// Update current selected node
	/// Remeber datanode selected in DataStorage.
	m_SelectedNode = dataNode;

	bool has_many_images = false;
	
	if (dataNode != nullptr)
	{// dataNode == nullptr when nothing is selected or the selection doesn't contain an image.
		//setLabelWidget();

		/// Chcek if it contains more than one-two images.
		string tag = Elements::get_property("DICOM.0020.0013", "dicom.image.0020.0013", dataNode);
		has_many_images = tag.length() > 30;

		/// If so, enable volume rendering if it was selected for first time.
		if (has_many_images)
		{
			// Check if it is selected for first time
			bool property_autorotation;
			bool is_autorotation_found = dataNode->GetBoolProperty("autorotation", property_autorotation);
			if (!is_autorotation_found)
			{
				bool def_val = m_ToolsPluginPreferencesNode->GetBool("3D autorotation", true);
				const_cast<mitk::DataNode*>(dataNode)->SetBoolProperty("autorotation", def_val);
				//property_autorotation = true;
				bool is_found = dataNode->GetBoolProperty("autorotation", property_autorotation);
				assert(is_found && property_autorotation == def_val);
			}
			if (property_autorotation)
			{
				// Enable 3D representation
				enable3DRepresentation(true);
			}
		}
	}
	/// Update Volume Rendering.
	ui.checkbox_EnableVolumeRendering->setEnabled(has_many_images);
	bool is_volume_rendering_enabled = has_many_images; // default value if not specified
	if (dataNode != nullptr)
	{
		dataNode->GetBoolProperty("volumerendering", is_volume_rendering_enabled);
		if (!has_many_images && is_volume_rendering_enabled)
		{
			is_volume_rendering_enabled = false;
			//const_cast<mitk::DataNode*>(dataNode)->SetBoolProperty("volumerendering", false); --> will be called
		}
	}
	ui.checkbox_EnableVolumeRendering->setChecked(is_volume_rendering_enabled);

	ctkDictionary properties;
	properties["has_many_images"] = has_many_images;
	emit NodeHasManyImages(properties);

	/// Update views.
	updateAfterSelectionChanged();

	/// Update statistics
	if (dataNode != nullptr)
	{
		auto baseData = dataNode->GetData();
		mitk::Image* image = dynamic_cast<mitk::Image*>(baseData);
		if (!image)
			return;
		//auto statistics = img->GetStatistics();

		// initialize thread and trigger it
		m_CalculationThread->SetIgnoreZeroValueVoxel(false);
		m_CalculationThread->Initialize(image, /*m_SelectedImageMask*/nullptr, /*m_SelectedPlanarFigure*/nullptr);
		// m_CalculationThread->SetUseDefaultBinSize(m_Controls->m_UseDefaultBinSizeBox->isChecked());
		//int timeStep = 0;
		mitk::IRenderWindowPart* renderPart = this->GetRenderWindowPart();
		unsigned int timeStep = 0;
		if (renderPart != nullptr)
		{
			timeStep = renderPart->GetTimeNavigationController()->GetTime()->GetPos();
		}
		// check time step validity
		if (image->GetDimension() <= 3 && timeStep > image->GetDimension(3) - 1)
		{
			timeStep = image->GetDimension(3) - 1;
		}
		m_CalculationThread->SetTimeStep(timeStep);
		try
		{// Compute statistics
			m_CalculationThread->start();
			m_StatisticsImage = image;
		}
		catch (const mitk::Exception& e)
		{
			std::stringstream message;
			message << "<font color='red'>" << e.GetDescription() << "</font>";
			MITK_ERROR << message.str();
			//this->m_StatisticsUpdatePending = false;
			m_StatisticsImage = nullptr;
		}
		catch (const std::runtime_error &e)
		{
			std::stringstream message;
			message << "<font color='red'>" << e.what() << "</font>";
			MITK_ERROR << message.str();
			//this->m_StatisticsUpdatePending = false;
			m_StatisticsImage = nullptr;
		}
		catch (const std::exception &e)
		{
			MITK_ERROR << "Caught exception: " << e.what();
			std::stringstream message;
			message << "<font color='red'>Error! Unequal Dimensions of Image and Segmentation. No recompute possible </font>";
			MITK_ERROR << message.str();
			//this->m_StatisticsUpdatePending = false;
			m_StatisticsImage = nullptr;
		}
		//// wait until thread has finished
		//while (m_CalculationThread->isRunning())
		//{
		//	itksys::SystemTools::Delay(100);
		//}
	}
}
void ToolsPlugin::OnPreferencesChanged(const berry::IBerryPreferences*)
{
	updateShowPatientData();
	updateTagRepresentation();
	updateShowStatistics();
	updateShowHistogram();

	updateEditableControls();
}

void ToolsPlugin::on_checkbox_EnableVolumeRendering_toggled(bool volRen)
{
	auto node = getFirstSelectedNode();
	if (node)
		node->SetBoolProperty("volumerendering", volRen);
	if (node && !volRen)
	{
		bool property_autorotation = false; // default value
		bool is_autorotation_found = node->GetBoolProperty("autorotation", property_autorotation);
		if (is_autorotation_found && property_autorotation)
		{
			//node->SetBoolProperty("autorotation", false);
			enable3DRepresentation(false);
		}
	}
}
void ToolsPlugin::on_checkBox_ShowPatientData_toggled(bool)
{
	m_ToolsPluginPreferencesNode->PutBool("show patient data", ui.checkBox_ShowPatientData->isChecked());
	//updateShowPatientData(); -> will be called by OnPreferencesChanged()
}
void ToolsPlugin::on_checkBox_GroupTags_toggled(bool isGrouped)
{
	m_ToolsPluginPreferencesNode->PutBool("group tags", ui.checkBox_GroupTags->isChecked());
	//updateTagRepresentation(); -> will be called by OnPreferencesChanged()
}
void ToolsPlugin::on_checkBox_ShowStatistics_toggled(bool)
{
	m_ToolsPluginPreferencesNode->PutBool("show statistics", ui.checkBox_ShowStatistics->isChecked());
	//updateShowStatistics(); -> will be called by OnPreferencesChanged()
}
void ToolsPlugin::on_checkBox_ShowHistogram_toggled(bool)
{
	m_ToolsPluginPreferencesNode->PutBool("show histogram", ui.checkBox_ShowHistogram->isChecked());
	//updateShowHistogram(); -> will be called by OnPreferencesChanged()
}
void ToolsPlugin::on_histogram_PageSuccessfullyLoaded()
{
	berry::IPreferencesService* prefService = berry::WorkbenchPlugin::GetDefault()->GetPreferencesService();
	auto m_StylePref = prefService->GetSystemPreferences()->Node(berry::QtPreferences::QT_STYLES_NODE);
	QString styleName = m_StylePref->Get(berry::QtPreferences::QT_STYLE_NAME, "");
	QmitkChartWidget::ChartStyle current_style = (styleName == ":/org.blueberry.ui.qt/darkstyle.qss") ? QmitkChartWidget::ChartStyle::darkstyle : QmitkChartWidget::ChartStyle::defaultstyle;
	ui.JSHistogram->SetTheme(current_style);
}
void ToolsPlugin::on_imageNavigator_timeChanged(const itk::EventObject& e)
{
	if (m_SelectedNode == nullptr)
		return;

	auto timeEvent = dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent*>(&e);
	assert(timeEvent != nullptr);
	if (timeEvent == nullptr)
		return;
	int timestep = timeEvent->GetPos();

	auto baseData = m_SelectedNode->GetData();
	mitk::Image* image = dynamic_cast<mitk::Image*>(baseData);
	if (!image)
		return;

	auto num_timeSteps = image->GetTimeSteps();
	if (num_timeSteps > 1)
	{
		for (int x = 0; x < ui.tableWidget_Statistics->columnCount(); x++)
		{
			for (int y = 0; y < ui.tableWidget_Statistics->rowCount(); y++)
			{
				QTableWidgetItem* item = ui.tableWidget_Statistics->item(y, x);
				if (item == nullptr)
					break;

				if (x == timestep)
				{
					item->setBackgroundColor(Qt::yellow);
				}
				else
				{
					if (y % 2 == 0)
						item->setBackground(ui.tableWidget_Statistics->palette().base());
					else
						item->setBackground(ui.tableWidget_Statistics->palette().alternateBase());
				}
			}
		}

		ui.tableWidget_Statistics->viewport()->update();
	}

	if ((num_timeSteps == 1 && timestep == 0) || num_timeSteps > 1)
	{
		displayHistogram(timestep);
	}
}

void ToolsPlugin::on_pixel_selected(QVector<double> point)
{
	using namespace mitk;
	// TODO: exlude binary images !!!!!
	// !!!!!!!!
	TNodePredicateDataType<Image>::Pointer isImage = TNodePredicateDataType<Image>::New();
	auto nodePredicateProperty = NodePredicateProperty::New("visible", BoolProperty::New(false));
	NodePredicateNot::Pointer isNotVisible = NodePredicateNot::New(nodePredicateProperty);
	NodePredicateAnd::Pointer validVisibleImage = NodePredicateAnd::New(isImage, isNotVisible);

	/// Parse all visible images and list the values of the clicked position
	DataStorage::SetOfObjects::ConstPointer patientImages = this->GetDataStorage()->GetSubset(validVisibleImage);
	vector<DataStorage::SetOfObjects::Element> images;

	Point3D p3d;
	p3d[0] = point[0];
	p3d[1] = point[1];
	p3d[2] = point[2];

	for (auto image : *patientImages)
	{
		if (image.IsNull())
			continue;
		Image* img = dynamic_cast<Image*>(image->GetData());
		if (img == nullptr)
			continue;

		images.push_back(image);
	}

	int images_size = images.size();
	stringstream text;
	text << "<html><head/><body>PixelValue(s) <span style=\"vertical-align:sub;\">Shift+Left_Click</span>: ";
	if (images_size > 1)
	{
		text << "<br>";
	}
	for (int i = 0; i < images_size; i++)
	{
		if (images_size > 1)
		{
			text << Elements::get_short_name(images[i]->GetName()) << ": ";
		}
		Image* img = dynamic_cast<Image*>(images[i]->GetData());
		double value = img->GetPixelValueByWorldCoordinate(p3d);
		text << "<b>" << to_string(int(round(value))) << "</b>";
		if (i < images_size - 1)
		{
			text << "<br>";
		}
	}
	text << "</body></html>";
	ui.label_PixelValue->setText(QString::fromStdString(text.str()));
}
void ToolsPlugin::on_MainWindow_Representation3D_changed(const ctkEvent& event)
{
	bool enable3D = event.getProperty("enable3D").toBool();
	if (m_SelectedNode)
	{
		if (enable3D)
		{
			// Chcek if it contains more than one-two images.
			string tag = Elements::get_property("DICOM.0020.0013", "dicom.image.0020.0013", m_SelectedNode);
			bool has_many_images = tag.length() > 30;
			if (has_many_images)
			{
				// Enable volume rendering
				bool property_volumerendering;
				bool is_volume_rendering_property_found = m_SelectedNode->GetBoolProperty("volumerendering", property_volumerendering);
				if (!is_volume_rendering_property_found || !property_volumerendering)
				{
					//m_SelectedNode->SetBoolProperty("volumerendering", true); --> will be called
					ui.checkbox_EnableVolumeRendering->setChecked(true);
				}
				property_volumerendering = false;
				m_SelectedNode->GetBoolProperty("volumerendering", property_volumerendering);
				assert(property_volumerendering);
			}
		}
		// Enable/disable autorotation
		m_SelectedNode->SetBoolProperty("autorotation", enable3D);
	}
}

QString GetFormattedString(double value, unsigned int decimals)
{
	typedef mitk::ImageStatisticsCalculator::StatisticsContainer::RealType RealType;
	RealType maxVal = std::numeric_limits<RealType>::max();

	if (value == maxVal)
	{
		return QString("NA");
	}
	else
	{
		return QString("%1").arg(value, 0, 'f', decimals);
	}
}
QString GetFormattedIndex(const vnl_vector<int>& vector)
{
	if (vector.empty())
	{
		return QString();
	}
	QString formattedIndex("(");
	for (const auto& entry : vector)
	{
		formattedIndex += QString::number(entry);
		formattedIndex += ",";
	}
	formattedIndex.chop(1);
	formattedIndex += ")";
	return formattedIndex;
}

void detect_beginning(vector<double> diff, const double& min_threshold, const double& max_threshold, int* i_beginning, int* i_end)
{
	// Start of start region
	int i1_1 = 0;
	for (int i = i1_1; i < diff.size(); i++)
	{
		i1_1 = i;
		if (diff[i] > max_threshold)
			break;
	}
	// End of start region
	int i1_2 = i1_1 + 1;
	for (int i = i1_2; i < diff.size(); i++)
	{
		i1_2 = i;
		if (diff[i] <= min_threshold)
			break;
	}
	*i_beginning = i1_1;
	*i_end = i1_2;
}
void detect_end(vector<double> diff, const double& min_threshold, const double& max_threshold, int* i_beginning, int* i_end)
{
	// End of end region
	int i2_2 = diff.size() - 1;
	for (int i = i2_2; i > 0; i--)
	{
		i2_2 = i;
		if (diff[i] > max_threshold)
			break;
	}
	// End point
	int i2_1 = i2_2 - 1;
	for (int i = i2_1; i > 0; i--)
	{
		i2_1 = i;
		if (diff[i] <= min_threshold)
			break;
	}
	*i_beginning = i2_1;
	*i_end = i2_2;
}
void ToolsPlugin::on_ThreadedStatisticsCalculation_ends()
{
	ui.JSHistogram->Clear();
	disconnect((QObject*)(this->ui.JSHistogram), SIGNAL(PageSuccessfullyLoaded()), 0, 0);
	connect((QObject*)(this->ui.JSHistogram), SIGNAL(PageSuccessfullyLoaded()), (QObject*)this, SLOT(on_histogram_PageSuccessfullyLoaded()));

	bool is_ok = m_CalculationThread->GetStatisticsUpdateSuccessFlag();
	if (!is_ok)
		return;

	auto baseData = m_SelectedNode->GetData();
	mitk::Image* image = dynamic_cast<mitk::Image*>(baseData);
	if (!image)
		return;

	if (image != m_StatisticsImage)
		return;

	/// Update statistics
	auto statistics = m_CalculationThread->GetStatisticsData();

	unsigned int num_timeSteps = image->GetTimeSteps();
	ui.tableWidget_Statistics->setColumnCount(num_timeSteps);
	ui.tableWidget_Statistics->horizontalHeader()->setVisible(num_timeSteps > 1);

	assert(statistics.size() == num_timeSteps);
	num_timeSteps = max(num_timeSteps, (unsigned int)statistics.size());

	for (unsigned int t = 0; t < num_timeSteps; t++)
	{
		ui.tableWidget_Statistics->setHorizontalHeaderItem(t, new QTableWidgetItem(QString::number(t)));

		/*if (statistics.at(t)->GetMaxIndex().size() == 3)
		{
			mitk::Point3D index, max, min;
			index[0] = statistics.at(t)->GetMaxIndex()[0];
			index[1] = statistics.at(t)->GetMaxIndex()[1];
			index[2] = statistics.at(t)->GetMaxIndex()[2];
			image->GetGeometry()->IndexToWorld(index, max);
			this->m_WorldMaxList.push_back(max);
			index[0] = statistics.at(t)->GetMinIndex()[0];
			index[1] = statistics.at(t)->GetMinIndex()[1];
			index[2] = statistics.at(t)->GetMinIndex()[2];
			m_SelectedImage->GetGeometry()->IndexToWorld(index, min);
			this->m_WorldMinList.push_back(min);
		}*/

		auto stat_t = statistics.at(t).GetPointer();
		vector<QString> elements;
		unsigned int decimals = 2;
		unsigned int decimalsHigherOrderStatistics = 5; //statistics of higher order should have 5 decimal places because they used to be very small

		if (image->GetPixelType().GetComponentType() == itk::ImageIOBase::DOUBLE || image->GetPixelType().GetComponentType() == itk::ImageIOBase::FLOAT)
		{
			decimals = 5;
		}

		elements.push_back(GetFormattedString(stat_t->GetMean(), decimals));
		elements.push_back(GetFormattedString(stat_t->GetMedian(), decimals));
		elements.push_back(GetFormattedString(stat_t->GetStd(), decimals));
		elements.push_back(GetFormattedString(stat_t->GetRMS(), decimals));
		elements.push_back(GetFormattedString(stat_t->GetMax(), decimals) + " " + GetFormattedIndex(stat_t->GetMaxIndex()));
		elements.push_back(GetFormattedString(stat_t->GetMin(), decimals) + " " + GetFormattedIndex(stat_t->GetMinIndex()));
		//to prevent large negative values of empty image statistics
		if (stat_t->GetN() != std::numeric_limits<long>::min())
		{
			elements.push_back(GetFormattedString(stat_t->GetN(), 0));

			const mitk::BaseGeometry *geometry = image->GetGeometry();
			bool noVolumeDefined = false;
			if (geometry != nullptr && !noVolumeDefined)
			{
				const mitk::Vector3D &spacing = image->GetGeometry()->GetSpacing();
				double volume = spacing[0] * spacing[1] * spacing[2] * static_cast<double>(stat_t->GetN());
				elements.push_back(GetFormattedString(volume, decimals));
			}
			else
			{
				elements.push_back("NA");
			}
		}
		else {
			elements.push_back("NA");
			elements.push_back("NA");
		}

		elements.push_back(GetFormattedString(stat_t->GetSkewness(), decimalsHigherOrderStatistics));
		elements.push_back(GetFormattedString(stat_t->GetKurtosis(), decimalsHigherOrderStatistics));
		elements.push_back(GetFormattedString(stat_t->GetUniformity(), decimalsHigherOrderStatistics));
		elements.push_back(GetFormattedString(stat_t->GetEntropy(), decimalsHigherOrderStatistics));
		elements.push_back(GetFormattedString(stat_t->GetMPP(), decimals));
		elements.push_back(GetFormattedString(stat_t->GetUPP(), decimalsHigherOrderStatistics));

		auto hist_t = stat_t->GetHistogram();
		// Percentiles
		vector<double> percetiles(num_percentiles);
		for (int i = 0; i < percetiles.size(); i++)
		{
			double val = hist_t->Quantile(0, 0.01*i);
			percetiles[i] = val;
		}
		// Difference
		vector<double> diff(percetiles.size() - 1);
		for (int i = 0; i < percetiles.size() - 1; i++)
		{
			double d = percetiles[i + 1] - percetiles[i];
			diff[i] = d;
		}
		// Output
		for (int i = 0; i < diff.size(); i++)
		{
			stringstream ss;
			ss << setprecision(1) << std::fixed << setfill('0') << setw(4) << percetiles[i] << " / " << setw(4) << diff[i];
			elements.push_back(QString::fromStdString(ss.str()));
		}
		// Detect the maximum gap
		//auto it_max_gap = max_element(diff.begin(), diff.end());
		//double max_gap = *it_max_gap;
		//int i_max_gap = distance(diff.begin(), it_max_gap);
		// Mean, min, max gap
		vector<double> diff_sorted = diff;
		sort(diff_sorted.begin(), diff_sorted.end());
		double mean_gap = diff_sorted[0.5 * diff_sorted.size()];
		double min_gap = diff_sorted[0.05 * diff_sorted.size()];
		double max_gap = diff_sorted[0.95 * diff_sorted.size()];
		double max_diff = diff_sorted.back();
		// Threshold
		double min_threshold_1 = 2 * min_gap;
		double max_threshold_1 = 3 * min_gap;
		double min_threshold_2 = 2 * max_gap;
		double max_threshold_2 = 3 * max_gap;
		//!! check that there are enough elements in between
		int i2_2; // End of end region
		int i2_1; // End of end region
		detect_end(diff, min_threshold_2, max_threshold_2, &i2_1, &i2_2);
		int i1_1; // Start of start region
		int i1_2; // End of start region
		detect_beginning(diff, min_threshold_1, max_threshold_1, &i1_1, &i1_2);
		stringstream ss;
		ss.precision(1);
		ss << std::fixed << i1_1 << "_" << i2_1;
		// ...Check if the image is too dark (then there is no black reference)
		// Set the new range
		double min = percetiles[i1_1 + 1];
		double max = percetiles[i2_1 + 1];
		int i1 = i1_1;
		// Additional correction if the range is huge
		bool is_corrected = false;
		if (i2_1 - i1 > 20 && max - min > 500 && min < -300)
		{
			// Use end of start region?
			double min_2 = percetiles[i1_2 + 1];
			if (i1_2 - i1_1 < 20 && i2_1 - i1_2 > 20 && max - min_2 > 250)
			{
				i1 = i1_2;
				min = min_2;
				is_corrected = true;
			}
			// New start?
			int i1_1a; // Start of start region
			int i1_2a; // End of start region
			double min_threshold_1a = mean_gap + 0.2 * (max_gap - min_gap);
			double max_threshold_1a = mean_gap + 0.3 * (max_gap - min_gap);
			detect_beginning(diff, min_threshold_1a, max_threshold_1a, &i1_1a, &i1_2a);
			double min_a = percetiles[i1_2a + 1];
			if (i1_2a > i1 && i2_1 - i1_2a > 20 && max - min_a > 250)
			{
				i1_2 = i1_2a;
				min = min_a;
				is_corrected = true;
			}
			// New end?
			int i2_2a; // End of end region
			int i2_1a; // End of end region
			double min_threshold_2a = min_threshold_1a;
			double max_threshold_2a = max_threshold_1a;
			detect_end(diff, min_threshold_2a, max_threshold_2a, &i2_1a, &i2_2a);
			double max_a = percetiles[i2_1a + 1];
			if (i2_1a < i2_1 && i2_1a - i1_2 > 15 && max_a - min > 200)
			{
				i2_1 = i2_1a;
				max = max_a;
				is_corrected = true;
			}
		}
		if (is_corrected)
			ss << " / " << i1 << "_" << i2_1;
		ss << " v=" << (max + min) / 2 << "*" << (max - min);
		ss << ", thr1=" << min_threshold_1 << ".." << max_threshold_1;
		ss << ", thr2=" << min_threshold_2 << ".." << max_threshold_2;
		elements.push_back(QString::fromStdString(ss.str()));
		if (max > min)
		{
			// Range (-2 * 0%)
			double range = (max - min);
			double shift = range * 0.00;
			min += shift;
			max -= shift;
			// Update the range of LevelWindow
			ctkDictionary properties;
			properties["min"] = min; // -80.0;
			properties["max"] = max; // 150.0;
			emit SetLevelWindowRange(properties);
		}

		// Update UI
		unsigned int count = 0;
		for (const auto& entry : elements)
		{
			auto item = new QTableWidgetItem(entry);
			ui.tableWidget_Statistics->setItem(count, t, item);
			count++;
		}

	}

	ui.tableWidget_Statistics->resizeColumnsToContents();
	int height = STAT_TABLE_BASE_HEIGHT;

	if (ui.tableWidget_Statistics->horizontalHeader()->isVisible())
		height += ui.tableWidget_Statistics->horizontalHeader()->height();

	if (ui.tableWidget_Statistics->horizontalScrollBar()->isVisible())
		height += ui.tableWidget_Statistics->horizontalScrollBar()->height();

	ui.tableWidget_Statistics->setMinimumHeight(height);

	// make sure the current timestep's column is highlighted (and the correct histogram is displayed)
	unsigned int t = this->GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();
	mitk::SliceNavigationController::GeometryTimeEvent timeEvent(image->GetTimeGeometry(), t);
	this->on_imageNavigator_timeChanged(timeEvent);
	
	/// Display histogram.
	//int timestep = m_CalculationThread->GetTimeStep();
	//displayHistogram(timestep); --> will be displayed in this->on_imageNavigator_timeChanged(timeEvent) [see above]

	//!! Temp code
	int num_t = statistics.size();
	vector<long> n(num_t);
	vector<double> max(num_t);
	vector<double> min(num_t);
	vector<double> mean(num_t);
	vector<double> meanPP(num_t);
	vector<double> median(num_t);
	vector<double> stdDev(num_t);
	vector<unsigned long long> total_frequency(num_t);
	vector<vector<double>> frequency(num_t);
	vector<vector<double>> bin(num_t);
	for (int t = 0; t < num_t; t++)
	{
		auto stat = statistics[0];
		n[t] = stat->GetN();
		max[t] = stat->GetMax();
		min[t] = stat->GetMin();
		mean[t] = stat->GetMean();
		meanPP[t] = stat->GetMPP();
		median[t] = stat->GetMedian();
		stdDev[t] = stat->GetStd();
		auto hist = stat->GetHistogram();
		auto num_bins = hist->Size();
		auto dimemsions_size = hist->GetSize();
		auto size = dimemsions_size.GetSize();
		assert(size == 1);
		//auto size_0 = dimemsions_size[0];
		total_frequency[t] = hist->GetTotalFrequency();
		frequency[t].resize(num_bins);
		bin[t].resize(num_bins);
		for (int i = 0; i < num_bins; i++)
		{
			frequency[t][i] = hist->GetFrequency(i);
			auto max = hist->GetBinMax(0, i);
			auto min = hist->GetBinMin(0, i);
			bin[t][i] = (max + min) / 2;
		}
	}
}

//mitk::DataNode* ToolsPlugin::GetWorkingNode()
//{
//	mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
//	assert(workingNode);
//	return workingNode;
//}
mitk::DataNode* ToolsPlugin::getFirstSelectedNode(shared_ptr<DataNodeList> dataNodes)
{
	if (dataNodes == nullptr)
	{
		return m_SelectedNode;

		// ... or instead:
		auto selection = GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager");
		if (selection == nullptr)
		{
			return nullptr;
		}
		auto node_selection = selection.Cast<const mitk::DataNodeSelection>();
		if (node_selection == nullptr)
		{
			return nullptr;
		}
		dataNodes = shared_ptr<DataNodeList>(new DataNodeList);
		*dataNodes = node_selection->GetSelectedDataNodes();
	}
	for (const auto& dataNode : *dataNodes)
	{
		// Write robust code. Always check pointers before using them. If the
		// data node pointer is null, the second half of our condition isn't
		// even evaluated and we're safe (C++ short-circuit evaluation).
		if (dataNode.IsNotNull() && dynamic_cast<mitk::Image*>(dataNode->GetData()) != nullptr)
		{
			return dataNode;
		}
	}
	return nullptr;
}
void ToolsPlugin::enable3DRepresentation(bool flag)
{
	ctkDictionary properties;
	properties["enable3D"] = flag;
	emit Representation3DHasToBeInitiated(properties);
}