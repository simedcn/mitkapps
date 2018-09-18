#include "ManualRegistration.h"
#include "inova_registration_views_manualregistration_Activator.h"
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
#include <mitkResultNodeGenerationHelper.h>
#include <mitkUIDHelper.h>

// CTK
#include <ctkPluginActivator.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEventConstants.h>

// Qmitk
#include "QmitkRenderWindow.h"
#include <QmitkMappingJob.h>

// Qt
#include <QMessageBox>
#include <QErrorMessage>
#include <QTimer>
#include <QThreadPool>

//MatchPoint
#include <mapRegistrationManipulator.h>
#include <mapPreCachedRegistrationKernel.h>
#include <mapCombinedRegistrationKernel.h>
#include <mapNullRegistrationKernel.h>
#include <mapRegistrationCombinator.h>

#include <itkCompositeTransform.h>

#include <boost/math/constants/constants.hpp>

#include <sstream>
#include <climits>

using string = std::string;


const string ManualRegistration::VIEW_ID = "inova.registration.views.manualregistration";
const QString ManualRegistration::PLUGIN_ID = QString::fromStdString(VIEW_ID);

const string ManualRegistration::HelperNodeName = "RegistrationManipulationEvaluationHelper";


ManualRegistration::ManualRegistration()
	: m_Parent(nullptr)
	, m_activeManipulation(false)
	, m_currentSelectedTimeStep(0)
	, m_internalUpdate(false)
{
	m_currentSelectedPosition.Fill(0.0);
}

ManualRegistration::~ManualRegistration()
{
	if (m_activeManipulation)
		StopSession();
	if (this->m_EvalNode.IsNotNull() && this->GetDataStorage().IsNotNull())
	{
		this->GetDataStorage()->Remove(this->m_EvalNode);
	}
}

void ManualRegistration::SetFocus()
{

}

void ManualRegistration::Error(QString msg)
{
	mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
	MITK_ERROR << msg.toStdString().c_str();
}

void ManualRegistration::CreateQtPartControl(QWidget* parent)
{
	// create GUI widgets from the Qt Designer's .ui file
	ui.setupUi(parent);

	m_Parent = parent;

	//connect(ui.pbStart, SIGNAL(clicked()), this, SLOT(OnStartBtnPushed()));
	//connect(ui.pbCancel, SIGNAL(clicked()), this, SLOT(OnCancelBtnPushed()));
	connect(ui.pbStore, SIGNAL(clicked()), this, SLOT(OnStoreBtnPushed()));
	connect(ui.evalSettings, SIGNAL(SettingsChanged(mitk::DataNode*)), this, SLOT(OnSettingsChanged(mitk::DataNode*)));

	connect(ui.slideRotX, SIGNAL(valueChanged(int)), this, SLOT(OnRotXSlideChanged(int)));
	connect(ui.sbRotX, SIGNAL(valueChanged(double)), this, SLOT(OnRotXChanged(double)));
	connect(ui.slideRotY, SIGNAL(valueChanged(int)), this, SLOT(OnRotYSlideChanged(int)));
	connect(ui.sbRotY, SIGNAL(valueChanged(double)), this, SLOT(OnRotYChanged(double)));
	connect(ui.slideRotZ, SIGNAL(valueChanged(int)), this, SLOT(OnRotZSlideChanged(int)));
	connect(ui.sbRotZ, SIGNAL(valueChanged(double)), this, SLOT(OnRotZChanged(double)));

	connect(ui.slideTransX, SIGNAL(valueChanged(int)), this, SLOT(OnTransXSlideChanged(int)));
	connect(ui.sbTransX, SIGNAL(valueChanged(double)), this, SLOT(OnTransXChanged(double)));
	connect(ui.slideTransY, SIGNAL(valueChanged(int)), this, SLOT(OnTransYSlideChanged(int)));
	connect(ui.sbTransY, SIGNAL(valueChanged(double)), this, SLOT(OnTransYChanged(double)));
	connect(ui.slideTransZ, SIGNAL(valueChanged(int)), this, SLOT(OnTransZSlideChanged(int)));
	connect(ui.sbTransZ, SIGNAL(valueChanged(double)), this, SLOT(OnTransZChanged(double)));

	connect(ui.comboCenter, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCenterTypeChanged(int)));

	/// CTK slots.
	//auto pluginContext = inova_registration_views_manualregistration_Activator::GetContext();
	//ctkDictionary propsForSlot;
	//ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	//if (ref)
	//{
	//	ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
	//	propsForSlot[ctkEventConstants::EVENT_TOPIC] = "plugin/HIDDEN";
	//	eventAdmin->subscribeSlot(this, SLOT(on_Plugin_hidden(ctkEvent)), propsForSlot);
	//}

	this->m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
	connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));

	ui.groupScale->setVisible(false);
	ui.label_PreRegistration->setVisible(false);
	ui.checkMapEntity->setVisible(false);
	ui.checkMapEntity->setChecked(true);

	m_EvalNode = this->GetDataStorage()->GetNamedNode(HelperNodeName);

	this->CheckInputs();
	this->StopSession();
	this->ConfigureControls();
	this->StartRegistration();
}

void ManualRegistration::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
	this->m_SliceChangeListener.RenderWindowPartActivated(renderWindowPart);
}

void ManualRegistration::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
	this->m_SliceChangeListener.RenderWindowPartDeactivated(renderWindowPart);
}

bool ManualRegistration::CheckInputs()
{
	// Current selection
	MAPRegistrationType::ConstPointer SelectedPreReg = nullptr;
	mitk::DataNode::Pointer SelectedPreRegNode = nullptr;
	mitk::DataNode::Pointer SelectedMovingNode = nullptr;
	mitk::DataNode::Pointer SelectedTargetNode = nullptr;

	QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();
	if (dataNodes.size() != 0)
	{
		/// Check if a registration node is selected.
		for (auto datanode : dataNodes)
		{
			if (!mitk::MITKRegistrationHelper::IsRegNode(datanode))
				continue;

			mitk::MAPRegistrationWrapper* regWrapper = dynamic_cast<mitk::MAPRegistrationWrapper*>(datanode->GetData());
			const MAPRegistrationType* selected_reg = nullptr;
			if (regWrapper)
				selected_reg = dynamic_cast<const MAPRegistrationType*>(regWrapper->GetRegistration());
			else
				continue;

			if (selected_reg == nullptr)
				continue;

			mitk::BaseProperty* uidProp = datanode->GetData()->GetProperty(mitk::Prop_RegAlgMovingData);
			if (uidProp == nullptr)
				continue;
			// Prepare the moving node
			mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID, uidProp);
			auto selectedMovingNode = this->GetDataStorage()->GetNode(predicate);

			uidProp = datanode->GetData()->GetProperty(mitk::Prop_RegAlgTargetData);
			if (uidProp == nullptr)
				continue;

			// Set the target node
			predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID, uidProp);
			SelectedTargetNode = this->GetDataStorage()->GetNode(predicate);

			// Set the selected items if everithing has been found
			SelectedMovingNode = selectedMovingNode;
			SelectedPreReg = selected_reg;
			SelectedPreRegNode = datanode;
			break;
		}

		/// If not, set tagret and moving images.
		if (SelectedPreRegNode == nullptr)
		{
			for (auto datanode : dataNodes)
			{
				mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
				if (!image)
					continue;
				// Reverse direction
				if (SelectedMovingNode == nullptr)
				{
					SelectedMovingNode = datanode;
				}
				else
				{
					SelectedTargetNode = datanode;
					break;
				}
			}
		}
	}

	bool is_changed = (m_SelectedPreReg != SelectedPreReg || m_SelectedMovingNode != SelectedMovingNode || m_SelectedTargetNode != SelectedTargetNode || m_SelectedPreRegNode != SelectedPreRegNode);
	/// Reset the current selection if not active.
	if (is_changed && !m_activeManipulation)
	{
		this->m_SelectedPreReg = SelectedPreReg;
		this->m_SelectedMovingNode = SelectedMovingNode;
		this->m_SelectedTargetNode = SelectedTargetNode;
		this->m_SelectedPreRegNode = SelectedPreRegNode;
	}
	return is_changed;
}

void ManualRegistration::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&)
{
	bool is_changed = CheckInputs();
	if (is_changed && m_activeManipulation)
	{
		StopSession();
		CheckInputs();
	}
	ConfigureControls();
	StartRegistration();
};

void ManualRegistration::NodeRemoved(const mitk::DataNode* node)
{
	if (node == this->m_SelectedMovingNode
		|| node == this->m_SelectedTargetNode
		|| node == this->m_EvalNode)
	{
		if (node == this->m_EvalNode)
		{
			this->m_EvalNode = nullptr;
			StopSession();
		}
		else
		{
			//this->OnCancelBtnPushed();
			this->StopSession();
			this->CheckInputs();
			this->ConfigureControls();
			this->GetRenderWindowPart()->RequestUpdate();
		}
		MITK_INFO << "Stopped current MatchPoint manual registration session because at least one relevant node was removed from storage.";
	}
}

void ManualRegistration::ConfigureControls()
{
	//configure input data widgets
	bool is_prereg = (m_SelectedPreRegNode != nullptr);
	ui.label_PreRegistration->setVisible(is_prereg);
	if (is_prereg)
	{
		string name = m_SelectedPreRegNode->GetName();
		QString short_name = Elements::get_short_name_for_image(name);
		ui.label_PreRegistration->setText("<b>Pre-Registration:</b> " + short_name);
		ui.label_PreRegistration->setToolTip(QString::fromStdString(name));
	}
	else
	{
		ui.label_PreRegistration->setText("");
		ui.label_PreRegistration->setToolTip("");
	}

	bool is_moving_image = (m_SelectedMovingNode != nullptr);
	if (is_moving_image)
	{
		string name = m_SelectedMovingNode->GetName();
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

	bool is_target_image = (m_SelectedTargetNode != nullptr);
	if (is_target_image)
	{
		string name = m_SelectedTargetNode->GetName();
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

	//if (!m_activeManipulation)
	//{
		//QString name = "ManualRegistration";
		//if (m_SelectedPreRegNode.IsNotNull())
		//	name = QString::fromStdString(m_SelectedPreRegNode->GetName()) + " Refined";
		//this->ui.lbNewRegName->setText(name);
		auto dataStorage = GetDataStorage();
		string name;
		if (m_SelectedPreRegNode != nullptr)
		{
			string reg_name = m_SelectedPreRegNode->GetName();
			if (reg_name.length() >= 4 && reg_name.substr(0, 4) == "Reg_")
				reg_name = reg_name.substr(4);
			name = Elements::get_next_name(reg_name, dataStorage);
		}
		else
		{
			name = is_moving_image ? 
				Elements::get_next_name(m_SelectedMovingNode->GetName(), dataStorage) :
				"ManualRegistration";
		}
		this->ui.lbNewRegName->setText(QString::fromStdString(name));
	//}

	//config settings widget
	//this->ui.pbStart->setEnabled(m_SelectedMovingNode.IsNotNull() && m_SelectedTargetNode.IsNotNull() && !m_activeManipulation);
	this->ui.lbNewRegName->setEnabled(m_activeManipulation);
	this->ui.label_NewRegName->setEnabled(m_activeManipulation);
	this->ui.label_Reg->setEnabled(m_activeManipulation);
	this->ui.checkMapEntity->setEnabled(m_activeManipulation);
	this->ui.tabWidget->setEnabled(m_activeManipulation);
	//this->ui.pbCancel->setEnabled(m_activeManipulation);
	this->ui.pbStore->setEnabled(m_activeManipulation);

	this->UpdateTransformWidgets();
}

void ManualRegistration::StartRegistration()
{
	if (m_SelectedMovingNode == nullptr || m_SelectedTargetNode == nullptr || m_activeManipulation)
		return;

	this->InitSession();
	this->OnSliceChanged();

	this->GetRenderWindowPart()->RequestUpdate();

	this->CheckInputs();
	this->ConfigureControls();
}

void ManualRegistration::InitSession()
{
	this->m_InverseCurrentTransform = TransformType::New();
	this->m_InverseCurrentTransform->SetIdentity();
	this->m_DirectCurrentTransform = TransformType::New();
	this->m_DirectCurrentTransform->SetIdentity();

	this->m_CurrentRegistrationWrapper = mitk::MAPRegistrationWrapper::New();
	m_CurrentRegistration = MAPRegistrationType::New();

	this->m_CurrentRegistrationWrapper->SetRegistration(m_CurrentRegistration);

	::map::core::RegistrationManipulator<MAPRegistrationType> manipulator(m_CurrentRegistration);
	::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
	manipulator.setDirectMapping(::map::core::NullRegistrationKernel<3, 3>::New());

	if (m_SelectedPreRegNode == nullptr)
	{ //new registration
		kernel->setTransformModel(m_InverseCurrentTransform);
		manipulator.setInverseMapping(kernel);

		//init to map the image centers
		auto movingCenter = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
		auto targetCenter = m_SelectedTargetNode->GetData()->GetTimeGeometry()->GetCenterInWorld();

		auto offset = targetCenter - movingCenter;
		m_DirectCurrentTransform->SetOffset(offset);
		m_DirectCurrentTransform->GetInverse(m_InverseCurrentTransform);
	}
	else
	{ //use selected pre registration as baseline
		itk::CompositeTransform <::map::core::continuous::ScalarType, 3>::Pointer compTransform = itk::CompositeTransform <::map::core::continuous::ScalarType, 3>::New();
		const ::map::core::RegistrationKernel<3, 3>* preKernel = dynamic_cast<const ::map::core::RegistrationKernel<3, 3>*>(&this->m_SelectedPreReg->getInverseMapping());
		compTransform->AddTransform(preKernel->getTransformModel()->Clone());
		compTransform->AddTransform(this->m_InverseCurrentTransform);

		kernel->setTransformModel(compTransform);
		manipulator.setInverseMapping(kernel);
	}

	ui.comboCenter->setCurrentIndex(0);
	this->ConfigureTransformCenter(0);

	//set bounds of the translation slider widget to have sensible ranges
	auto currenttrans = m_DirectCurrentTransform->GetTranslation();
	this->ui.slideTransX->setMinimum(currenttrans[0] - 250);
	this->ui.slideTransY->setMinimum(currenttrans[1] - 250);
	this->ui.slideTransZ->setMinimum(currenttrans[2] - 250);
	this->ui.slideTransX->setMaximum(currenttrans[0] + 250);
	this->ui.slideTransY->setMaximum(currenttrans[1] + 250);
	this->ui.slideTransZ->setMaximum(currenttrans[2] + 250);

	//reinit view
	mitk::RenderingManager::GetInstance()->InitializeViews(m_SelectedTargetNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

	//generate evaluation node
	mitk::RegEvaluationObject::Pointer regEval = mitk::RegEvaluationObject::New();
	regEval->SetRegistration(this->m_CurrentRegistrationWrapper);
	regEval->SetTargetNode(this->m_SelectedTargetNode);
	regEval->SetMovingNode(this->m_SelectedMovingNode);

	this->m_EvalNode = mitk::DataNode::New();
	this->m_EvalNode->SetData(regEval);

	mitk::RegEvaluationMapper2D::SetDefaultProperties(this->m_EvalNode);
	this->m_EvalNode->SetName(HelperNodeName);
	this->m_EvalNode->SetBoolProperty("helper object", true);
	this->GetDataStorage()->Add(this->m_EvalNode);

	this->ui.evalSettings->SetNode(this->m_EvalNode);

	this->m_activeManipulation = true;
};

void ManualRegistration::StopSession()
{
	this->m_InverseCurrentTransform = TransformType::New();
	this->m_InverseCurrentTransform->SetIdentity();
	this->m_DirectCurrentTransform = TransformType::New();
	this->m_DirectCurrentTransform->SetIdentity();
	this->m_CurrentRegistration = nullptr;
	this->m_CurrentRegistrationWrapper = nullptr;

	if (this->m_EvalNode.IsNotNull())
	{
		this->GetDataStorage()->Remove(this->m_EvalNode);
	}

	this->m_EvalNode = nullptr;

	this->m_activeManipulation = false;
};

void ManualRegistration::UpdateTransformWidgets()
{
	this->m_internalUpdate = true;
	this->ui.sbTransX->setValue(this->m_DirectCurrentTransform->GetTranslation()[0]);
	this->ui.sbTransY->setValue(this->m_DirectCurrentTransform->GetTranslation()[1]);
	this->ui.sbTransZ->setValue(this->m_DirectCurrentTransform->GetTranslation()[2]);
	this->ui.slideTransX->setValue(this->m_DirectCurrentTransform->GetTranslation()[0]);
	this->ui.slideTransY->setValue(this->m_DirectCurrentTransform->GetTranslation()[1]);
	this->ui.slideTransZ->setValue(this->m_DirectCurrentTransform->GetTranslation()[2]);

	this->ui.sbRotX->setValue(this->m_DirectCurrentTransform->GetAngleX()*(180 / boost::math::double_constants::pi));
	this->ui.sbRotY->setValue(this->m_DirectCurrentTransform->GetAngleY()*(180 / boost::math::double_constants::pi));
	this->ui.sbRotZ->setValue(this->m_DirectCurrentTransform->GetAngleZ()*(180 / boost::math::double_constants::pi));
	this->ui.slideRotX->setValue(this->m_DirectCurrentTransform->GetAngleX()*(180 / boost::math::double_constants::pi));
	this->ui.slideRotY->setValue(this->m_DirectCurrentTransform->GetAngleY()*(180 / boost::math::double_constants::pi));
	this->ui.slideRotZ->setValue(this->m_DirectCurrentTransform->GetAngleZ()*(180 / boost::math::double_constants::pi));
	this->m_internalUpdate = false;
};

void ManualRegistration::UpdateTransform(bool updateRotation)
{
	if (updateRotation)
	{
		if (ui.comboCenter->currentIndex() == 2)
		{
		 ConfigureTransformCenter(2);
		}
		this->m_DirectCurrentTransform->SetRotation(
			this->ui.sbRotX->value()*(boost::math::double_constants::pi / 180),
			this->ui.sbRotY->value()*(boost::math::double_constants::pi / 180),
			this->ui.sbRotZ->value()*(boost::math::double_constants::pi / 180)
		);
	}
	else
	{
		TransformType::OutputVectorType trans;
		trans[0] = this->ui.sbTransX->value();
		trans[1] = this->ui.sbTransY->value();
		trans[2] = this->ui.sbTransZ->value();

		this->m_DirectCurrentTransform->SetTranslation(trans);
	}

	this->m_DirectCurrentTransform->GetInverse(this->m_InverseCurrentTransform);

	this->UpdateTransformWidgets();

	this->m_EvalNode->Modified();
	this->m_CurrentRegistrationWrapper->Modified();
	this->GetRenderWindowPart()->RequestUpdate();
};

void ManualRegistration::OnSliceChanged()
{
	mitk::Point3D currentSelectedPosition = GetRenderWindowPart()->GetSelectedPosition(nullptr);
	unsigned int currentSelectedTimeStep = GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();

	if (m_currentSelectedPosition != currentSelectedPosition || m_currentSelectedTimeStep != currentSelectedTimeStep || m_selectedNodeTime > m_currentPositionTime)
	{
		//the current position has been changed or the selected node has been changed since the last position validation -> check position
		m_currentSelectedPosition = currentSelectedPosition;
		m_currentSelectedTimeStep = currentSelectedTimeStep;
		m_currentPositionTime.Modified();

		if (this->m_EvalNode.IsNotNull())
		{
			this->m_EvalNode->SetProperty(mitk::nodeProp_RegEvalCurrentPosition, mitk::GenericProperty<mitk::Point3D>::New(currentSelectedPosition));
		}

		if (m_activeManipulation && ui.comboCenter->currentIndex() == 2)
		{ //update transform with the current position.
			OnCenterTypeChanged(ui.comboCenter->currentIndex());
		}
	}
}

void ManualRegistration::OnSettingsChanged(mitk::DataNode*)
{
	this->GetRenderWindowPart()->RequestUpdate();
};

void ManualRegistration::OnStoreBtnPushed()
{
	mitk::MAPRegistrationWrapper::Pointer newRegWrapper = mitk::MAPRegistrationWrapper::New();
	MAPRegistrationType::Pointer newReg = MAPRegistrationType::New();

	newRegWrapper->SetRegistration(newReg);

	::map::core::RegistrationManipulator<MAPRegistrationType> manipulator(newReg);

	::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
	kernel->setTransformModel(m_InverseCurrentTransform);

	::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel2 = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
	kernel2->setTransformModel(m_InverseCurrentTransform->GetInverseTransform());

	manipulator.setInverseMapping(kernel);
	manipulator.setDirectMapping(kernel2);

	if (m_SelectedPreRegNode != nullptr)
	{// Combine registration with selected pre registration as baseline
		typedef ::map::core::RegistrationCombinator<MAPRegistrationType, MAPRegistrationType> CombinatorType;
		CombinatorType::Pointer combinator = CombinatorType::New();
		newReg = combinator->process(*m_SelectedPreReg,*newReg);
		newRegWrapper->SetRegistration(newReg);
	}

	string regName = ("Reg_" + this->ui.lbNewRegName->text()).toStdString();
	mitk::DataNode::Pointer spResultRegistrationNode = mitk::generateRegistrationResultNode(regName, newRegWrapper, "org.mitk::manual_registration",
		mitk::EnsureUID(m_SelectedMovingNode->GetData()), mitk::EnsureUID(m_SelectedTargetNode->GetData()));

	this->GetDataStorage()->Add(spResultRegistrationNode);

	if (ui.checkMapEntity->isChecked())
	{
		QmitkMappingJob* pMapJob = new QmitkMappingJob();
		pMapJob->setAutoDelete(true);

		pMapJob->m_spInputData = this->m_SelectedMovingNode->GetData();
		pMapJob->m_InputDataUID = mitk::EnsureUID(m_SelectedMovingNode->GetData());
		pMapJob->m_spRegNode = spResultRegistrationNode;
		pMapJob->m_doGeometryRefinement = false;
		pMapJob->m_spRefGeometry = this->m_SelectedTargetNode->GetData()->GetGeometry()->Clone().GetPointer();

		pMapJob->m_MappedName = this->ui.lbNewRegName->text().toStdString();// +std::string(" mapped moving data");
		pMapJob->m_allowUndefPixels = true;
		pMapJob->m_paddingValue = 100;
		pMapJob->m_allowUnregPixels = true;
		pMapJob->m_errorValue = 200;
		pMapJob->m_InterpolatorLabel = "Linear Interpolation";
		pMapJob->m_InterpolatorType = mitk::ImageMappingInterpolator::Linear;

		connect(pMapJob, SIGNAL(Error(QString)), this, SLOT(OnMapJobError(QString)));
		connect(pMapJob, SIGNAL(MapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
			this, SLOT(OnMapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
			Qt::BlockingQueuedConnection);

		QThreadPool* threadPool = QThreadPool::globalInstance();
		threadPool->start(pMapJob);
	}

	this->StopSession();

	this->CheckInputs();
	this->ConfigureControls();
	this->GetRenderWindowPart()->RequestUpdate();
	this->StartRegistration();
}

void ManualRegistration::OnMapJobError(QString err)
{
	Error(err);
}

void ManualRegistration::OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob* job)
{
	mitk::DataNode::Pointer spMappedNode = mitk::generateMappedResultNode(job->m_MappedName,
		spMappedData, job->GetRegistration()->getRegistrationUID(), job->m_InputDataUID,
		job->m_doGeometryRefinement, job->m_InterpolatorLabel);
	this->GetDataStorage()->Add(spMappedNode);
	this->ConfigureControls();
	this->GetRenderWindowPart()->RequestUpdate();
};

void ManualRegistration::OnRotXChanged(double x)
{
	if (!m_internalUpdate)
	{
		m_internalUpdate = true;
		this->ui.slideRotX->setValue(x);
		m_internalUpdate = false;
		this->UpdateTransform(true);
	}
};

void ManualRegistration::OnRotXSlideChanged(int x)
{
	if (!m_internalUpdate)
	{
		this->ui.sbRotX->setValue(x);
	}
};

void ManualRegistration::OnRotYChanged(double y)
{
	if (!m_internalUpdate)
	{
		m_internalUpdate = true;
		this->ui.slideRotY->setValue(y);
		m_internalUpdate = false;
		this->UpdateTransform(true);
	}
};

void ManualRegistration::OnRotYSlideChanged(int y)
{
	if (!m_internalUpdate)
	{
		this->ui.sbRotY->setValue(y);
	}
};

void ManualRegistration::OnRotZChanged(double z)
{
	if (!m_internalUpdate)
	{
		m_internalUpdate = true;
		this->ui.slideRotZ->setValue(z);
		m_internalUpdate = false;
		this->UpdateTransform(true);
	}
};

void ManualRegistration::OnRotZSlideChanged(int z)
{
	if (!m_internalUpdate)
	{
		this->ui.sbRotZ->setValue(z);
	}
};

void ManualRegistration::OnTransXChanged(double x)
{
	if (!m_internalUpdate)
	{
		m_internalUpdate = true;
		this->ui.slideTransX->setValue(x);
		m_internalUpdate = false;
		this->UpdateTransform();
	}
};

void ManualRegistration::OnTransXSlideChanged(int x)
{
	if (!m_internalUpdate)
	{
		this->ui.sbTransX->setValue(x);
	}
};

void ManualRegistration::OnTransYChanged(double y)
{
	if (!m_internalUpdate)
	{
		m_internalUpdate = true;
		this->ui.slideTransY->setValue(y);
		m_internalUpdate = false;
		this->UpdateTransform();
	}
};

void ManualRegistration::OnTransYSlideChanged(int y)
{
	if (!m_internalUpdate)
	{
		this->ui.sbTransY->setValue(y);
	}
};

void ManualRegistration::OnTransZChanged(double z)
{
	if (!m_internalUpdate)
	{
		m_internalUpdate = true;
		this->ui.slideTransZ->setValue(z);
		m_internalUpdate = false;
		this->UpdateTransform();
	}
};

void ManualRegistration::OnTransZSlideChanged(int z)
{
	if (!m_internalUpdate)
	{
		this->ui.sbTransZ->setValue(z);
	}
};

void ManualRegistration::OnCenterTypeChanged(int index)
{
	ConfigureTransformCenter(index);

	this->UpdateTransformWidgets();

	if (this->m_EvalNode.IsNotNull())
	{
		this->m_EvalNode->Modified();
	}
	this->m_CurrentRegistrationWrapper->Modified();
	this->GetRenderWindowPart()->RequestUpdate();
};

void ManualRegistration::ConfigureTransformCenter(int centerType)
{
	auto offset = m_DirectCurrentTransform->GetOffset();

	if (centerType == 0)
	{ //image center
		auto center = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
		m_DirectCurrentTransform->SetCenter(center);
	}
	else if (centerType == 1)
	{ //world origin
		TransformType::OutputPointType itkCenter;
		itkCenter.Fill(0.0);
		m_DirectCurrentTransform->SetCenter(itkCenter);
	}
	else
	{ //current selected point
		auto newCenter = m_InverseCurrentTransform->TransformPoint(m_currentSelectedPosition);
		m_DirectCurrentTransform->SetCenter(newCenter);
	}

	m_DirectCurrentTransform->SetOffset(offset);
	m_DirectCurrentTransform->GetInverse(m_InverseCurrentTransform);
};

/*void ManualRegistration::on_Plugin_hidden(ctkEvent event)
{
	QString plugin_id = event.getProperty("id").toString();
	if (plugin_id != PLUGIN_ID)
		return;

	StopSession();
}*/