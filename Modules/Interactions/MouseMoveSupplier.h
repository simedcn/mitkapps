#ifndef MouseMoveSupplier_h
#define MouseMoveSupplier_h

#include <mitkInteractionEvent.h>
#include <mitkInteractionEventObserver.h>
#include <AppInteractionExports.h>
#include <QObject>
#include <QString>
#include <QMap>
#include <memory>

/**
*\class MouseMoveSupplier - This Event Observer listens to all MouseMove Events in the RenderWindows
* and calls the function that was registered using RegisterForUpdates and supplies the Display World Corrdinates that were clicked.
**/
class AppInteraction_EXPORT MouseMoveSupplier : public mitk::InteractionEventObserver
{
public:
	MouseMoveSupplier();
	~MouseMoveSupplier();

	/**
	* By this function the Observer gets notified about new events.
	*/
	virtual void Notify(mitk::InteractionEvent* interactionEvent, bool) override;

	virtual void RegisterForUpdates(QObject* obj, QString& functionName);

protected:
	std::shared_ptr<mitk::Point3D> m_startPoint;
	std::shared_ptr<mitk::Point3D> m_startPoint2D;

private:
	QMap<QObject*, QString> m_Observers;
};

#endif
