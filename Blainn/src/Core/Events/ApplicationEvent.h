#pragma once

#include "Event.h"

#include <sstream>

namespace Blainn
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(WPARAM wParam, unsigned int width, unsigned int height)
			: m_wParam(wParam), m_Width(width), m_Height(height) {}

		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
		inline WPARAM GetWParam() const { return m_wParam; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		int m_Width, m_Height;
		WPARAM m_wParam;
	};

	class WindowMovedEvent : public Event
	{
	public:
		WindowMovedEvent(bool moveStarted) : m_bMoveStarted(moveStarted) {}

		bool GetMoveStarted() const { return m_bMoveStarted; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowMovedEvent: " << m_bMoveStarted;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		bool m_bMoveStarted;
	};

	class WindowMinimizeEvent : public Event
	{
	public:
		WindowMinimizeEvent(bool bMinimized)
			: m_bMinimized(bMinimized) {}

		inline bool IsMinimized() const { return m_bMinimized; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowMinimizeEvent: " << m_bMinimized;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowMinimize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		bool m_bMinimized;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowTitleBarHitTestEvent : public Event
	{
	public:
		WindowTitleBarHitTestEvent(int x, int y, int& hit)
			: m_X(x), m_Y(y), m_Hit(hit) {}

		inline int GetX() const { return m_X; }
		inline int GetY() const { return m_Y; }
		inline void SetHit(bool hit) { m_Hit = (int)hit; }

		EVENT_CLASS_TYPE(WindowTitleBarHitTest)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		int m_X;
		int m_Y;
		int& m_Hit;
	};

	class ComboboxOptionSelectedEvent : public Event
	{
	public:
		ComboboxOptionSelectedEvent(int itemIndex, const std::wstring& itemString)
			: m_ItemIndex(itemIndex), m_ItemString(itemString)
		{}

		EVENT_CLASS_TYPE(ComboboxOptionSelected)
		EVENT_CLASS_CATEGORY(EventCategoryInput)

		std::wstring ToWString() const
		{
			std::wstringstream wss;
			wss << L"Chose item " << m_ItemIndex << ": " << m_ItemString << L"\n";
			return wss.str();
		}

	private:
		int m_ItemIndex;
		std::wstring m_ItemString;
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() {}

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() {}

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() {}

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
}
