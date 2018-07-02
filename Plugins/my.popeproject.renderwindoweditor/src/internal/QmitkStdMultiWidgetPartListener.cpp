#include "QmitkStdMultiWidgetPartListener.h"

#include <QmitkStdMultiWidget.h>
#include "PopeRenderWindowEditorPrivate.h"
#include "PopeRenderWindowEditor.h"


QmitkStdMultiWidgetPartListener::QmitkStdMultiWidgetPartListener(PopeRenderWindowEditorPrivate* dd)
	: d(dd)
{}

berry::IPartListener::Events::Types QmitkStdMultiWidgetPartListener::GetPartEventTypes() const
{
	return Events::CLOSED | Events::HIDDEN | Events::VISIBLE | Events::OPENED;
}

void QmitkStdMultiWidgetPartListener::PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef)
{
	if (partRef->GetId() == PopeRenderWindowEditor::EDITOR_ID)
	{
		PopeRenderWindowEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<PopeRenderWindowEditor>();

		if (d->m_StdMultiWidget == stdMultiWidgetEditor->GetStdMultiWidget())
		{
			d->m_StdMultiWidget->RemovePlanesFromDataStorage();
			stdMultiWidgetEditor->RequestActivateMenuWidget(false);
		}
	}
}

void QmitkStdMultiWidgetPartListener::PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef)
{
	if (partRef->GetId() == PopeRenderWindowEditor::EDITOR_ID)
	{
		PopeRenderWindowEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<PopeRenderWindowEditor>();

		if (d->m_StdMultiWidget == stdMultiWidgetEditor->GetStdMultiWidget())
		{
			stdMultiWidgetEditor->RequestActivateMenuWidget(false);
		}
	}
}

void QmitkStdMultiWidgetPartListener::PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef)
{
	if (partRef->GetId() == PopeRenderWindowEditor::EDITOR_ID)
	{
		PopeRenderWindowEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<PopeRenderWindowEditor>();

		if (d->m_StdMultiWidget == stdMultiWidgetEditor->GetStdMultiWidget())
		{
			stdMultiWidgetEditor->RequestActivateMenuWidget(true);
		}
	}
}

void QmitkStdMultiWidgetPartListener::PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef)
{
	if (partRef->GetId() == PopeRenderWindowEditor::EDITOR_ID)
	{
		PopeRenderWindowEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<PopeRenderWindowEditor>();

		if (d->m_StdMultiWidget == stdMultiWidgetEditor->GetStdMultiWidget())
		{
			d->m_StdMultiWidget->AddPlanesToDataStorage();
			stdMultiWidgetEditor->RequestActivateMenuWidget(true);
		}
	}
}
