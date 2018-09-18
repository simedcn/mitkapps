#include "inova_registration_views_visualizer_Activator.h"
#include "RegistrationVisualizer.h"
#include <PopeElements.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Mitk
#include <mitkStatusBar.h>
#include <mitkProperties.h>
#include <mitkColorProperty.h>
#include <mitkNodePredicateDataProperty.h>
#include "mitkRegVisDirectionProperty.h"
#include "mitkRegVisStyleProperty.h"
#include "mitkRegVisColorStyleProperty.h"
#include "mitkRegVisPropertyTags.h"
#include "mitkRegVisHelper.h"
#include "mitkMatchPointPropertyTags.h"
#include "mitkRegistrationHelper.h"

// Qt
#include <QMessageBox>
#include <QErrorMessage>

const std::string RegistrationVisualizer::VIEW_ID = "inova.registration.views.visualizer";

RegistrationVisualizer::RegistrationVisualizer()
	: m_Parent(nullptr)
	, m_internalUpdateGuard(false)
	, m_spSelectedFOVRefNode(nullptr)
	, m_spSelectedRegNode(nullptr)
{
	m_isUpdateNeeded = false;
	m_isBusy = false;
	m_timerForUpdate = new QTimer(this);
	m_timerForUpdate->setSingleShot(true);
}

void RegistrationVisualizer::SetFocus()
{
	//ui->buttonPerformImageProcessing->setFocus();
}

void RegistrationVisualizer::CreateConnections()
{
	/// Create connections.
	// Show first page
	connect(ui->m_pbLockReg, SIGNAL(clicked()), this, SLOT(OnLockRegButtonPushed()));

	connect(ui->m_pbStyleGrid, SIGNAL(clicked()), this, SLOT(OnStyleButtonPushed()));
	connect(ui->m_pbStyleGlyph, SIGNAL(clicked()), this, SLOT(OnStyleButtonPushed()));
	connect(ui->m_pbStylePoints, SIGNAL(clicked()), this, SLOT(OnStyleButtonPushed()));

	connect(ui->m_comboDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDirectionChanged(int)));
	connect(ui->m_pbUpdateViz, SIGNAL(clicked()), this, SLOT(OnUpdateBtnPushed()));

	connect(ui->m_pbUseFOVRef, SIGNAL(clicked()), this, SLOT(OnUseFOVRefBtnPushed()));

	connect(ui->radioColorUni, SIGNAL(toggled(bool)), ui->btnUniColor, SLOT(setEnabled(bool)));
	connect(ui->radioColorVecMag, SIGNAL(toggled(bool)), ui->groupColorCoding, SLOT(setEnabled(bool)));

	connect(ui->m_pbStyleGrid, SIGNAL(toggled(bool)), ui->tabGrid, SLOT(setEnabled(bool)));

	connect(ui->cbVevMagInterlolate, SIGNAL(toggled(bool)), this, SLOT(OnColorInterpolationChecked(bool)));

	connect(m_timerForUpdate, SIGNAL(timeout()), this, SLOT(UpdateVisualization()));

	/// Connections to update visualization.
	//connect(ui->m_comboDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(RequestUpdateVisualization()));
	//connect(ui->m_pbStyleGrid, SIGNAL(clicked()), this, SLOT(RequestUpdateVisualization()));
	//connect(ui->m_pbStyleGlyph, SIGNAL(clicked()), this, SLOT(RequestUpdateVisualization()));
	//connect(ui->m_pbStylePoints, SIGNAL(clicked()), this, SLOT(RequestUpdateVisualization()));
	connect(ui->radioColorUni, SIGNAL(toggled(bool)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->radioColorVecMag, SIGNAL(toggled(bool)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->btnUniColor, SIGNAL(colorChanged(QColor)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->btnVecMagColorNeg, SIGNAL(colorChanged(QColor)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->btnVecMagColorSmall, SIGNAL(colorChanged(QColor)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->btnVecMagColorMedium, SIGNAL(colorChanged(QColor)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->btnVecMagColorLarge, SIGNAL(colorChanged(QColor)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->sbVecMagSmall, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->sbVecMagMedium, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->sbVecMagLarge, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	//connect(ui->cbVevMagInterlolate, SIGNAL(toggled(bool)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_sbGridFrequency, SIGNAL(valueChanged(int)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_groupShowStartGrid, SIGNAL(toggled(bool)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->btnStartGridColor, SIGNAL(colorChanged(QColor)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_sbFOVSizeX, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_sbFOVSizeY, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_sbFOVSizeZ, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_sbFOVOriginX, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_sbFOVOriginY, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_sbFOVOriginZ, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_sbGridSpX, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_sbGridSpY, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_sbGridSpZ, SIGNAL(valueChanged(double)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_tableOrientation, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_checkUseRefSize, SIGNAL(toggled(bool)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_checkUseRefOrigin, SIGNAL(toggled(bool)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_checkUseRefSpacing, SIGNAL(toggled(bool)), this, SLOT(RequestUpdateVisualization()));
	connect(ui->m_checkUseRefOrientation, SIGNAL(toggled(bool)), this, SLOT(RequestUpdateVisualization()));
}

void RegistrationVisualizer::Error(QString msg)
{
	mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
	MITK_ERROR << msg.toStdString().c_str();
}

void RegistrationVisualizer::CreateQtPartControl(QWidget* parent)
{
	/// create GUI widgets from the Qt Designer's .ui file.
	ui = new Ui::RegistrationVisualizerControls;
	ui->setupUi(parent);
	m_Parent = parent;

	/// Set defaults.
	this->ui->m_pbLockReg->setVisible(false);
	this->ui->m_pbLockReg->setChecked(false);
	this->ui->m_pbUpdateViz->setVisible(false);

	this->ui->btnVecMagColorSmall->setDisplayColorName(false);
	this->ui->btnVecMagColorMedium->setDisplayColorName(false);
	this->ui->btnVecMagColorLarge->setDisplayColorName(false);
	this->ui->btnVecMagColorNeg->setDisplayColorName(false);
	this->ui->btnUniColor->setDisplayColorName(false);
	this->ui->btnStartGridColor->setDisplayColorName(false);

	this->ui->radioColorUni->setChecked(false);
	this->ui->radioColorVecMag->setChecked(true);

	this->CheckInputs();
	this->LoadStateFromNode();
	this->ConfigureVisualizationControls();

	/// Create signal/slot connections after all updates.
	this->CreateConnections();
}

mitk::MAPRegistrationWrapper* RegistrationVisualizer::GetCurrentRegistration()
{
	mitk::MAPRegistrationWrapper* result = nullptr;

	if (this->m_spSelectedRegNode.IsNotNull())
	{
		result = dynamic_cast<mitk::MAPRegistrationWrapper*>(this->m_spSelectedRegNode->GetData());
		assert(result);
	}

	return result;
}

mitk::DataNode::Pointer RegistrationVisualizer::GetSelectedRegNode() const
{
	mitk::DataNode::Pointer spResult = nullptr;

	typedef QList<mitk::DataNode::Pointer> NodeListType;

	NodeListType nodes = this->GetDataManagerSelection();

	for (NodeListType::iterator pos = nodes.begin(); pos != nodes.end(); ++pos)
	{
		if (mitk::MITKRegistrationHelper::IsRegNode(*pos))
		{
			spResult = *pos;
			break;
		}
	}

	return spResult;
}

mitk::DataNode::Pointer RegistrationVisualizer::GetRefNodeOfReg(bool target) const
{
	mitk::DataNode::Pointer spResult = nullptr;

	if (this->m_spSelectedRegNode.IsNotNull() && m_spSelectedRegNode->GetData())
	{
		std::string nodeName;
		mitk::BaseProperty* uidProp;

		if (target)
		{
		  uidProp = m_spSelectedRegNode->GetData()->GetProperty(mitk::Prop_RegAlgTargetData);
		}
		else
		{
		  uidProp = m_spSelectedRegNode->GetData()->GetProperty(mitk::Prop_RegAlgMovingData);
		}

		if (uidProp)
		{
			//search for the target node
			mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID, uidProp);
			spResult = this->GetDataStorage()->GetNode(predicate);
		}
	}

	return spResult;
}

mitk::DataNode::Pointer RegistrationVisualizer::GetSelectedDataNode()
{
	mitk::DataNode::Pointer result;

	QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
	for (auto& node : nodes)
	{
		if (!mitk::MITKRegistrationHelper::IsRegNode(node) && node->GetData() && node->GetData()->GetGeometry())
		{
			result = node;
			break;
		}
	}

	return result;
}

void RegistrationVisualizer::CheckInputs()
{
	mitk::DataNode::Pointer regNode = this->GetSelectedRegNode();

	if (!ui->m_pbLockReg->isChecked())
	{
		this->m_spSelectedRegNode = regNode;
	}

	this->InitRegNode();

	mitk::DataNode::Pointer fovNode = this->GetSelectedDataNode();

	if (!ui->m_pbLockFOVRef->isChecked())
	{
		this->m_spSelectedFOVRefNode = fovNode;
	}
}

void RegistrationVisualizer::ConfigureVisualizationControls()
{
	if (!m_internalUpdateGuard)
	{
		m_internalUpdateGuard = true;
		ui->groupViz->setVisible(this->m_spSelectedRegNode.IsNotNull());

		ui->m_pbUpdateViz->setEnabled(this->m_spSelectedRegNode.IsNotNull());
		ui->m_boxSettings->setEnabled(this->m_spSelectedRegNode.IsNotNull());
		ui->m_boxStyles->setEnabled(this->m_spSelectedRegNode.IsNotNull());

		this->ActualizeRegInfo(this->GetCurrentRegistration());

		if (this->m_spSelectedRegNode.IsNull())
		{
			ui->m_lbRegistrationName->setText(QString("<font color='red'>No registration selected.</font>"));
		}
		else
		{
			QString short_name = Elements::get_short_name_for_image(this->m_spSelectedRegNode->GetName()); // , 40);
			ui->m_lbRegistrationName->setText(short_name);
		}

		this->ui->m_pbLockReg->setEnabled(this->m_spSelectedRegNode.IsNotNull());
		this->ui->m_pbUseFOVRef->setEnabled(this->m_spSelectedRegNode.IsNotNull() && this->m_spSelectedFOVRefNode.IsNotNull());
		this->ui->m_checkUseRefSize->setEnabled(this->m_spSelectedRegNode.IsNotNull() && this->m_spSelectedFOVRefNode.IsNotNull());
		this->ui->m_checkUseRefOrigin->setEnabled(this->m_spSelectedRegNode.IsNotNull() && this->m_spSelectedFOVRefNode.IsNotNull());
		this->ui->m_checkUseRefSpacing->setEnabled(this->m_spSelectedRegNode.IsNotNull() && this->m_spSelectedFOVRefNode.IsNotNull());

		this->ui->m_pbLockFOVRef->setEnabled(this->m_spSelectedFOVRefNode.IsNotNull());

		if (this->m_spSelectedFOVRefNode.IsNull())
		{
			ui->m_lbFOVRef->setText(QString("<font color='red'>no valid reference selected!</font>"));
		}
		else
		{
			QString short_name = Elements::get_short_name_for_image(this->m_spSelectedFOVRefNode->GetName()); //, 40);
			ui->m_lbFOVRef->setText(short_name);
		}

		m_internalUpdateGuard = false;
	}
}

void RegistrationVisualizer::StoreStateInNode()
{
	if (this->m_spSelectedRegNode == nullptr)
		return;

	//general
	this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisDirection, mitk::RegVisDirectionProperty::New(this->ui->m_comboDirection->currentIndex()));

	this->m_spSelectedRegNode->SetBoolProperty(mitk::nodeProp_RegVisGrid, this->ui->m_pbStyleGrid->isChecked());
	this->m_spSelectedRegNode->SetBoolProperty(mitk::nodeProp_RegVisGlyph, this->ui->m_pbStyleGlyph->isChecked());
	this->m_spSelectedRegNode->SetBoolProperty(mitk::nodeProp_RegVisPoints, this->ui->m_pbStylePoints->isChecked());

	//Visualization
	if (this->ui->radioColorUni->isChecked())
	{
		this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisColorStyle, mitk::RegVisColorStyleProperty::New(0));
	}
	else
	{
		this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisColorStyle, mitk::RegVisColorStyleProperty::New(1));
	}

	float tmpColor[3];

	tmpColor[0] = this->ui->btnUniColor->color().redF();
	tmpColor[1] = this->ui->btnUniColor->color().greenF();
	tmpColor[2] = this->ui->btnUniColor->color().blueF();
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColorUni, mitk::ColorProperty::New(tmpColor), nullptr, true);

	tmpColor[0] = this->ui->btnVecMagColorNeg->color().redF();
	tmpColor[1] = this->ui->btnVecMagColorNeg->color().greenF();
	tmpColor[2] = this->ui->btnVecMagColorNeg->color().blueF();
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor1Value, mitk::ColorProperty::New(tmpColor), nullptr, true);

	tmpColor[0] = this->ui->btnVecMagColorSmall->color().redF();
	tmpColor[1] = this->ui->btnVecMagColorSmall->color().greenF();
	tmpColor[2] = this->ui->btnVecMagColorSmall->color().blueF();
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor2Value, mitk::ColorProperty::New(tmpColor), nullptr, true);
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor2Magnitude, mitk::DoubleProperty::New(this->ui->sbVecMagSmall->value()), nullptr, true);

	tmpColor[0] = this->ui->btnVecMagColorMedium->color().redF();
	tmpColor[1] = this->ui->btnVecMagColorMedium->color().greenF();
	tmpColor[2] = this->ui->btnVecMagColorMedium->color().blueF();
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor3Value, mitk::ColorProperty::New(tmpColor), nullptr, true);
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor3Magnitude, mitk::DoubleProperty::New(this->ui->sbVecMagMedium->value()), nullptr, true);

	tmpColor[0] = this->ui->btnVecMagColorLarge->color().redF();
	tmpColor[1] = this->ui->btnVecMagColorLarge->color().greenF();
	tmpColor[2] = this->ui->btnVecMagColorLarge->color().blueF();
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor4Value, mitk::ColorProperty::New(tmpColor), nullptr, true);
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor4Magnitude, mitk::DoubleProperty::New(this->ui->sbVecMagLarge->value()), nullptr, true);

	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColorInterpolate, mitk::BoolProperty::New(this->ui->cbVevMagInterlolate->isChecked()), nullptr, true);

	//Grid Settings
	this->m_spSelectedRegNode->SetIntProperty(mitk::nodeProp_RegVisGridFrequence, this->ui->m_sbGridFrequency->value());
	this->m_spSelectedRegNode->SetBoolProperty(mitk::nodeProp_RegVisGridShowStart, this->ui->m_groupShowStartGrid->isChecked());
	tmpColor[0] = this->ui->btnStartGridColor->color().redF();
	tmpColor[1] = this->ui->btnStartGridColor->color().greenF();
	tmpColor[2] = this->ui->btnStartGridColor->color().blueF();
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGridStartColor, mitk::ColorProperty::New(tmpColor), nullptr, true);

	//FOV
	mitk::Vector3D value;
	value[0] = this->ui->m_sbFOVSizeX->value();
	value[1] = this->ui->m_sbFOVSizeY->value();
	value[2] = this->ui->m_sbFOVSizeZ->value();
	this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVSize, mitk::Vector3DProperty::New(value));

	value[0] = this->ui->m_sbGridSpX->value();
	value[1] = this->ui->m_sbGridSpY->value();
	value[2] = this->ui->m_sbGridSpZ->value();
	this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVSpacing, mitk::Vector3DProperty::New(value));

	mitk::Point3D origin;
	origin[0] = this->ui->m_sbFOVOriginX->value();
	origin[1] = this->ui->m_sbFOVOriginY->value();
	origin[2] = this->ui->m_sbFOVOriginZ->value();
	this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVOrigin, mitk::Point3dProperty::New(origin));

	mitk::Vector3D orientationRow1;
	mitk::Vector3D orientationRow2;
	mitk::Vector3D orientationRow3;
	orientationRow1.SetVnlVector(m_FOVRefOrientation.GetVnlMatrix().get_row(0));
	orientationRow2.SetVnlVector(m_FOVRefOrientation.GetVnlMatrix().get_row(1));
	orientationRow3.SetVnlVector(m_FOVRefOrientation.GetVnlMatrix().get_row(2));
	this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVOrientation1, mitk::Vector3DProperty::New(orientationRow1));
	this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVOrientation2, mitk::Vector3DProperty::New(orientationRow2));
	this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVOrientation3, mitk::Vector3DProperty::New(orientationRow3));
}

void RegistrationVisualizer::LoadStateFromNode()
{
	if (this->m_spSelectedRegNode.IsNotNull())
	{
		mitk::RegVisDirectionProperty* directionProp = nullptr;

		if (this->m_spSelectedRegNode->GetProperty(directionProp, mitk::nodeProp_RegVisDirection))
		{
			unsigned int index = directionProp->GetValueAsId();
			//auto maxCount = this->ui->m_comboDirection->maxCount();
			//auto count = this->ui->m_comboDirection->count();
			this->ui->m_comboDirection->setCurrentIndex(index);
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(mitk::nodeProp_RegVisDirection) + QString(" has not the assumed type."));
		}

		bool styleActive = false;

		if (this->m_spSelectedRegNode->GetBoolProperty(mitk::nodeProp_RegVisGrid, styleActive))
		{
			this->ui->m_pbStyleGrid->setChecked(styleActive);
			this->ui->tabGrid->setEnabled(styleActive);
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(mitk::nodeProp_RegVisGrid) + QString(" has not the assumed type."));
		}

		if (this->m_spSelectedRegNode->GetBoolProperty(mitk::nodeProp_RegVisGlyph, styleActive))
		{
			this->ui->m_pbStyleGlyph->setChecked(styleActive);
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(mitk::nodeProp_RegVisGlyph) + QString(" has not the assumed type."));
		}

		if (this->m_spSelectedRegNode->GetBoolProperty(mitk::nodeProp_RegVisPoints, styleActive))
		{
			this->ui->m_pbStylePoints->setChecked(styleActive);
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(mitk::nodeProp_RegVisPoints) + QString(" has not the assumed type."));
		}

		///////////////////////////////////////////////////////
		//visualization
		mitk::RegVisColorStyleProperty* colorStyleProp = nullptr;

		if (this->m_spSelectedRegNode->GetProperty(colorStyleProp, mitk::nodeProp_RegVisColorStyle))
		{
			this->ui->radioColorUni->setChecked(colorStyleProp->GetValueAsId() == 0);
			this->ui->radioColorVecMag->setChecked(colorStyleProp->GetValueAsId() == 1);
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(mitk::nodeProp_RegVisColorStyle) + QString(" has not the assumed type."));
		}

		QColor tmpColor;
		float colorUni[3] = { 0.0, 0.0, 0.0 };
		this->m_spSelectedRegNode->GetColor(colorUni, nullptr, mitk::nodeProp_RegVisColorUni);
		tmpColor.setRgbF(colorUni[0], colorUni[1], colorUni[2]);
		this->ui->btnUniColor->setColor(tmpColor);

		float color1[3] = { 0.0, 0.0, 0.0 };
		this->m_spSelectedRegNode->GetColor(color1, nullptr, mitk::nodeProp_RegVisColor1Value);
		tmpColor.setRgbF(color1[0], color1[1], color1[2]);
		this->ui->btnVecMagColorNeg->setColor(tmpColor);

		float color2[3] = { 0.25, 0.25, 0.25 };
		this->m_spSelectedRegNode->GetColor(color2, nullptr, mitk::nodeProp_RegVisColor2Value);
		tmpColor.setRgbF(color2[0], color2[1], color2[2]);
		this->ui->btnVecMagColorSmall->setColor(tmpColor);

		float color3[3] = { 0.5, 0.5, 0.5 };
		this->m_spSelectedRegNode->GetColor(color3, nullptr, mitk::nodeProp_RegVisColor3Value);
		tmpColor.setRgbF(color3[0], color3[1], color3[2]);
		this->ui->btnVecMagColorMedium->setColor(tmpColor);

		float color4[3] = { 1.0, 1.0, 1.0 };
		this->m_spSelectedRegNode->GetColor(color4, nullptr, mitk::nodeProp_RegVisColor4Value);
		tmpColor.setRgbF(color4[0], color4[1], color4[2]);
		this->ui->btnVecMagColorLarge->setColor(tmpColor);

		double mag2 = 0;
		this->m_spSelectedRegNode->GetPropertyValue(mitk::nodeProp_RegVisColor2Magnitude, mag2);
		double mag3 = 0;
		this->m_spSelectedRegNode->GetPropertyValue(mitk::nodeProp_RegVisColor3Magnitude, mag3);
		double mag4 = 0;
		this->m_spSelectedRegNode->GetPropertyValue(mitk::nodeProp_RegVisColor4Magnitude, mag4);

		bool interpolate = true;
		this->m_spSelectedRegNode->GetBoolProperty(mitk::nodeProp_RegVisColorInterpolate, interpolate);

		this->ui->sbVecMagSmall->setValue(mag2);
		this->ui->sbVecMagMedium->setValue(mag3);
		this->ui->sbVecMagLarge->setValue(mag4);

		this->ui->cbVevMagInterlolate->setChecked(interpolate);

		///////////////////////////////////////////////////////
		//Grid general
		bool showStart = false;

		if (this->m_spSelectedRegNode->GetBoolProperty(mitk::nodeProp_RegVisGridShowStart, showStart))
		{
			this->ui->m_groupShowStartGrid->setChecked(showStart);
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(mitk::nodeProp_RegVisGridShowStart) + QString(" is not correctly defined."));
		}

		int gridFrequ = 5;

		if (this->m_spSelectedRegNode->GetIntProperty(mitk::nodeProp_RegVisGridFrequence, gridFrequ))
		{
			this->ui->m_sbGridFrequency->setValue(gridFrequ);
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(mitk::nodeProp_RegVisGridFrequence) + QString(" is not correctly defined."));
		}

		float colorStart[3] = { 0.0, 0.0, 0.0 };
		this->m_spSelectedRegNode->GetColor(colorStart, nullptr, mitk::nodeProp_RegVisGridStartColor);
		tmpColor.setRgbF(colorStart[0], colorStart[1], colorStart[2]);
		this->ui->btnStartGridColor->setColor(tmpColor);

		///////////////////////////////////////////////////////
		//FOV
		mitk::Vector3DProperty* valueProp = nullptr;

		if (this->m_spSelectedRegNode->GetProperty(valueProp, mitk::nodeProp_RegVisFOVSize))
		{
			this->ui->m_sbFOVSizeX->setValue(valueProp->GetValue()[0]);
			this->ui->m_sbFOVSizeY->setValue(valueProp->GetValue()[1]);
			this->ui->m_sbFOVSizeZ->setValue(valueProp->GetValue()[2]);
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(mitk::nodeProp_RegVisFOVSize) + QString(" is not correctly defined."));
		}

		if (this->m_spSelectedRegNode->GetProperty(valueProp, mitk::nodeProp_RegVisFOVSpacing))
		{
			this->ui->m_sbGridSpX->setValue(valueProp->GetValue()[0]);
			this->ui->m_sbGridSpY->setValue(valueProp->GetValue()[1]);
			this->ui->m_sbGridSpZ->setValue(valueProp->GetValue()[2]);
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(mitk::nodeProp_RegVisFOVSpacing) + QString(" is not correctly defined."));
		}

		mitk::Point3dProperty* originProp = nullptr;

		if (this->m_spSelectedRegNode->GetProperty(originProp, mitk::nodeProp_RegVisFOVOrigin))
		{
			this->ui->m_sbFOVOriginX->setValue(originProp->GetValue()[0]);
			this->ui->m_sbFOVOriginY->setValue(originProp->GetValue()[1]);
			this->ui->m_sbFOVOriginZ->setValue(originProp->GetValue()[2]);
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(mitk::nodeProp_RegVisFOVOrigin) + QString(" is not correctly defined."));
		}

		mitk::Vector3DProperty* orientationProp1;
		mitk::Vector3DProperty* orientationProp2;
		mitk::Vector3DProperty* orientationProp3;

		if (this->m_spSelectedRegNode->GetProperty(orientationProp1, mitk::nodeProp_RegVisFOVOrientation1) &&
			this->m_spSelectedRegNode->GetProperty(orientationProp2, mitk::nodeProp_RegVisFOVOrientation2) &&
			this->m_spSelectedRegNode->GetProperty(orientationProp3, mitk::nodeProp_RegVisFOVOrientation3))
		{
			this->ui->m_sbFOVOriginX->setValue(originProp->GetValue()[0]);
			this->ui->m_sbFOVOriginY->setValue(originProp->GetValue()[1]);
			this->ui->m_sbFOVOriginZ->setValue(originProp->GetValue()[2]);
			m_FOVRefOrientation.GetVnlMatrix().set_row(0, orientationProp1->GetValue().GetVnlVector());
			m_FOVRefOrientation.GetVnlMatrix().set_row(1, orientationProp2->GetValue().GetVnlVector());
			m_FOVRefOrientation.GetVnlMatrix().set_row(2, orientationProp3->GetValue().GetVnlVector());
		}
		else
		{
			m_FOVRefOrientation.SetIdentity();

			this->Error(QString("Cannot configure plugin controlls correctly. One of the node propertiesy ") +
				QString(mitk::nodeProp_RegVisFOVOrientation1) + QString(mitk::nodeProp_RegVisFOVOrientation2) +
				QString(mitk::nodeProp_RegVisFOVOrientation3) + QString(" is not correctly defined."));
		}

		this->UpdateOrientationMatrixWidget();
	}
}

void RegistrationVisualizer::CheckAndSetDefaultFOVRef()
{
	//check if node has a default reference node.
	mitk::DataNode::Pointer defaultRef = this->GetRefNodeOfReg(ui->m_comboDirection->currentIndex() == 1); //direction value 1 = show inverse mapping -> we need the target image used for the registration.

	//if there is a default node and no m_spSelectedFOVRefNode is set -> set default node and transfer values
	if (defaultRef.IsNotNull() && this->m_spSelectedFOVRefNode.IsNull() && !(this->ui->m_pbLockFOVRef->isDown()))
	{
		//there is a default ref and no ref lock -> select default ref and transfer its values
		this->m_spSelectedFOVRefNode = defaultRef;
		this->ui->m_checkUseRefSize->setChecked(true);
		this->ui->m_checkUseRefOrigin->setChecked(true);
		this->ui->m_checkUseRefSpacing->setChecked(true);
		this->ui->m_checkUseRefOrientation->setChecked(true);

		//auto transfere values
		this->OnUseFOVRefBtnPushed();
	}
}

void RegistrationVisualizer::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&)
{
	bool is_busy = m_isBusy;
	m_isBusy = true;
	this->CheckInputs();
	this->LoadStateFromNode();
	this->CheckAndSetDefaultFOVRef();
	this->ConfigureVisualizationControls();
	m_isBusy = is_busy;
	RequestUpdateVisualization();
}

void RegistrationVisualizer::ActualizeRegInfo(mitk::MAPRegistrationWrapper* currentReg)
{
	std::stringstream descriptionString;

	ui->m_teRegInfo->clear();

	if (currentReg)
	{
		descriptionString << "<br/><b>Info:</b><br/>";
		descriptionString << "Moving dimension: " << currentReg->GetMovingDimensions() << "<br/>";
		descriptionString << "Target dimension: " << currentReg->GetTargetDimensions() << "<br/>";
		descriptionString << "Limited moving representation: " << currentReg->HasLimitedMovingRepresentation() << "<br/>";
		descriptionString << "Limited target representation: " << currentReg->HasLimitedTargetRepresentation() << "<br/>";

		mitk::MAPRegistrationWrapper::TagMapType tagMap = currentReg->GetTags();

		descriptionString << "<br/><b>Tags:</b><br/>";
		for (mitk::MAPRegistrationWrapper::TagMapType::const_iterator pos = tagMap.begin(); pos != tagMap.end(); ++pos)
		{
			descriptionString << pos->first << " : " << pos->second << "<br/>";
		}
	}
	else
	{
		descriptionString << "<font color='red'>No registration selected.</font>";
	}

	ui->m_teRegInfo->insertHtml(QString::fromStdString(descriptionString.str()));
}

void RegistrationVisualizer::InitRegNode()
{
	if (this->m_spSelectedRegNode == nullptr)
		return;

	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGrid, mitk::BoolProperty::New(true));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGlyph, mitk::BoolProperty::New(false));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisPoints, mitk::BoolProperty::New(false));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisDirection, mitk::RegVisDirectionProperty::New());
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColorStyle, mitk::RegVisColorStyleProperty::New(1));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColorUni, mitk::ColorProperty::New(0, 0.5, 0));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGridFrequence, mitk::IntProperty::New(3));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGridShowStart, mitk::BoolProperty::New(false));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGridStartColor, mitk::ColorProperty::New(0.5, 0, 0));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVSize, mitk::Vector3DProperty::New(mitk::Vector3D(100.0)));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVSpacing, mitk::Vector3DProperty::New(mitk::Vector3D(5.0)));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor1Value, mitk::ColorProperty::New(0, 0, 0.5));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor2Value, mitk::ColorProperty::New(0, 0.7, 0));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor2Magnitude, mitk::DoubleProperty::New(1));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor3Value, mitk::ColorProperty::New(1, 1, 0));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor3Magnitude, mitk::DoubleProperty::New(5));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor4Value, mitk::ColorProperty::New(1, 0, 0));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor4Magnitude, mitk::DoubleProperty::New(15));
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColorInterpolate, mitk::BoolProperty::New(true));

	mitk::Point3D origin;
	origin.Fill(0.0);
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVOrigin, mitk::Point3dProperty::New(mitk::Point3D(origin)));

	mitk::Vector3D vec(0.0);
	vec.SetElement(0, 1.0);
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVOrientation1, mitk::Vector3DProperty::New(vec));
	vec.Fill(0.0);
	vec.SetElement(1, 1.0);
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVOrientation2, mitk::Vector3DProperty::New(vec));
	vec.Fill(0.0);
	vec.SetElement(2, 1.0);
	this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVOrientation3, mitk::Vector3DProperty::New(vec));
}

mitk::ScalarType RegistrationVisualizer::GetSaveSpacing(mitk::ScalarType gridRes, mitk::ScalarType spacing, unsigned int maxGridRes) const
{
	mitk::ScalarType newSpacing = spacing;
	mitk::ScalarType scaling = gridRes / maxGridRes;

	if (scaling > 1.0)
	{
		newSpacing = spacing * scaling;
	}

	return newSpacing;
}

void RegistrationVisualizer::UpdateOrientationMatrixWidget()
{
	for (unsigned int r = 0; r < 3; ++r)
	{
		for (unsigned int c = 0; c < 3; ++c)
		{
			this->ui->m_tableOrientation->item(r, c)->setText(QString::number(this->m_FOVRefOrientation.GetVnlMatrix().get(r, c)));
		}
	}
}

void RegistrationVisualizer::RequestUpdateVisualization()
{
	m_isUpdateNeeded = true;

	//QTimer::singleShot(200, this, SLOT(updateCaption()));
	if (m_isBusy || m_timerForUpdate->remainingTime() >= 0)
		return;

	//m_timerForUpdate->start(1000);
	UpdateVisualization();
}
void RegistrationVisualizer::UpdateVisualization()
{
	if (!m_isUpdateNeeded)
		return;

	m_isUpdateNeeded = false;

	if (this->m_spSelectedRegNode == nullptr)
		return;

	this->StoreStateInNode();

	mitk::Geometry3D::Pointer gridDesc;
	unsigned int gridFrequ = 5;

	mitk::GetGridGeometryFromNode(this->m_spSelectedRegNode, gridDesc, gridFrequ);

	this->GetCurrentRegistration()->SetGeometry(gridDesc);
	assert(m_timerForUpdate->isSingleShot());
	m_timerForUpdate->start(500);
	mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void RegistrationVisualizer::OnDirectionChanged(int)
{
	bool is_busy = m_isBusy;
	m_isBusy = true;
	this->CheckAndSetDefaultFOVRef();
	this->ConfigureVisualizationControls();
	m_isBusy = is_busy;
	RequestUpdateVisualization();
};
void RegistrationVisualizer::OnLockRegButtonPushed()
{
	if (ui->m_pbLockReg->isChecked())
	{
		if (this->m_spSelectedRegNode.IsNotNull())
		{
			this->m_spSelectedRegNode->SetSelected(false);
			this->GetDataStorage()->Modified();
		}
	}

	bool is_busy = m_isBusy;
	m_isBusy = true;
	this->CheckInputs();
	this->ConfigureVisualizationControls();
	m_isBusy = is_busy;
	RequestUpdateVisualization();
}
void RegistrationVisualizer::OnUpdateBtnPushed()
{
	RequestUpdateVisualization();
}
void RegistrationVisualizer::OnStyleButtonPushed()
{

	RequestUpdateVisualization();
}
void RegistrationVisualizer::OnColorInterpolationChecked(bool checked)
{
	if (checked)
	{
		this->ui->labelVecMagSmall->setText(QString("="));
		this->ui->labelVecMagMedium->setText(QString("="));
		this->ui->labelVecMagLarge->setText(QString("="));
	}
	else
	{
		this->ui->labelVecMagSmall->setText(QString(">"));
		this->ui->labelVecMagMedium->setText(QString(">"));
		this->ui->labelVecMagLarge->setText(QString(">"));
	}
	RequestUpdateVisualization();
};
void RegistrationVisualizer::OnUseFOVRefBtnPushed()
{
	if (this->m_spSelectedFOVRefNode.IsNotNull())
	{
		assert(this->m_spSelectedFOVRefNode->GetData());
		assert(this->m_spSelectedFOVRefNode->GetData()->GetGeometry());

		mitk::BaseGeometry* gridRef = this->m_spSelectedFOVRefNode->GetData()->GetGeometry();

		mitk::Vector3D spacing = gridRef->GetSpacing();
		mitk::Point3D origin = gridRef->GetOrigin();
		mitk::Geometry3D::BoundsArrayType bounds = gridRef->GetBounds();
		mitk::AffineTransform3D::ConstPointer fovTransform = gridRef->GetIndexToWorldTransform();


		if (this->ui->m_checkUseRefSize->isChecked())
		{
			this->ui->m_sbFOVSizeX->setValue((bounds[1] - bounds[0])*spacing[0]);
			this->ui->m_sbFOVSizeY->setValue((bounds[3] - bounds[2])*spacing[1]);
			this->ui->m_sbFOVSizeZ->setValue((bounds[5] - bounds[4])*spacing[2]);
		}

		if (this->ui->m_checkUseRefSpacing->isChecked())
		{

			this->ui->m_sbGridSpX->setValue(GetSaveSpacing((bounds[1] - bounds[0]), spacing[0], 20));
			this->ui->m_sbGridSpY->setValue(GetSaveSpacing((bounds[3] - bounds[2]), spacing[1], 20));
			this->ui->m_sbGridSpZ->setValue(GetSaveSpacing((bounds[5] - bounds[4]), spacing[2], 20));

		}

		if (this->ui->m_checkUseRefOrigin->isChecked())
		{
			this->ui->m_sbFOVOriginX->setValue(origin[0]);
			this->ui->m_sbFOVOriginY->setValue(origin[1]);
			this->ui->m_sbFOVOriginZ->setValue(origin[2]);
		}

		if (this->ui->m_checkUseRefOrientation->isChecked())
		{
			this->m_FOVRefOrientation = fovTransform->GetMatrix();
			bool is_busy = m_isBusy;
			m_isBusy = true;
			this->UpdateOrientationMatrixWidget();
			m_isBusy = is_busy;
		}

		RequestUpdateVisualization();
	}
}
