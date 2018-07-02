
#include "ShowViewAction.h"

#include "berryUIException.h"
#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIProduct.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>

ViewInfo::ViewInfo(berry::IViewDescriptor::Pointer view)
{
	assert(view != nullptr);
	view_desc = view;
	auto view_id = view_desc->GetId();
	// Find appropriate PluginDescriptor
	plugin_desc = Elements::find_plugin(view_id);
}
QString ViewInfo::Id() const
{
	return view_desc->GetId();
}
QString ViewInfo::Name() const
{
	QString name = (plugin_desc != nullptr) ? plugin_desc->name : view_desc->GetLabel();
	return name;
}
int ViewInfo::Order() const
{
	int order = (plugin_desc != nullptr) ? plugin_desc->order : 999;
	return order;
}
void ViewInfo::ConfigureTitle(berry::IViewPart::Pointer view) const
{
	if (view == nullptr)
		return;

	auto pt = view.Cast<berry::WorkbenchPart>();
	if (pt == nullptr)
		return;

	auto name = Name();
	if (plugin_desc != nullptr && plugin_desc->show_title)
		pt->SetPartName(name);
	else
		pt->SetPartName(" ");
	pt->SetTitleToolTip(name);
}

ShowViewAction::ShowViewAction(berry::IWorkbenchWindow::Pointer window, pViewInfo viewInfo)
	: QAction(nullptr)
{
	this->setParent(static_cast<QWidget*>(window->GetShell()->GetControl()));
	m_Window = window.GetPointer();
	m_Info = viewInfo;

	// Set name
	QString name = m_Info->Name();
	this->setText(name);
	this->setToolTip(name);
	this->setIconVisibleInMenu(true);

	// Set icon
	auto desc = m_Info->view_desc;
	if (desc != nullptr)
	{
		QIcon icon = desc->GetImageDescriptor();
		this->setIcon(icon);
	}

	// Connect
	this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}
void ShowViewAction::Run()
{
	berry::IWorkbenchPage::Pointer page = m_Window->GetActivePage();
	if (page == nullptr)
		return;

	// Toggle
	try
	{
		QString view_id = m_Info->Id();
		auto view = page->FindView(view_id);
		// If this view is already visible, just return.
		if (view == nullptr)
		{
			try
			{
				view = page->ShowView(view_id);
				m_Info->ConfigureTitle(view);
			}
			catch (const berry::PartInitException& e)
			{
				BERRY_ERROR << "Error: " << e.what();
			}
		}
		else
		{// Hide view:
			//? SafeHandleNavigatorView(view_id);
			//bool isViewVisible = page->IsPartVisible(view);
			//if (isViewVisible)
			//{
				page->HideView(view);
			//}
		}
	}
	catch (const berry::PartInitException& e)
	{
		BERRY_ERROR << "Error: " << e.what();
	}
}
