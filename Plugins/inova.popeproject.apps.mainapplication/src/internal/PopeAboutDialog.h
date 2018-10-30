/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef PopeAboutDialog_h
#define PopeAboutDialog_h

#include "inova_popeproject_apps_mainapplication_Export.h"
#include <ui_PopeAboutDialog.h>

class inova_popeproject_apps_mainapplication_EXPORT PopeAboutDialog : public QDialog
{
	Q_OBJECT

public:
	PopeAboutDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
	~PopeAboutDialog() override;

	QString GetAboutText() const;
	QString GetCaptionText() const;
	QString GetRevisionText() const;

	void SetAboutText(const QString &text);
	void SetCaptionText(const QString &text);
	void SetRevisionText(const QString &text);

protected slots:
	void ShowModules();

private:
	Ui::PopeAboutDialog ui;
};

#endif
