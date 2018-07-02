
#include "StepSelector.h"

#include <berryISelectionService.h>
#include "berryUIException.h"
#include <berryWorkbenchPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>

#include <usModuleRegistry.h>
#include <QmitkDataStorageComboBox.h>

#include <QMessageBox>
#include <QPainter>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>

#include "mitkLabel.h"
#include "mitkToolManagerProvider.h"
#include "mitkOtsuSegmentationFilter.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkIOUtil.h"

// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usModuleResource.h"

#include "itkImageRegionIterator.h"
#include "mitkImageCast.h"
#include <mitkITKImageImport.h>
#include <mitkPaintbrushTool.h>

#include <itkBinaryThresholdImageFilter.h>


// Don't forget to initialize the VIEW_ID.
const string StepSelector::VIEW_ID = "my.organprint.views.stepselector";


StepDescriptor::StepDescriptor(const QString& pluginId, QPushButton* button)
	: pluginId(pluginId), button(button)
{}



void StepSelector::CreateQtPartControl(QWidget* parent)
{
	ui.setupUi(parent);
	this->GetRenderWindowPart(OPEN);
	this->RequestRenderWindowUpdate();

	// Wire up the UI widgets with our functionality.
	m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
	assert(m_ToolManager);

	// Set steps
	m_steps =
	{
		{ "my.awesomeproject.views.awesomeview",	ui.pushButton_1 },
		{ "org.mitk.views.basicimageprocessing",	ui.pushButton_2 },
		{ "org.mitk.views.segmentationutilities", 	ui.pushButton_3 },
		{ "org.mitk.views.properties", 				ui.pushButton_4 }
	};

	// Connect Signals and Slots of the Plugin UI
	for (unsigned long i = 0; i < m_steps.size(); i++)
	{
		connect(m_steps[i].button, &QPushButton::clicked, this, [this, i]{ on_pushButton_clicked(i); });
	}

	// Initialize the first step
	//selectView(0);
}

StepSelector::StepSelector()
	: m_currentStep(0)
{
}

StepSelector::~StepSelector()
{
}

void StepSelector::selectView(int n)
{
	if (n < 0 || n >= (int)m_steps.size())
		return;
	auto site = this->GetSite();
	if (site == nullptr)
		return;
	auto workbenchWindow = site->GetWorkbenchWindow();
	if (workbenchWindow == nullptr)
		return;
	auto page = workbenchWindow->GetActivePage();
	if (page == nullptr)
		return;

	m_currentStep = n;

	for (int i = 0; i < (int) m_steps.size(); i++)
	{
		const auto& step = m_steps[i];
		berry::IViewPart::Pointer view = page->FindView(step.pluginId);
		try
		{
			if (i == m_currentStep && view == nullptr)
			{// Open
				view = page->ShowView(step.pluginId);
				step.button->setStyleSheet("background-color: #3399cc");
			}
			else if (i != m_currentStep && view != nullptr)
			{// Close
				page->HideView(view);
				step.button->setStyleSheet("");
			}
		}
		catch (const berry::PartInitException& e)
		{
			BERRY_ERROR << "Error: " << e.what();
		}
	}
}

void StepSelector::SetFocus()
{
}

void StepSelector::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& dataNodes)
{
	
	cout << &dataNodes << endl;

}

void StepSelector::on_pushButton_clicked(int step)
{
	selectView(step);
}
