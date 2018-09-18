/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef REG_EVAL_SETTINGS_WIDGET_H
#define REG_EVAL_SETTINGS_WIDGET_H

#include <PopeLibExports.h>
#include "ui_RegEvalSettingsWidget.h"

#include <QWidget>
#include <mitkDataNode.h>

/**
 * \class RegEvalSettingsWidget
 * \brief Widget that views the information and profile of an algorithm stored in an DLLInfo object.
 */
class PopeLib_EXPORT RegEvalSettingsWidget : public QWidget, private Ui::RegEvalSettingsWidget
{
	Q_OBJECT

public:
	RegEvalSettingsWidget(QWidget *parent = nullptr);

	/**
	 * Configures the passed settings according to the current state of the
	 * widget.
	 * \param pointer to a instance based on QmitkMappingJobSettings.
	 * \pre settings must point to a valid instance..
	 */
	void ConfigureControls();
	bool eventFilter(QObject* object, QEvent* event) override;

public Q_SLOTS:
	/**
		* \brief Slot that can be used to set the node that should be configured by the widget.*/
	void SetNode(mitk::DataNode* node);
	//void SetNode(mitk::DataNode::Pointer node);

signals:
	void SettingsChanged(mitk::DataNode *node);

protected Q_SLOTS:
	void OnComboStyleChanged(int);
	void OnBlend50Pushed();
	void OnBlendTargetPushed();
	void OnBlendMovingPushed();
	void OnBlendTogglePushed();
	void OnSlideBlendChanged(int);
	void OnSpinBlendChanged(int);
	void OnSpinCheckerChanged(int);
	void OnWipeStyleChanged();
	void OnContourStyleChanged();

private:
	mitk::DataNode::Pointer m_selectedEvalNode;

	bool m_internalBlendUpdate;
	bool m_internalUpdate;
};

#endif // RegEvalSettingsWidget_H
