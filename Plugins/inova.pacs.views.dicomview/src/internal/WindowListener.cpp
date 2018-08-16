#include "WindowListener.h"

#include <berryPlatformUI.h>

#include <service/event/ctkEventConstants.h>

#include <QDir>
#include <QDateTime>

//#include "QmitkXnatEditor.h"


const QString PerspectiveListener::EDITOR_ID = "org.mitk.gui.qt.xnat";
const QString DEFAULT_PERSPECTIVE_ID = "inova.popeproject.perspectives.mainperspective";


IPerspectiveListener::Events::Types PerspectiveListener::GetPerspectiveEventTypes() const
{
  return Events::OPENED | Events::CHANGED;
}

void PerspectiveListener::PerspectiveOpened(const SmartPointer<IWorkbenchPage>& page, const IPerspectiveDescriptor::Pointer& perspective)
{
  // if no help editor is opened, open one showing the home page
  if (perspective->GetId() == DEFAULT_PERSPECTIVE_ID &&
      page->FindEditors(IEditorInput::Pointer(nullptr), PerspectiveListener::EDITOR_ID, IWorkbenchPage::MATCH_ID).empty())
  {
    //IEditorInput::Pointer input(new QmitkXnatEditor());
    //page->OpenEditor(input, PerspectiveListener::EDITOR_ID);
  }
}

void PerspectiveListener::PerspectiveChanged(const SmartPointer<IWorkbenchPage>& page, const IPerspectiveDescriptor::Pointer& perspective, const QString &changeId)
{
  if (perspective->GetId() == DEFAULT_PERSPECTIVE_ID && changeId == IWorkbenchPage::CHANGE_RESET)
  {
    PerspectiveOpened(page, perspective);
  }
}

WindowListener::WindowListener()
  : perspListener(new PerspectiveListener())
{
  // Register perspective listener for already opened windows
  typedef QList<IWorkbenchWindow::Pointer> WndVec;
  WndVec windows = PlatformUI::GetWorkbench()->GetWorkbenchWindows();
  for (WndVec::iterator i = windows.begin(); i != windows.end(); ++i)
  {
    (*i)->AddPerspectiveListener(perspListener.data());
  }
}

WindowListener::~WindowListener()
{
  if (!PlatformUI::IsWorkbenchRunning()) return;

  typedef QList<IWorkbenchWindow::Pointer> WndVec;
  WndVec windows = PlatformUI::GetWorkbench()->GetWorkbenchWindows();
  for (WndVec::iterator i = windows.begin(); i != windows.end(); ++i)
  {
    (*i)->RemovePerspectiveListener(perspListener.data());
  }
}

void WindowListener::WindowClosed(const IWorkbenchWindow::Pointer& window)
{
  window->RemovePerspectiveListener(perspListener.data());
}

void WindowListener::WindowOpened(const IWorkbenchWindow::Pointer& window)
{
  window->AddPerspectiveListener(perspListener.data());
}
