#include "PopeRenderWindowEditor.h"
#include "MainWindow.h"
#include "DataManager.h"
#include "PopeRenderWindowEditorPrivate.h"
#include "QmitkStdMultiWidgetPartListener.h"
#include "inova_popeproject_editors_renderwindow_Activator.h"


#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPartListener.h>
#include <berryIPreferences.h>
#include <berryIStructuredSelection.h>
#include <berryISelection.h>
#include <berryISelectionProvider.h>
#include <berryIWorkbenchPartSite.h>

#include <QWidget>

#include <mitkColorProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>
#include <mitkRegistrationHelper.h>
#include <mitkDataNodeObject.h>

#include <QmitkMouseModeSwitcher.h>
#include <QmitkStdMultiWidget.h>

const QString PopeRenderWindowEditor::EDITOR_ID = "inova.popeproject.editors.renderwindow";


PopeRenderWindowEditor::PopeRenderWindowEditor()
    : d(new PopeRenderWindowEditorPrivate)
{}

PopeRenderWindowEditor::~PopeRenderWindowEditor()
{
    berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
    s->RemoveSelectionListener(m_SelectionListener.data());

    this->GetSite()->GetPage()->RemovePartListener(d->m_PartListener.data());
}

QmitkStdMultiWidget* PopeRenderWindowEditor::GetStdMultiWidget()
{
    return d->m_StdMultiWidget;
}

QmitkRenderWindow *PopeRenderWindowEditor::GetActiveQmitkRenderWindow() const
{
    if (d->m_StdMultiWidget)
        return d->m_StdMultiWidget->GetRenderWindow1();
    return 0;
}

QHash<QString, QmitkRenderWindow*> PopeRenderWindowEditor::GetQmitkRenderWindows() const
{
    return d->m_RenderWindows;
}

QmitkRenderWindow *PopeRenderWindowEditor::GetQmitkRenderWindow(const QString &id) const
{
    if (d->m_RenderWindows.contains(id))
        return d->m_RenderWindows[id];

    return 0;
}

mitk::Point3D PopeRenderWindowEditor::GetSelectedPosition(const QString & /*id*/) const
{
    return d->m_StdMultiWidget->GetCrossPosition();
}

void PopeRenderWindowEditor::SetSelectedPosition(const mitk::Point3D &pos, const QString &/*id*/)
{
    d->m_StdMultiWidget->MoveCrossToPosition(pos);
}

void PopeRenderWindowEditor::EnableDecorations(bool enable, const QStringList &decorations)
{
    if (decorations.isEmpty() || decorations.contains(DECORATION_BORDER))
    {
        enable ? d->m_StdMultiWidget->EnableColoredRectangles()	 : d->m_StdMultiWidget->DisableColoredRectangles();
    }
    if (decorations.isEmpty() || decorations.contains(DECORATION_LOGO))
    {
        enable ? d->m_StdMultiWidget->EnableDepartmentLogo() : d->m_StdMultiWidget->DisableDepartmentLogo();
    }
    if (decorations.isEmpty() || decorations.contains(DECORATION_MENU))
    {
        d->m_StdMultiWidget->ActivateMenuWidget(enable);
    }
    if (decorations.isEmpty() || decorations.contains(DECORATION_BACKGROUND))
    {
        enable ? d->m_StdMultiWidget->EnableGradientBackground() : d->m_StdMultiWidget->DisableGradientBackground();
    }
    if (decorations.isEmpty() || decorations.contains(DECORATION_CORNER_ANNOTATION))
    {
        enable ? d->m_StdMultiWidget->SetCornerAnnotationVisibility(true) : d->m_StdMultiWidget->SetCornerAnnotationVisibility(false);
    }
}

bool PopeRenderWindowEditor::IsDecorationEnabled(const QString &decoration) const
{
    if (decoration == DECORATION_BORDER)
    {
        return d->m_StdMultiWidget->IsColoredRectanglesEnabled();
    }
    else if (decoration == DECORATION_LOGO)
    {
        return d->m_StdMultiWidget->IsColoredRectanglesEnabled();
    }
    else if (decoration == DECORATION_MENU)
    {
        return d->m_StdMultiWidget->IsMenuWidgetEnabled();
    }
    else if (decoration == DECORATION_BACKGROUND)
    {
        return d->m_StdMultiWidget->GetGradientBackgroundFlag();
    }
    else if (decoration == DECORATION_CORNER_ANNOTATION)
    {
        return d->m_StdMultiWidget->IsCornerAnnotationVisible();
    }

    return false;
}

QStringList PopeRenderWindowEditor::GetDecorations() const
{
    QStringList decorations;
    decorations << DECORATION_BORDER << DECORATION_LOGO << DECORATION_MENU << DECORATION_BACKGROUND << DECORATION_CORNER_ANNOTATION;
    return decorations;
}

void PopeRenderWindowEditor::EnableSlicingPlanes(bool enable)
{
    d->m_StdMultiWidget->SetWidgetPlanesVisibility(enable);
}

bool PopeRenderWindowEditor::IsSlicingPlanesEnabled() const
{
    mitk::DataNode::Pointer node = this->d->m_StdMultiWidget->GetWidgetPlane1();
    if (node.IsNotNull())
    {
        bool visible = false;
        node->GetVisibility(visible, 0);
        return visible;
    }
    else
    {
        return false;
    }
}

void PopeRenderWindowEditor::CreateQtPartControl(QWidget* parent)
{
    m_SelectionListener.reset(new berry::SelectionChangedAdapter<PopeRenderWindowEditor>(this, &PopeRenderWindowEditor::OnSelectionChanged));
    berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
    s->AddSelectionListener(m_SelectionListener.data());

    if (d->m_StdMultiWidget == 0)
    {
        QHBoxLayout* layout = new QHBoxLayout(parent);
        layout->setContentsMargins(0,0,0,0);

        if (d->m_MouseModeToolbar == nullptr)
        {
            d->m_MouseModeToolbar = new QmitkMouseModeSwitcher(parent); // delete by Qt via parent
            layout->addWidget(d->m_MouseModeToolbar);
        }

        berry::IPreferences::Pointer prefs = this->GetPreferences();


        mitk::BaseRenderer::RenderingMode::Type renderingMode = static_cast<mitk::BaseRenderer::RenderingMode::Type>(prefs->GetInt( "Rendering Mode", 0 ));

        d->m_StdMultiWidget = new QmitkStdMultiWidget(parent,0,0,renderingMode);
        d->m_RenderWindows.insert("axial", d->m_StdMultiWidget->GetRenderWindow1());
        d->m_RenderWindows.insert("sagittal", d->m_StdMultiWidget->GetRenderWindow2());
        d->m_RenderWindows.insert("coronal", d->m_StdMultiWidget->GetRenderWindow3());
        d->m_RenderWindows.insert("3d", d->m_StdMultiWidget->GetRenderWindow4());

        d->m_centralWindow = new MainWindow(d->m_StdMultiWidget, parent);

        d->m_MouseModeToolbar->setMouseModeSwitcher( d->m_StdMultiWidget->GetMouseModeSwitcher() );

        layout->addWidget(d->m_centralWindow);

        mitk::DataStorage::Pointer ds = this->GetDataStorage();

        m_DataManager = std::make_unique<DataManager>(ds.GetPointer());
        connect(d->m_centralWindow, &MainWindow::ImageSetHasToBeLoaded, m_DataManager.get(), &DataManager::on_LoadImageSet);
        connect(d->m_centralWindow, &MainWindow::ImageFolderHasToBeLoaded, m_DataManager.get(), &DataManager::on_LoadImageFolder);
        connect(d->m_centralWindow, &MainWindow::EnableAutoRotation, this, &PopeRenderWindowEditor::on_EnableAutoRotation);
        connect(m_DataManager.get(), &DataManager::GonnaAddNewDataNode, d->m_centralWindow, &MainWindow::On_DataManager_GonnaAddNewDataNode);
        connect(m_DataManager.get(), &DataManager::NewDataNodeAdded, d->m_centralWindow, &MainWindow::On_DataManager_NewDataNodeAdded);

        // Tell the multiWidget which (part of) the tree to render
        d->m_StdMultiWidget->SetDataStorage(ds);

        // Initialize views as axial, sagittal, coronar to all data objects in DataStorage
        // (from top-left to bottom)
        auto geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
        mitk::RenderingManager::GetInstance()->InitializeViews(geo);

        // Initialize bottom-right view as 3D view
        d->m_StdMultiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

        // Enable standard handler for levelwindow-slider
        d->m_StdMultiWidget->EnableStandardLevelWindow();
        d->m_centralWindow->RegisterLevelWindowObserver();

        // Add the displayed views to the tree to see their positions
        // in 2D and 3D
        d->m_StdMultiWidget->AddDisplayPlaneSubTree();

        //d->m_StdMultiWidget->EnableNavigationControllerEventListening();

        // Store the initial visibility status of the menu widget.
        d->m_MenuWidgetsEnabled = d->m_StdMultiWidget->IsMenuWidgetEnabled();

        this->GetSite()->GetPage()->AddPartListener(d->m_PartListener.data());

        berry::IBerryPreferences* berryprefs = dynamic_cast<berry::IBerryPreferences*>(prefs.GetPointer());
        InitializePreferences(berryprefs);
        berryprefs->PutBool("Show helper objects", true);
        berryprefs->PutBool("Show nodes containing no data", true);
        berryprefs->PutBool("enable developer mode",false);
        this->OnPreferencesChanged(berryprefs);

        this->RequestUpdate();
    }
}

void PopeRenderWindowEditor::OnSelectionChanged(const berry::IWorkbenchPart::Pointer& sourcepart, const berry::ISelection::ConstPointer& selection)
{
    if (sourcepart == this || selection.Cast<const berry::IStructuredSelection>() == nullptr)
        return;

    auto current_selection = selection.Cast<const berry::IStructuredSelection>(); // this->GetSite()->GetSelectionProvider()->GetSelection()
    //auto selectedDataNodes = current_selection->ToVector();

    // Make invisible all the nodes but the first selected node
    auto pluginContext = inova_popeproject_editors_renderwindow_Activator::GetPluginContext();
    ctkServiceReference serviceReference = pluginContext->getServiceReference<mitk::IDataStorageService>();
    mitk::IDataStorageService* storageService = pluginContext->getService<mitk::IDataStorageService>(serviceReference);
    mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();
    auto all_nodes = dataStorage->GetAll();
    for (auto datanode : *all_nodes)
    {
        mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
        mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(datanode->GetData());
        bool is_registration = (mitk::MITKRegistrationHelper::IsRegNode(datanode) && datanode->GetData());
        if (!image && !pointSet && !is_registration)
            continue;

        //bool is_visible = false;
        //bool is_property_found = datanode->GetBoolProperty("visible", is_visible);
        //bool is_selected_node = (datanode.GetPointer() == firstSelectedNode);
        //if (is_property_found && is_visible)
        //	datanode->SetBoolProperty("visible", is_selected_node);
        bool is_selected_node = false;
        //for (auto selected_node : *selectedDataNodes)
        for (auto it = current_selection->Begin(); it != current_selection->End(); ++it)
        {
            auto selected_node = *it;
            auto selected_datanode = selected_node.Cast<const mitk::DataNodeObject>();
            if (selected_datanode != nullptr && datanode.GetPointer() == selected_datanode->GetDataNode().GetPointer())
            {
                is_selected_node = true;
                break;
            }
        }
        datanode->SetBoolProperty("visible", is_selected_node);
    }
    this->RequestUpdate();
}

void PopeRenderWindowEditor::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
    //Update internal members
    this->FillMembersWithCurrentDecorations();
    this->GetPreferenceDecorations(prefs);
    //Now the members can be used to modify the stdmultiwidget
    mitk::Color upper = HexColorToMitkColor(d->m_WidgetBackgroundColor1[0]);
    mitk::Color lower = HexColorToMitkColor(d->m_WidgetBackgroundColor2[0]);
    d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 0);
    upper = HexColorToMitkColor(d->m_WidgetBackgroundColor1[1]);
    lower = HexColorToMitkColor(d->m_WidgetBackgroundColor2[1]);
    d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 1);
    upper = HexColorToMitkColor(d->m_WidgetBackgroundColor1[2]);
    lower = HexColorToMitkColor(d->m_WidgetBackgroundColor2[2]);
    d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 2);
    upper = HexColorToMitkColor(d->m_WidgetBackgroundColor1[3]);
    lower = HexColorToMitkColor(d->m_WidgetBackgroundColor2[3]);
    d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 3);
    d->m_StdMultiWidget->EnableGradientBackground();

    // preferences for renderWindows
    mitk::Color colorDecorationWidget1 = HexColorToMitkColor(d->m_WidgetDecorationColor[0]);
    mitk::Color colorDecorationWidget2 = HexColorToMitkColor(d->m_WidgetDecorationColor[1]);
    mitk::Color colorDecorationWidget3 = HexColorToMitkColor(d->m_WidgetDecorationColor[2]);
    mitk::Color colorDecorationWidget4 = HexColorToMitkColor(d->m_WidgetDecorationColor[3]);
    d->m_StdMultiWidget->SetDecorationColor(0, colorDecorationWidget1);
    d->m_StdMultiWidget->SetDecorationColor(1, colorDecorationWidget2);
    d->m_StdMultiWidget->SetDecorationColor(2, colorDecorationWidget3);
    d->m_StdMultiWidget->SetDecorationColor(3, colorDecorationWidget4);

    for (unsigned int i = 0; i < 4; ++i)
    {
        d->m_StdMultiWidget->SetDecorationProperties(d->m_WidgetAnnotation[i].toStdString(), HexColorToMitkColor(d->m_WidgetDecorationColor[i]), i);
    }
    //The crosshair gap
    int crosshairgapsize = prefs->GetInt("crosshair gap size", 0);
    d->m_StdMultiWidget->GetWidgetPlane1()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);
    d->m_StdMultiWidget->GetWidgetPlane2()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);
    d->m_StdMultiWidget->GetWidgetPlane3()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);

    //refresh colors of rectangles
    d->m_StdMultiWidget->DisableColoredRectangles();
    d->m_StdMultiWidget->DisableDepartmentLogo();

    // Set preferences respecting zooming and panning
    bool constrainedZooming = prefs->GetBool("Use constrained zooming and panning", true);

    mitk::RenderingManager::GetInstance()->SetConstrainedPanningZooming(constrainedZooming);

    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    // level window setting
    bool showLevelWindowWidget = prefs->GetBool("Show level/window widget", true);
    if (showLevelWindowWidget)
    {
        d->m_StdMultiWidget->EnableStandardLevelWindow();
    }
    else
    {
        d->m_StdMultiWidget->DisableStandardLevelWindow();
    }

    // mouse modes toolbar
    bool newMode = prefs->GetBool("PACS like mouse interaction", false);
    d->m_MouseModeToolbar->setVisible( newMode );
    d->m_StdMultiWidget->GetMouseModeSwitcher()->SetInteractionScheme( newMode ? mitk::MouseModeSwitcher::PACS : mitk::MouseModeSwitcher::MITK );
}

mitk::Color PopeRenderWindowEditor::HexColorToMitkColor(const QString& widgetColorInHex)
{
    QColor qColor(widgetColorInHex);
    mitk::Color returnColor;
    float colorMax = 255.0f;
    if (widgetColorInHex.isEmpty()) // default value
    {
        returnColor[0] = 1.0;
        returnColor[1] = 1.0;
        returnColor[2] = 1.0;
        MITK_ERROR << "Using default color for unknown widget " << qPrintable(widgetColorInHex);
    }
    else
    {
        returnColor[0] = qColor.red() / colorMax;
        returnColor[1] = qColor.green() / colorMax;
        returnColor[2] = qColor.blue() / colorMax;
    }
    return returnColor;
}

QString PopeRenderWindowEditor::MitkColorToHex(const mitk::Color& color)
{
    QColor returnColor;
    float colorMax = 255.0f;
    returnColor.setRed(static_cast<int>(color[0]* colorMax + 0.5));
    returnColor.setGreen(static_cast<int>(color[1]* colorMax + 0.5));
    returnColor.setBlue(static_cast<int>(color[2]* colorMax + 0.5));
    return returnColor.name();
}

void PopeRenderWindowEditor::FillMembersWithCurrentDecorations()
{
    //fill members with current values (or default values) from the std multi widget
    for (unsigned int i = 0; i < 4; ++i)
    {
        d->m_WidgetDecorationColor[i] = MitkColorToHex(d->m_StdMultiWidget->GetDecorationColor(i));
        d->m_WidgetBackgroundColor1[i] = MitkColorToHex(d->m_StdMultiWidget->GetGradientColors(i).first);
        d->m_WidgetBackgroundColor2[i] = MitkColorToHex(d->m_StdMultiWidget->GetGradientColors(i).second);
        d->m_WidgetAnnotation[i] = QString::fromStdString(d->m_StdMultiWidget->GetCornerAnnotationText(i));
    }
}

void PopeRenderWindowEditor::GetPreferenceDecorations(const berry::IBerryPreferences * preferences)
{
    //overwrite members with values from the preferences, if they the prefrence is defined
    d->m_WidgetBackgroundColor1[0] = preferences->Get("widget1 first background color", d->m_WidgetBackgroundColor1[0]);
    d->m_WidgetBackgroundColor2[0] = preferences->Get("widget1 second background color", d->m_WidgetBackgroundColor2[0]);
    d->m_WidgetBackgroundColor1[1] = preferences->Get("widget2 first background color", d->m_WidgetBackgroundColor1[1]);
    d->m_WidgetBackgroundColor2[1] = preferences->Get("widget2 second background color", d->m_WidgetBackgroundColor2[1]);
    d->m_WidgetBackgroundColor1[2] = preferences->Get("widget3 first background color", d->m_WidgetBackgroundColor1[2]);
    d->m_WidgetBackgroundColor2[2] = preferences->Get("widget3 second background color", d->m_WidgetBackgroundColor2[2]);
    d->m_WidgetBackgroundColor1[3] = preferences->Get("widget4 first background color", d->m_WidgetBackgroundColor1[3]);
    d->m_WidgetBackgroundColor2[3] = preferences->Get("widget4 second background color", d->m_WidgetBackgroundColor2[3]);

    d->m_WidgetDecorationColor[0] = preferences->Get("widget1 decoration color", d->m_WidgetDecorationColor[0]);
    d->m_WidgetDecorationColor[1] = preferences->Get("widget2 decoration color", d->m_WidgetDecorationColor[1]);
    d->m_WidgetDecorationColor[2] = preferences->Get("widget3 decoration color", d->m_WidgetDecorationColor[2]);
    d->m_WidgetDecorationColor[3] = preferences->Get("widget4 decoration color", d->m_WidgetDecorationColor[3]);

    d->m_WidgetAnnotation[0] = preferences->Get("widget1 corner annotation", d->m_WidgetAnnotation[0]);
    d->m_WidgetAnnotation[1] = preferences->Get("widget2 corner annotation", d->m_WidgetAnnotation[1]);
    d->m_WidgetAnnotation[2] = preferences->Get("widget3 corner annotation", d->m_WidgetAnnotation[2]);
    d->m_WidgetAnnotation[3] = preferences->Get("widget4 corner annotation", d->m_WidgetAnnotation[3]);
}

void PopeRenderWindowEditor::InitializePreferences(berry::IBerryPreferences * preferences)
{
    this->FillMembersWithCurrentDecorations(); //fill members
    this->GetPreferenceDecorations(preferences); //overwrite if preferences are defined

    //create new preferences
    preferences->Put("widget1 corner annotation", d->m_WidgetAnnotation[0]);
    preferences->Put("widget2 corner annotation", d->m_WidgetAnnotation[1]);
    preferences->Put("widget3 corner annotation", d->m_WidgetAnnotation[2]);
    preferences->Put("widget4 corner annotation", d->m_WidgetAnnotation[3]);

    preferences->Put("widget1 decoration color", d->m_WidgetDecorationColor[0]);
    preferences->Put("widget2 decoration color", d->m_WidgetDecorationColor[1]);
    preferences->Put("widget3 decoration color", d->m_WidgetDecorationColor[2]);
    preferences->Put("widget4 decoration color", d->m_WidgetDecorationColor[3]);

    preferences->Put("widget1 first background color", d->m_WidgetBackgroundColor1[0]);
    preferences->Put("widget2 first background color", d->m_WidgetBackgroundColor1[1]);
    preferences->Put("widget3 first background color", d->m_WidgetBackgroundColor1[2]);
    preferences->Put("widget4 first background color", d->m_WidgetBackgroundColor1[3]);
    preferences->Put("widget1 second background color", d->m_WidgetBackgroundColor2[0]);
    preferences->Put("widget2 second background color", d->m_WidgetBackgroundColor2[1]);
    preferences->Put("widget3 second background color", d->m_WidgetBackgroundColor2[2]);
    preferences->Put("widget4 second background color", d->m_WidgetBackgroundColor2[3]);
}

void PopeRenderWindowEditor::SetFocus()
{
    if (d->m_StdMultiWidget != 0)
        d->m_StdMultiWidget->setFocus();
}

void PopeRenderWindowEditor::RequestActivateMenuWidget(bool on)
{
    if (d->m_StdMultiWidget)
    {
        if (on)
        {
            d->m_StdMultiWidget->ActivateMenuWidget(d->m_MenuWidgetsEnabled);
        }
        else
        {
            d->m_MenuWidgetsEnabled = d->m_StdMultiWidget->IsMenuWidgetEnabled();
            d->m_StdMultiWidget->ActivateMenuWidget(false);
        }
    }
}

void PopeRenderWindowEditor::on_EnableAutoRotation(bool flag)
{
    this->GetStdMultiWidget()->mitkWidget4->EnableAutoRotation(flag);
	//auto w = this->GetQmitkRenderWindow("stdmulti.widget4");
    //rw->EnableAutoRotation(flag);
}
