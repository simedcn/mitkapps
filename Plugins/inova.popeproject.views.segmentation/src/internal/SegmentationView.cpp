#include "SegmentationView.h"
#include "inova_popeproject_views_segmentation_PluginActivator.h"

#include <berryIWorkbenchPage.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <usModuleRegistry.h>
#include <QmitkDataStorageComboBox.h>

#include <QMessageBox>
#include <QPainter>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QTreeWidgetItem>

//#include <PopeImageFilter.h>
//#include <PopeImageInteractor.h>

#include <vtkSTLWriter.h>

#include <ctkPluginActivator.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEventConstants.h>

#include <mitkLabel.h>
#include <mitkToolManagerProvider.h>
#include <mitkOtsuSegmentationFilter.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkIOUtil.h>
#include <mitkIRenderWindowPart.h>

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <itkImageRegionIterator.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkPaintbrushTool.h>

#include <itkBinaryThresholdImageFilter.h>

#include <QmitkDataNodeSelectionProvider.h>

#include <QmitkDataManagerView.h>

#include <algorithm>

using namespace std;

// Don't forget to initialize the VIEW_ID.
const string SegmentationView::VIEW_ID = "inova.popeproject.views.segmentation";


SegmentationView::SegmentationView()
{}
SegmentationView::~SegmentationView()
{}

#include <berryPlatformUI.h>
#include <berryQtWorkbenchAdvisor.h>
void SegmentationView::CreateQtPartControl(QWidget* parent)
{
	/// Setting up the UI is a true pleasure when using .ui files, isn't it?
	ui.setupUi(parent);
	this->GetRenderWindowPart(OPEN);
	this->RequestRenderWindowUpdate();

	/// Wire up the UI widgets with our functionality.

	setLabelWidget();
	updateAfterSelectionChanged();

	/// Connect Signals and Slots of the Plugin UI.
	connect(this->ui.button_STLExport, SIGNAL(clicked()), this, SLOT(on_button_STLExport_clicked()));
	connect(this->ui.button_OtsuSegmentation, SIGNAL(clicked()), this, SLOT(on_button_OtsuSegmentation_clicked()));
	connect(this->ui.button_ExportCSV, SIGNAL(clicked()), this, SLOT(on_button_ExportCSV_clicked()));
	connect(this->ui.button_Paint, SIGNAL(clicked()), this, SLOT(on_button_Paint_clicked()));
}
void SegmentationView::setLabelWidget()
{
	ui.table_LabelSet->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	ui.table_LabelSet->setTabKeyNavigation(false);
	ui.table_LabelSet->setAlternatingRowColors(false);
	ui.table_LabelSet->setFocusPolicy(Qt::NoFocus);
	ui.table_LabelSet->setColumnCount(3);
	ui.table_LabelSet->resizeColumnToContents(NAME_COL);
	ui.table_LabelSet->setColumnWidth(COLOR_COL, 50);
	ui.table_LabelSet->setColumnWidth(VISIBLE_COL, 50);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	ui.table_LabelSet->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
	ui.table_LabelSet->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
#endif
	ui.table_LabelSet->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.table_LabelSet->horizontalHeader()->hide();
	ui.table_LabelSet->setSortingEnabled(false);
	ui.table_LabelSet->verticalHeader()->hide();
	ui.table_LabelSet->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.table_LabelSet->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.table_LabelSet->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void SegmentationView::updateAfterSelectionChanged()
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
		ui.label_ImageNotSelected->setToolTip("");
		ui.label_ImageNotSelected->setStyleSheet("color: #E02000;\nbackground-color: #efef95;");
		ui.label_ImageNotSelected->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	}
	ui.label_SelectedImage->setVisible(is_selected_node);

	ui.button_STLExport->setEnabled(is_selected_node);
	ui.button_ExportCSV->setEnabled(is_selected_node);
	ui.button_OtsuSegmentation->setEnabled(is_selected_node);
	ui.button_Paint->setEnabled(is_selected_node);
}

void SegmentationView::NodeAdded(const mitk::DataNode* node)
{
	// Binary / segmentation
	bool binary = false;
	node->GetBoolProperty("binary", binary);
	bool segmentation = false;
	node->GetBoolProperty("segmentation", segmentation);//??
	if (!binary || !segmentation)
	{
		//mitk::Image* img = dynamic_cast<mitk::Image*>(node->GetData());
		//if (img)
		//{
		//	ui.list_Images->addItem(new QListWidgetItem(QString::fromStdString(node->GetName())));
		//}
		return;
	}

	itk::SmartPointer<mitk::Label> label = mitk::Label::New();
	string name;
	node->GetStringProperty("name", name);
	label->SetName(name);

	float rgb[3];
	node->GetColor(rgb);
	mitk::Color color;
	color.SetRed(rgb[0]);
	color.SetGreen(rgb[1]);
	color.SetBlue(rgb[2]);

	QString styleSheet = "background-color:rgb(";
	styleSheet.append(QString::number(color[0] * 255));
	styleSheet.append(",");
	styleSheet.append(QString::number(color[1] * 255));
	styleSheet.append(",");
	styleSheet.append(QString::number(color[2] * 255));
	styleSheet.append(")");

	QTableWidget* tableWidget = ui.table_LabelSet;
	int colWidth = (tableWidget->columnWidth(NAME_COL) < 180) ? 180 : tableWidget->columnWidth(NAME_COL) - 2;
	QString text = tableWidget->fontMetrics().elidedText(label->GetName().c_str(), Qt::ElideMiddle, colWidth);
	QTableWidgetItem *nameItem = new QTableWidgetItem(text);
	nameItem->setTextAlignment(Qt::AlignCenter | Qt::AlignLeft);
	// ---!---
	// IMPORTANT: ADD PIXELVALUE TO TABLEWIDGETITEM.DATA

	nameItem->setData(Qt::UserRole, QVariant(QString::fromStdString(name)));
	// ---!---

	QPushButton *pbColor = new QPushButton(tableWidget);
	pbColor->setFixedSize(24, 24);
	pbColor->setCheckable(false);
	pbColor->setAutoFillBackground(true);
	pbColor->setToolTip("Change label color");
	pbColor->setStyleSheet(styleSheet);

	connect(pbColor, SIGNAL(clicked()), this, SLOT(OnColorButtonClicked()));

	QPushButton *pbVisible = new QPushButton(tableWidget);
	pbVisible->setAutoRepeat(false);
	QIcon *iconVisible = new QIcon();
	iconVisible->addFile(QString::fromUtf8(":/Pope/visible.png"), QSize(), QIcon::Normal, QIcon::Off);
	iconVisible->addFile(QString::fromUtf8(":/Pope/invisible.png"), QSize(), QIcon::Normal, QIcon::On);
	pbVisible->setIcon(*iconVisible);
	pbVisible->setIconSize(QSize(24, 24));
	pbVisible->setCheckable(true);
	pbVisible->setToolTip("Show/hide segmentation");
	pbVisible->setChecked(!label->GetVisible());

	connect(pbVisible, SIGNAL(clicked()), this, SLOT(OnVisibleButtonClicked()));

	int row = tableWidget->rowCount();
	tableWidget->insertRow(row);
	tableWidget->setRowHeight(row, 24);
	tableWidget->setItem(row, NAME_COL, nameItem);
	tableWidget->setCellWidget(row, COLOR_COL, pbColor);
	tableWidget->setCellWidget(row, VISIBLE_COL, pbVisible);
	tableWidget->selectRow(row);

	// m_LabelSetImage->SetActiveLabel(label->GetPixelValue());
	// m_ToolManager->WorkingDataModified.Send();
	// emit activeLabelChanged(label->GetPixelValue());

	if (tableWidget->rowCount() == 0)
	{
		tableWidget->hideRow(row); // hide exterior label
	}
}
void SegmentationView::NodeRemoved(const mitk::DataNode* node)
{
	QTableWidget* tableWidget = ui.table_LabelSet;
	for (int i = 0; i < tableWidget->rowCount(); i++)
	{
		if (tableWidget->item(i, NAME_COL)->text().toStdString().compare(node->GetName()) == 0)
		{
			tableWidget->removeRow(i);
			return;
		}
	}
}

void SegmentationView::SetFocus()
{
	ui.table_LabelSet->setFocus();
}

void SegmentationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& dataNodes)
{
	auto nodes = shared_ptr<DataNodeList>(new DataNodeList);
	*nodes = dataNodes.toStdList();
	auto dataNode = getFirstSelectedNode(nodes);

	// Update current selected node
	/// Remeber datanode selected in DataStorage.
	m_SelectedNode = dataNode;

	if (dataNode != nullptr)
	{// dataNode == nullptr when nothing is selected or the selection doesn't contain an image.
		//setLabelWidget();
	}

	/// Update views.
	updateAfterSelectionChanged();
}

void SegmentationView::on_button_STLExport_clicked()
{
	MITK_INFO << "Export STLs";
	itk::SmartPointer<mitk::ImageToSurfaceFilter> i2sf = mitk::ImageToSurfaceFilter::New();
	itk::SmartPointer<mitk::Surface> surface = mitk::Surface::New();
	//vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();

	if (ui.table_LabelSet->rowCount() <= 0)
	{
		MITK_WARN << "There is no label set to export.";
		return;
	}

	QString dir = QFileDialog::getExistingDirectory(nullptr, tr("Open Directory"),
		QString(), // standard path
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	QString csvPath = dir + "/segmentation.csv";
	QFile data(csvPath);
	if (!data.open(QFile::WriteOnly | QFile::Truncate))
	{
		MITK_WARN << "Failed to open file \"" << csvPath << "\" to write.";
		return;
	}

	for (int row = 0; row < ui.table_LabelSet->rowCount(); ++row)
	{
		string name_selected = ui.table_LabelSet->item(row, 0)->data(Qt::UserRole).toString().toStdString();
		mitk::DataNode* node_selected = GetDataStorage()->GetNamedNode(name_selected);
		string name;
		node_selected->GetStringProperty("name", name);
		MITK_INFO << name;

		i2sf->SetInput(dynamic_cast<mitk::Image*> (node_selected->GetData()));
		i2sf->Update();
		surface = i2sf->GetOutput();
		MITK_INFO << surface->GetVtkPolyData()->GetNumberOfPoints();

		stringstream sstr;
		sstr << dir.toStdString() << "/" << name << ".stl";
		MITK_INFO << sstr.str();

		float rgb[3];
		node_selected->GetColor(rgb);
		QTextStream output(&data);
		output << dir << "/" << QString::fromStdString(name) << ".stl; #" << QString("%1%2%3").arg(int(rgb[0] * 255), 2, 16, QChar('0')).arg(int(rgb[1] * 255), 2, 16, QChar('0')).arg(int(rgb[2] * 255), 2, 16, QChar('0')).toUpper();

		mitk::IOUtil::Save(surface, sstr.str());
		/*writer->SetFileName(sstr.str().c_str());
		writer->SetInputData(surface->GetVtkPolyData());
		writer->Update();*/
	}
}
void SegmentationView::on_button_ExportCSV_clicked()
{
	/// Export image values into CSV:
	/// x,y,z, Segmentation_value, Modality1_Value, Modality2_Value, Modality3_Value,...

	MITK_INFO << "Export CSV";

	auto dataNode = getFirstSelectedNode();
	if (dataNode == nullptr)
	{
		MITK_WARN << "No image selected.";
		return;
	}

	QString imagePath = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Save to folder ..."), "", tr("CSV Export (*.csv);"));

	QFile data(imagePath);
	if (!data.open(QFile::WriteOnly | QFile::Truncate))
	{
		MITK_WARN << "Failed to open file \"" << imagePath << "\" to write.";
		return;
	}

	string img_name = dataNode->GetName();
	//string img_name_2;
	//dataNode->GetStringProperty("name", img_name_2);
	//assert(img_name == img_name_2);
	MITK_INFO << img_name;

	QModelIndex segmentationSelection = ui.table_LabelSet->currentIndex();
	QString segmentationName = segmentationSelection.data(Qt::DisplayRole).toString();

	QTextStream output(&data);
	QString header = "x,y,z";
	header = header + "," + segmentationName;
	auto nodes = GetDataStorage()->GetAll();
	MITK_INFO << "Number of nodes: " << nodes->Size();
	if (nodes != nullptr)
	{
		for (const auto node : *nodes)
		{
			mitk::Image* img = dynamic_cast<mitk::Image*>(node->GetData());
			if (img)
				header += "," + QString::fromStdString(node->GetName());
		}
	}
	output << header << "\n";

	// Now iterate through image and write down all voxel positions and image values
	// Here we use ITK to do this
	using TSegmentationImage = itk::Image<unsigned char, 3>;
	using TFeatureImage = itk::Image<double, 3>;
	using PTFeatureImage = TFeatureImage::Pointer;

	mitk::Image* img = dynamic_cast<mitk::Image*>(GetDataStorage()->GetNamedNode(segmentationName.toStdString())->GetData());
	// Create a new ITK image and assign the content of the MITK image to it
	TSegmentationImage::Pointer itk_img = TSegmentationImage::New();
	mitk::CastToItkImage(img, itk_img);

	vector<PTFeatureImage> featImgs;
	if (nodes != nullptr)
	{
		for (const auto node : *nodes)
		{
			mitk::Image* img = dynamic_cast<mitk::Image*>(node->GetData());
			if (img)
			{
				PTFeatureImage itk_img = TFeatureImage::New();
				mitk::CastToItkImage(img, itk_img);
				featImgs.push_back(itk_img);
			}
		}
	}

	// Use an itk image iterator to walk through all voxels of the segmentation
	itk::ImageRegionIterator<TSegmentationImage> it(itk_img, itk_img->GetLargestPossibleRegion());
	while (!it.IsAtEnd())
	{
		QString voxelValues;
		if (it.Get() != 0)
		{
			// the position of the iterator in index position
			TSegmentationImage::IndexType index =  it.GetIndex();
			// using the segmentation images geometry the index is mapped to a point in world coordinates
			itk::Point<double, 3> worldPos;
			itk_img->TransformIndexToPhysicalPoint(index, worldPos);
			voxelValues = voxelValues + QString::number(worldPos[0]) + "," + QString::number(worldPos[1]) + "," + QString::number(worldPos[2]);
			voxelValues = voxelValues + "," + QString::number(it.Get());
			// mapping the world coordinates to index coordinates for each image separately
			// NOTE: this is only necesary when we cannot be sure that all images match perfectly
			// Index Coordinates can be used directly if the following is given:
			// - Images all have the same dimensions, i.e. same resolution in each direction AND
			// - origin, spacing and orientation are exactly the same.
			for (auto featureImg : featImgs)
			{
				TFeatureImage::IndexType fIndex;
				featureImg->TransformPhysicalPointToIndex(worldPos, fIndex);
				// check if this position exists in the current image
				if (featureImg->GetLargestPossibleRegion().IsInside(fIndex))
					voxelValues = voxelValues + "," + QString::number(featureImg->GetPixel(fIndex));
				else
					voxelValues = voxelValues + ",0";
			}
			output << voxelValues << "\n";
		}
		++it;
	}
}
void SegmentationView::on_button_OtsuSegmentation_clicked()
{
	auto dataNode = getFirstSelectedNode();
	if (!dataNode)
	{
		MITK_WARN << "OtsuSegmentation skipped because there is no selected image.";
		return;
	}
	mitk::Image* image = dynamic_cast<mitk::Image*>(dataNode->GetData());
	assert(image != nullptr);
	//if (!image)
	//{
	//	MITK_WARN << "OtsuSegmentation skipped because there is no selected image.";
	//	return;
	//}

	// Once we have the image, there are different ways to perform image filters on it
	// Here using an MITK image filter

	//binärbild (segmentierung) auswählen, zb im datamanager oder per combobox
	// image mit segmentierung maskieren und ergebnis -> otsufilter->SetInput(maskedImage);
	mitk::OtsuSegmentationFilter::Pointer otsuFilter = mitk::OtsuSegmentationFilter::New();
	otsuFilter->SetNumberOfThresholds(3);
	otsuFilter->SetValleyEmphasis(false);
	otsuFilter->SetNumberOfBins(255);
	otsuFilter->SetInput(image);

	try
	{
		otsuFilter->Update();
	}
	catch (...)
	{
		mitkThrow() << "itkOtsuFilter error (image dimension must be in {2, 3} and image must not be RGB)";
	}

	using TImage = itk::Image<unsigned char, 3>;
	mitk::Image* img = otsuFilter->GetOutput();
	// Create a new ITK image and assign the content of the MITK image to it
	TImage::Pointer itkImg = TImage::New();
	mitk::CastToItkImage(img,itkImg);
	// Not we can put the image into an ITK Filter

	using FilterType = itk::BinaryThresholdImageFilter<TImage, TImage>;
	typename FilterType::Pointer filter = FilterType::New();
	// set properties for filter
	filter->SetInput(itkImg);

	// accept values 1 to 2
	filter->SetLowerThreshold(1);
	filter->SetUpperThreshold(2);
	filter->SetInsideValue(1);
	filter->SetOutsideValue(0);
	filter->Update();

	typename TImage::Pointer itkBinaryResultImage;
	mitk::Image::Pointer binarySegmentation;

	itkBinaryResultImage = filter->GetOutput();
	mitk::CastToMitkImage(itkBinaryResultImage, binarySegmentation);

	// Create a new DataNode to put the output image in;
	mitk::DataNode::Pointer resultImage = mitk::DataNode::New();
	resultImage->SetData(binarySegmentation);
	resultImage->SetName("Binary Otsu Result");
	this->GetDataStorage()->Add(resultImage);
}
void SegmentationView::on_button_Paint_clicked()
{
	auto tm = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
	// tm->SetReferenceData(const_cast<mitk::DataNode*>(referenceData));
	// tm->SetWorkingData(const_cast<mitk::DataNode*>(workingData));
	tm->ActivateTool(tm->GetToolIdByToolType<mitk::PaintbrushTool>());
}

void SegmentationView::OnColorButtonClicked()
{
	int row = -1;
	for (int i = 0; i < ui.table_LabelSet->rowCount(); ++i)
	{
		if (sender() == ui.table_LabelSet->cellWidget(i, COLOR_COL))
		{
			row = i;
		}
	}

	if (row >= 0 && row < ui.table_LabelSet->rowCount())
	{
		string name_selected = ui.table_LabelSet->item(row, 0)->data(Qt::UserRole).toString().toStdString();
		//string name_selected = ui.table_LabelSet->item(row, 0)->text().toStdString();
		float rgb[3];
		mitk::DataNode* node_selected = GetDataStorage()->GetNamedNode(name_selected);
		node_selected->GetColor(rgb);
		mitk::Color color;
		color.SetRed(rgb[0]);
		color.SetGreen(rgb[1]);
		color.SetBlue(rgb[2]);
		QColor initial(color.GetRed() * 255, color.GetGreen() * 255, color.GetBlue() * 255);
		QColor qcolor = QColorDialog::getColor(initial, 0, QString("Change color"));
		if (!qcolor.isValid())
		{
			return;
		}

		QPushButton *button = static_cast<QPushButton*>(ui.table_LabelSet->cellWidget(row, COLOR_COL));
		if (!button)
		{
			return;
		}

		button->setAutoFillBackground(true);

		QString styleSheet = "background-color:rgb(";
		styleSheet.append(QString::number(qcolor.red()));
		styleSheet.append(",");
		styleSheet.append(QString::number(qcolor.green()));
		styleSheet.append(",");
		styleSheet.append(QString::number(qcolor.blue()));
		styleSheet.append(")");
		button->setStyleSheet(styleSheet);

		mitk::Color newColor;
		newColor.SetRed(qcolor.red() / 255.0);
		newColor.SetGreen(qcolor.green() / 255.0);
		newColor.SetBlue(qcolor.blue() / 255.0);

		node_selected->SetColor(newColor);
	}
}
void SegmentationView::OnVisibleButtonClicked()
{
	int row = -1;
	for (int i = 0; i < ui.table_LabelSet->rowCount(); ++i)
	{
		if (sender() == ui.table_LabelSet->cellWidget(i, VISIBLE_COL))
		{
			row = i;
			break;
		}
	}
	if (row >= 0 && row < ui.table_LabelSet->rowCount())
	{
		//QTableWidgetItem *item = ui.table_LabelSet->item(row, 0);
		string name_selected = ui.table_LabelSet->item(row, 0)->data(Qt::UserRole).toString().toStdString();
		mitk::DataNode* node_selected = GetDataStorage()->GetNamedNode(name_selected);
		bool visible = false;
		node_selected->GetVisibility(visible,nullptr);
		node_selected->SetVisibility(!visible);
	}
}

mitk::DataNode* SegmentationView::getFirstSelectedNode(shared_ptr<DataNodeList> dataNodes)
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
