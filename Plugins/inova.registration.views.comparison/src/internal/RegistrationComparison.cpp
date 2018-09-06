#include "RegistrationComparison.h"
#include <PopeElements.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Mitk
#include <mitkStatusBar.h>
#include <mitkNodePredicateDataProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkMAPRegistrationWrapper.h>
#include "mitkRegVisPropertyTags.h"
#include "mitkMatchPointPropertyTags.h"
#include "mitkRegEvaluationObject.h"
#include "mitkRegistrationHelper.h"
#include "mitkRegEvaluationMapper2D.h"
#include <mitkAlgorithmHelper.h>

// Qmitk
#include "QmitkRenderWindow.h"

// Qt
#include <QMessageBox>
#include <QErrorMessage>
#include <QTimer>


const std::string RegistrationComparison::VIEW_ID = "inova.registration.views.comparison";

const std::string RegistrationComparison::HelperNodeName = "RegistrationEvaluationHelper";

RegistrationComparison::RegistrationComparison()
	: m_Parent(nullptr)
	, m_activeEvaluation(false)
	, m_autoMoving(false)
	, m_autoTarget(false)
	, m_currentSelectedTimeStep(0)
{
	m_currentSelectedPosition.Fill(0.0);
}

RegistrationComparison::~RegistrationComparison()
{
	//if (m_activeEvaluation)
	//	StopEvaluation();
	if (this->m_selectedEvalNode.IsNotNull() && this->GetDataStorage().IsNotNull())
	{
		this->GetDataStorage()->Remove(this->m_selectedEvalNode);
	}
}

void RegistrationComparison::SetFocus()
{

}

void RegistrationComparison::Error(QString msg)
{
	mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
	MITK_ERROR << msg.toStdString().c_str();
}

void RegistrationComparison::CreateQtPartControl(QWidget* parent)
{
	// create GUI widgets from the Qt Designer's .ui file
	ui.setupUi(parent);

	m_Parent = parent;

	connect(ui.evalSettings, SIGNAL(SettingsChanged(mitk::DataNode*)), this, SLOT(OnSettingsChanged(mitk::DataNode*)));

	this->m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
	connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));

	m_selectedEvalNode = this->GetDataStorage()->GetNamedNode(HelperNodeName);

	this->CheckInputs();
	this->ConfigureControls();
	this->StartEvaluation();
}

void RegistrationComparison::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
	this->m_SliceChangeListener.RenderWindowPartActivated(renderWindowPart);
}

void RegistrationComparison::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
	this->m_SliceChangeListener.RenderWindowPartDeactivated(renderWindowPart);
}

bool RegistrationComparison::CheckInputs()
{
	bool autoMoving = false;
	bool autoTarget = false;
	mitk::DataNode::Pointer spSelectedRegNode = nullptr;
	mitk::DataNode::Pointer spSelectedMovingNode = nullptr;
	mitk::DataNode::Pointer spSelectedTargetNode = nullptr;

	QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();
	if (dataNodes.size() > 0)
	{
		/// Check if a registration node is selected.
		for (auto datanode : dataNodes)
		{
			// Test if auto select works
			if (!mitk::MITKRegistrationHelper::IsRegNode(datanode) || !datanode->GetData())
				continue;

			mitk::BaseProperty* uidProp = datanode->GetData()->GetProperty(mitk::Prop_RegAlgMovingData);
			if (!uidProp)
				continue;
			// Search for the moving node
			mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID, uidProp);
			mitk::DataNode::Pointer selectedMovingNode = this->GetDataStorage()->GetNode(predicate);
			if (!selectedMovingNode)
				continue;

			uidProp = datanode->GetData()->GetProperty(mitk::Prop_RegAlgTargetData);
			if (!uidProp)
				continue;
			// Search for the target node
			predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID, uidProp);
			mitk::DataNode::Pointer selectedTargetNode = this->GetDataStorage()->GetNode(predicate);
			if (!selectedTargetNode)
				continue;

			spSelectedRegNode = datanode;
			spSelectedMovingNode = selectedMovingNode;
			spSelectedTargetNode = selectedTargetNode;
			autoMoving = spSelectedMovingNode.IsNotNull();
			autoTarget = spSelectedTargetNode.IsNotNull();
		}

		/// If not, set tagret and moving images.
		if (spSelectedRegNode == nullptr)
		{
			for (auto datanode : dataNodes)
			{
				mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
				if (!image)
					continue;
				// Reverse direction
				if (spSelectedMovingNode == nullptr)
				{
					spSelectedMovingNode = datanode;
				}
				else
				{
					spSelectedTargetNode = datanode;
					break;
				}
			}
		}
	}

	bool is_changed = (m_autoMoving != autoMoving || m_autoTarget != autoTarget || m_spSelectedMovingNode != spSelectedMovingNode
		|| m_spSelectedTargetNode != spSelectedTargetNode || m_spSelectedRegNode != spSelectedRegNode);
	/// Reset the current selection if not active.
	if (is_changed && !m_activeEvaluation)
	{
		this->m_autoMoving = autoMoving;
		this->m_autoTarget = autoTarget;
		this->m_spSelectedMovingNode = spSelectedMovingNode;
		this->m_spSelectedTargetNode = spSelectedTargetNode;
		this->m_spSelectedRegNode = spSelectedRegNode;
	}
	return is_changed;
}

void RegistrationComparison::StartEvaluation()
{
	if (this->m_spSelectedMovingNode == nullptr || this->m_spSelectedTargetNode == nullptr || m_activeEvaluation)
		return;

	//reinit view
	mitk::RenderingManager::GetInstance()->InitializeViews(m_spSelectedTargetNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

	mitk::RegEvaluationObject::Pointer regEval = mitk::RegEvaluationObject::New();

	mitk::MAPRegistrationWrapper::Pointer reg;

	if (m_spSelectedRegNode.IsNotNull())
	{
		reg = dynamic_cast<mitk::MAPRegistrationWrapper*>(this->m_spSelectedRegNode->GetData());
	}
	else
	{
		//generate a dymme reg to use
		reg = mitk::GenerateIdentityRegistration3D();
	}

	regEval->SetRegistration(reg);
	regEval->SetTargetNode(this->m_spSelectedTargetNode);
	regEval->SetMovingNode(this->m_spSelectedMovingNode);

	if (this->m_selectedEvalNode.IsNotNull() && this->GetDataStorage().IsNotNull())
	{
		this->GetDataStorage()->Remove(this->m_selectedEvalNode);
	}

	this->m_selectedEvalNode = mitk::DataNode::New();
	this->m_selectedEvalNode->SetData(regEval);

	mitk::RegEvaluationMapper2D::SetDefaultProperties(this->m_selectedEvalNode);
	this->m_selectedEvalNode->SetName(HelperNodeName);
	this->m_selectedEvalNode->SetBoolProperty("helper object", true);
	this->GetDataStorage()->Add(this->m_selectedEvalNode);

	this->ui.evalSettings->SetNode(this->m_selectedEvalNode);
	this->OnSliceChanged();

	this->GetRenderWindowPart()->RequestUpdate();

	this->m_activeEvaluation = true;
	this->CheckInputs();
	this->ConfigureControls();
}
void RegistrationComparison::StopEvaluation()
{
	this->m_activeEvaluation = false;

	if (this->m_selectedEvalNode.IsNotNull())
	{
		if (this->GetDataStorage().IsNotNull())
			this->GetDataStorage()->Remove(this->m_selectedEvalNode);
		this->m_selectedEvalNode = nullptr;
		//this->m_selectedEvalNode = mitk::DataNode::New();
	}

	//this->ui.evalSettings->SetNode(this->m_selectedEvalNode);
}


void RegistrationComparison::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&)
{
	bool is_changed = CheckInputs();
	if (is_changed && m_activeEvaluation)
	{
		StopEvaluation();
		CheckInputs();
	}
	this->ConfigureControls();
	this->StartEvaluation();
};


void RegistrationComparison::NodeRemoved(const mitk::DataNode* node)
{
	if (node == this->m_spSelectedMovingNode
		|| node == this->m_spSelectedRegNode
		|| node == this->m_spSelectedTargetNode
		|| node == this->m_selectedEvalNode)
	{
		bool is_evalNode_removed = (node == this->m_selectedEvalNode);		
		StopEvaluation();
		if (!is_evalNode_removed)
		{
			CheckInputs();
			ConfigureControls();
			this->GetRenderWindowPart()->RequestUpdate();
		}

		MITK_INFO << "Stopped current registration comparison session because at least one relevant node was removed from storage.";
	}
}

void RegistrationComparison::ConfigureControls()
{
	/// Configure input data widgets.
	bool is_reg = (m_spSelectedRegNode != nullptr);
	//ui.label_Registration->setVisible(is_prereg);
	if (is_reg)
	{
		std::string name = m_spSelectedRegNode->GetName();
		QString short_name = Elements::get_short_name_for_image(name);
		ui.label_Registration->setText("<b>Registration:</b> " + short_name);
		ui.label_Registration->setToolTip(QString::fromStdString(name));
		ui.label_Registration->setStyleSheet("");
		ui.label_Registration->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		if (m_spSelectedMovingNode != nullptr && m_spSelectedTargetNode != nullptr)
		{
			ui.label_Registration->setText(QString("<font color='gray'>No registration selected. Direct comparison.</font>"));
			ui.label_Registration->setStyleSheet("");
		}
		else
		{
			ui.label_Registration->setText(QString("<font color=#E02000><b>Registration:</b> Please select in Data Manager.</font>"));
			ui.label_Registration->setStyleSheet("background-color: #efef95;");
		}
		ui.label_Registration->setToolTip("");
		ui.label_Registration->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}

	bool is_moving_image = (m_spSelectedMovingNode != nullptr);
	if (is_moving_image)
	{
		std::string name = m_spSelectedMovingNode->GetName();
		QString short_name = Elements::get_short_name_for_image(name);
		if (m_autoMoving)
			ui.label_MovingImage->setText(QString("<b>Moving image:</b> <font color='gray'>") + short_name + QString("</font>")); // + QString(" (auto selected)</font>"));
		else
			ui.label_MovingImage->setText("<b>Moving image:</b> " + short_name);
		ui.label_MovingImage->setToolTip(QString::fromStdString(name));
		ui.label_MovingImage->setStyleSheet("");
		ui.label_MovingImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		ui.label_MovingImage->setText("<font color=#E02000><b>Moving image:</b> Please select in Data Manager.</font>");
		ui.label_MovingImage->setToolTip("");
		ui.label_MovingImage->setStyleSheet("background-color: #efef95;");
		ui.label_MovingImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}

	bool is_target_image = (m_spSelectedTargetNode != nullptr);
	if (is_target_image)
	{
		std::string name = m_spSelectedTargetNode->GetName();
		QString short_name = Elements::get_short_name_for_image(name);
		if (m_autoTarget)
			ui.label_TargetImage->setText(QString("<b>Target image:</b> <font color='gray'>") + short_name + QString("</font>"));// + QString(" (auto selected)</font>"));
		else
			ui.label_TargetImage->setText("<b>Target image:</b> " + short_name);
		ui.label_TargetImage->setToolTip(QString::fromStdString(name));
		ui.label_TargetImage->setStyleSheet("");
		ui.label_TargetImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		ui.label_TargetImage->setText("<font color=#E02000><b>Target image:</b> Please select in Data Manager.</font>");
		ui.label_TargetImage->setToolTip("");
		ui.label_TargetImage->setStyleSheet("background-color: #efef95;");
		ui.label_TargetImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}

	/// Config settings widget.
	this->ui.evalSettings->setVisible(m_activeEvaluation);
	this->ui.label_Registration->setEnabled(!m_activeEvaluation);
	this->ui.label_MovingImage->setEnabled(!m_activeEvaluation);
	this->ui.label_TargetImage->setEnabled(!m_activeEvaluation);
}

void RegistrationComparison::OnSliceChanged()
{
	mitk::Point3D currentSelectedPosition = GetRenderWindowPart()->GetSelectedPosition(nullptr);
	unsigned int currentSelectedTimeStep = GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();

	if (m_currentSelectedPosition != currentSelectedPosition
		|| m_currentSelectedTimeStep != currentSelectedTimeStep
		|| m_selectedNodeTime > m_currentPositionTime)
	{
		//the current position has been changed or the selected node has been changed since the last position validation -> check position
		m_currentSelectedPosition = currentSelectedPosition;
		m_currentSelectedTimeStep = currentSelectedTimeStep;
		m_currentPositionTime.Modified();

		if (this->m_selectedEvalNode.IsNotNull())
		{
			this->m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalCurrentPosition, mitk::GenericProperty<mitk::Point3D>::New(currentSelectedPosition));
		}
	}
}

void RegistrationComparison::OnSettingsChanged(mitk::DataNode*)
{
	this->GetRenderWindowPart()->RequestUpdate();
};
