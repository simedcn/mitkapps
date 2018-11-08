
#include "OrganPrintPerspective.h"
#include "berryIViewLayout.h"

OrganPrintPerspective::OrganPrintPerspective()
{
}
 
OrganPrintPerspective::OrganPrintPerspective(const OrganPrintPerspective& other)
: QObject()
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

void OrganPrintPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  QString editorArea = layout->GetEditorArea();

  layout->AddView("org.mitk.views.datamanager", berry::IPageLayout::LEFT, 0.3f, editorArea);

  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.datamanager");
  lo->SetCloseable(false);

  layout->AddView("org.mitk.views.imagenavigator", berry::IPageLayout::BOTTOM, 0.5f, "org.mitk.views.datamanager");

  berry::IFolderLayout::Pointer bottomFolder = layout->CreateFolder("bottom", berry::IPageLayout::BOTTOM, 0.7f, editorArea);
  bottomFolder->AddView("org.mitk.views.propertylistview");
  bottomFolder->AddView("org.blueberry.views.logview");
}
