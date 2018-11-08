
#ifndef ORGANPRINTAPPLICATION_H_
#define ORGANPRINTAPPLICATION_H_

#include <berryIApplication.h>

class OrganPrintApplication : public QObject, public berry::IApplication
{
	Q_OBJECT
	Q_INTERFACES(berry::IApplication)

public:

	OrganPrintApplication();
	OrganPrintApplication(const OrganPrintApplication& other);

	QVariant Start(berry::IApplicationContext* context);
	void Stop();
};

#endif /*ORGANPRINTAPPLICATION_H_*/
