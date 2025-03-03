#include "pch.h"
#include "DXRenderingContext.h"

#include "Core/GameTimer.h"
#include "Core/Window.h"
#include "DXFrameResource.h"
#include "DXGraphicsPrimitive.h"
#include "DXShader.h"
#include "Scene/Actor.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

extern const int g_NumFrameResources = 3;
extern const UINT32 g_NumObjects = 10000;

namespace Blainn
{

	DXRenderingContext::~DXRenderingContext()
	{
	}

	void DXRenderingContext::Init(std::shared_ptr<Window> wnd)
	{
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_DXGIFactory)));

		m_Device = std::make_shared<DXDevice>();

		ThrowIfFailed(m_Device->Device()->CreateFence(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_Fence)
		));

		m_RtvDescriptorSize = m_Device->Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescriptorSize = m_Device->Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvUavDescriptorSize = m_Device->Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ThrowIfFailed(m_Device->Device()->CreateCommandQueue(
			&queueDesc,
			IID_PPV_ARGS(&m_CommandQueue)
		));

		ThrowIfFailed(m_Device->Device()->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_DirectCmdListAlloc)
		));

		ThrowIfFailed(m_Device->Device()->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_DirectCmdListAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(&m_CommandList)
		));

		m_CommandList->Close();

		m_SwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = wnd->GetWidth();
		sd.BufferDesc.Height = wnd->GetHeight();
		sd.BufferDesc.RefreshRate.Numerator = 144;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = m_BackBufferFormat;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = s_SwapChainBufferCount;
		sd.OutputWindow = wnd->GetNativeWindow();
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ThrowIfFailed(m_DXGIFactory->CreateSwapChain(
			m_CommandQueue.Get(),
			&sd,
			m_SwapChain.GetAddressOf()
		));

		m_bIsInitialized = true;
	}

	void DXRenderingContext::CreateResources()
	{
		ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

		BuildRootSignature();
		BuildFrameResources();
		BuildDescriptorHeaps();
		BuildConstantBufferViews();
		BuildPipelineState();

		ThrowIfFailed(m_CommandList->Close());
		ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
		FlushCommandQueue();
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> DXRenderingContext::BeginFrame()
	{
		auto cmdListAlloc = m_CurrentFrameResource->GetCommandAlloc();
		ThrowIfFailed(cmdListAlloc->Reset());
		
		ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque"].Get()));

		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

		m_CommandList->RSSetViewports(1, &m_ScreenViewport);
		m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

		m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::SteelBlue, 0, nullptr);
		m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
		
		m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

		ID3D12DescriptorHeap* descriptorHeaps[]{ m_CBVHeap.Get() };
		m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

		int passCbvIndex = m_PassConstantBufferOffset + m_CurrentFrameResourceIndex;
		auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CBVHeap->GetGPUDescriptorHandleForHeapStart());
		passCbvHandle.Offset(passCbvIndex, m_CbvSrvUavDescriptorSize);
		m_CommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);
		return m_CommandList;
	}

	void DXRenderingContext::EndFrame()
	{
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		));

		ThrowIfFailed(m_CommandList->Close());

		ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		ThrowIfFailed(m_SwapChain->Present(0, 0));

		m_CurrBackBuffer = (m_CurrBackBuffer + 1) % s_SwapChainBufferCount;

		m_CurrentFrameResource->SetFence(++m_CurrentFence);

		m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);
	}

	void DXRenderingContext::DrawRenderActors(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<Actor>>& actors)
	{
		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

		auto objectCB = m_CurrentFrameResource->GetObjectBufferResource();

		for (UINT64 i = 0; i < actors.size(); ++i)
		{
			auto& actor = actors[i];
			UINT cbvIndex = m_CurrentFrameResourceIndex * g_NumObjects + actor->m_ObjectConstantBufferIndex;
			auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CBVHeap->GetGPUDescriptorHandleForHeapStart());
			cbvHandle.Offset(cbvIndex, m_CbvSrvUavDescriptorSize);

			cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);

			actor->m_GraphicsPrimitive->Draw();
		}
	}

	void DXRenderingContext::OnUpdate()
	{
		m_CurrentFrameResourceIndex = (m_CurrentFrameResourceIndex + 1) % g_NumFrameResources;
		m_CurrentFrameResource = m_FrameResources[m_CurrentFrameResourceIndex].get();

		if (m_CurrentFrameResource->GetFence() != 0 && m_Fence->GetCompletedValue() < m_CurrentFrameResource->GetFence())
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
			ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFrameResource->GetFence(), eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	void DXRenderingContext::UpdateObjectsConstantBuffers(const std::vector<std::shared_ptr<Actor>>& objects)
	{
		auto currentObjectCB = m_CurrentFrameResource->GetObjectConstantBuffer();
		for (auto& obj : objects)
		{
			if (obj->m_NumFramesDirty > 0)
			{
				ObjectConstants objConstants;
				objConstants.World = obj->m_WorldMatrix.Transpose();

				currentObjectCB->CopyData(obj->m_ObjectConstantBufferIndex, objConstants);

				obj->m_NumFramesDirty--;
			}
		}
	}

	void DXRenderingContext::UpdateMainPassConstantBuffers(const GameTimer& gt, const DirectX::SimpleMath::Matrix& viewM, const DirectX::SimpleMath::Matrix& projM, const DirectX::SimpleMath::Vector3& eyePos)
	{
		using namespace DirectX;
		DirectX::SimpleMath::Matrix view = viewM;
		DirectX::SimpleMath::Matrix proj = projM;

		DirectX::SimpleMath::Matrix viewProj = view * proj;
		DirectX::SimpleMath::Matrix invView = view.Invert();
		DirectX::SimpleMath::Matrix invProj = proj.Invert();
		DirectX::SimpleMath::Matrix invViewProj = viewProj.Invert();

		m_MainPassCB.View = view.Transpose();
		m_MainPassCB.InvView = invView.Transpose();
		m_MainPassCB.Proj = proj.Transpose();
		m_MainPassCB.InvProj = invProj.Transpose();
		m_MainPassCB.ViewProj = viewProj.Transpose();
		//m_MainPassCB.ViewProj = {
			//1.63085, 0.52443, 0.37662, 0.37625,
			//0.0, 2.09077, -0.5005, -0.5,
			//-0.78665, 1.08724, 0.7808, 0.78002,
			//8.63390e-07, -1.15119e-06, 14.01401, 15
		//};
		m_MainPassCB.InvViewProj = invViewProj.Transpose();
		m_MainPassCB.EyePosW = eyePos;
		m_MainPassCB.RenderTargetSize = { 100, 100 };
		m_MainPassCB.InvRenderTargetSize = SimpleMath::Vector2(1.f / m_MainPassCB.RenderTargetSize.x, 1.f / m_MainPassCB.RenderTargetSize.y);
		m_MainPassCB.NearZ = 0.01f;
		m_MainPassCB.FarZ = 100000.f;
		m_MainPassCB.TotalTime = gt.TotalTime();
		m_MainPassCB.DeltaTime = gt.DeltaTime();

		DXUploadBuffer<PassConstants>* currPassCB = m_CurrentFrameResource->GetPassConstantBuffer();
		currPassCB->CopyData(0, m_MainPassCB);
	}

	void DXRenderingContext::Resize(int newWidth, int newHeight)
	{
		assert(m_Device->Device());
		assert(m_SwapChain);
		assert(m_DirectCmdListAlloc);

		FlushCommandQueue();

		ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

		for (size_t i = 0; i < s_SwapChainBufferCount; ++i)
			m_SwapChainBuffer[i].Reset();
		m_DepthStencilBuffer.Reset();

		ThrowIfFailed(m_SwapChain->ResizeBuffers(
			s_SwapChainBufferCount,
			newWidth, newHeight,
			m_BackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		));

		m_CurrBackBuffer = 0;

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());
		for (size_t i = 0; i < s_SwapChainBufferCount; ++i)
		{
			ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
			m_Device->Device()->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
			rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
		}

		D3D12_RESOURCE_DESC dsDesc;
		dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dsDesc.Alignment = 0;
		dsDesc.Width = newWidth;
		dsDesc.Height = newHeight;
		dsDesc.DepthOrArraySize = 1;
		dsDesc.MipLevels = 1;

		dsDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = m_DepthStencilFormat;
		optClear.DepthStencil.Depth = 1.f;
		optClear.DepthStencil.Stencil = 0;
		ThrowIfFailed(m_Device->Device()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&dsDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())
		));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = m_DepthStencilFormat;
		dsvDesc.Texture2D.MipSlice = 0;
		m_Device->Device()->CreateDepthStencilView(
			m_DepthStencilBuffer.Get(),
			&dsvDesc,
			DepthStencilView()
		);

		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_DepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		));

		ThrowIfFailed(m_CommandList->Close());
		ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		FlushCommandQueue();

		m_ScreenViewport.TopLeftX = 0;
		m_ScreenViewport.TopLeftY = 0;
		m_ScreenViewport.Width = (float)newWidth;
		m_ScreenViewport.Height = (float)newHeight;
		m_ScreenViewport.MinDepth = 0.f;
		m_ScreenViewport.MaxDepth = 1.f;

		m_ScissorRect = { 0, 0, newWidth, newHeight };
	}

	void DXRenderingContext::FlushCommandQueue()
	{
		m_CurrentFence++;

		ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

		if (m_Fence->GetCompletedValue() < m_CurrentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

			ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	void DXRenderingContext::WaitForGPU()
	{
	}

	void DXRenderingContext::CreateDepthStencilBuffer(int width, int height)
	{
	}

	void DXRenderingContext::BuildDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = s_SwapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_Device->Device()->CreateDescriptorHeap(
			&rtvHeapDesc,
			IID_PPV_ARGS(m_RTVHeap.GetAddressOf())
		));


		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_Device->Device()->CreateDescriptorHeap(
			&dsvHeapDesc,
			IID_PPV_ARGS(m_DSVHeap.GetAddressOf())
		));

		m_PassConstantBufferOffset = g_NumObjects * g_NumFrameResources;

		D3D12_DESCRIPTOR_HEAP_DESC CBVDesc;
		CBVDesc.NumDescriptors = (g_NumObjects + 1) * g_NumFrameResources;
		CBVDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		CBVDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		CBVDesc.NodeMask = 0;
		ThrowIfFailed(m_Device->Device()->CreateDescriptorHeap(
			&CBVDesc,
			IID_PPV_ARGS(&m_CBVHeap)
		));
	}

	void DXRenderingContext::BuildConstantBufferViews()
	{
		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
		
		for (int frameIndex = 0; frameIndex < g_NumFrameResources; ++frameIndex)
		{
			auto objectCBResource = m_FrameResources[frameIndex]->GetObjectBufferResource();
			for (UINT i = 0; i < g_NumObjects; ++i)
			{
				D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCBResource->GetGPUVirtualAddress();

				cbAddress += i * objCBByteSize;

				INT heapIndex = frameIndex * g_NumObjects + i;
				auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CBVHeap->GetCPUDescriptorHandleForHeapStart());
				handle.Offset(heapIndex, m_CbvSrvUavDescriptorSize);

				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
				cbvDesc.BufferLocation = cbAddress;
				cbvDesc.SizeInBytes = objCBByteSize;
				
				m_Device->Device()->CreateConstantBufferView(&cbvDesc, handle);
			}
		}

		for (int frameIndex = 0; frameIndex < g_NumFrameResources; ++frameIndex)
		{
			auto passCBResource = m_FrameResources[frameIndex]->GetPassBufferResource();
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCBResource->GetGPUVirtualAddress();

			INT heapIndex = m_PassConstantBufferOffset + frameIndex;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CBVHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, m_CbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = passCBByteSize;

			m_Device->Device()->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	void DXRenderingContext::BuildRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE cbvTable0;
		cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE cbvTable1;
		cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

		CD3DX12_ROOT_PARAMETER slotRootParameter[2];

		slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
		slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
			_countof(slotRootParameter),
			slotRootParameter,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig, &errorBlob);

		if (errorBlob)
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		ThrowIfFailed(hr);

		ThrowIfFailed(m_Device->Device()->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&m_RootSignature)
		));

	}

	void DXRenderingContext::BuildPipelineState()
	{
		m_Shaders["vertex_color"] = std::make_shared<DXShader>(L"src\\Shaders\\color.hlsl", true, nullptr, "VSmain", "vs_5_1");
		m_Shaders["pixel_color"] = std::make_shared<DXShader>(L"src\\Shaders\\color.hlsl", true, nullptr, "PSmain", "ps_5_1");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		auto inputLayout = DXGraphicsPrimitive::Vertex::GetElementLayout();
		psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(m_Shaders["vertex_color"]->GetByteCode()->GetBufferPointer()),
			m_Shaders["vertex_color"]->GetByteCode()->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(m_Shaders["pixel_color"]->GetByteCode()->GetBufferPointer()),
			m_Shaders["pixel_color"]->GetByteCode()->GetBufferSize()
		};
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = m_BackBufferFormat;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.DSVFormat = m_DepthStencilFormat;
		ThrowIfFailed(m_Device->Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSOs["opaque"])));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC wireframePSOdesc = psoDesc;
		wireframePSOdesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		ThrowIfFailed(m_Device->Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSOs["wireframe"])));
	}

	void DXRenderingContext::BuildFrameResources()
	{
		for (int i = 0; i < g_NumFrameResources; i++)
			m_FrameResources.push_back(
				std::make_unique<DXFrameResource>(1, g_NumObjects));
	}

}

