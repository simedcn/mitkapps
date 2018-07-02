//#pragma once
#ifndef PopeRenderWindowEditorPrivate_H
#define PopeRenderWindowEditorPrivate_H

#include <berryIPartListener.h>

class QmitkStdMultiWidget;
class QmitkMouseModeSwitcher;
class MainWindow;
class QmitkRenderWindow;

class PopeRenderWindowEditorPrivate
{
public:
	PopeRenderWindowEditorPrivate();
	~PopeRenderWindowEditorPrivate();

	QmitkStdMultiWidget* m_StdMultiWidget;
	MainWindow* m_centralWindow;
	QmitkMouseModeSwitcher* m_MouseModeToolbar;
	/**
	* @brief Members for the MultiWidget decorations.
	*/
	QString m_WidgetBackgroundColor1[4];
	QString m_WidgetBackgroundColor2[4];
	QString m_WidgetDecorationColor[4];
	QString m_WidgetAnnotation[4];
	bool m_MenuWidgetsEnabled;
	QScopedPointer<berry::IPartListener> m_PartListener;
	QHash<QString, QmitkRenderWindow*> m_RenderWindows;
};

#endif // PopeRenderWindowEditorPrivate_H