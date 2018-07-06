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

#include "ExportPanel.h"

#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <usModuleRegistry.h>

#include <QMessageBox>

#include <mitkWorkbenchUtil.h>

#include <QmitkIOUtil.h>

#include <berryIPreferences.h>

#include <QFileDialog>

// Don't forget to initialize the VIEW_ID.
const std::string orgpnt::ExportPanel::VIEW_ID = "my.organprint.views.exportpanel";



void orgpnt::ExportPanel::CreateQtPartControl(QWidget* parent)
{
    // Setting up the UI is a true pleasure when using .ui files, isn't it?
    m_Controls.setupUi(parent);

    // Wire up the UI widgets with our functionality.
    connect(m_Controls.openImageButton, SIGNAL(clicked()), this, SLOT(OpenImageFromDisk()));
    connect(m_Controls.queryPacsButton, SIGNAL(clicked()), this, SLOT(QueryPacs()));
}

void orgpnt::ExportPanel::SetFocus()
{
    m_Controls.openImageButton->setFocus();
}



void orgpnt::ExportPanel::OpenImageFromDisk()
{

    cout << "Thank you for clicking" << endl;

    // Ask the user for a list of files to open
    QStringList fileNames = QFileDialog::getOpenFileNames(nullptr, "Open",
                            nullptr,
                            QmitkIOUtil::GetFileOpenFilterString());

    if (fileNames.empty())
        return;

    //d->setLastFileOpenPath(fileNames.front());
    mitk::WorkbenchUtil::LoadFiles(fileNames, berry::IWorkbenchWindow::Pointer(nullptr),
                                   false);

    cout << "I guess that's it" << endl;
}

void orgpnt::ExportPanel::QueryPacs() {
    cout << "Thank you for querying PACS" << endl;
}

