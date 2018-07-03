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

#include "ImportPanel.h"

#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <usModuleRegistry.h>

#include <QMessageBox>


// Don't forget to initialize the VIEW_ID.
const std::string orgpnt::ImportPanel::VIEW_ID = "my.organprint.views.importpanel";



void orgpnt::ImportPanel::CreateQtPartControl(QWidget* parent)
{
    // Setting up the UI is a true pleasure when using .ui files, isn't it?
    m_Controls.setupUi(parent);

    // Wire up the UI widgets with our functionality.
    connect(m_Controls.openImageButton, SIGNAL(clicked()), this, SLOT(OpenImageFromDisk()));
    connect(m_Controls.queryPacsButton, SIGNAL(clicked()), this, SLOT(QueryPacs()));
}

void orgpnt::ImportPanel::SetFocus()
{
    m_Controls.openImageButton->setFocus();
}



void orgpnt::ImportPanel::OpenImageFromDisk()
{

    cout << "Thank you for clicking" << endl;
    /*
    QString imagePath = QFileDialog::getOpenFileName(
                            this, tr("Open File")
                        );
    DataStorage::*/
}

void orgpnt::ImportPanel::QueryPacs() {
    cout << "Thank you for querying PACS" << endl;
}

