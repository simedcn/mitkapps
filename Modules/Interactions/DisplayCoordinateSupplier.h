#ifndef DisplayCoordinateSupplier_h
#define DisplayCoordinateSupplier_h

#include "mitkInteractionEvent.h"
#include "mitkInteractionEventObserver.h"
#include <AppInteractionExports.h>
#include <QObject>
#include <QString>
#include <QMap>

  /**
     *\class DisplayCoordinateSupplier - This Event Observer listens to all LeftLick + Shift Events in the RenderWindows
     * and calls the function that was registered using RegisterForUpdates and supplies the Display World Corrdinates that were cliked.
     **/
  class AppInteraction_EXPORT DisplayCoordinateSupplier : public mitk::InteractionEventObserver
  {
  public:
    DisplayCoordinateSupplier();
    ~DisplayCoordinateSupplier();

    /**
       * By this function the Observer gets notified about new events.
       */
    virtual void Notify(mitk::InteractionEvent *interactionEvent, bool) override;

    virtual void RegisterForUpdates(QObject* obj, QString& functionName);


  private:
      QMap<QObject*,QString> m_Observers;
  };

#endif
