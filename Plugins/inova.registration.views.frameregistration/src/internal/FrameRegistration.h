#ifndef __Q_inova_registration_views_frameregistration_H
#define __Q_inova_registration_views_frameregistration_H

#include <QmitkAbstractView.h>

#include "ui_FrameRegistrationControls.h"

#include <mitkImage.h>

// MatchPoint
#include <mapDeploymentDLLInfo.h>
#include <mapDeploymentDLLHandle.h>
#include <mapRegistrationAlgorithmBase.h>
#include <mapIterativeAlgorithmInterface.h>
#include <mapMultiResRegistrationAlgorithmInterface.h>
#include <mapStoppableAlgorithmInterface.h>

#include <mitkMAPRegistrationWrapper.h>
#include <service/event/ctkEventAdmin.h>
#include <QmitkFramesRegistrationJob.h>


struct AlgorithmDescription;

/*!
\brief View for motion artefact correction of images.

The view utalizes MatchPoint registration algorithms and the mitk::TimeFramesRegistrationHelper and implemnts the GUI
business logic to make frame correction aka motion artefact correction on 3D+t images.

*/
class FrameRegistration : public QmitkAbstractView
{
	// this is needed for all Qt objects that should have a Qt meta-object (everything that derives from QObject and wants to have signal/slots)
	Q_OBJECT

public:
	static const std::string VIEW_ID;

	/**
	* Creates smartpointer typedefs
	*/
	berryObjectMacro(FrameRegistration)

	FrameRegistration();
	~FrameRegistration();

protected:
	virtual void CreateQtPartControl(QWidget* parent);
	virtual void CreateConnections();

	virtual void SetFocus();

	/// \brief called by QmitkFunctionality when DataManager's selection has changed
	virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source, const QList<mitk::DataNode::Pointer>& nodes);
	virtual void NodeRemoved(const mitk::DataNode* node) override;

private:
	/**
	* @brief Adapt the visibility of GUI elements depending on the current data	loaded
	*/
	void AdaptFolderGUIElements();

	void Error(QString msg);

	/**
	* checks if appropriated nodes are selected in the data manager. If nodes are selected,
	* they are stored m_MovingData and m_TargetData. It also sets the info lables accordingly.
	* @return True: All inputs are set and valid (images). False: At least one input is not set
	* or invalid */
	bool CheckInputs();

	/**
	* Updates the state of registration control buttons. Regarding to selected
	* inputs, loaded algorithm and its state.*/
	void ConfigureRegistrationControls();

	/**
	* Configures the progress bars according to the chosen algorithm.
	*/
	void ConfigureProgressInfos();

	/**configure the frame list widget based on the selected target.*/
	void ConfigureFrameList();

	/**generates the ignore list based on the frame list widget selection.*/
	mitk::TimeFramesRegistrationHelper::IgnoreListType GenerateIgnoreList() const;

	/** Methods returns a list of all nodes in the data manager containing a registration wrapper.
		* The list may be empty.*/
	mitk::DataStorage::SetOfObjects::Pointer GetRegNodes() const;

	/** Returns a proposal for a (unique) default reg job name */
	std::string GetDefaultJobName() const;

	/** Returns the display name of the passed node. Normally it is just node->GetName().
	 * if the node contains a point set it is additionally checked if the point set node
	 * has a source node and its name will be added in parentheses.*/
	QString GetInputNodeDisplayName(const mitk::DataNode* node, const std::string& name) const;

	/** Returns the Pointer to the DLL info of the algorithm currently selected by the system.
	The info is received via m_AlgorithmSelectionListener.
	@return If there is no item selected the returning pointer
	will be null.
	*/
	const map::deployment::DLLInfo* GetSelectedAlgorithmDLL() const;
	void LoadAlgorithmInfo();
	void SetListOfAlgorithms();

	void UpdateAlgorithmSelection();

	void StopAlgorithm(bool force = false);

signals:
	void PluginIsBusy(const ctkDictionary&);
	void PluginIsIdle(const ctkDictionary&);

protected slots:
	/// \brief Called when the user clicks the GUI button
	void OnMaskCheckBoxToggeled(bool checked);

	void OnStartRegBtnPushed();
	void OnStopRegBtnPushed();
	void OnSaveLogBtnPushed();

	void OnFramesSelectAllPushed();
	void OnFramesDeSelectAllPushed();
	void OnFramesInvertPushed();

	void OnRegJobError(QString err);
	void OnRegJobFinished();
	void OnMapJobError(QString err);
	void OnMapResultIsAvailable(mitk::Image::Pointer spMappedData, const QmitkFramesRegistrationJob* job);
	void OnAlgorithmIterated(QString info, bool hasIterationCount, unsigned long currentIteration);
	void OnLevelChanged(QString info, bool hasLevelCount, unsigned long currentLevel);
	void OnAlgorithmStatusChanged(QString info);
	void OnAlgorithmInfo(QString info);
	void OnFrameProcessed(double progress);
	void OnFrameRegistered(double progress);
	void OnFrameMapped(double progress);

	void on_comboBox_Algorithm_currentIndexChanged(int);

	friend struct berry::SelectionChangedAdapter<FrameRegistration>;

protected:
	QWidget* m_Parent;

	/** @brief this pointer holds the algorithm selection listener */
	QScopedPointer<berry::ISelectionListener> m_AlgorithmSelectionListener;

	::map::deployment::DLLHandle::Pointer m_LoadedDLLHandle;
	::map::algorithm::RegistrationAlgorithmBase::Pointer m_LoadedAlgorithm;
	::map::deployment::DLLInfo::ConstPointer m_SelectedAlgorithmInfo;

	using IIterativeAlgorithm = map::algorithm::facet::IterativeAlgorithmInterface;
	using IMultiResAlgorithm = map::algorithm::facet::MultiResRegistrationAlgorithmInterface;
	using IStoppableAlgorithm = map::algorithm::facet::StoppableAlgorithmInterface;

	mitk::DataNode::Pointer m_spSelectedTargetNode;
	/*Data of the selected target node that should be used for registration.
	Can be the direct return of node->GetData(), but can also be a sub
	set (like a special time frame).*/
	mitk::BaseData::ConstPointer m_spSelectedTargetData;

	mitk::DataNode::Pointer m_spSelectedTargetMaskNode;
	mitk::Image::ConstPointer m_spSelectedTargetMaskData;

	// boolean variables to control visibility of GUI elements
	bool m_CanLoadAlgorithm;
	bool m_ValidInputs;
	bool m_Working;
	bool m_Quitting;

	Ui::FrameRegistrationControls ui;

	std::vector<AlgorithmDescription> algorithm_descs;
	static std::string def_algorithm_short_name;
};

struct AlgorithmDescription
{
	AlgorithmDescription(std::initializer_list<std::string> args);

	std::string short_name;
	std::string name;
	std::string matchPoint_name;
	map::deployment::DLLInfo::Pointer info = nullptr;
};
#endif // inova_registration_views_frameregistration_h

