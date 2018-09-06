#ifndef __Q_inova_registration_views_comparison_H
#define __Q_inova_registration_views_comparison_H

#include "ui_RegistrationComparisonControls.h"

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>
#include <QmitkSliceNavigationListener.h>

/*!
\brief RegistrationComparison

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class RegistrationComparison : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
	// this is needed for all Qt objects that should have a Qt meta-object
	// (everything that derives from QObject and wants to have signal/slots)
	Q_OBJECT

public:

	static const std::string VIEW_ID;

	/**
	* Creates smartpointer typedefs
	*/
	berryObjectMacro(RegistrationComparison)

	RegistrationComparison();
	~RegistrationComparison();

	virtual void CreateQtPartControl(QWidget *parent);

protected slots:
	void OnSettingsChanged(mitk::DataNode*);
	void OnSliceChanged();

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
	* They are also checked for vadility and stored in m_ValidInput,... .
	* It also sets the info lables accordingly.*/
	bool CheckInputs();

	void StartEvaluation();
	void StopEvaluation();

	/**
	* Updates the state of controls regarding to selected eval object.*/
	void ConfigureControls();

protected:
	Ui::RegistrationComparisonControls ui;

private:
	QWidget *m_Parent;

	mitk::DataNode::Pointer m_selectedEvalNode;

	QmitkSliceNavigationListener m_SliceChangeListener;

	itk::TimeStamp m_selectedNodeTime;
	itk::TimeStamp m_currentPositionTime;

	bool m_activeEvaluation;
	bool m_autoMoving;
	bool m_autoTarget;

	/** @brief currently valid selected position in the inspector*/
	mitk::Point3D m_currentSelectedPosition;
	/** @brief indicates if the currently selected position is valid for the currently selected fit.
	* This it is within the input image */
	unsigned int m_currentSelectedTimeStep;

	mitk::DataNode::Pointer m_spSelectedRegNode;
	mitk::DataNode::Pointer m_spSelectedMovingNode;
	mitk::DataNode::Pointer m_spSelectedTargetNode;

	static const std::string HelperNodeName;
};

#endif // __Q_inova_registration_views_comparison_H

