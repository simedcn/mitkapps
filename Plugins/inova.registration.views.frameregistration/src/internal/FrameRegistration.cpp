#include "inova_registration_views_frameregistration_Activator.h"
#include "FrameRegistration.h"
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
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>
#include <mitkMAPAlgorithmInfoSelection.h>
#include <mitkRegistrationHelper.h>
#include <mitkResultNodeGenerationHelper.h>

// Qmitk
#include <QmitkRegistrationJob.h>
#include <QmitkMappingJob.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QErrorMessage>
#include <QThreadPool>
#include <QDateTime>

// MatchPoint
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


const std::string FrameRegistration::VIEW_ID = "inova.registration.views.frameregistration";
string FrameRegistration::def_algorithm_short_name = "Thiron's demons (Fast, Refined)";

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


FrameRegistration::FrameRegistration()
    : m_Parent(nullptr)
    , m_LoadedDLLHandle(nullptr)
    , m_LoadedAlgorithm(nullptr)
    , m_CanLoadAlgorithm(false)
    , m_ValidInputs(false)
    , m_Working(false)
    , m_Quitting(false)
{
    m_spSelectedTargetData = nullptr;
    m_spSelectedTargetMaskData = nullptr;

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
FrameRegistration::~FrameRegistration()
{
    this->m_Quitting = true;
    StopAlgorithm(true);
}

void FrameRegistration::CreateQtPartControl(QWidget* parent)
{
    // create GUI widgets from the Qt Designer's .ui file
    ui.setupUi(parent);
    m_Parent = parent;

    ui.checkTargetMask->setChecked(false);
    //ui.m_tabs->setCurrentIndex(0);
    ui.m_tabs->setCurrentIndex(1);
    ui.m_mapperSettings->AllowSampling(false);

    this->CreateConnections();
    this->AdaptFolderGUIElements();
    this->CheckInputs();
    this->ConfigureProgressInfos();
    this->ConfigureRegistrationControls();

    this->LoadAlgorithmInfo();
    this->SetListOfAlgorithms();
    this->UpdateAlgorithmSelection();
}

void FrameRegistration::CreateConnections()
{

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

    // -----
    // Tab 4 - Frames
    // -----
    connect(ui.m_btnFrameSelAll, SIGNAL(clicked()), this, SLOT(OnFramesSelectAllPushed()));
    connect(ui.m_btnFrameDeSelAll, SIGNAL(clicked()), this, SLOT(OnFramesDeSelectAllPushed()));
    connect(ui.m_btnFrameInvert, SIGNAL(clicked()), this, SLOT(OnFramesInvertPushed()));

    /// CTK signals.
    auto pluginContext = inova_registration_views_frameregistration_Activator::GetContext();
    ctkDictionary propsForSlot;
    ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
    if (ref)
    {
        ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
        eventAdmin->publishSignal(this, SIGNAL(PluginIsBusy(const ctkDictionary&)), "registration/PLUGINISBUSY", Qt::DirectConnection);
        eventAdmin->publishSignal(this, SIGNAL(PluginIsIdle(const ctkDictionary&)), "registration/PLUGINISIDLE", Qt::DirectConnection);
    }
}

void FrameRegistration::LoadAlgorithmInfo()
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
}
void FrameRegistration::SetListOfAlgorithms()
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

const map::deployment::DLLInfo* FrameRegistration::GetSelectedAlgorithmDLL() const
{
    return m_SelectedAlgorithmInfo;
}

void FrameRegistration::SetFocus()
{
}

void FrameRegistration::Error(QString msg)
{
    mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
    MITK_ERROR << msg.toStdString().c_str();

    ui.m_teLog->append(QString("<font color='red'><b>") + msg + QString("</b></font>"));
}

void FrameRegistration::AdaptFolderGUIElements()
{
    ui.comboBox_Algorithm->setEnabled(m_CanLoadAlgorithm);
}

bool FrameRegistration::CheckInputs()
{
    bool validTarget = false;
    bool validTargetMask = false;

    mitk::DataNode::Pointer oldTargetNode = m_spSelectedTargetNode;

    m_spSelectedTargetNode = nullptr;
    m_spSelectedTargetData = nullptr;

    m_spSelectedTargetMaskNode = nullptr;
    m_spSelectedTargetMaskData = nullptr;

    mitk::NodePredicateDataType::Pointer isLabelSet = mitk::NodePredicateDataType::New("LabelSetImage");
    mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
    mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateAnd::Pointer isLegacyMask = mitk::NodePredicateAnd::New(isImage, isBinary);
    mitk::NodePredicateOr::Pointer maskPredicate = mitk::NodePredicateOr::New(isLegacyMask, isLabelSet);

    bool is_label_TargetImage_set = false;
    bool is_lbTargetMaskName_set = false;

    if (m_LoadedAlgorithm.IsNotNull())
    {
        /// Check the algorithm selected.
        using MaskRegInterface = map::algorithm::facet::MaskedRegistrationAlgorithmInterface<3, 3>;
        MaskRegInterface* pMaskInterface = dynamic_cast<MaskRegInterface*>(m_LoadedAlgorithm.GetPointer());

        /// Set the target images as well as its mask if enabled.
        QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();
        for (auto it = dataNodes.begin(); it != dataNodes.end(); ++it) //? or: Reverse direction?
        {
            auto datanode = *it;
            mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
            if (!image)
                continue;
            if (m_spSelectedTargetNode == nullptr)
            {
                m_spSelectedTargetNode = datanode;
                m_spSelectedTargetData = datanode->GetData();
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
            else
                break;
        }

        /// Check if the algorithm is OK with the selected data.
        if (m_spSelectedTargetNode != nullptr)
        {
            auto pData = m_spSelectedTargetNode->GetData();
            mitk::Image* targetImage = dynamic_cast<mitk::Image*>(pData);

            if (targetImage)
            {
                if (targetImage->GetTimeSteps() < 1)
                {
                    ui.label_TargetImage->setText(QString("<b>Dataset:</b> <font color='red'>Image has no mulitple time steps.</font>"));
                    is_label_TargetImage_set = true;
                }
                else if (targetImage->GetDimension() != m_LoadedAlgorithm->getTargetDimensions() + 1)
                {
                    ui.label_TargetImage->setText(QString("<b>Dataset:</b> <font color='red'>wrong image dimension. ") + QString::number(m_LoadedAlgorithm->getTargetDimensions()) + QString("D+t needed.</font>"));
                    is_label_TargetImage_set = true;
                }
                else
                {
                    validTarget = true;
                }
            }
            else
            {
                ui.label_TargetImage->setText(QString("<b>Dataset:</b> <font color='red'>no supported data selected.</font>"));
                is_label_TargetImage_set = true;
            }
        }

        /// Set the target mask.
        if (ui.checkTargetMask->isChecked())
        {
            if (m_spSelectedTargetMaskNode == nullptr)
            {
                ui.m_lbTargetMaskName->setText(QString("<font color='red'>No mask selected.</font>"));
                is_lbTargetMaskName_set = true;
            }
            else
            {
                mitk::Image* targetMaskImage = dynamic_cast<mitk::Image*>(m_spSelectedTargetMaskNode->GetData());
                bool isMask = maskPredicate->CheckNode(m_spSelectedTargetMaskNode);
                if (!isMask)
                {
                    ui.m_lbTargetMaskName->setText(QString("<font color='red'>No mask selected.</font>"));
                    is_lbTargetMaskName_set = true;
                }
                else if (targetMaskImage && pMaskInterface)
                {
                    m_spSelectedTargetMaskData = targetMaskImage;
                    validTargetMask = true;
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
    if (validTarget)
    {
        string name = m_spSelectedTargetNode->GetName();
        QString short_name = GetInputNodeDisplayName(m_spSelectedTargetNode, name);
        ui.label_TargetImage->setText("<b>Dataset:</b> " + short_name);
        ui.label_TargetImage->setToolTip(QString::fromStdString(name));
        ui.label_TargetImage->setStyleSheet("");
        ui.label_TargetImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        if (oldTargetNode != m_spSelectedTargetNode)
            ConfigureFrameList();
    }
    else
    {
        if (!is_label_TargetImage_set)
            ui.label_TargetImage->setText("<b>Dataset:</b> Please select in Data Manager.");
        ui.label_TargetImage->setToolTip("");
        ui.label_TargetImage->setStyleSheet("color: #E02000;\nbackground-color: #efef95;");
        ui.label_TargetImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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

    m_ValidInputs = validTarget && validTargetMask;
    //ui.m_tabs->setCurrentIndex(m_ValidInputs ? 1 : 0);
    ui.m_tabs->setCurrentIndex(1);
    return m_ValidInputs;
}

QString FrameRegistration::GetInputNodeDisplayName(const mitk::DataNode* node, const string& name) const
{
    QString result = "UNDEFINED/nullptr";

    if (node)
    {
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

mitk::DataStorage::SetOfObjects::Pointer FrameRegistration::GetRegNodes() const
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
std::string FrameRegistration::GetDefaultJobName() const
{
    string name = (m_spSelectedTargetNode != nullptr) ?
                  Elements::get_next_name(m_spSelectedTargetNode->GetName(), GetDataStorage()) :
                  "FrameRegistration";
    return name;

    //mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetRegNodes().GetPointer();
    //mitk::DataStorage::SetOfObjects::ElementIdentifier newIndex = 0;
    //
    //bool isUnique = false;
    //std::string baseName = "Corrected #";
    //
    //if (m_spSelectedTargetNode.IsNotNull())
    //{
    //	baseName = m_spSelectedTargetNode->GetName() + " corrected #";
    //}
    //
    //std::string result = baseName;
    //
    //while (!isUnique)
    //{
    //	++newIndex;
    //	result = baseName + map::core::convert::toStr(newIndex);
    //	isUnique = (this->GetDataStorage()->GetNamedNode(result) == nullptr);
    //}
    //
    //return result;
}
void FrameRegistration::ConfigureRegistrationControls()
{
    ui.m_tabSelection->setEnabled(!m_Working);
    ui.m_leRegJobName->setEnabled(!m_Working);
    ui.groupMasks->setEnabled(!m_Working);

    ui.m_pbStartReg->setEnabled(false);
    ui.m_pbStopReg->setEnabled(false);
    ui.m_pbStopReg->setVisible(false);

    ui.m_lbTargetMaskName->setVisible(ui.checkTargetMask->isChecked());

    if (m_LoadedAlgorithm.IsNotNull())
    {
        ui.m_tabSettings->setEnabled(!m_Working);
        ui.m_tabExclusion->setEnabled(!m_Working);
        ui.m_tabExecution->setEnabled(true);
        ui.m_pbStartReg->setEnabled(m_ValidInputs && !m_Working);
        ui.m_leRegJobName->setEnabled(!m_Working);

        const IStoppableAlgorithm* pIterativ = dynamic_cast<const IStoppableAlgorithm*>(m_LoadedAlgorithm.GetPointer());
        if (pIterativ)
            ui.m_pbStopReg->setVisible(pIterativ->isStoppable());

        using MaskRegInterface = map::algorithm::facet::MaskedRegistrationAlgorithmInterface<3, 3>;
        const MaskRegInterface* pMaskReg = dynamic_cast<const MaskRegInterface*>(m_LoadedAlgorithm.GetPointer());

        ui.groupMasks->setVisible(pMaskReg != nullptr);

        //if the stop button is set to visible and the algorithm is working -> then the algorithm is stoppable, thus enable the button.
        ui.m_pbStopReg->setEnabled(ui.m_pbStopReg->isVisible() && m_Working);
    }
    else
    {
        ui.m_tabSettings->setEnabled(false);
        ui.m_tabExclusion->setEnabled(false);
        ui.m_tabExecution->setEnabled(false);
        ui.groupMasks->setVisible(false);
    }

    if (!m_Working)
    {
        this->ui.m_leRegJobName->setText(QString::fromStdString(this->GetDefaultJobName()));
    }
}
void FrameRegistration::ConfigureProgressInfos()
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
    ui.m_progBarFrame->reset();
}
void FrameRegistration::ConfigureFrameList()
{
    ui.m_listFrames->clear();

    if (m_spSelectedTargetData.IsNotNull())
    {
        mitk::TimeGeometry::ConstPointer tg = m_spSelectedTargetData->GetTimeGeometry();

        for (unsigned int i = 1; i < tg->CountTimeSteps(); ++i)
        {
            QString lable = "Timepoint #" + QString::number(i) + QString(" (") + QString::number(tg->GetMinimumTimePoint(i)) + QString(" ms - " + QString::number(tg->GetMaximumTimePoint(i)) + QString(" ms)"));
            QListWidgetItem* item = new QListWidgetItem(lable, ui.m_listFrames);
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(Qt::Checked);
        }
    }
}

void FrameRegistration::UpdateAlgorithmSelection()
{
    int index = ui.comboBox_Algorithm->currentIndex();
    if (index < 0 || (uint) index >= algorithm_descs.size())
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

void FrameRegistration::StopAlgorithm(bool force)
{
    if (!m_Working)
        return;

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
    if (force)
    {
        this->m_Working = false;
        ctkDictionary properties;
        properties["id"] = QString::fromStdString(VIEW_ID);
        emit PluginIsIdle(properties);
        if (!m_Quitting)
            ui.m_pbStopReg->setEnabled(false);
    }
}

void FrameRegistration::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&)
{
    if (!m_Working)
    {
        CheckInputs();
        ConfigureRegistrationControls();
    }
}
void FrameRegistration::NodeRemoved(const mitk::DataNode* node)
{
    if (!m_Working)
        return;

    if (node == this->m_spSelectedTargetNode)
        StopAlgorithm();
}

void FrameRegistration::OnMaskCheckBoxToggeled(bool)
{
    if (!m_Working)
    {
        CheckInputs();
        ConfigureRegistrationControls();
    }
};
void FrameRegistration::on_comboBox_Algorithm_currentIndexChanged(int index)
{
    if (index < 0 || (uint) index >= algorithm_descs.size())
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
    ui.checkTargetMask->setChecked(false);

    this->AdaptFolderGUIElements();
    this->CheckInputs();
    this->ConfigureRegistrationControls();
    this->ConfigureProgressInfos();
    //ui.m_tabs->setCurrentIndex(1);

    UpdateAlgorithmSelection();
}
void FrameRegistration::OnFramesSelectAllPushed()
{
    for (int row = 0; row < ui.m_listFrames->count(); row++)
    {
        QListWidgetItem* item = ui.m_listFrames->item(row);
        item->setCheckState(Qt::Checked);
    }
}
void FrameRegistration::OnFramesDeSelectAllPushed()
{
    for (int row = 0; row < ui.m_listFrames->count(); row++)
    {
        QListWidgetItem* item = ui.m_listFrames->item(row);
        item->setCheckState(Qt::Unchecked);
    }
}
void FrameRegistration::OnFramesInvertPushed()
{
    for (int row = 0; row < ui.m_listFrames->count(); row++)
    {
        QListWidgetItem* item = ui.m_listFrames->item(row);

        if (item->checkState() == Qt::Unchecked)
        {
            item->setCheckState(Qt::Checked);
        }
        else
        {
            item->setCheckState(Qt::Unchecked);
        }
    }
}
mitk::TimeFramesRegistrationHelper::IgnoreListType FrameRegistration::GenerateIgnoreList() const
{
    mitk::TimeFramesRegistrationHelper::IgnoreListType result;

    for (int row = 0; row < ui.m_listFrames->count(); row++)
    {
        QListWidgetItem* item = ui.m_listFrames->item(row);

        if (item->checkState() == Qt::Unchecked)
        {
            result.push_back(row + 1);
        }
    }

    return result;
}

void FrameRegistration::OnStartRegBtnPushed()
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
    QmitkFramesRegistrationJob* pJob = new QmitkFramesRegistrationJob(m_LoadedAlgorithm);
    pJob->setAutoDelete(true);

    pJob->m_spTargetData = m_spSelectedTargetData;
    pJob->m_TargetDataUID = mitk::EnsureUID(this->m_spSelectedTargetNode->GetData());
    pJob->m_IgnoreList = this->GenerateIgnoreList();

    if (m_spSelectedTargetMaskData.IsNotNull())
    {
        pJob->m_spTargetMask = m_spSelectedTargetMaskData;
        pJob->m_TargetMaskDataUID = mitk::EnsureUID(this->m_spSelectedTargetMaskNode->GetData());
    }

    pJob->m_MappedName = ui.m_leRegJobName->text().toStdString();

    ui.m_mapperSettings->ConfigureJobSettings(pJob);

    connect(pJob, SIGNAL(Error(QString)), this, SLOT(OnRegJobError(QString)));
    connect(pJob, SIGNAL(Finished()), this, SLOT(OnRegJobFinished()));
    connect(pJob, SIGNAL(ResultIsAvailable(mitk::Image::Pointer, const QmitkFramesRegistrationJob*)), this, SLOT(OnMapResultIsAvailable(mitk::Image::Pointer, const QmitkFramesRegistrationJob*)), Qt::BlockingQueuedConnection);

    connect(pJob, SIGNAL(AlgorithmInfo(QString)), this, SLOT(OnAlgorithmInfo(QString)));
    connect(pJob, SIGNAL(AlgorithmStatusChanged(QString)), this, SLOT(OnAlgorithmStatusChanged(QString)));
    connect(pJob, SIGNAL(AlgorithmIterated(QString, bool, unsigned long)), this, SLOT(OnAlgorithmIterated(QString, bool, unsigned long)));
    connect(pJob, SIGNAL(LevelChanged(QString, bool, unsigned long)), this, SLOT(OnLevelChanged(QString, bool, unsigned long)));
    connect(pJob, SIGNAL(FrameRegistered(double)), this, SLOT(OnFrameRegistered(double)));
    connect(pJob, SIGNAL(FrameMapped(double)), this, SLOT(OnFrameMapped(double)));
    connect(pJob, SIGNAL(FrameProcessed(double)), this, SLOT(OnFrameProcessed(double)));

    QThreadPool* threadPool = QThreadPool::globalInstance();
    threadPool->start(pJob);
}
void FrameRegistration::OnStopRegBtnPushed()
{
    StopAlgorithm();
}
void FrameRegistration::OnSaveLogBtnPushed()
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
void FrameRegistration::OnRegJobError(QString err)
{
    Error(err);
};
void FrameRegistration::OnRegJobFinished()
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

void FrameRegistration::OnMapResultIsAvailable(mitk::Image::Pointer spMappedData, const QmitkFramesRegistrationJob* job)
{
    if (m_Quitting)
        return;

    ui.m_teLog->append(QString("<b><font color='blue'>Corrected image stored. Name: ") + QString::fromStdString(job->m_MappedName) + QString("</font></b>"));

    mitk::DataNode::Pointer spResultNode = mitk::generateMappedResultNode(job->m_MappedName, spMappedData.GetPointer(), "", job->m_TargetDataUID, false, job->m_InterpolatorLabel);

    this->GetDataStorage()->Add(spResultNode, this->m_spSelectedTargetNode);
    this->GetRenderWindowPart()->RequestUpdate();
};
void FrameRegistration::OnMapJobError(QString err)
{
    if (m_Quitting)
        return;

    Error(err);
};
void FrameRegistration::OnAlgorithmIterated(QString info, bool hasIterationCount, unsigned long currentIteration)
{
    if (m_Quitting)
        return;

    if (hasIterationCount)
    {
        ui.m_progBarIteration->setValue(currentIteration);
    }

    ui.m_teLog->append(info);
};
void FrameRegistration::OnLevelChanged(QString info, bool hasLevelCount, unsigned long currentLevel)
{
    if (m_Quitting)
        return;

    if (hasLevelCount)
    {
        ui.m_progBarLevel->setValue(currentLevel);
    }

    ui.m_teLog->append(QString("<b><font color='green'>") + info + QString("</font></b>"));
};
void FrameRegistration::OnAlgorithmStatusChanged(QString info)
{
    if (!m_Quitting)
        ui.m_teLog->append(QString("<b><font color='blue'>") + info + QString(" </font></b>"));
};
void FrameRegistration::OnAlgorithmInfo(QString info)
{
    if (!m_Quitting)
        ui.m_teLog->append(QString("<font color='gray'><i>") + info + QString("</i></font>"));
};

void FrameRegistration::OnFrameProcessed(double progress)
{
    if (m_Quitting)
        return;

    ui.m_teLog->append(QString("<b><font color='blue'>Frame processed...</font></b>"));
    ui.m_progBarFrame->setValue(100 * progress);
};
void FrameRegistration::OnFrameRegistered(double progress)
{
    if (m_Quitting)
        return;

    ui.m_teLog->append(QString("<b><font color='blue'>Frame registered...</font></b>"));
    ui.m_progBarFrame->setValue(100 * progress);
};
void FrameRegistration::OnFrameMapped(double progress)
{
    if (m_Quitting)
        return;

    ui.m_teLog->append(QString("<b><font color='blue'>Frame mapped...</font></b>"));
    ui.m_progBarFrame->setValue(100 * progress);
};
