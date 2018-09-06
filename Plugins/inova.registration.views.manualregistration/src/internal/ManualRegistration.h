
#ifndef __Q_ManualRegistration_H
#define __Q_ManualRegistration_H

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>
#include <QmitkSliceNavigationListener.h>
#include <mitkMAPRegistrationWrapper.h>
#include <itkEuler3DTransform.h>
#include "ui_ManualRegistration.h"

#include <service/event/ctkEventAdmin.h>


class QmitkMappingJob;

/*!
\brief ManualRegistration

\warning	This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class ManualRegistration : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
	Q_OBJECT

public:
	static const std::string VIEW_ID;
	static const QString PLUGIN_ID;

	/**
	* Creates smartpointer typedefs
	*/
	berryObjectMacro(ManualRegistration)

	ManualRegistration();
	~ManualRegistration();

	virtual void CreateQtPartControl(QWidget *parent);

protected:
	/// \brief called by QmitkFunctionality when DataManager's selection has changed
	virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source, const QList<mitk::DataNode::Pointer>& nodes) override;
	virtual void NodeRemoved(const mitk::DataNode* node) override;

	virtual void SetFocus();

	virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart);
	virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart);

private:
	void Error(QString msg);

	/**
	* Checks if appropriated nodes are selected in the data manager. If nodes are selected,
	* they are stored m_spSelectedRegNode, m_spSelectedInputNode and m_spSelectedRefNode.
	* They are also checked for vadility.*/
	bool CheckInputs();

	/**
	* Updates the state of controls regarding to the state of the view and it objects.*/
	void ConfigureControls();

	void StartRegistration();

	/** Initialize the state of the view, so the manipulation can start.*/
	void InitSession();

	/** Stops session, removes all obsolite members (e.g. RegEvalObject). After that the view is in a valid but inactive state.*/
	void StopSession();

	/**
	* Updates the widgets that manipulate the transform according to the transform.*/
	void UpdateTransformWidgets();

	/**
	* Updates the transform according to the widgets that manipulate the transform.*/
	void UpdateTransform(bool updateRotation = false);

	void ConfigureTransformCenter(int centerType);

protected slots:
	/// \brief Called when the user clicks the GUI button
	//void OnStartBtnPushed();
	//void OnCancelBtnPushed();
	void OnStoreBtnPushed();
	void OnSettingsChanged(mitk::DataNode*);

	void OnRotXChanged(double);
	void OnRotYChanged(double);
	void OnRotZChanged(double);

	void OnTransXChanged(double);
	void OnTransYChanged(double);
	void OnTransZChanged(double);

	void OnRotXSlideChanged(int);
	void OnRotYSlideChanged(int);
	void OnRotZSlideChanged(int);

	void OnTransXSlideChanged(int);
	void OnTransYSlideChanged(int);
	void OnTransZSlideChanged(int);

	void OnCenterTypeChanged(int);

	void OnSliceChanged();

	void OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob* job);

	//void on_Plugin_hidden(ctkEvent event);

protected:
	QWidget *m_Parent;
	Ui::ManualRegistrationControls ui;

	mitk::DataNode::Pointer m_EvalNode;

	QmitkSliceNavigationListener m_SliceChangeListener;

	itk::TimeStamp m_selectedNodeTime;
	itk::TimeStamp m_currentPositionTime;

	bool m_activeManipulation;

	using MAPRegistrationType = ::map::core::Registration<3, 3>;

	MAPRegistrationType::ConstPointer m_SelectedPreReg;

	mitk::DataNode::Pointer m_SelectedPreRegNode;
	mitk::DataNode::Pointer m_SelectedMovingNode;
	mitk::DataNode::Pointer m_SelectedTargetNode;

	/** @brief currently valid selected position in the inspector*/
	mitk::Point3D m_currentSelectedPosition;
	/** @brief indicates if the currently selected position is valid for the currently selected fit.
	* This it is within the input image */
	unsigned int m_currentSelectedTimeStep;


	mitk::MAPRegistrationWrapper::Pointer m_CurrentRegistrationWrapper;
	using TransformType = itk::Euler3DTransform<::map::core::continuous::ScalarType>;
	TransformType::Pointer m_InverseCurrentTransform;
	TransformType::Pointer m_DirectCurrentTransform;
	MAPRegistrationType::Pointer m_CurrentRegistration;

	bool m_internalUpdate;
	static const std::string HelperNodeName;
};

#endif // __Q_ManualRegistration_H

