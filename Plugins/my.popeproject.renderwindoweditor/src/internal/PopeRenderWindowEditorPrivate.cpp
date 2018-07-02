#include "PopeRenderWindowEditorPrivate.h"

#include "QmitkStdMultiWidgetPartListener.h"


PopeRenderWindowEditorPrivate::PopeRenderWindowEditorPrivate()
	: m_StdMultiWidget(0), m_MouseModeToolbar(0)
	, m_MenuWidgetsEnabled(true)
	, m_PartListener(new QmitkStdMultiWidgetPartListener(this))
{}

PopeRenderWindowEditorPrivate::~PopeRenderWindowEditorPrivate()
{}
