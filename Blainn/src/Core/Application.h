#pragma once

#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "GameTimer.h"
#include "LayerStack.h"
#include "Scene/Scene.h"
#include "Window.h"

#include "Util/MathHelper.h"
#include "Util/Util.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

extern const int g_NumFrameResources;
extern const UINT32 g_NumObjects;

namespace Blainn
{
	class DXRenderingContext;
	class DXResourceManager;


	struct ApplicationDesc
	{
		std::string Name = "Blainnflare";
		uint32_t WindowWidth = 1600, WindowHeight = 800;
		bool WindowDecorated = false;
		bool Fullscreen = false;
		bool VSync = true;
	};

	class Application
	{
	public:
		Application(HINSTANCE hinstance, const ApplicationDesc& description = ApplicationDesc());
		virtual ~Application();

		static inline Application& Get() { return *s_Instance; }

		bool Initialize();

		int Run();
		void Close();

		virtual void OnInit() {};
		virtual void OnShutdown() {};
		virtual void OnUpdate() {};

		virtual void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::shared_ptr<Scene> GetScene() { return m_Scene; }
		inline HINSTANCE GetAppInstance() const { return m_hInstance; }
		inline Window& GetWindow() const { return *m_Window; }

		HINSTANCE GetNativeInstance() const { return m_hInstance; }
		std::shared_ptr<DXRenderingContext> GetRenderingContext() const { return m_RenderingContext; }

		float AspectRatio() const;

	protected:
		virtual void OnResize();
		virtual void Update(const GameTimer& timer);
		virtual void Draw(const GameTimer& timer);

		// Window events
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowMoved(WindowMovedEvent& e);
		bool OnWindowMinimize(WindowMinimizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);

	protected:
		void CalculateFrameStats();

	protected:
		ApplicationDesc m_AppDescription;

		HINSTANCE m_hInstance;

		static Application* s_Instance;

		std::shared_ptr<Window> m_Window;
		std::shared_ptr<DXRenderingContext> m_RenderingContext;

		GameTimer m_Timer;

		float m_lastFrameTime = 0.f;

		LayerStack m_LayerStack;

		int m_ClientWidth;
		int m_ClientHeight;

		bool m_bPaused = false;
		bool m_bMinimized = false;
		bool m_bMaximized = false;
		bool m_bResizing = false;
		bool m_bFullscreen = false;

		std::shared_ptr<Scene> m_Scene;

		POINT m_LastMousePos;
	};


	Application* CreateApplication(HINSTANCE hInstance);
}
