#pragma once

#include "DX12/DXGraphicsPrimitive.h"
#include "DX12/DXRenderingContext.h"
#include "DX12/DXResourceManager.h"
#include "DX12/DXShader.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "GameTimer.h"
#include "LayerStack.h"
#include "Window.h"

#include "DX12/DXUploadBuffer.h"

#include "Util/MathHelper.h"
#include "Util/Util.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Blainn
{
	struct ObjectConstants
	{
		DirectX::XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
	};

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

		inline HINSTANCE GetAppInstance() const { return m_hInstance; }
		inline Window& GetWindow() const { return *m_Window; }

		HINSTANCE GetNativeInstance() const { return m_hInstance; }

	protected:
		virtual void OnResize();
		virtual void Update(const GameTimer& timer);
		virtual void Draw(const GameTimer& timer);

		// Window events
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowMoved(WindowMovedEvent& e);
		bool OnWindowMinimize(WindowMinimizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);

		virtual bool OnMouseDown(MouseButtonDownEvent& e);
		virtual bool OnMouseUp(MouseButtonReleasedEvent& e);
		virtual bool OnMouseMove(MouseMovedEvent& e);

		bool OnKeyPressed(KeyPressedEvent& e);


		void BuildDescriptorHeaps();
		void BuildConstantBuffers();
		void BuildRootSignature();
		void BuildShaders();
		void BuildPSO();

		float AspectRatio() const;

	protected:
		void CalculateFrameStats();

	protected:
		ApplicationDesc m_AppDescription;

		HINSTANCE m_hInstance;

		static Application* s_Instance;

		std::shared_ptr<Window> m_Window;
		std::shared_ptr<DXRenderingContext> m_RenderingContext;
		std::shared_ptr<DXResourceManager> m_ResourceManager;

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

		std::shared_ptr<DXGraphicsPrimitive> box;
		std::shared_ptr<DXGraphicsPrimitive> m_Square;

		std::shared_ptr<DXShader> m_VShader;
		std::shared_ptr<DXShader> m_PShader;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CBVHeap;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PSO;
		std::unique_ptr<DXUploadBuffer<ObjectConstants>> m_OjbectCB;

		// Should probably be a part of a camera class.
		DirectX::XMFLOAT4X4 m_View = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 m_World = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 m_Proj = MathHelper::Identity4x4();

		float m_Theta = 1.5 * DirectX::XM_PI;
		float m_Phi = DirectX::XM_PIDIV4;
		float m_Radius = 5.f;
		// ...

		POINT m_LastMousePos;
	};


	Application* CreateApplication(HINSTANCE hInstance);
}
