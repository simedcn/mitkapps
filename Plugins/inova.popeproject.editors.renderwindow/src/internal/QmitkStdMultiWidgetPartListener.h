//#pragma once
#ifndef QmitkStdMultiWidgetPartListener_H
#define QmitkStdMultiWidgetPartListener_H

#include <berryIPartListener.h>

class PopeRenderWindowEditorPrivate;

struct QmitkStdMultiWidgetPartListener : public berry::IPartListener
{
public:
	QmitkStdMultiWidgetPartListener(PopeRenderWindowEditorPrivate* dd);

	Events::Types GetPartEventTypes() const override;

	void PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef) override;
	void PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef) override;
	void PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef) override;
	void PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef) override;

private:
	PopeRenderWindowEditorPrivate* const d;
};


#endif // QmitkStdMultiWidgetPartListener_H