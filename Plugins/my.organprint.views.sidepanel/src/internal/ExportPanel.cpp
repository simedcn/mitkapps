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


#include "STLExportService.h"
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
    connect(m_Controls.exportSTLButton, SIGNAL(clicked()), this, SLOT(ExportInSTL()));
    connect(m_Controls.saveButton, SIGNAL(clicked()), this, SLOT(SaveProject()));
}

void orgpnt::ExportPanel::SetFocus()
{
    //m_Controls.openImageButton->setFocus();
}



void orgpnt::ExportPanel::ExportInSTL() {

    STLExportService * service = new STLExportService();
    QString path("/home/cyril/");
    service->exportTo(path,GetDataStorage());

}

void orgpnt::ExportPanel::SaveProject() {

}

