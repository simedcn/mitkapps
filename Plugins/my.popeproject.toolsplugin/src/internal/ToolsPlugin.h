
#ifndef ToolsPlugin_h
#define ToolsPlugin_h

#include "ImageStatisticsCalculationThread.h"

#include <mitkILifecycleAwarePart.h>
#include <berryIPartListener.h>

#include <berryIPreferences.h>
#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>
#include <DisplayCoordinateSupplier.h>
#include <usServiceRegistration.h>

#include <ctkPluginContext.h>

//#include <QTimer>
#include <memory>

// There's an item "ToolsPluginControls.ui" in the UI_FILES list in
// files.cmake. The Qt UI Compiler will parse this file and generate a
// header file prefixed with "ui_", which is located in the build directory.
// Use Qt Creator to view and edit .ui files. The generated header file
// provides a class that contains all of the UI widgets.
#include <ui_ToolsPluginControls.h>

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

struct TagTree;
using TagNode = shared_ptr<TagTree>;
using StatData = vector<mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer>;

// All views in MITK derive from QmitkAbstractView. You have to override
// at least the two methods CreateQtPartControl() and SetFocus().
class ToolsPlugin : public QmitkAbstractView, public mitk::ILifecycleAwarePart//, public berry::IPartListener
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
	static const int STAT_TABLE_BASE_HEIGHT;

	ToolsPlugin();
	~ToolsPlugin();

	// In this method we initialize the GUI components and connect the
	// associated signals and slots.
	void CreateQtPartControl(QWidget* parent) override;
	void updateAfterSelectionChanged();
	void updateTags();
	shared_ptr<QList<QTreeWidgetItem*>> createTagItems(TagNode tree, const string& prefix = "");
	int setTagCounts(QTreeWidgetItem* item);
	void updateTagRepresentation();
	void updateShowPatientData();
	void updateShowStatistics();
	void updateStatistics(const StatData& statistics, mitk::Image* image);
	void updateEditableControls(bool update_tags = true);

protected:
	void NodeAdded(const mitk::DataNode* node) override;
	void NodeRemoved(const mitk::DataNode* node) override;

protected:
	void Activated() override;
	void Deactivated() override;
	void Visible() override;
	void Hidden() override;

	// Typically a one-liner. Set the focus to the default widget.
	void SetFocus() override;
	// This method is conveniently called whenever the selection of Data Manager items changes.
	void OnSelectionChanged(berry::IWorkbenchPart::Pointer source, const QList<mitk::DataNode::Pointer>& dataNodes) override;
	void OnPreferencesChanged(const berry::IBerryPreferences*) override;

protected:
	void SelectionChanged(mitk::DataNode* dataNode);
	/// Method called when itkModifiedEvent is called by selected data.
	void SelectedDataModified();

private:
	//mitk::DataNode* GetWorkingNode();
	using DataNodeList = list<mitk::DataNode::Pointer>;
	mitk::DataNode* getFirstSelectedNode(shared_ptr<DataNodeList> dataNodes = nullptr);
	void enable3DRepresentation(bool flag);
	void updateCrosshair();

private:
	// Generated from the associated UI file, it encapsulates all the widgets of our view.
	Ui::ToolsPluginControls ui;
	mitk::ToolManager* m_ToolManager;
	berry::IPreferences::Pointer m_ToolsPluginPreferencesNode;
	mitk::DataNode* m_SelectedNode = nullptr;
	ImageStatisticsCalculationThread* m_CalculationThread;
	std::unique_ptr<DisplayCoordinateSupplier> m_DisplayCoordinateSupplier;
	us::ServiceRegistration<mitk::InteractionEventObserver> m_ServiceRegistration;
	long m_TimeObserverTag;
	mitk::Image* m_StatisticsImage = nullptr;
	bool m_StatisticsUpdatePending;
	map<size_t, StatData> m_statistics;
	bool m_DataNodeSelectionChanged;
	//bool m_Visible;

	static const int num_percentiles = 100 + 1;

	enum TagTreeColumns
	{
		NAME_TAG = 0,
		VALUE_TAG = 1
	};

signals:
	void Representation3DHasToBeInitiated(const ctkDictionary&);
	void UpdateCrosshair(const ctkDictionary&);
	void NodeHasManyImages(const ctkDictionary&);
	void SetLevelWindowRange(const ctkDictionary&);
	void SelecedDataNodeChanged(const ctkDictionary&);
	void AllNodesRemoved(const ctkDictionary&);

	/// Method to set the data to the member and start the threaded statistics update
	void StatisticsUpdate();

public slots:
	void on_pixel_selected(QVector<double> point);
	void on_MainWindow_Representation3D_changed(const ctkEvent& event);

protected slots:
	void on_ThreadedStatisticsCalculation_finished();
	/// Checks if update is possible and calls StatisticsUpdate() possible
	void RequestStatisticsUpdate();

private slots:
	void on_checkbox_EnableVolumeRendering_toggled(bool volRen);
	void on_checkBox_ShowPatientData_toggled(bool);
	void on_checkBox_GroupTags_toggled(bool);
	void on_checkBox_ShowStatistics_toggled(bool);
	/// Is called from the image navigator once the time step has changed
	void on_imageNavigator_timeChanged(const itk::EventObject&);
};

#endif
