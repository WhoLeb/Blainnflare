#pragma once

#include "Event.h"

#include "Core/KeyCodes.h"

#include <sstream>

namespace Blainn
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(int x, int y)
			: m_MouseX(x), m_MouseY(y)
		{}

		inline int GetX() const { return m_MouseX; }
		inline int GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	protected:
		int m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(int x, int y)
			: m_XOffset(x), m_YOffset(y) {}

		inline int GetXOffset() const { return m_XOffset; }
		inline int GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		int m_XOffset, m_YOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		inline MouseButton GetMouseButton() const { return m_Button; }
		inline int GetXOffset() const { return m_XOffset; }
		inline int GetYOffset() const { return m_YOffset; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
		
	protected:
		MouseButtonEvent(MouseButton button, int xOffset, int yOffset)
			: m_Button(button), m_XOffset(xOffset), m_YOffset(yOffset) {}

		MouseButton m_Button;

		int m_XOffset, m_YOffset;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseButton button, int xOffset, int yOffset)
			: MouseButtonEvent(button, xOffset, yOffset) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button << " (" << m_XOffset << ", " << m_YOffset << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(MouseButton button, int xOffset, int yOffset)
			: MouseButtonEvent(button, xOffset, yOffset) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button << " (" << m_XOffset << ", " << m_YOffset << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

	class MouseButtonDownEvent : public MouseButtonEvent
	{
	public:
		MouseButtonDownEvent(MouseButton button, int xOffset, int yOffset)
			: MouseButtonEvent(button, xOffset, yOffset) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonDownEvent: " << m_Button << " (" << m_XOffset << ", " << m_YOffset << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonDown)
	};

}
