#pragma once

#include "Events/Event.h"

namespace Blainn
{
	struct WindowDesc
	{
		std::string Title = "Blainnflare";
		UINT32 Width = 1600;
		UINT32 Height = 800;
		bool Decorated = true;
		bool Fullscreen = false;
		bool VSync = true;
		bool Minimized = false;
		bool Resizing = false;
	};

	class DXSwapChain;

	class Window
	{
		Window(const WindowDesc& description);
	public:
		using EventCallbackFn = std::function<void(Event&)>; // TODO: Events

		virtual ~Window();

		static Window* Create(const WindowDesc& description = WindowDesc());

		virtual bool Init();
		virtual void Show();
		//virtual void ProcessEvents();

		inline UINT32 GetWidth() const { return m_Data.Width; }
		inline UINT32 GetHeight() const { return m_Data.Height; }

		virtual inline std::pair<UINT32, UINT32> GetSize() const { return { m_Data.Width, m_Data.Height }; }
		//virtual inline std::pair<float, float> GetWindowPos() const;

		virtual void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; };
		virtual const std::string& GetTitle() const { return m_Data.Title; }

		inline HWND GetNativeWindow() const { return m_Window; }


	private:
		virtual void Shutdown();
		
		static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		HWND m_Window;

		bool m_bIsInitialized = false;

		WindowDesc m_Description;

		struct WindowData
		{
			std::string Title;
			UINT32 Width, Height;

			EventCallbackFn EventCallback;
		};
		WindowData m_Data;

		HINSTANCE m_ApplicationInstance;

		DXSwapChain* m_SwapChain;
	};
}
