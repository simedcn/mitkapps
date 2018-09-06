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

#include <mitkWorkbenchUtil.h>

#include <QmitkIOUtil.h>

#include <berryIPreferences.h>

#include <QFileDialog>
#include <berryPlatformUI.h>
#include "my_awesomeproject_exampleplugin_PluginActivator.h"
#include <QVariant>
#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <DicomViewDialog.h>
#include <PopeElements.h>

// Don't forget to initialize the VIEW_ID.
const std::string orgpnt::ImportPanel::VIEW_ID = "my.organprint.views.importpanel";



void orgpnt::ImportPanel::CreateQtPartControl(QWidget* parent)
{

    this->parent = parent;
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






    // Ask the user for a list of files to open
    QStringList fileNames = QFileDialog::getOpenFileNames(nullptr, "Open",
                            nullptr,
                            QmitkIOUtil::GetFileOpenFilterString());


    if (fileNames.empty())
        return;
    cout << "Loading the the files..." << endl;
    //d->setLastFileOpenPath(fileNames.front());



    berry::IWorkbenchWindow::Pointer window = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow();

    /*
        QString patientId = Elements::get_patientId_or_patientName(fileNames.at(0));

          bool erasePrevious = patientId != m_PatientId;

          m_PatientId = patientId;

          if(erasePrevious) {
              mitk::DataStorage * storage =
              GetDataStorage()->Remove(GetDataStorage()->GetAll());
          }
          */
    mitk::WorkbenchUtil::LoadFiles(fileNames, berry::IWorkbenchWindow::Pointer(window),
                                   true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();


    next(1);

}

void orgpnt::ImportPanel::QueryPacs() {

    DicomViewDialog * dialog = new DicomViewDialog(parent);
    dialog->exec();
    //DicomView * view = new DicomView();
    //view->CreateQtPartControl(parent);
}

void orgpnt::ImportPanel::next(int step = 1) {
    ctkPluginContext * context = my_awesomeproject_exampleplugin_PluginActivator::GetPluginContext();
    ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();
    if (ref)
    {
        ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
        ctkDictionary properties;
        properties["step"] = QVariant(step);
        ctkEvent changeStepEvent("my/organprint/stepselector", properties);
        eventAdmin->sendEvent(changeStepEvent);
    }
}

