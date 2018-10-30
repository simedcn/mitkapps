#include "MouseMoveSupplier.h"

#include <mitkInteractionPositionEvent.h>
#include <mitkInteractionEventConst.h>
#include <mitkMouseMoveEvent.h>
#include <mitkMousePressEvent.h>
#include <mitkMouseReleaseEvent.h>
#include <mitkBaseRenderer.h>
// QT
#include <QVector>

#include <limits>

static mitk::Point3D GetPositionInWorld(mitk::InteractionEvent* event)
{
	mitk::Point3D p;
	mitk::InteractionPositionEvent* pe = dynamic_cast<mitk::InteractionPositionEvent*>(event);
	if (pe != nullptr)
	{
		p = pe->GetPositionInWorld();
	}
	return p;
}

static mitk::Point3D GetPositionIn2D(mitk::InteractionEvent* event)
{
	mitk::Point3D p;
	mitk::InteractionPositionEvent* pe = dynamic_cast<mitk::InteractionPositionEvent*>(event);
	if (pe != nullptr)
	{
		auto point = pe->GetPointerPositionOnScreen();
		p[0] = point[0];
		p[1] = point[1];
		p[2] = 0;
		//auto r = pe->GetSender()->GetBounds();
		//auto d0 = r[0];
		//auto d1 = r[1];
	}
	return p;
}


MouseMoveSupplier::MouseMoveSupplier()
	: m_startPoint(nullptr)
	, m_startPoint2D(nullptr)
{
}

MouseMoveSupplier::~MouseMoveSupplier()
{
}


void MouseMoveSupplier::RegisterForUpdates(QObject* obj, QString& functionName)
{
	this->m_Observers[obj] = functionName;
}

void MouseMoveSupplier::Notify(mitk::InteractionEvent* interactionEvent, bool /*isHandled*/)
{
	// Check if the event is acceptable
	mitk::InteractionPositionEvent* posEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
	if (!posEvent)
		return;
	mitk::InteractionPositionEvent* pe = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
	if (pe == nullptr)
		return;

	mitk::MouseMoveEvent* move_event = dynamic_cast<mitk::MouseMoveEvent*>(interactionEvent);
	mitk::MouseReleaseEvent* release_event = dynamic_cast<mitk::MouseReleaseEvent*>(interactionEvent);
	mitk::MousePressEvent* press_event = dynamic_cast<mitk::MousePressEvent*>(interactionEvent);
	if (move_event == nullptr && release_event == nullptr && press_event == nullptr)
		return;

	if (release_event != nullptr || (move_event != nullptr && m_startPoint != nullptr)) //|| (move_event != nullptr && !(move_event->GetButtonStates() & mitk::InteractionEvent::LeftMouseButton)))
	{
		if (m_startPoint != nullptr)
		{
			auto mapperID = interactionEvent->GetSender()->GetMapperID();
			if (interactionEvent->GetSender() == nullptr || mapperID == mitk::BaseRenderer::Standard3D)
			{
				m_startPoint = nullptr;
				m_startPoint2D = nullptr;
				return;
			}

			QVector<double> qPoint(3);
			mitk::Point3D point = GetPositionInWorld(posEvent);
			if (std::isnan(point[0]) || std::isnan(point[1]) || std::isnan(point[2]) || std::isinf(point[0]) || std::isinf(point[1]) || std::isinf(point[2]))
			{
				m_startPoint = nullptr;
				m_startPoint2D = nullptr;
				return;
			}
			mitk::Point3D point2D = GetPositionIn2D(posEvent);

			const mitk::Point3D& start_point = *m_startPoint;
			qPoint[0] = point[0] - start_point[0];
			qPoint[1] = point[1] - start_point[1];
			qPoint[2] = point[2] - start_point[2];

			const double dbl_min = 6 * std::numeric_limits<double>::epsilon();

			bool is_rotation = (move_event != nullptr && (move_event->GetModifiers() & (mitk::InteractionEvent::ShiftKey | mitk::InteractionEvent::ControlKey)))
				|| (release_event != nullptr && (release_event->GetModifiers() & (mitk::InteractionEvent::ShiftKey | mitk::InteractionEvent::ControlKey)));
			if (is_rotation)
			{
				
				bool is_0 = (abs(qPoint[0]) > dbl_min);
				bool is_1 = (abs(qPoint[1]) > dbl_min);
				bool is_2 = (abs(qPoint[2]) > dbl_min);
				int i_view = -1;
				if (is_0 && is_1)
					i_view = 2;
				else if (is_0 && is_2)
					i_view = 1;
				else if (is_1 && is_2)
					i_view = 0;
				else if (is_0)
					i_view = 1;
				else if (is_1)
					i_view = 2;
				else if (is_2)
					i_view = 0;
				if (i_view == -1)
				{
					m_startPoint = nullptr;
					m_startPoint2D = nullptr;
					return;
				}

				const mitk::Point3D& start_point2D = *m_startPoint2D;
				qPoint[0] = point2D[0] - start_point2D[0];
				qPoint[1] = point2D[1] - start_point2D[1];
				qPoint[2] = 100000 + i_view;
			}
			else
			{
				if (std::isnan(qPoint[0]) || std::isnan(qPoint[1]) || std::isnan(qPoint[2])
					|| std::isinf(qPoint[0]) || std::isinf(qPoint[1]) || std::isinf(qPoint[2])
					|| (abs(qPoint[0]) < dbl_min && abs(qPoint[1]) < dbl_min && abs(qPoint[2]) < dbl_min))
				{
					MITK_INFO << "MouseMove event: zero";
					m_startPoint = nullptr;
					m_startPoint2D = nullptr;
					return;
				}
			}

			MITK_INFO << (is_rotation ? "ROT(" : "(") << qPoint[0] << ", " << qPoint[1] << ", " << qPoint[2] << ")";
			bool is_movement_ongoing = (move_event != nullptr && (move_event->GetButtonStates() & mitk::InteractionEvent::LeftMouseButton));
			if (release_event == nullptr && is_movement_ongoing)
			{
				m_startPoint = std::make_shared<mitk::Point3D>(point);
				m_startPoint2D = std::make_shared<mitk::Point3D>(point2D);
			}
			else
			{
				m_startPoint = nullptr;
				m_startPoint2D = nullptr;
			}

			for (QObject* e : this->m_Observers.keys())
			{
				QString name = m_Observers.value(e);
				//if (name == "on_mouse_moved" && !is_rotation)
					QMetaObject::invokeMethod(e, name.toUtf8().constData(), Q_ARG(QVector<double>, qPoint));
				//else if (name == "on_mouse_rotated" && is_rotation)
				//	QMetaObject::invokeMethod(e, name.toUtf8().constData(), Q_ARG(QVector<double>, qPoint));
			}
		}
		return;
	}

	if (press_event != nullptr || (move_event != nullptr && (move_event->GetButtonStates() & mitk::InteractionEvent::LeftMouseButton)))
	{
		if (m_startPoint == nullptr)
		{
			if (interactionEvent->GetSender() == nullptr)
				return;
			if (interactionEvent->GetSender()->GetMapperID() == mitk::BaseRenderer::Standard3D)
				return;
			mitk::Point3D point = GetPositionInWorld(posEvent);
			if (std::isnan(point[0]) || std::isnan(point[1]) || std::isnan(point[2]) || std::isinf(point[0]) || std::isinf(point[1]) || std::isinf(point[2]))
				return;
			mitk::Point3D point2D = GetPositionIn2D(posEvent);

			m_startPoint = std::make_shared<mitk::Point3D>(point);
			m_startPoint2D = std::make_shared<mitk::Point3D>(point2D);
		}
		return;
	}
}
