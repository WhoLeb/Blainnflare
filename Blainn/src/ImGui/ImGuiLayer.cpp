#include "pch.h"
#include "ImGuiLayer.h"

//#include "imgui.h"
//#include "backends/imgui_impl_win32.h"
//#include "backends/imgui_impl_dx12.h"

#include "Core/Application.h"
#include "Core/GameTimer.h"

namespace Blainn
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach()
	{
		//IMGUI_CHECKVERSION();
		//ImGui::CreateContext();
		//ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		//ImGui::StyleColorsDark();

		//ImGuiStyle& style = ImGui::GetStyle();
		//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		//{
		//	style.WindowRounding = 0.0f;
		//	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		//}

		//ImGui_ImplWin32_Init(Application::Get().GetWindow().GetNativeWindow());

		//ImGui_ImplDX12_InitInfo init_info = {};
		//init_info.Device = m_D3dDevice.Get();
		//init_info.CommandQueue = m_CommandQueue.Get();
		//init_info.NumFramesInFlight = s_SwapChainBufferCount;
		//init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		//init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		//// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
		//// (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
		//init_info.SrvDescriptorHeap = g_pd3dSrvDescHeap;
		//init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
		//init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };
		//ImGui_ImplDX12_Init(&init_info);
	}

	void ImGuiLayer::OnDetach()
	{
	}

	void ImGuiLayer::OnUpdate(const GameTimer& gt)
	{
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& event) { return true; });
	}
}
