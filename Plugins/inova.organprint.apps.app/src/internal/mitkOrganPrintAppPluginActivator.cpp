
#include "mitkOrganPrintAppPluginActivator.h"
#include "OrganPrintApplication.h"
#include "OrganPrintPerspective.h"

#include <berryLog.h>

#include <mitkVersion.h>

#include <QFileInfo>
#include <QDateTime>

namespace mitk {

OrganPrintAppPluginActivator* OrganPrintAppPluginActivator::inst = 0;

OrganPrintAppPluginActivator::OrganPrintAppPluginActivator()
{
  inst = this;
}

OrganPrintAppPluginActivator::~OrganPrintAppPluginActivator()
{
}

OrganPrintAppPluginActivator* OrganPrintAppPluginActivator::GetDefault()
{
  return inst;
}

void OrganPrintAppPluginActivator::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);
  
  this->context = context;
  
  BERRY_REGISTER_EXTENSION_CLASS(OrganPrintApplication, context);
  BERRY_REGISTER_EXTENSION_CLASS(OrganPrintPerspective, context);

  QString collectionFile = GetQtHelpCollectionFile();

  // berry::QtAssistantUtil::SetHelpCollectionFile(collectionFile);
  // berry::QtAssistantUtil::SetDefaultHelpUrl("qthelp://inova.organprint.OrganPrintapp/bundle/index.html");
}

ctkPluginContext* OrganPrintAppPluginActivator::GetPluginContext() const
{
  return context;
}

QString OrganPrintAppPluginActivator::GetQtHelpCollectionFile() const
{
  if (!helpCollectionFile.isEmpty())
  {
    return helpCollectionFile;
  }

  QString collectionFilename = "OrganPrintAppQtHelpCollection.qhc";

  QFileInfo collectionFileInfo = context->getDataFile(collectionFilename);
  QFileInfo pluginFileInfo = QFileInfo(QUrl(context->getPlugin()->getLocation()).toLocalFile());
  if (!collectionFileInfo.exists() ||
      pluginFileInfo.lastModified() > collectionFileInfo.lastModified())
  {
    // extract the qhc file from the plug-in
    QByteArray content = context->getPlugin()->getResource(collectionFilename);
    if (content.isEmpty())
    {
      BERRY_WARN << "Could not get plug-in resource: " << collectionFilename.toStdString();
    }
    else
    {
      QFile file(collectionFileInfo.absoluteFilePath());
      file.open(QIODevice::WriteOnly);
      file.write(content);
      file.close();
    }
  }

  if (QFile::exists(collectionFileInfo.absoluteFilePath()))
  {
    helpCollectionFile = collectionFileInfo.absoluteFilePath();
  }

  return helpCollectionFile;
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  #include <QtPlugin>
  Q_EXPORT_PLUGIN2(inova_organprint_apps_app, mitk::OrganPrintAppPluginActivator)
#endif
