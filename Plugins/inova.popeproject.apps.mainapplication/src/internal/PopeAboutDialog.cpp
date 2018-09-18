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

#include "PopeAboutDialog.h"
#include "QmitkModulesDialog.h"
#include <QPushButton>
#include <itkConfigure.h>
#include <mitkVersion.h>
#include <vtkConfigure.h>
#include <vtkVersionMacros.h>

PopeAboutDialog::PopeAboutDialog(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
  ui.setupUi(this);

  QString mitkRevision(MITK_REVISION);
  QString mitkRevisionDescription(MITK_REVISION_DESC);
  QString itkVersion = QString("%1.%2.%3").arg(ITK_VERSION_MAJOR).arg(ITK_VERSION_MINOR).arg(ITK_VERSION_PATCH);
  QString vtkVersion = QString("%1.%2.%3").arg(VTK_MAJOR_VERSION).arg(VTK_MINOR_VERSION).arg(VTK_BUILD_VERSION);

  QString revisionText = QString("Revision: %1").arg(MITK_REVISION);

  QString mitkVersion;
  if (!QString(MITK_REVISION_DESC).isEmpty())
  {
	  //revisionText += QString("\nDescription: %1").arg(MITK_REVISION_DESC);
	  mitkVersion = MITK_REVISION_DESC;
	  mitkVersion.remove("[local changes]");
	  mitkVersion = mitkVersion.trimmed();
  }

  ui.m_RevisionLabel->setText(revisionText);
  ui.m_ToolkitVersionsLabel->setText(QString("ITK %1, VTK %2, Qt %3, MITK %4").arg(itkVersion, vtkVersion, QT_VERSION_STR, mitkVersion));

  QPushButton *btnModules = new QPushButton(QIcon(":/QtWidgetsExt/ModuleView.png"), "Modules");
  ui.m_ButtonBox->addButton(btnModules, QDialogButtonBox::ActionRole);

  connect(btnModules, SIGNAL(clicked()), this, SLOT(ShowModules()));
  connect(ui.m_ButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

PopeAboutDialog::~PopeAboutDialog()
{
}

void PopeAboutDialog::ShowModules()
{
  QmitkModulesDialog dialog(this);
  dialog.exec();
}

QString PopeAboutDialog::GetAboutText() const
{
  return ui.m_AboutLabel->text();
}

QString PopeAboutDialog::GetCaptionText() const
{
  return ui.m_CaptionLabel->text();
}

QString PopeAboutDialog::GetRevisionText() const
{
  return ui.m_RevisionLabel->text();
}

void PopeAboutDialog::SetAboutText(const QString &text)
{
  ui.m_AboutLabel->setText(text);
}

void PopeAboutDialog::SetCaptionText(const QString &text)
{
  ui.m_CaptionLabel->setText(text);
}

void PopeAboutDialog::SetRevisionText(const QString &text)
{
  ui.m_RevisionLabel->setText(text);
}
