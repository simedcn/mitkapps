#include "DisplayCoordinateSupplier.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkInteractionEventConst.h"
#include "mitkMousePressEvent.h"
#include "mitkMouseReleaseEvent.h"
#include "mitkBaseRenderer.h"
// QT
#include <QVector>

static mitk::Point3D GetPositionInWorld(mitk::InteractionEvent *event)
{
  mitk::Point3D p;
  mitk::InteractionPositionEvent *pe = dynamic_cast<mitk::InteractionPositionEvent*>(event);
  if (pe != nullptr)
  {
    p = pe->GetPositionInWorld();
  }
  return p;
}


DisplayCoordinateSupplier::DisplayCoordinateSupplier()
{
}

DisplayCoordinateSupplier::~DisplayCoordinateSupplier()
{

}


void DisplayCoordinateSupplier::RegisterForUpdates(QObject* obj, QString& functionName)
{
    this->m_Observers[obj] = functionName;
}

void DisplayCoordinateSupplier::Notify(mitk::InteractionEvent *interactionEvent, bool /*isHandled*/)
{
  // Check if the event is acceptable
  mitk::InteractionPositionEvent* posEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (!posEvent)
      return;
  mitk::InteractionPositionEvent *pe = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (pe == nullptr)
    return;

  mitk::MousePressEvent *mme = dynamic_cast<mitk::MousePressEvent *>(interactionEvent);
  if (mme == nullptr)
    return;

  if (mme)
  {
      if (!((mme->GetButtonStates() & mitk::InteractionEvent::LeftMouseButton) &&
            (mme->GetModifiers() &  mitk::InteractionEvent::ShiftKey) )
      )
      return;
  }
  if (interactionEvent->GetSender() == nullptr)
    return;

  if (interactionEvent->GetSender()->GetMapperID() == mitk::BaseRenderer::Standard3D)
    return;

  mitk::Point3D point = GetPositionInWorld(posEvent);

  QVector<double> qPoint(3);
  qPoint[0]=point[0];
  qPoint[1]=point[1];
  qPoint[2]=point[2];

  for(QObject* e : this->m_Observers.keys())
  {
     QMetaObject::invokeMethod(e, m_Observers.value(e).toUtf8().constData(), Q_ARG(QVector<double>,qPoint) );
  }
}



