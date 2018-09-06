#include "inova_registration_views_registrationalgorithms_Activator.h"
#include "RegistrationAlgorithms.h"
#include <PopeElements.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryISelectionProvider.h>
#include <berryQModelIndexObject.h>

// Mitk
#include <mitkStatusBar.h>
#include <mitkPointSet.h>
#include <mitkImageTimeSelector.h>
#include <mitkMAPAlgorithmInfoSelection.h>
#include <mitkRegistrationHelper.h>
#include <mitkResultNodeGenerationHelper.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>

// Qmitk
#include <QmitkRegistrationJob.h>
#include <QmitkMappingJob.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QErrorMessage>
#include <QThreadPool>
#include <QDateTime>

// RegistrationAlgorithms
#include <mapImageRegistrationAlgorithmInterface.h>
#include <mapPointSetRegistrationAlgorithmInterface.h>
#include <mapRegistrationAlgorithmInterface.h>
#include <mapMaskedRegistrationAlgorithmInterface.h>
#include <mapAlgorithmEvents.h>
#include <mapAlgorithmWrapperEvent.h>
#include <mapExceptionObjectMacros.h>
#include <mapConvert.h>
#include <mapDeploymentDLLAccess.h>
#include <mapDeploymentDLLDirectoryBrowser.h>

using string = std::string;


const std::string RegistrationAlgorithms::VIEW_ID = "inova.registration.views.registrationalgorithms";
string RegistrationAlgorithms::def_algorithm_short_name = "Thiron's demons (Fast, Refined)";

AlgorithmDescription::AlgorithmDescription(const string& short_name, const string& name, const string& matchPoint_name, map::deployment::DLLInfo* info)
	: short_name(short_name), name(name), matchPoint_name(matchPoint_name), info(info)
{}
AlgorithmDescription::AlgorithmDescription(std::initializer_list<std::string> args)
	: info(nullptr)
{
	auto it = args.begin();
	short_name = *it;
	++it;
	name = *it;
	++it;
	matchPoint_name = *it;
}

RegistrationAlgorithms::RegistrationAlgorithms()
	: m_Parent(nullptr)
	, m_LoadedDLLHandle(nullptr)
	, m_LoadedAlgorithm(nullptr)
{
	m_CanLoadAlgorithm = false;
	m_ValidInputs = false;
	m_Working = false;
	m_Quitting = false;
	m_spSelectedTargetData = nullptr;
	m_spSelectedMovingData = nullptr;
	m_spSelectedTargetMaskData = nullptr;
	m_spSelectedMovingMaskData = nullptr;

	algorithm_descs =
	{
		{ "Translation",						"Rigid: Translation",													"TranslationMattesMIMultiResAlgorithm.3D.default" },
		{ "T+A   [Translation + Angles]",		"Rigid: Translation + Euler Angles",									"Euler3DMSAlgorithm.3D.default" },
		{ "T+A (Mutual Info, MultiRes)",		"Rigid: Translation + Euler Angles (Mutual Info, Multi Resolution)",	"Euler3DMattesMIMultiResAlgorithm.3D.default" },
		{ "T+A (Mutual Info)",					"Rigid: Translation + Euler Angles (Mutual Info)",						"Euler3DMattesMIAlgorithm.3D.default" },
		{ "Level Set Motion (MultiRes)",		"Deformable: Level Set Motion (Multi Resolution)",						"LevelSetMotion.3D.multiRes.default" },
		{ "Level Set Motion",					"Deformable: Level Set Motion",											"LevelSetMotion.3D.default" },
		{ "Thiron's demons (Fast, MultiRes)",	"Deformable: Thiron's demons (Fast, Multi Resolution)",					"Demons.FastSymmetricForces.3D.multiRes.default" },
		{ "Thiron's demons (Fast)",				"Deformable: Thiron's demons (Fast)",									"Demons.FastSymmetricForces.3D.default" },
		{ "Thiron's demons (Fast, Refined)",	"Deformable: Thiron's demons (Fast, Refined)",							"Demons.new.3D.default" },
		{ "Thiron's demons",					"Deformable: Thiron's demons",											"Demons.SymmetricForces.3D.default" },
		{ "Affine Registration",				"Affine: Translation + Rotation + Scaling + Shearing",					"AffineMattesMIMultiResAlgorithm.3D.default" },
	};
}
RegistrationAlgorithms::~RegistrationAlgorithms()
{
	this->m_Quitting = true;
	StopAlgorithm(true);
}

void RegistrationAlgorithms::CreateQtPartControl(QWidget* parent)
{
	// create GUI widgets from the Qt Designer's .ui file
	ui.setupUi(parent);
	m_Parent = parent;

	ui.checkMovingMask->setChecked(false);
	ui.checkTargetMask->setChecked(false);
	ui.m_checkStoreReg->setVisible(false);
	ui.m_checkStoreReg->setChecked(true);
	ui.m_checkMapEntity->setVisible(false);
	ui.m_checkMapEntity->setChecked(true);
	ui.m_tabs->setCurrentIndex(0);

	this->CreateConnections();
	this->AdaptFolderGUIElements();
	this->CheckInputs();
	this->ConfigureProgressInfos();
	this->ConfigureRegistrationControls();

	this->LoadAlgorithmInfo();
	this->SetListOfAlgorithms();
	this->UpdateAlgorithmSelection();
}

void RegistrationAlgorithms::CreateConnections()
{
	connect(ui.checkMovingMask, SIGNAL(toggled(bool)), this, SLOT(OnMaskCheckBoxToggeled(bool)));
	connect(ui.checkTargetMask, SIGNAL(toggled(bool)), this, SLOT(OnMaskCheckBoxToggeled(bool)));

	// ------
	// Tab 1 - Shared library loading interface
	// ------

	connect(this->ui.comboBox_Algorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_Algorithm_currentIndexChanged(int)));

	// -----
	// Tab 2 - Execution
	// -----
	connect(ui.m_pbStartReg, SIGNAL(clicked()), this, SLOT(OnStartRegBtnPushed()));
	connect(ui.m_pbStopReg, SIGNAL(clicked()), this, SLOT(OnStopRegBtnPushed()));
	connect(ui.m_pbSaveLog, SIGNAL(clicked()), this, SLOT(OnSaveLogBtnPushed()));

	/// CTK signals.
	auto pluginContext = inova_registration_views_registrationalgorithms_Activator::GetContext();
	ctkDictionary propsForSlot;
	ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		eventAdmin->publishSignal(this, SIGNAL(PluginIsBusy(const ctkDictionary&)), "registration/PLUGINISBUSY", Qt::DirectConnection);
		eventAdmin->publishSignal(this, SIGNAL(PluginIsIdle(const ctkDictionary&)), "registration/PLUGINISIDLE", Qt::DirectConnection);
	}
}

void RegistrationAlgorithms::LoadAlgorithmInfo()
{
	map::deployment::DLLDirectoryBrowser::Pointer browser = map::deployment::DLLDirectoryBrowser::New();
	string path = qApp->applicationDirPath().toStdString() + "/MatchPoint"; // "C:\\o\\baP1806\\bin\\plugins\\Debug\\MatchPoint"
	browser->addDLLSearchLocation(path);
	browser->update();
	auto DLLInfoList = browser->getLibraryInfos();
	/// Associate the algorithms.
	for (auto& alg : algorithm_descs)
	{
		for (auto dll_info : DLLInfoList)
		{
			auto dll_alg_name = dll_info->getAlgorithmUID().getName();
			if (alg.matchPoint_name == dll_alg_name)
			{
				alg.info = dll_info;
			}
		}
	}
	/// Delete the algorithms that aren't available.
	auto it = algorithm_descs.begin();
	while (it != algorithm_descs.end())
	{
		auto& alg = *it;
		if (alg.info == nullptr)
			it = algorithm_descs.erase(it);
		else
			++it;
	}

	//for (auto dll_info : DLLInfoList)
	//{
	//	MITK_INFO << dll_info->getAlgorithmUID().getName();
	//}
}
void RegistrationAlgorithms::SetListOfAlgorithms()
{
	ui.comboBox_Algorithm->setMaxCount(algorithm_descs.size());
	ui.comboBox_Algorithm->setMaxVisibleItems(algorithm_descs.size());
	int i_def_alg = -1;
	for (size_t i = 0; i < algorithm_descs.size(); i++)
	{
		ui.comboBox_Algorithm->setItemText(i, QString::fromStdString(algorithm_descs[i].short_name));
		ui.comboBox_Algorithm->setItemData(i, QString::fromStdString(algorithm_descs[i].name), Qt::ToolTipRole);
		if (algorithm_descs[i].short_name == def_algorithm_short_name)
			i_def_alg = i;
	}
	if (i_def_alg >= 0)
		ui.comboBox_Algorithm->setCurrentIndex(i_def_alg);
	else if (ui.comboBox_Algorithm->maxCount() > 0)
		ui.comboBox_Algorithm->setCurrentIndex(0);
}

const map::deployment::DLLInfo* RegistrationAlgorithms::GetSelectedAlgorithmDLL() const
{
	return m_SelectedAlgorithmInfo;
}

void RegistrationAlgorithms::SetFocus()
{
}

void RegistrationAlgorithms::Error(QString msg)
{
	mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
	MITK_ERROR << msg.toStdString().c_str();

	ui.m_teLog->append(QString("<font color='red'><b>") + msg + QString("</b></font>"));
}
void RegistrationAlgorithms::AdaptFolderGUIElements()
{
	ui.comboBox_Algorithm->setEnabled(m_CanLoadAlgorithm);
}

bool RegistrationAlgorithms::CheckInputs()
{
	bool validMoving = false;
	bool validTarget = false;
	bool validMovingMask = false;
	bool validTargetMask = false;

	m_spSelectedMovingNode = nullptr;
	m_spSelectedMovingData = nullptr;
	m_spSelectedTargetNode = nullptr;
	m_spSelectedTargetData = nullptr;

	m_spSelectedMovingMaskNode = nullptr;
	m_spSelectedMovingMaskData = nullptr;
	m_spSelectedTargetMaskNode = nullptr;
	m_spSelectedTargetMaskData = nullptr;

	mitk::NodePredicateDataType::Pointer isLabelSet = mitk::NodePredicateDataType::New("LabelSetImage");
	mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
	mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
	mitk::NodePredicateAnd::Pointer isLegacyMask = mitk::NodePredicateAnd::New(isImage, isBinary);
	mitk::NodePredicateOr::Pointer maskPredicate = mitk::NodePredicateOr::New(isLegacyMask, isLabelSet);

	bool is_label_MovingImage_set = false;
	bool is_label_TargetImage_set = false;
	bool is_lbMovingMaskName_set = false;
	bool is_lbTargetMaskName_set = false;

	if (m_LoadedAlgorithm.IsNotNull())
	{
		/// Check the algorithm selected.
		using InternalDefaultPointSetType = ::map::core::continuous::Elements<3>::InternalPointSetType ;
		using PointSetRegInterface = ::map::algorithm::facet::PointSetRegistrationAlgorithmInterface<InternalDefaultPointSetType, InternalDefaultPointSetType>;
		using MaskRegInterface = ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<3, 3>;

		PointSetRegInterface* pPointSetInterface = dynamic_cast<PointSetRegInterface*>(m_LoadedAlgorithm.GetPointer());
		MaskRegInterface* pMaskInterface = dynamic_cast<MaskRegInterface*>(m_LoadedAlgorithm.GetPointer());

		/// Set the moving and target images as well as their masks if enabled.
		QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();
		for (auto it = dataNodes.begin(); it != dataNodes.end(); ++it) //? or: Reverse direction?
		{
			auto datanode = *it;
			mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
			mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(datanode->GetData());
			if (!image && !pointSet)
				continue;
			if (m_spSelectedMovingNode == nullptr)
			{
				m_spSelectedMovingNode = datanode;
				m_spSelectedMovingData = datanode->GetData();
			}
			else if (m_spSelectedTargetNode == nullptr)
			{
				m_spSelectedTargetNode = datanode;
				m_spSelectedTargetData = datanode->GetData();
				if (!ui.checkMovingMask->isChecked() && !ui.checkTargetMask->isChecked())
					break;
			}
			else
			{
				if (!image)
					continue;
				if (ui.checkMovingMask->isChecked() && m_spSelectedMovingMaskNode == nullptr)
				{
					if (!maskPredicate->CheckNode(datanode))
						continue;
					m_spSelectedMovingMaskNode = datanode;
					if (!ui.checkTargetMask->isChecked())
						break;
				}
				else if (ui.checkTargetMask->isChecked() && m_spSelectedTargetMaskNode == nullptr)
				{
					if (!maskPredicate->CheckNode(datanode))
						continue;
					m_spSelectedTargetMaskNode = datanode;
					break;
				}
			}
		}

		/// Check if the algorithm is OK with the selected data.
		if (m_spSelectedMovingNode != nullptr)
		{
			auto pData = m_spSelectedMovingNode->GetData();
			mitk::Image* movingImage = dynamic_cast<mitk::Image*>(pData);
			mitk::PointSet* movingPointSet = dynamic_cast<mitk::PointSet*>(pData);

			if (movingPointSet && pPointSetInterface)
			{
				validMoving = true;
			}
			else if (movingImage && !pPointSetInterface)
			{
				if (movingImage->GetDimension() - 1 == m_LoadedAlgorithm->getMovingDimensions() && movingImage->GetTimeSteps() > 1)
				{
					//images has multiple time steps and a time step has the correct dimensionality
					mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
					imageTimeSelector->SetInput(movingImage);
					imageTimeSelector->SetTimeNr(0);
					imageTimeSelector->UpdateLargestPossibleRegion();

					m_spSelectedMovingData = imageTimeSelector->GetOutput();
					validMoving = true;
					ui.m_teLog->append(QString("<font color='gray'><i>Selected moving image has multiple time steps. First time step is used as moving image.</i></font>"));
				}
				else if (movingImage->GetDimension() != m_LoadedAlgorithm->getMovingDimensions())
				{
					ui.label_MovingImage->setText(QString("<b>Moving dataset:</b> <font color='red'>wrong image dimension. ") + QString::number(m_LoadedAlgorithm->getMovingDimensions()) + QString("D needed.</font>"));
					is_label_MovingImage_set = true;
				}
				else
				{
					validMoving = true;
				}
			}
			else
			{
				ui.label_MovingImage->setText(QString("<b>Moving dataset:</b> <font color='red'>no supported data selected.</font>"));
				is_label_MovingImage_set = true;
			}
		}

		if (m_spSelectedTargetNode != nullptr)
		{
			auto pData = m_spSelectedTargetNode->GetData();
			mitk::Image* targetImage = dynamic_cast<mitk::Image*>(pData);
			mitk::PointSet* targetPointSet = dynamic_cast<mitk::PointSet*>(pData);

			if (targetPointSet && pPointSetInterface)
			{
				validTarget = true;
			}
			else if (targetImage && !pPointSetInterface)
			{
				if (targetImage->GetDimension() - 1 == m_LoadedAlgorithm->getTargetDimensions() && targetImage->GetTimeSteps() > 1)
				{
					//images has multiple time steps and a time step has the correct dimensionality
					mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
					imageTimeSelector->SetInput(targetImage);
					imageTimeSelector->SetTimeNr(0);
					imageTimeSelector->UpdateLargestPossibleRegion();

					m_spSelectedTargetData = imageTimeSelector->GetOutput();
					validTarget = true;
					ui.m_teLog->append(QString("<font color='gray'><i>Selected target image has multiple time steps. First time step is used as target image.</i></font>"));
				}
				else if (targetImage->GetDimension() != m_LoadedAlgorithm->getTargetDimensions())
				{
					ui.label_TargetImage->setText(QString("<b>Target dataset:</b> <font color='red'>wrong image dimension. ") + QString::number(m_LoadedAlgorithm->getTargetDimensions()) + QString("D needed.</font>"));
					is_label_TargetImage_set = true;
				}
				else
				{
					validTarget = true;
				}
			}
			else
			{
				ui.label_TargetImage->setText(QString("<b>Target dataset:</b> <font color='red'>no supported data selected.</font>"));
				is_label_TargetImage_set = true;
			}
		}

		/// Set the moving and target masks.
		if (ui.checkMovingMask->isChecked())
		{
			if (m_spSelectedMovingMaskNode == nullptr)
			{
				ui.m_lbMovingMaskName->setText(QString("<font color='red'>No mask selected.</font>"));
				is_lbMovingMaskName_set = true;
			}
			else
			{
				mitk::Image* movingMaskImage = dynamic_cast<mitk::Image*>(m_spSelectedMovingMaskNode->GetData());
				if (movingMaskImage && pMaskInterface)
				{
					if (movingMaskImage->GetDimension() - 1 == m_LoadedAlgorithm->getMovingDimensions() && movingMaskImage->GetTimeSteps() > 1)
					{
						//images has multiple time steps and a time step has the correct dimensionality
						mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
						imageTimeSelector->SetInput(movingMaskImage);
						imageTimeSelector->SetTimeNr(0);
						imageTimeSelector->UpdateLargestPossibleRegion();

						m_spSelectedMovingMaskData = imageTimeSelector->GetOutput();
						validMovingMask = true;
						ui.m_teLog->append(QString("<font color='gray'><i>Selected moving mask has multiple time steps. First time step is used as moving mask.</i></font>"));
					}
					else if (movingMaskImage->GetDimension() != m_LoadedAlgorithm->getMovingDimensions())
					{
						ui.m_lbMovingMaskName->setText(QString("<font color='red'>Wrong image dimension. ") + QString::number(m_LoadedAlgorithm->getMovingDimensions()) + QString("D needed.</font>"));
						is_lbMovingMaskName_set = true;
					}
					else
					{
						m_spSelectedMovingMaskData = movingMaskImage;
						validMovingMask = true;
					}
				}
				else
				{
					ui.m_lbMovingMaskName->setText(QString("<font color='red'>No supported data selected!</font>"));
					is_lbMovingMaskName_set = true;
				}
			}
		}
		else
		{
			ui.m_lbMovingMaskName->setText(QString(""));
			is_lbMovingMaskName_set = true;
			validMovingMask = true;
		}

		if (ui.checkTargetMask->isChecked())
		{
			if (m_spSelectedTargetMaskNode == nullptr)
			{
				ui.m_lbTargetMaskName->setText(QString("<font color='red'>No data selected.</font>"));
				is_lbTargetMaskName_set = true;
			}
			else
			{
				mitk::Image* targetMaskImage = dynamic_cast<mitk::Image*>(m_spSelectedTargetMaskNode->GetData());
				if (targetMaskImage && pMaskInterface)
				{
					if (targetMaskImage->GetDimension() - 1 == m_LoadedAlgorithm->getTargetDimensions() && targetMaskImage->GetTimeSteps() > 1)
					{
						//images has multiple time steps and a time step has the correct dimensionality
						mitk::ImageTimeSelector::Pointer imageTimeSelector =	 mitk::ImageTimeSelector::New();
						imageTimeSelector->SetInput(targetMaskImage);
						imageTimeSelector->SetTimeNr(0);
						imageTimeSelector->UpdateLargestPossibleRegion();

						m_spSelectedTargetMaskData = imageTimeSelector->GetOutput();
						validTargetMask = true;
						ui.m_teLog->append(QString("<font color='gray'><i>Selected target mask has multiple time steps. First time step is used as target mask.</i></font>"));
					}
					else if (targetMaskImage->GetDimension() != m_LoadedAlgorithm->getTargetDimensions())
					{
						ui.m_lbTargetMaskName->setText(QString("<font color='red'>Wrong image dimension. ") + QString::number(m_LoadedAlgorithm->getTargetDimensions()) + QString("D needed.</font>"));
						is_lbTargetMaskName_set = true;
					}
					else
					{
						m_spSelectedTargetMaskData = targetMaskImage;
						validTargetMask = true;
					}
				}
				else
				{
					ui.m_lbTargetMaskName->setText(QString("<font color='red'>No supported data selected.</font>"));
					is_lbTargetMaskName_set = true;
				}
			}

		}
		else
		{
			ui.m_lbTargetMaskName->setText(QString(""));
			is_lbTargetMaskName_set = true;
			validTargetMask = true;
		}

	}

	/// Update UI accordingly.
	if (validMoving)
	{
		string name = m_spSelectedMovingNode->GetName();
		QString short_name = GetInputNodeDisplayName(m_spSelectedMovingNode, name);
		ui.label_MovingImage->setText("<b>Moving dataset:</b> " + short_name);
		ui.label_MovingImage->setToolTip(QString::fromStdString(name));
		ui.label_MovingImage->setStyleSheet("");
		ui.label_MovingImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		if (!is_label_MovingImage_set)
			ui.label_MovingImage->setText("<b>Moving dataset:</b> Please select in Data Manager.");
		ui.label_MovingImage->setToolTip("");
		ui.label_MovingImage->setStyleSheet("color: #E02000;\nbackground-color: #efef95;");
		ui.label_MovingImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}

	if (validTarget)
	{
		string name = m_spSelectedTargetNode->GetName();
		QString short_name = GetInputNodeDisplayName(m_spSelectedTargetNode, name);
		ui.label_TargetImage->setText("<b>Target dataset:</b> " + short_name);
		ui.label_TargetImage->setToolTip(QString::fromStdString(name));
		ui.label_TargetImage->setStyleSheet("");
		ui.label_TargetImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		if (!is_label_TargetImage_set)
			ui.label_TargetImage->setText("<b>Target dataset:</b> Please select in Data Manager.");
		ui.label_TargetImage->setToolTip("");
		ui.label_TargetImage->setStyleSheet("color: #E02000;\nbackground-color: #efef95;");
		ui.label_TargetImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	
	if (validMovingMask && ui.checkMovingMask->isChecked())
	{
		string name = m_spSelectedMovingMaskNode->GetName();
		QString short_name = GetInputNodeDisplayName(m_spSelectedMovingMaskNode, name);
		ui.m_lbMovingMaskName->setText(short_name);
		ui.m_lbMovingMaskName->setToolTip(QString::fromStdString(name));
	}
	else
	{
		if (!is_lbMovingMaskName_set)
			ui.m_lbMovingMaskName->setText("");
		ui.m_lbMovingMaskName->setToolTip("");
	}

	if (validTargetMask && ui.checkTargetMask->isChecked())
	{
		string name = m_spSelectedTargetMaskNode->GetName();
		QString short_name = GetInputNodeDisplayName(m_spSelectedTargetMaskNode, name);
		ui.m_lbTargetMaskName->setText(short_name);
		ui.m_lbTargetMaskName->setToolTip(QString::fromStdString(name));
	}
	else
	{
		if (!is_lbTargetMaskName_set)
			ui.m_lbTargetMaskName->setText("");
		ui.m_lbTargetMaskName->setToolTip("");
	}

	m_ValidInputs = validMoving && validTarget && validMovingMask && validTargetMask;
	ui.m_tabs->setCurrentIndex(m_ValidInputs ? 1 : 0);
	return m_ValidInputs;
}

QString RegistrationAlgorithms::GetInputNodeDisplayName(const mitk::DataNode* node, string name) const
{
	QString result = "UNDEFINED/nullptr";

	if (node)
	{
		if (name.empty())
			name = node->GetName();
		result = Elements::get_short_name_for_image(name);

		const mitk::PointSet* pointSet = dynamic_cast<const mitk::PointSet*>(node->GetData());
		if (pointSet)
		{
			mitk::DataStorage::SetOfObjects::ConstPointer sources = this->GetDataStorage()->GetSources(node);
			if (sources.IsNotNull() && sources->Size() > 0)
				result += " (" + QString::fromStdString(sources->GetElement(0)->GetName()) + ")";
		}
	}

	return result;
}
mitk::DataStorage::SetOfObjects::Pointer RegistrationAlgorithms::GetRegNodes() const
{
	mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetDataStorage()->GetAll();
	mitk::DataStorage::SetOfObjects::Pointer result = mitk::DataStorage::SetOfObjects::New();

	for (auto node : *nodes)
	{
		if (mitk::MITKRegistrationHelper::IsRegNode(node))
			result->push_back(node);
	}

	return result;
}
std::string RegistrationAlgorithms::GetDefaultRegJobName() const
{
	string name = (m_spSelectedMovingNode != nullptr) ?
		Elements::get_next_name(m_spSelectedMovingNode->GetName(), GetDataStorage()) :
		"AutomaticRegistration";
	return name;

	//mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetRegNodes().GetPointer();
	//mitk::DataStorage::SetOfObjects::ElementIdentifier estimatedIndex = nodes->Size();

	//bool isUnique = false;
	//std::string result = "Unnamed Reg";

	//while (!isUnique)
	//{
	//	++estimatedIndex;
	//	result = "Reg #" + map::core::convert::toStr(estimatedIndex);
	//	isUnique = (this->GetDataStorage()->GetNamedNode(result) == nullptr);
	//}

	//return result;
}
void RegistrationAlgorithms::ConfigureRegistrationControls()
{
	ui.m_tabAlgorithm->setEnabled(!m_Working);
	ui.m_leRegJobName->setEnabled(!m_Working);
	ui.groupMasks->setEnabled(!m_Working);

	ui.m_pbStartReg->setEnabled(false);
	ui.m_pbStopReg->setEnabled(false);
	ui.m_pbStopReg->setVisible(false);

	ui.m_lbMovingMaskName->setVisible(ui.checkMovingMask->isChecked());
	ui.m_lbTargetMaskName->setVisible(ui.checkTargetMask->isChecked());

	if (m_LoadedAlgorithm.IsNotNull())
	{
		ui.m_tabSettings->setEnabled(!m_Working);
		ui.m_tabExecution->setEnabled(true);
		ui.m_pbStartReg->setEnabled(m_ValidInputs && !m_Working);
		ui.m_leRegJobName->setEnabled(!m_Working);
		ui.m_checkMapEntity->setEnabled(!m_Working);
		ui.m_checkStoreReg->setEnabled(!m_Working);

		const IStoppableAlgorithm* pIterativ = dynamic_cast<const IStoppableAlgorithm*>(m_LoadedAlgorithm.GetPointer());

		if (pIterativ)
		{
			ui.m_pbStopReg->setVisible(pIterativ->isStoppable());
		}

		using MaskRegInterface = map::algorithm::facet::MaskedRegistrationAlgorithmInterface<3, 3>;
		const MaskRegInterface* pMaskReg = dynamic_cast<const MaskRegInterface*>(m_LoadedAlgorithm.GetPointer());

		ui.groupMasks->setVisible(pMaskReg != nullptr);

		//if the stop button is set to visible and the algorithm is working ->
		//then the algorithm is stoppable, thus enable the button.
		ui.m_pbStopReg->setEnabled(ui.m_pbStopReg->isVisible() && m_Working);
	}
	else
	{
		ui.m_tabSettings->setEnabled(false);
		ui.m_tabExecution->setEnabled(false);
		ui.groupMasks->setVisible(false);
	}

	if (!m_Working)
	{
		this->ui.m_leRegJobName->setText(QString::fromStdString(this->GetDefaultRegJobName()));
	}
}
void RegistrationAlgorithms::ConfigureProgressInfos()
{
	const IIterativeAlgorithm* pIterative = dynamic_cast<const IIterativeAlgorithm*>(m_LoadedAlgorithm.GetPointer());
	const IMultiResAlgorithm* pMultiRes = dynamic_cast<const IMultiResAlgorithm*>(m_LoadedAlgorithm.GetPointer());

	ui.m_progBarIteration->setVisible(pIterative);
	ui.m_lbProgBarIteration->setVisible(pIterative);

	if (pIterative)
	{
		QString format = "%p% (%v/%m)";

		if (!pIterative->hasMaxIterationCount())
		{
			format = "%v";
			ui.m_progBarIteration->setMaximum(0);
		}
		else
		{
			ui.m_progBarIteration->setMaximum(pIterative->getMaxIterations());
		}

		ui.m_progBarIteration->setFormat(format);
	}

	if (pMultiRes)
	{
		ui.m_progBarLevel->setMaximum(pMultiRes->getResolutionLevels());
	}
	else
	{
		ui.m_progBarLevel->setMaximum(1);
	}

	ui.m_progBarIteration->reset();
	ui.m_progBarLevel->reset();
}

void RegistrationAlgorithms::UpdateAlgorithmSelection()
{
	int index = ui.comboBox_Algorithm->currentIndex();
	if (index < 0 || index >= algorithm_descs.size())
	{
		ui.comboBox_Algorithm->setToolTip("");
		Error(QString("No valid algorithm is selected. ABORTING."));
		return;
	}
	auto& algorithm = algorithm_descs[index];
	ui.comboBox_Algorithm->setToolTip(QString::fromStdString(algorithm.name));
	this->m_SelectedAlgorithmInfo = algorithm.info;
	ui.m_teAlgorithmDetails->updateInfo(m_SelectedAlgorithmInfo);

	// enable loading
	m_CanLoadAlgorithm = true;
	this->AdaptFolderGUIElements();
};

/*void disconnectAlgorithm()
{
	disconnect(pJob, SIGNAL(Error(QString)), this, SLOT(OnRegJobError(QString)));
	disconnect(pJob, SIGNAL(Finished()), this, SLOT(OnRegJobFinished()));
	disconnect(pJob, SIGNAL(RegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer, const QmitkRegistrationJob*)), this, SLOT(OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer, const QmitkRegistrationJob*)), Qt::BlockingQueuedConnection);
	disconnect(pJob, SIGNAL(AlgorithmInfo(QString)), this, SLOT(OnAlgorithmInfo(QString)));
	disconnect(pJob, SIGNAL(AlgorithmStatusChanged(QString)), this, SLOT(OnAlgorithmStatusChanged(QString)));
	disconnect(pJob, SIGNAL(AlgorithmIterated(QString, bool, unsigned long)), this, SLOT(OnAlgorithmIterated(QString, bool, unsigned long)));
	disconnect(pJob, SIGNAL(LevelChanged(QString, bool, unsigned long)), this, SLOT(OnLevelChanged(QString, bool, unsigned long)));
}*/
void RegistrationAlgorithms::StopAlgorithm(bool force)
{
	if (m_LoadedAlgorithm.IsNotNull())
	{
		IStoppableAlgorithm* pIterativ = dynamic_cast<IStoppableAlgorithm*>(m_LoadedAlgorithm.GetPointer());

		if (pIterativ && pIterativ->isStoppable())
		{
			bool is_stopped = pIterativ->stopAlgorithm();
			if (is_stopped)
			{
				this->m_Working = false;
				ctkDictionary properties;
				properties["id"] = QString::fromStdString(VIEW_ID);
				emit PluginIsIdle(properties);
			}
			else
			{
				MITK_INFO << "Cannot stop the registration process.";
			}

			if (!m_Quitting)
				ui.m_pbStopReg->setEnabled(false);
		}
		else
		{
			MITK_INFO << "Cannot stop the registration process because the algorithm is not stoppable.";
		}
	}
	if (m_Working && force)
	{
		this->m_Working = false;
		ctkDictionary properties;
		properties["id"] = QString::fromStdString(VIEW_ID);
		emit PluginIsIdle(properties);
		if (!m_Quitting)
			ui.m_pbStopReg->setEnabled(false);
	}
}

void RegistrationAlgorithms::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&)
{
	if (!m_Working)
	{
		CheckInputs();
		ConfigureRegistrationControls();
	}
}
void RegistrationAlgorithms::NodeRemoved(const mitk::DataNode* node)
{
	if (!m_Working)
		return;

	if (node == this->m_spSelectedMovingNode || node == this->m_spSelectedTargetNode)
		StopAlgorithm();
}

void RegistrationAlgorithms::OnMaskCheckBoxToggeled(bool)
{
	if (!m_Working)
	{
		CheckInputs();
		ConfigureRegistrationControls();
	}
};
void RegistrationAlgorithms::on_comboBox_Algorithm_currentIndexChanged(int index)
{
	if (index < 0 || index >= algorithm_descs.size())
	{
		MITK_INFO << "Selected a wrong index in comboBox.";
		return;
	}
	auto dllInfo = algorithm_descs[index].info;
	assert(dllInfo != nullptr);
	auto dllHandle = map::deployment::openDeploymentDLL(dllInfo->getLibraryFilePath());
	auto algorithm = map::deployment::getRegistrationAlgorithm(dllHandle);
	if (algorithm == nullptr)
	{
		Error(QString("Error. Cannot load selected algorithm."));
		return;
	}
	this->m_LoadedAlgorithm = algorithm;
	this->m_LoadedDLLHandle = dllHandle;

	ui.m_AlgoConfigurator->setAlgorithm(m_LoadedAlgorithm);
	ui.checkMovingMask->setChecked(false);
	ui.checkTargetMask->setChecked(false);

	this->AdaptFolderGUIElements();
	this->CheckInputs();
	this->ConfigureRegistrationControls();
	this->ConfigureProgressInfos();
	//ui.m_tabs->setCurrentIndex(1);

	UpdateAlgorithmSelection();
}
void RegistrationAlgorithms::OnStartRegBtnPushed()
{
	this->m_Working = true;
	ctkDictionary properties;
	properties["id"] = QString::fromStdString(VIEW_ID);
	emit PluginIsBusy(properties);

	////////////////////////////////
	//configure GUI
	this->ConfigureProgressInfos();

	ui.m_progBarIteration->reset();
	ui.m_progBarLevel->reset();

	this->ConfigureRegistrationControls();

	if (ui.m_checkClearLog->checkState() == Qt::Checked)
	{
		this->ui.m_teLog->clear();
	}

	/////////////////////////
	//create job and put it into the thread pool
	QmitkRegistrationJob* pJob = new QmitkRegistrationJob(m_LoadedAlgorithm);
	pJob->setAutoDelete(true);

	pJob->m_spTargetData = m_spSelectedTargetData;
	pJob->m_spMovingData = m_spSelectedMovingData;
	pJob->m_TargetDataUID = mitk::EnsureUID(this->m_spSelectedTargetNode->GetData());
	pJob->m_MovingDataUID = mitk::EnsureUID(this->m_spSelectedMovingNode->GetData());

	if (m_spSelectedTargetMaskData.IsNotNull())
	{
		pJob->m_spTargetMask = m_spSelectedTargetMaskData;
		pJob->m_TargetMaskDataUID = mitk::EnsureUID(this->m_spSelectedTargetMaskNode->GetData());
	}

	if (m_spSelectedMovingMaskData.IsNotNull())
	{
		pJob->m_spMovingMask = m_spSelectedMovingMaskData;
		pJob->m_MovingMaskDataUID = mitk::EnsureUID(this->m_spSelectedMovingMaskNode->GetData());
	}

	pJob->m_JobName = ui.m_leRegJobName->text().toStdString();

	pJob->m_StoreReg = ui.m_checkStoreReg->isChecked();

	connect(pJob, SIGNAL(Error(QString)), this, SLOT(OnRegJobError(QString)));
	connect(pJob, SIGNAL(Finished()), this, SLOT(OnRegJobFinished()));
	connect(pJob, SIGNAL(RegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer, const QmitkRegistrationJob*)), this, SLOT(OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer, const QmitkRegistrationJob*)), Qt::BlockingQueuedConnection);

	connect(pJob, SIGNAL(AlgorithmInfo(QString)), this, SLOT(OnAlgorithmInfo(QString)));
	connect(pJob, SIGNAL(AlgorithmStatusChanged(QString)), this, SLOT(OnAlgorithmStatusChanged(QString)));
	connect(pJob, SIGNAL(AlgorithmIterated(QString, bool, unsigned long)), this, SLOT(OnAlgorithmIterated(QString, bool, unsigned long)));
	connect(pJob, SIGNAL(LevelChanged(QString, bool, unsigned long)), this, SLOT(OnLevelChanged(QString, bool, unsigned long)));

	QThreadPool* threadPool = QThreadPool::globalInstance();
	threadPool->start(pJob);
}
void RegistrationAlgorithms::OnStopRegBtnPushed()
{
	StopAlgorithm();
}
void RegistrationAlgorithms::OnSaveLogBtnPushed()
{
	QDateTime currentTime = QDateTime::currentDateTime();
	QString fileName = tr("registration_log_") + currentTime.toString(tr("yyyy-MM-dd_hh-mm-ss")) + tr(".txt");
	fileName = QFileDialog::getSaveFileName(nullptr, tr("Save registration log"), fileName, tr("Text files (*.txt)"));

	if (fileName.isEmpty())
	{
		QMessageBox::critical(nullptr, tr("No file selected!"), tr("Cannot save registration log file. Please selected a file."));
	}
	else
	{
		std::ofstream file;

		std::ios_base::openmode iOpenFlag = std::ios_base::out | std::ios_base::trunc;
		file.open(fileName.toStdString().c_str(), iOpenFlag);

		if (!file.is_open())
		{
			mitkThrow() << "Cannot open or create specified file to save. File path: " << fileName.toStdString();
		}

		file << this->ui.m_teLog->toPlainText().toStdString() << std::endl;

		file.close();
	}

}
void RegistrationAlgorithms::OnRegJobError(QString err)
{
	Error(err);
};
void RegistrationAlgorithms::OnRegJobFinished()
{
	this->m_Working = false;
	ctkDictionary properties;
	properties["id"] = QString::fromStdString(VIEW_ID);
	emit PluginIsIdle(properties);

	this->GetRenderWindowPart()->RequestUpdate();

	this->CheckInputs();
	this->ConfigureRegistrationControls();
	this->ConfigureProgressInfos();
};

void RegistrationAlgorithms::OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer spResultRegistration, const QmitkRegistrationJob* pRegJob)
{
	if (m_Quitting)
		return;

	string reg_name = "Reg_" + pRegJob->m_JobName;
	mitk::DataNode::Pointer spResultRegistrationNode = mitk::generateRegistrationResultNode(reg_name, spResultRegistration, pRegJob->GetLoadedAlgorithm()->getUID()->toStr(), pRegJob->m_MovingDataUID, pRegJob->m_TargetDataUID);

	if (pRegJob->m_StoreReg)
	{
		ui.m_teLog->append(QString("<b><font color='blue'> Storing registration object in data manager ... </font></b>"));

		this->GetDataStorage()->Add(spResultRegistrationNode);
		this->GetRenderWindowPart()->RequestUpdate();
	}

	if (ui.m_checkMapEntity->isChecked())
	{
		QmitkMappingJob* pMapJob = new QmitkMappingJob();
		pMapJob->setAutoDelete(true);

		pMapJob->m_spInputData = pRegJob->m_spMovingData;
		pMapJob->m_InputDataUID = pRegJob->m_MovingDataUID;
		pMapJob->m_spRegNode = spResultRegistrationNode;
		pMapJob->m_doGeometryRefinement = false;
		pMapJob->m_spRefGeometry = pRegJob->m_spTargetData->GetGeometry()->Clone().GetPointer();

		pMapJob->m_MappedName = pRegJob->m_JobName;// +std::string(" mapped moving data");
		pMapJob->m_allowUndefPixels = true;
		pMapJob->m_paddingValue = 100;
		pMapJob->m_allowUnregPixels = true;
		pMapJob->m_errorValue = 200;
		pMapJob->m_InterpolatorLabel = "Linear Interpolation";
		pMapJob->m_InterpolatorType = mitk::ImageMappingInterpolator::Linear;

		connect(pMapJob, SIGNAL(Error(QString)), this, SLOT(OnMapJobError(QString)));
		connect(pMapJob, SIGNAL(MapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)), this, SLOT(OnMapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)), Qt::BlockingQueuedConnection);
		connect(pMapJob, SIGNAL(AlgorithmInfo(QString)), this, SLOT(OnAlgorithmInfo(QString)));

		ui.m_teLog->append(QString("<b><font color='blue'>Started mapping input data...</font></b>"));

		QThreadPool* threadPool = QThreadPool::globalInstance();
		threadPool->start(pMapJob);
	}
};
void RegistrationAlgorithms::OnMapJobError(QString err)
{
	if (m_Quitting)
		return;

	Error(err);
};
void RegistrationAlgorithms::OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob* job)
{
	if (m_Quitting)
		return;

	ui.m_teLog->append(QString("<b><font color='blue'>Mapped entity stored. Name: ") + QString::fromStdString(job->m_MappedName) + QString("</font></b>"));

	mitk::DataNode::Pointer spMappedNode = mitk::generateMappedResultNode(job->m_MappedName, spMappedData, job->GetRegistration()->getRegistrationUID(), job->m_InputDataUID, job->m_doGeometryRefinement, job->m_InterpolatorLabel);
	this->GetDataStorage()->Add(spMappedNode);
	this->GetRenderWindowPart()->RequestUpdate();
};
void RegistrationAlgorithms::OnAlgorithmIterated(QString info, bool hasIterationCount, unsigned long currentIteration)
{
	if (m_Quitting)
		return;

	if (hasIterationCount)
	{
		ui.m_progBarIteration->setValue(currentIteration);
	}

	ui.m_teLog->append(info);
};
void RegistrationAlgorithms::OnLevelChanged(QString info, bool hasLevelCount, unsigned long currentLevel)
{
	if (m_Quitting)
		return;

	if (hasLevelCount)
	{
		ui.m_progBarLevel->setValue(currentLevel);
	}

	ui.m_teLog->append(QString("<b><font color='green'>") + info + QString("</font></b>"));
};
void RegistrationAlgorithms::OnAlgorithmStatusChanged(QString info)
{
	if (!m_Quitting)
		ui.m_teLog->append(QString("<b><font color='blue'>") + info + QString(" </font></b>"));
};
void RegistrationAlgorithms::OnAlgorithmInfo(QString info)
{
	if (!m_Quitting)
		ui.m_teLog->append(QString("<font color='gray'><i>") + info + QString("</i></font>"));
};
