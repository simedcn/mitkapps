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

#ifndef SegmentationView_h
#define SegmentationView_h

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>
#include <DisplayCoordinateSupplier.h>
#include <usServiceRegistration.h>

#include <ctkPluginContext.h>

//#include <QTimer>
#include <memory>

// There's an item "SegmentationView.ui" in the UI_FILES list in
// files.cmake. The Qt UI Compiler will parse this file and generate a
// header file prefixed with "ui_", which is located in the build directory.
// Use Qt Creator to view and edit .ui files. The generated header file
// provides a class that contains all of the UI widgets.
#include <ui_SegmentationView.h>

#include <service/event/ctkEventAdmin.h>

using namespace std;

namespace mitk
{
	class LabelSetImage;
	class LabelSet;
	class Label;
	class DataStorage;
	class ToolManager;
	class DataNode;
}

// All views in MITK derive from QmitkAbstractView. You have to override
// at least the two methods CreateQtPartControl() and SetFocus().
class SegmentationView : public QmitkAbstractView
{
	// As QmitkAbstractView derives from QObject and we want to use the Qt
	// signal and slot mechanism, we must not forget the Q_OBJECT macro.
	// This header file also has to be listed in MOC_H_FILES in files.cmake,
	// in order that the Qt Meta-Object Compiler can find and process this
	// class declaration.
	Q_OBJECT

public:
	// This is a tricky one and will give you some headache later on in
	// your debug sessions if it has been forgotten. Also, don't forget
	// to initialize it in the implementation file.
	static const std::string VIEW_ID;

	SegmentationView();
	~SegmentationView();

	// In this method we initialize the GUI components and connect the
	// associated signals and slots.
	void CreateQtPartControl(QWidget* parent) override;
	void setLabelWidget();
	void updateAfterSelectionChanged();

protected:
	void NodeAdded(const mitk::DataNode* node) override;
	void NodeRemoved(const mitk::DataNode* node) override;

private:
	// Typically a one-liner. Set the focus to the default widget.
	void SetFocus() override;
	// This method is conveniently called whenever the selection of Data Manager items changes.
	void OnSelectionChanged(berry::IWorkbenchPart::Pointer source, const QList<mitk::DataNode::Pointer>& dataNodes) override;

private:
	using DataNodeList = list<mitk::DataNode::Pointer>;
	mitk::DataNode* getFirstSelectedNode(shared_ptr<DataNodeList> dataNodes = nullptr);

private:
	// Generated from the associated UI file, it encapsulates all the widgets
	// of our view.
	Ui::SegmentationView ui;

	mitk::DataNode* m_SelectedNode = nullptr;

	enum TableColumns
	{
		NAME_COL = 0,
		COLOR_COL,
		VISIBLE_COL
	};

private slots:
	void OnColorButtonClicked();
	void OnVisibleButtonClicked();
	void on_button_Paint_clicked();
	void on_button_OtsuSegmentation_clicked();
	void on_button_STLExport_clicked();
	void on_button_ExportCSV_clicked();
};

#endif
