#pragma once
#include "string"
#include "Windows.h"

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
	public:
		//using EventCallbackFn = std::function<void(Event&)>; // TODO: Events

		Window(const HINSTANCE hInstance, const WindowDesc& description);
		virtual ~Window();

		virtual bool Init();
		//virtual void ProcessEvents(); // TODO: handel input events and process them here
		//virtual void SwapBuffers();

		inline UINT32 GetWidth() const { return m_Data.Width; }
		inline UINT32 GetHeight() const { return m_Data.Height; }

		virtual inline std::pair<UINT32, UINT32> GetSize() const { return { m_Data.Width, m_Data.Height }; }
		//virtual inline std::pair<float, float> GetWindowPos() const;

		//virtual void SetEventCallback(); // TODO
		//virtual void SetVSync(bool enabled);
		//virtual bool IsVSync() const;
		//virtual void SetResizable(bool resizable);

		//virtual void Maximize();
		//virtual void CenterWindow();

		virtual const std::string& GetTitle() const { return m_Data.Title; }
		//virtual void SetTitle(const std::string& title);

		inline HWND GetNativeWindow() const { return m_Window; }

	public:
		static Window* Create(const HINSTANCE hInstance, const WindowDesc& description = WindowDesc());

	private:
		virtual void Shutdown();
		
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		HWND m_Window;

		bool m_bIsInitialized = false;

		WindowDesc m_Description;
		struct WindowData
		{
			std::string Title;
			UINT32 Width, Height;

		};
		WindowData m_Data;

		HINSTANCE m_ApplicationInstance;

		DXSwapChain* m_SwapChain;
	};
}
