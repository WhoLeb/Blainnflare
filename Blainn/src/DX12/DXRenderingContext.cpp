#include "pch.h"
#include "DXRenderingContext.h"

#include "Components/ActorComponents/StaticMeshComponent.h"
#include "Components/ActorComponents/TransformComponent.h"
#include "Components/DebugComponents/WorldGridComponent.h"
#include "Components/ComponentManager.h"
#include "Core/Camera.h"
#include "Core/GameObject.h"
#include "Core/GameTimer.h"
#include "Core/Window.h"
#include "DXDevice.h"
#include "DXStaticMesh.h"
#include "DXShader.h"
#include "DXMaterial.h"
#include "DXModel.h"
#include "Scene/Scene.h"

#include <unordered_set>
#include <DX12Lib/DescriptorAllocator.h>
#include <dx12lib/RootSignature.h>

#include "D3D12MemAlloc.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

extern const int g_NumFrameResources = 3;
extern const UINT32 g_NumObjects = 10000;

namespace Blainn
{
	DXRenderingContext::~DXRenderingContext()
	{
		FlushCommandQueue();
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

	void DXRenderingContext::CreateResources(std::shared_ptr<DXResourceManager> resourceManager)
	{
		m_ResourceManager = resourceManager;
		ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

		BuildRootSignature();
		BuildFrameResources();
		BuildDescriptorHeaps();
		BuildPipelineState();

		ThrowIfFailed(m_CommandList->Close());
		ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
		FlushCommandQueue();
	}

	void DXRenderingContext::BeginFrame()
	{
		auto cmdListAlloc = m_CurrentFrameResource->GetCommandAlloc();
		ThrowIfFailed(cmdListAlloc->Reset());

		//m_ResourceManager->ResetDynamicDescriptorHeaps();
		
		ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["Opaque"].Get()));

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

		//ID3D12DescriptorHeap* descriptorHeaps[]{ m_CBVHeap.Get() };
		//m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		//m_CommandList->SetGraphicsRootSignature(m_RootSignature->GetD3D12RootSignature().Get());
		m_ResourceManager->SetRootSignature(m_CommandList, m_RootSignature);
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
		FlushCommandQueue();
	}

	void DXRenderingContext::Draw()
	{
		BeginFrame();

		auto passCB = m_CurrentFrameResource->GetPassBufferResource();
		m_CommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

		auto objectCB = m_CurrentFrameResource->GetObjectBufferResource();
		auto matCB = m_CurrentFrameResource->GetMaterialsBufferResource();

		const auto& meshes = ComponentManager::Get().GetComponents<StaticMeshComponent>();//scene.GetRenderObjects();

		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		for (auto& it : meshes)
		{
			auto owner = it->GetOwner();
			if (!owner) continue;

			DXFrameInfo frameInfo;
			frameInfo.CommandList = m_CommandList;
			frameInfo.ObjectCBAddress = objectCB->GetGPUVirtualAddress()
				+ CBIndexManager::Get().GetCBIdx(owner->GetUUID()) * objCBByteSize;
			frameInfo.MaterialCBAddress = matCB->GetGPUVirtualAddress();
			frameInfo.MatCBSize = matCBByteSize;

			//auto objCBAddress = objectCB->GetGPUVirtualAddress()

			//auto matCBAddress = 

			//m_CommandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
			//m_CommandList->SetGraphicsRootConstantBufferView(1, matCBAddress);

			it->OnRender(frameInfo);
		}

		m_CommandList->SetPipelineState(m_PSOs["LineList"].Get());
		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		auto& lineListComponents = ComponentManager::Get().GetComponents<WorldGridComponent>();
		for (auto& lineListComponent : lineListComponents)
		{
			auto owner = lineListComponent->GetOwner();
			if (!owner) continue;

			auto bufferIndex = CBIndexManager::Get().GetCBIdx(owner->GetUUID());

			auto objCBAddress = objectCB->GetGPUVirtualAddress();
			objCBAddress += bufferIndex * objCBByteSize;

			m_CommandList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			lineListComponent->Render();
		}

		EndFrame();
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

	void DXRenderingContext::UpdateObjectsConstantBuffers()
	{
		auto currentObjectCB = m_CurrentFrameResource->GetObjectConstantBuffer();
		
		std::unordered_set<DXMaterial*> dirtyMats;
		std::unordered_set<UUID> updatedThisFrame;

		const auto& renderObjects = ComponentManager::Get().GetComponents<StaticMeshComponent>();
		for (auto& mesh : renderObjects)
		{
			auto owner = mesh->GetOwner();
			if (!owner) continue;

			auto transform = owner->GetComponent<TransformComponent>();
			if (!transform) continue;

			auto meshMaterials = mesh->GetModel()->GetMaterials();
			for (auto& mat : meshMaterials)
				if (mat->NumFramesDirty > 0)
					dirtyMats.insert(mat);

			UUID uuid = owner->GetUUID();
			UINT32 bufferIdx = CBIndexManager::Get().GetCBIdx(uuid);
			if (bufferIdx == UINT32_MAX)
			{
				OutputDebugString(L"Object uuid was not correlated with buffer index correctly");
				continue;
			}

			if (transform->GetFramesDirty() > 0 && updatedThisFrame.find(uuid) == updatedThisFrame.end())
			{
				updatedThisFrame.insert(uuid);

				ObjectConstants objConstants;
				objConstants.World = transform->GetWorldMatrix().Transpose();

				currentObjectCB->CopyData(bufferIdx, objConstants);

				transform->DecreaseFramesDirty();
			}
		}

		const auto& worldGrid = ComponentManager::Get().GetComponents<WorldGridComponent>();
		for (auto& wg : worldGrid)
		{
			UINT32 idx = CBIndexManager::Get().GetCBIdx(wg->GetOwner()->GetUUID());

			ObjectConstants objectConstants;
			objectConstants.World = DirectX::SimpleMath::Matrix::Identity;

			currentObjectCB->CopyData(idx, objectConstants);
		}

		UpdateMaterialsConstantBuffers(dirtyMats);
	}

	void DXRenderingContext::UpdateMaterialsConstantBuffers(std::unordered_set<DXMaterial*> materials)
	{
		auto currMatCB = m_CurrentFrameResource->GetMaterialsConstantBuffer();
		for (auto& mat : materials)
		{
			DirectX::SimpleMath::Matrix matTransform = mat->MatTransform;
			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.Frensel = mat->Fresel;
			matConstants.Roughness = mat->Roughness;

			currMatCB->CopyData(
				MaterialIndexManager::Get().GetMatIdx(mat->uuid),
				matConstants
			);
			mat->NumFramesDirty--;
		}
	}

	void DXRenderingContext::UpdateMainPassConstantBuffers(const GameTimer& gt, const Camera& camera)
	{
		using namespace DirectX;
		PassConstants passCB;

		DirectX::SimpleMath::Matrix view = camera.GetViewMatrix();
		DirectX::SimpleMath::Matrix proj = camera.GetProjectionMatrix();

		DirectX::SimpleMath::Matrix viewProj = view * proj;
		DirectX::SimpleMath::Matrix invView = view.Invert();
		DirectX::SimpleMath::Matrix invProj = proj.Invert();
		DirectX::SimpleMath::Matrix invViewProj = viewProj.Invert();

		passCB.View = view.Transpose();
		passCB.InvView = invView.Transpose();
		passCB.Proj = proj.Transpose();
		passCB.InvProj = invProj.Transpose();
		passCB.ViewProj = viewProj.Transpose();
		passCB.InvViewProj = invViewProj.Transpose();
		passCB.EyePosW = camera.GetPosition();
		passCB.RenderTargetSize = { float(camera.GetViewportWidth()), float(camera.GetViewportHeight()) };
		passCB.InvRenderTargetSize = SimpleMath::Vector2(1.f / passCB.RenderTargetSize.x, 1.f / passCB.RenderTargetSize.y);
		passCB.NearZ = camera.GetNearPlane();
		passCB.FarZ = camera.GetFarPlane();
		passCB.TotalTime = gt.TotalTime();
		passCB.DeltaTime = gt.DeltaTime();

		passCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
		passCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		passCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
		passCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		passCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
		passCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
		passCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

		DirectX::SimpleMath::Vector3 pos;
		pos.x = 3 * cos(gt.TotalTime());
		pos.y = 3.f;
		pos.z = 3 * sin(gt.TotalTime());
		passCB.Lights[3].Strength = { 1.7f, 1.0f, 1.0f };
		passCB.Lights[3].Position = pos;
		passCB.Lights[3].falloffStart = 0.1f;
		passCB.Lights[3].falloffEnd = 10.f;
		//passCB.Lights[3] = 10.f;


		DXUploadBuffer<PassConstants>* currPassCB = m_CurrentFrameResource->GetPassConstantBuffer();
		currPassCB->CopyData(0, passCB);
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

		m_RTVAllocation = std::make_unique<dx12lib::DescriptorAllocation>(m_ResourceManager->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, s_SwapChainBufferCount));
		
		for (UINT i = 0; i < s_SwapChainBufferCount; ++i)
		{
			ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
			m_Device->Device()->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, m_RTVAllocation->GetDescriptorHandle(i));
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

		D3D12MA::ALLOCATION_DESC allocationDesc{};
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		D3D12MA::Allocation* allocation;
		//ThrowIfFailed(m_Device->Device()->CreateCommittedResource(
		//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		//	D3D12_HEAP_FLAG_NONE,
		//	&dsDesc,
		//	D3D12_RESOURCE_STATE_COMMON,
		//	&optClear,
		//	IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())
		//));
		ThrowIfFailed(m_ResourceManager->GetResourceAllocator()->CreateResource(
			&allocationDesc,
			&dsDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			&allocation,
			IID_PPV_ARGS(&m_DepthStencilBuffer)
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
		//D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		//rtvHeapDesc.NumDescriptors = s_SwapChainBufferCount;
		//rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		//rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//rtvHeapDesc.NodeMask = 0;
		//ThrowIfFailed(m_Device->Device()->CreateDescriptorHeap(
		//	&rtvHeapDesc,
		//	IID_PPV_ARGS(m_RTVHeap.GetAddressOf())
		//));

		m_DSVAllocation =
			std::make_unique<dx12lib::DescriptorAllocation>(
				m_ResourceManager->AllocateDescriptors(
					D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
					1
				)
			);

		//D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		//dsvHeapDesc.NumDescriptors = 1;
		//dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		//dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//dsvHeapDesc.NodeMask = 0;
		//ThrowIfFailed(m_Device->Device()->CreateDescriptorHeap(
		//	&dsvHeapDesc,
		//	IID_PPV_ARGS(m_DSVHeap.GetAddressOf())
		//));

		//m_PassConstantBufferOffset = g_NumObjects * g_NumFrameResources;

		//D3D12_DESCRIPTOR_HEAP_DESC CBVDesc;
		//CBVDesc.NumDescriptors = (g_NumObjects + 1) * g_NumFrameResources;
		//CBVDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		//CBVDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		//CBVDesc.NodeMask = 0;
		//ThrowIfFailed(m_Device->Device()->CreateDescriptorHeap(
		//	&CBVDesc,
		//	IID_PPV_ARGS(&m_CBVHeap)
		//));
	}

	void DXRenderingContext::BuildRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE1 texTable;
		texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER1 slotRootParameter[4];

		slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[1].InitAsConstantBufferView(0);
		slotRootParameter[2].InitAsConstantBufferView(1);
		slotRootParameter[3].InitAsConstantBufferView(2);

		auto staticSamplers = GetStaticSamplers();

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc(
			_countof(slotRootParameter),
			slotRootParameter,
			(UINT)staticSamplers.size(),
			staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		m_RootSignature = std::make_shared<dx12lib::RootSignature>(m_Device->Device(), rootSigDesc.Desc_1_1);

		//Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		//Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

		//HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		//	&serializedRootSig, &errorBlob);

		//if (errorBlob)
		//	OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		//ThrowIfFailed(hr);

		//ThrowIfFailed(m_Device->Device()->CreateRootSignature(
		//	0,
		//	serializedRootSig->GetBufferPointer(),
		//	serializedRootSig->GetBufferSize(),
		//	IID_PPV_ARGS(&m_RootSignature)
		//));

	}

	void DXRenderingContext::BuildPipelineState()
	{
		m_Shaders["vertex_color"] = std::make_shared<DXShader>(L"src\\Shaders\\color.hlsl", true, nullptr, "VSmain", "vs_5_1");
		m_Shaders["pixel_color"] = std::make_shared<DXShader>(L"src\\Shaders\\color.hlsl", true, nullptr, "PSmain", "ps_5_1");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		auto inputLayout = DXStaticMesh::Vertex::GetElementLayout();
		psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
		psoDesc.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
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
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = m_BackBufferFormat;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.DSVFormat = m_DepthStencilFormat;
		ThrowIfFailed(m_Device->Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSOs["Opaque"])));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC wireframePSOdesc = psoDesc;
		wireframePSOdesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		ThrowIfFailed(m_Device->Device()->CreateGraphicsPipelineState(&wireframePSOdesc, IID_PPV_ARGS(&m_PSOs["Wireframe"])));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC linelistDesc = psoDesc;
		linelistDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		linelistDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		ThrowIfFailed(m_Device->Device()->CreateGraphicsPipelineState(&linelistDesc, IID_PPV_ARGS(&m_PSOs["LineList"])));
	}

	void DXRenderingContext::BuildFrameResources()
	{
		for (int i = 0; i < g_NumFrameResources; i++)
			m_FrameResources.push_back(
				std::make_unique<DXFrameResource>(1, g_NumObjects));
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DXRenderingContext::DepthStencilView() const
	{
		return m_DSVAllocation->GetDescriptorHandle(0);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DXRenderingContext::CurrentBackBufferView() const
	{
		return m_RTVAllocation->GetDescriptorHandle(m_CurrBackBuffer);
	}

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> DXRenderingContext::GetStaticSamplers()
	{
		const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
			0,
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP
		);
		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			1,
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		);
		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			2,
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP
		);
		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			3,
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		);
		const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
			4,
			D3D12_FILTER_ANISOTROPIC,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.f,
			8
		);
		const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
			5,
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.f,
			8
		);
		return {
			pointWrap, pointClamp,
			linearWrap, linearClamp,
			anisotropicWrap, anisotropicClamp
		};
	}

}

