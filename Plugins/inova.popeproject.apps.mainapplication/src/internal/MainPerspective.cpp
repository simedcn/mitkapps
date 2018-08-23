#include "MainPerspective.h"
#include "../../Classes/PluginDescriptors.h"
#include "inova_popeproject_apps_mainapplication_Activator.h"

// Berry
#include <berryIViewLayout.h>
#include <berryIFolderLayout.h>
#include <berryIQtStyleManager.h>
#include <berryIEditorInput.h>
#include <berryFileEditorInput.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindowConfigurer.h>
#include <berryPlatformUI.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>
#include <berryQtPreferences.h>

enum CloseableMoveablePlugins : int
{
	NotForAll = 0,
	YesForAllButMain = 1,
	YesForAll = 2
};


MainPerspective::MainPerspective()
{
}

void MainPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
	QString editorArea = layout->GetEditorArea();
	//layout->AddView("org.mitk.views.datamanager", berry::IPageLayout::LEFT, 0.22f, editorArea);
	auto top_left = layout->CreateFolder("top_left", berry::IPageLayout::LEFT, 0.24f, editorArea);
	auto right = layout->CreateFolder("right", berry::IPageLayout::RIGHT, 0.67f, editorArea);
	auto bottom = layout->CreateFolder("bottom", berry::IPageLayout::BOTTOM, 0.7f, editorArea);
	auto mid_left = layout->CreateFolder("mid_left", berry::IPageLayout::BOTTOM, 0.4f, "top_left");
	auto bottom_left = layout->CreateFolder("bottom_left", berry::IPageLayout::BOTTOM, 0.5f, "mid_left");
	//auto bottom_right = layout->CreateFolder("bottom_right", berry::IPageLayout::BOTTOM, 0.78f, "right");
	QString current_selector_item;
	for (const auto& plugin : PluginDescriptors::get())
	{
		if (plugin.role == PluginDescriptor::PluginRole_selector)
		{
			current_selector_item = plugin.id;
		}
		if (current_selector_item.isEmpty() || plugin.role != PluginDescriptor::PluginRole_selectorItem)
		{
			switch (plugin.position)
			{
			//case PluginDescriptor::PluginPosistion::PluginPosistion_bottom_right:
			//	bottom_right->AddView(plugin.id);
			//	break;
			case PluginDescriptor::PluginPosistion::PluginPosistion_bottom:
				bottom->AddView(plugin.id);
				break;
			case PluginDescriptor::PluginPosistion::PluginPosistion_top_left:
				top_left->AddView(plugin.id);
				break;
			case PluginDescriptor::PluginPosistion::PluginPosistion_mid_left:
				mid_left->AddView(plugin.id);
				break;
			case PluginDescriptor::PluginPosistion::PluginPosistion_bottom_left:
				bottom_left->AddView(plugin.id);
				break;
			case PluginDescriptor::PluginPosistion::PluginPosistion_right:
			default:
				right->AddView(plugin.id);
				break;
			}
		}
		else
		{
			int pos =
				plugin.position == PluginDescriptor::PluginPosistion_bottom ? berry::IPageLayout::BOTTOM :
				plugin.position == PluginDescriptor::PluginPosistion_top_left ? berry::IPageLayout::LEFT :
				plugin.position == PluginDescriptor::PluginPosistion_right ? berry::IPageLayout::RIGHT :
				berry::IPageLayout::RIGHT;
			layout->AddStandaloneView(plugin.id, false, pos, 0.20f, current_selector_item);
			current_selector_item = plugin.id;
		}
	}
	//right->AddView("org.mitk.views.imagestatistics"); --> crashing
	//right->AddView("org.mitk.views.aicpregistration"); --> no surface

	//  layout->GetViewLayout("inova.popeproject.editors.renderwindow");
	//  editor->SetCloseable(false);
	//  editor->SetMoveable(false);

	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	auto preferencesNode = prefService->GetSystemPreferences()->Node("/inova.popeproject.editors.renderwindow");
	int closeable_plugins = preferencesNode->GetInt("closeable plugins", CloseableMoveablePlugins::NotForAll);
	bool is_closeable = (closeable_plugins != NotForAll);
	bool is_main_closeable = (closeable_plugins != NotForAll && closeable_plugins != YesForAllButMain);
	int moveable_plugins = preferencesNode->GetInt("moveable plugins", CloseableMoveablePlugins::YesForAllButMain);//!! not only here
	bool is_moveable = (moveable_plugins != NotForAll);
	bool is_main_moveable = (moveable_plugins != NotForAll && moveable_plugins != YesForAllButMain);

	//berry::IViewLayout::Pointer view;
	for (const auto& plugin : PluginDescriptors::get())
	{
		auto view = layout->GetViewLayout(plugin.id);
		bool closeable = 
			(plugin.role == PluginDescriptor::PluginRole_main) ? is_main_closeable :
			(plugin.role == PluginDescriptor::PluginRole_secondary) ? is_closeable :
			(plugin.role == PluginDescriptor::PluginRole_pacs) ? true :
			(plugin.role == PluginDescriptor::PluginRole_selector) ? is_main_closeable :
			(plugin.role == PluginDescriptor::PluginRole_selectorItem) ? false :
			true;
		bool moveable = 
			(plugin.role == PluginDescriptor::PluginRole_main) ? is_main_moveable : 
			(plugin.role == PluginDescriptor::PluginRole_selector) ? false : 
			(plugin.role == PluginDescriptor::PluginRole_selectorItem) ? false : 
			is_moveable;
		view->SetCloseable(closeable);
		view->SetMoveable(moveable);
	}
	//layout->GetViewLayout("org.mitk.views.datamanager");
	//layout->SetFixed(true);
  
	ctkPluginContext* context = inova_popeproject_apps_mainapplication_Activator::GetContext();
	ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
	if (styleManagerRef)
	{
		auto styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
		//berry::IQtStyleManager::StyleList styles;
		//styleManager->GetStyles(styles);
		//for (auto s : styles)
		//	MITK_INFO << s.name << " " << s.fileName;
		QString darkStyle = ":/org.blueberry.ui.qt/darkstyle.qss";
		styleManager->SetStyle(darkStyle);
		berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
		if (prefService != nullptr)
		{
			auto systemPrefs = prefService->GetSystemPreferences();
			if (systemPrefs != nullptr)
			{
				berry::IPreferences::Pointer stylePref = systemPrefs->Node(berry::QtPreferences::QT_STYLES_NODE);
				if (stylePref != nullptr)
				{
					stylePref->Put(berry::QtPreferences::QT_STYLE_NAME, darkStyle);
				}
			}
		}
	}

	/// Configure plugins
	//layout->GetViewLayout("org.mitk.views.segmentation");
	//layout->
}
