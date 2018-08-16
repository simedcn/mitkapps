#include "DicomPerspective.h"

#include <berryIFolderLayout.h>

DicomPerspective::DicomPerspective()
{
}

// //! [DicomPerspCreateLayout]
void DicomPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  QString editorArea = layout->GetEditorArea();
  //layout->SetEditorAreaVisible(false);
  //layout->AddStandaloneView(
  //  "inova.pacs.views.dicomview", false, berry::IPageLayout::LEFT, 1.0f, layout->GetEditorArea());
  //layout->GetViewLayout("inova.pacs.views.dicomview")->SetCloseable(false);
  //layout->GetViewLayout("inova.pacs.views.dicomview")->SetMoveable(false);
}
// //! [DicomPerspCreateLayout]
