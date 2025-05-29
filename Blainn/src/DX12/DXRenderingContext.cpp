#include "pch.h"
#include "DXRenderingContext.h"

#include <comdef.h>

#include "CascadeShadowMaps.h"
#include "Components/ActorComponents/DirectionalLightComponent.h"
#include "Components/ActorComponents/PointLightComponent.h"
#include "Components/ActorComponents/StaticMeshComponent.h"
#include "Components/ActorComponents/TransformComponent.h"
#include "Components/DebugComponents/WorldGridComponent.h"
#include "Components/ComponentManager.h"
#include "Core/Camera.h"
#include "Core/GameObject.h"
#include "Core/GameTimer.h"
#include "Core/Window.h"
#include "DXSceneVisitor.h"
#include "DXShader.h"
#include "DXModel.h"
#include "EffectPSO.h"
#include "Scene/Scene.h"
#include "ShaderTypes.h"
#include "TexturedQuadPSO.h"

#include <unordered_set>

#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/Device.h>
#include <dx12lib/DescriptorAllocator.h>
#include <dx12lib/Material.h>
#include <dx12lib/Mesh.h>
#include <dx12lib/PipelineStateObject.h>
#include <dx12lib/RenderTarget.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/Scene.h>
#include <dx12lib/SceneNode.h>
#include <dx12lib/SwapChain.h>
#include <dx12lib/Texture.h>

#include "D3D12MemAlloc.h"
#include "DeferredLightingPSO.h"
#include "GBuffer.h"
#include "GPassPSO.h"
#include "VertexTypes.h"
#include "assimp/Vertex.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

extern const int g_NumFrameResources = 3;
extern const UINT32 g_NumObjects = 10000;

inline void GenerateCubeMesh(std::vector<dx12lib::VertexPosition>& vertices, std::vector<UINT>& indices)
{
	using namespace dx12lib;
	
	vertices.push_back(dx12lib::VertexPosition(DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f))); // 0: Bottom-left-front
	vertices.push_back(dx12lib::VertexPosition(DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f))); // 1: Bottom-right-front
	vertices.push_back(dx12lib::VertexPosition(DirectX::XMFLOAT3( 1.0f,  1.0f, -1.0f))); // 2: Top-right-front
	vertices.push_back(dx12lib::VertexPosition(DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f))); // 3: Top-left-front
	vertices.push_back(dx12lib::VertexPosition(DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f))); // 4: Bottom-left-back
	vertices.push_back(dx12lib::VertexPosition(DirectX::XMFLOAT3( 1.0f, -1.0f,  1.0f))); // 5: Bottom-right-back
	vertices.push_back(dx12lib::VertexPosition(DirectX::XMFLOAT3( 1.0f,  1.0f,  1.0f))); // 6: Top-right-back
	vertices.push_back(dx12lib::VertexPosition(DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f))); // 7: Top-left-back
	
	// Define indices for 12 triangles (2 per face), counter-clockwise when viewed from outside
	indices = {
		// Front face (z = -1)
		0, 3, 1,
		1, 3, 2,

		// Back face (z = 1)
		5, 6, 4,
		4, 6, 7,

		// Left face (x = -1)
		4, 7, 0,
		0, 7, 3,

		// Right face (x = 1)
		1, 2, 5,
		5, 2, 6,

		// Top face (y = 1)
		3, 7, 2,
		2, 7, 6,

		// Bottom face (y = -1)
		0, 1, 4,
		4, 1, 5 
	};	
}

inline void GenerateQuad(
	std::pair<float, float> topLeft,
	std::pair<float, float> bottomRight,
	std::vector<dx12lib::VertexPosition>& vertices,
	std::vector<UINT>& indices)
{
	vertices.clear();
	indices.clear();

	dx12lib::VertexPosition vertex;
	auto& pos = vertex.Position;
	pos.x = topLeft.first;
	pos.y = topLeft.second;
	pos.z = 0.f;
	vertices.push_back(vertex);

	pos.x = bottomRight.first;
	pos.y = topLeft.second;
	pos.z = 0.f;
	vertices.push_back(vertex);

	pos.x = bottomRight.first;
	pos.y = bottomRight.second;
	pos.z = 0.f;
	vertices.push_back(vertex);

	pos.x = topLeft.first;
	pos.y = bottomRight.second;
	pos.z = 0.f;
	vertices.push_back(vertex);

	indices	= {
		0, 2, 3,
		0, 1, 2
	};
}

static void GenerateSphereMesh(std::vector<dx12lib::VertexPosition>& vertices, std::vector<UINT>& indices) {
	int segments = 16;
	float pi = 3.1415926535f;

	// Generate vertices
	for (int i = 0; i <= segments; ++i) {
		float theta = (float)i / segments * pi;
		for (int j = 0; j <= segments; ++j) {
			float phi = (float)j / segments * 2.0f * pi;
			dx12lib::VertexPosition v;
			v.Position.x = sin(theta) * cos(phi);
			v.Position.y = sin(theta) * sin(phi);
			v.Position.z = cos(theta);
			vertices.push_back(v);
		}
	}

	// Generate indices
	for (int i = 0; i < segments; ++i) {
		for (int j = 0; j < segments; ++j) {
			int idx0 = i * (segments + 1) + j;
			int idx1 = idx0 + 1;
			int idx2 = (i + 1) * (segments + 1) + j;
			int idx3 = idx2 + 1;

			// First triangle
			indices.push_back(idx0);
			indices.push_back(idx1);
			indices.push_back(idx2);

			// Second triangle
			indices.push_back(idx1);
			indices.push_back(idx3);
			indices.push_back(idx2);
		}
	}
}

namespace Blainn
{
	DXRenderingContext::~DXRenderingContext()
	{
		m_Device->GetCommandQueue().Flush();
	}


	void DXRenderingContext::Init(std::shared_ptr<Window> wnd)
	{
		//CD3DX12_RESOURCE_DESC::Tex2D();

		ID3D12Debug* dController;
		ID3D12Debug1* dController1;
		D3D12GetDebugInterface(IID_PPV_ARGS(&dController));
		dController->EnableDebugLayer();
		dController->QueryInterface(IID_PPV_ARGS(&dController1));
		dController1->SetEnableGPUBasedValidation(true);



		m_Device = dx12lib::Device::Create();
		m_SwapChain = m_Device->CreateSwapChain(wnd->GetNativeWindow(), m_BackBufferFormat);
		//m_SwapChain->SetVSync(false);

		m_ScreenViewport = CD3DX12_VIEWPORT(0.f, 0.f, float(wnd->GetWidth()), float(wnd->GetHeight()));
		m_ScissorRect = { 0, 0, LONG_MAX, LONG_MAX };

		m_bIsInitialized = true;
	}


	void DXRenderingContext::CreateResources()
	{
		auto vertexShader = DXShader(L"src\\Shaders\\ShadowMap.hlsl", true, nullptr, "main", "vs_5_1");
		m_SMPSO = std::make_shared<ShadowMapPSO>(
			m_Device,
			vertexShader.GetByteCode()
		);

		UINT width = 4096, height = 4096;

		m_CascadeShadowMaps = std::make_shared<Blainn::CascadeShadowMaps>(m_Device, DirectX::XMUINT2{ width, height });
		m_CascadeShadowMaps->UpdateCascadeDistances({20.f, 50.f, 100.f, 1000.f});
		
		m_GBuffer = std::make_shared<GBuffer>(m_Device, width, height);

		vertexShader = DXShader(L"src\\Shaders\\DeferredShading\\VS_FullScreenQuad.hlsl", true, nullptr, "VS_FullScreenQuad", "vs_5_1");
		auto pixelShader = DXShader(L"src\\Shaders\\DeferredShading\\PS_DirectionalLight.hlsl", true, nullptr, "PS_DirectionalLight", "ps_5_1");
		m_DirLightPSO = std::make_shared<DirectLightsPSO>(m_Device, vertexShader.GetByteCode(), pixelShader.GetByteCode());
		m_DirLightPSO->SetGBuffer(m_GBuffer);
		m_DirLightPSO->SetShadowMap(m_CascadeShadowMaps);

		vertexShader = DXShader(L"src\\Shaders\\DeferredShading\\VS_LightVolumes.hlsl", true, nullptr, "VS_LightVolume", "vs_5_1");
		pixelShader = DXShader(L"src\\Shaders\\DeferredShading\\PS_PointLight.hlsl", true, nullptr, "PS_PointLight", "ps_5_1");
		m_PointLightPSO = std::make_shared<PointLightsPSO>(m_Device, vertexShader.GetByteCode(), pixelShader.GetByteCode());
		m_PointLightPSO->SetGBuffer(m_GBuffer);

		vertexShader = DXShader(L"src\\Shaders\\TexturedQuad.hlsl", true, nullptr, "VS_TexturedQuad", "vs_5_1");
		pixelShader = DXShader(L"src\\Shaders\\TexturedQuad.hlsl", true, nullptr, "PS_TexturedQuad", "ps_5_1");
		m_TexturedQuadPSO = std::make_shared<TexturedQuadPSO>(m_Device, vertexShader.GetByteCode(), pixelShader.GetByteCode());

		// creating necessary meshes
		{
			auto& queue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
			auto commandList = queue.GetCommandList();
		
			dx12lib::VertexPosition pos(DirectX::SimpleMath::Vector3{ 0.0f, 0.0f, 0.0f });
			std::vector<dx12lib::VertexPosition> positions(4, pos);
			m_FullQuadVertexBuffer = commandList->CopyVertexBuffer(positions);
			{
				std::vector<dx12lib::VertexPosition> vertices;
				std::vector<UINT> indices;

				GenerateQuad({ -1.f, 1.f }, { -0.6f, 0.6f }, vertices, indices);
				auto quad = std::make_shared<dx12lib::Mesh>();
				auto quadVB = commandList->CopyVertexBuffer(vertices);
				auto quadIB = commandList->CopyIndexBuffer(indices);
				quad->SetVertexBuffer(0, quadVB);
				quad->SetIndexBuffer(quadIB);
				m_DebugBufferQuads.push_back(quad);
			}

			{
				std::vector<dx12lib::VertexPosition> vertices;
				std::vector<UINT> indices;

				GenerateQuad({ -0.6f, 1.f }, { -0.2f, 0.6f }, vertices, indices);
				auto quad = std::make_shared<dx12lib::Mesh>();
				auto quadVB = commandList->CopyVertexBuffer(vertices);
				auto quadIB = commandList->CopyIndexBuffer(indices);
				quad->SetVertexBuffer(0, quadVB);
				quad->SetIndexBuffer(quadIB);
				m_DebugBufferQuads.push_back(quad);
			}

			{
				std::vector<dx12lib::VertexPosition> vertices;
				std::vector<UINT> indices;

				GenerateQuad({ -0.2f, 1.f }, { 0.2f, 0.6f }, vertices, indices);
				auto quad = std::make_shared<dx12lib::Mesh>();
				auto quadVB = commandList->CopyVertexBuffer(vertices);
				auto quadIB = commandList->CopyIndexBuffer(indices);
				quad->SetVertexBuffer(0, quadVB);
				quad->SetIndexBuffer(quadIB);
				m_DebugBufferQuads.push_back(quad);
			}

			m_SphereLightVolumeMesh = std::make_shared<dx12lib::Mesh>();
			std::vector<dx12lib::VertexPosition> sphereVertices;
			std::vector<UINT> sphereIndices;
			GenerateSphereMesh(sphereVertices, sphereIndices);
			auto sphereVertexBuffer = commandList->CopyVertexBuffer(sphereVertices);
			auto sphereIndexBuffer = commandList->CopyIndexBuffer(sphereIndices);
		
			m_SphereLightVolumeMesh->SetVertexBuffer(0, sphereVertexBuffer);
			m_SphereLightVolumeMesh->SetIndexBuffer(sphereIndexBuffer);
		
			queue.ExecuteCommandList(commandList);
		}

		DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };
		width = UINT(m_ScreenViewport.Width);
		height = UINT(m_ScreenViewport.Height);
		auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_BackBufferFormat, width, height, 1, 1,
													sampleDesc.Count, sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		
		D3D12_CLEAR_VALUE colorClearValue;
		colorClearValue.Format = colorDesc.Format;
		colorClearValue.Color[0] = 0.0;
		colorClearValue.Color[1] = 0.0;
		colorClearValue.Color[2] = 0.0;
		colorClearValue.Color[3] = 1.f;
		
		auto colorTexture = m_Device->CreateTexture(colorDesc, &colorClearValue);
		colorTexture->SetName(L"Color Render Target");
		
		auto depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_DepthStencilFormat, width, height, 1, 1,
													sampleDesc.Count, sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		
		D3D12_CLEAR_VALUE depthClearValue;
		depthClearValue.Format = depthDesc.Format;
		depthClearValue.DepthStencil = { 1.0f, 0 };
		
		auto depthTexture = m_Device->CreateTexture(depthDesc, &depthClearValue);
		depthTexture->SetName(L"Depth Render Target");
		
		m_RenderTarget.AttachTexture(dx12lib::AttachmentPoint::Color0, colorTexture);
		m_RenderTarget.AttachTexture(dx12lib::AttachmentPoint::DepthStencil, depthTexture);
	}


	void DXRenderingContext::OnUpdate()
	{
	}


	void DXRenderingContext::UpdateObjectsConstantBuffers()
	{
	}


	void DXRenderingContext::UpdateMaterialsConstantBuffers(std::unordered_set<DXMaterial*> materials)
	{
	}


	void DXRenderingContext::UpdateMainPassConstantBuffers(const GameTimer& gt, const Camera& camera)
	{
		using namespace DirectX;
		PerPassData passCB;

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
		passCB.RenderTargetSize = { float(m_ScreenViewport.Width), float(m_ScreenViewport.Height) };
		passCB.InvRenderTargetSize = SimpleMath::Vector2(1.f / passCB.RenderTargetSize.x, 1.f / passCB.RenderTargetSize.y);
		passCB.NearZ = camera.GetNearPlane();
		passCB.FarZ = camera.GetFarPlane();
		passCB.TotalTime = gt.TotalTime();
		passCB.DeltaTime = gt.DeltaTime();

		auto& dirLightComponents = ComponentManager::Get().GetComponents<DirectionalLightComponent>();
		std::vector<DirectionalLight> dirLights;
		dirLights.reserve(dirLightComponents.size());
		for (auto& dl : dirLightComponents)
		{
			auto owner = dl->GetOwner();
			if (!owner)
				continue;
		
			auto transform = owner->GetComponent<TransformComponent>();
			if (!transform)
				continue;
		
			auto& d = dl->GetDirectionalLight();
			d.DirectionWS = SimpleMath::Vector4(transform->GetWorldForwardVector());

			m_CascadeShadowMaps->UpdateCascadeMatrices(camera, DirectX::SimpleMath::Vector3(d.DirectionWS));
			break;
		}

		m_GBuffer->GetGPassPSO()->SetPerPassData(passCB);
		
		m_DirLightPSO->SetPassData(passCB);
		m_DirLightPSO->SetCascadeData(m_CascadeShadowMaps->GetCascadeData());
		
		m_PointLightPSO->SetPassData(passCB);
	}


	void DXRenderingContext::Draw()
	{
		m_SwapChain->WaitForSwapChain();

		const auto& meshes = ComponentManager::Get().GetComponents<StaticMeshComponent>();//scene.GetRenderObjects();

		auto& commandQueue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

		CascadeShadowMapsPass(meshes);
		GeometryPass(meshes);
		DeferredLightingPass();

		{
			auto commandList = commandQueue.GetCommandList();
			commandList->SetViewport(m_ScreenViewport);
			commandList->SetScissorRect(m_ScissorRect);
			commandList->SetRenderTarget(m_RenderTarget);
		
			m_TexturedQuadPSO->SetTexture(m_GBuffer->GetTexture(GBuffer::TextureType::AlbedoOpacity));
			m_TexturedQuadPSO->Apply(*commandList);
			m_DebugBufferQuads[0]->Draw(*commandList);
		
			m_TexturedQuadPSO->SetTexture(m_GBuffer->GetTexture(GBuffer::TextureType::NormalSpec));
			m_TexturedQuadPSO->Apply(*commandList);
			m_DebugBufferQuads[1]->Draw(*commandList);
		
			m_TexturedQuadPSO->SetTexture(m_GBuffer->GetTexture(GBuffer::TextureType::Depth));
			m_TexturedQuadPSO->Apply(*commandList);
			m_DebugBufferQuads[2]->Draw(*commandList);
		
			commandQueue.ExecuteCommandList(commandList);
		}

		auto commandList = commandQueue.GetCommandList();

		auto swapChainBackBuffer = m_SwapChain->GetRenderTarget().GetTexture(dx12lib::AttachmentPoint::Color0);
		auto renderTarget = m_RenderTarget.GetTexture(dx12lib::AttachmentPoint::Color0);

		D3D12_RESOURCE_DESC srcDesc = renderTarget->GetD3D12ResourceDesc();
		D3D12_RESOURCE_DESC dstDesc = swapChainBackBuffer->GetD3D12ResourceDesc();

		//commandList->ResolveSubresource(swapChainBackBuffer, renderTarget);
		commandList->CopyResource(swapChainBackBuffer, renderTarget);

		commandQueue.ExecuteCommandList(commandList);

		m_SwapChain->Present(renderTarget);
	}


	void DXRenderingContext::Resize(int newWidth, int newHeight)
	{
		m_SwapChain->Resize(newWidth, newHeight);

		m_ScreenViewport = CD3DX12_VIEWPORT(0.f, 0.f, float(newWidth), float(newHeight));

		m_RenderTarget.Resize(newWidth, newHeight);
		m_GBuffer->GetRenderTarget().Resize(newWidth, newHeight);
	}

	void DXRenderingContext::CascadeShadowMapsPass(
		const std::unordered_set<std::shared_ptr<StaticMeshComponent>>& meshes)
	{
		auto& commandQueue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

		std::vector<std::shared_ptr<dx12lib::CommandList>> shadowCommandLists(CASCADE_COUNT);

		for(int32_t i = 0; i < CASCADE_COUNT; ++i)
		{
			auto commandList = commandQueue.GetCommandList();
			shadowCommandLists[i] = commandList;

			ShadowVisitor shadowPass(*commandList, *m_SMPSO);

			auto sliceData = m_CascadeShadowMaps->GetCascadeData();
			ShadowMapPSO::PerPassData smPassData;
			smPassData.ViewProj = sliceData.viewProjMats[i];

			m_SMPSO->SetPerPassData(smPassData);
			commandList->SetViewport(m_CascadeShadowMaps->GetViewport());
			commandList->SetScissorRect(m_ScissorRect);

			auto& rt = m_CascadeShadowMaps->GetRenderTarget(CascadeSlice(i));

			commandList->ClearDepthStencilTexture(
				rt.GetTexture(dx12lib::AttachmentPoint::DepthStencil),
				D3D12_CLEAR_FLAG_DEPTH);

			commandList->SetRenderTarget(rt);

			for (auto& it : meshes)
			{
				auto owners = it->GetOwners();
				if (owners.empty())
					continue;

				std::vector<DirectX::SimpleMath::Matrix> worldMats;
				worldMats.reserve(owners.size());
				for (auto weakOwner : owners)
				{
					auto owner = weakOwner.lock();
					auto transform = owner->GetComponent<TransformComponent>();
					if (!transform)
						continue;

					auto ttr = transform->GetWorldMatrix().Transpose();
					worldMats.push_back(ttr);
				}

				m_SMPSO->SetWorldMatrices(worldMats);
				it->OnRender(shadowPass);
			}
		}

		commandQueue.ExecuteCommandLists(shadowCommandLists);
	}

	void DXRenderingContext::GeometryPass(const std::unordered_set<std::shared_ptr<StaticMeshComponent>>& meshes)
	{
		auto& commandQueue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

		auto commandList = commandQueue.GetCommandList();

		m_GBuffer->ClearRenderTarget(commandList);

		commandList->SetViewport(m_ScreenViewport);
		commandList->SetScissorRect(m_ScissorRect);
		commandList->SetRenderTarget(m_GBuffer->GetRenderTarget());

		GeometryVisitor geometryPass(*commandList, *m_GBuffer->GetGPassPSO());

		// auto& pointLightComponents = ComponentManager::Get().GetComponents<PointLightComponent>();
		// for (auto& pl : pointLightComponents)
		// {
		// 	auto owner = pl->GetOwner();
		// 	if (!owner)
		// 		continue;
		//
		// 	auto transform = owner->GetComponent<TransformComponent>();
		// 	if (!transform)
		// 		continue;
		//
		// 	auto l = pl->GetPointLight();
		// 	l.PositionWS = SimpleMath::Vector4(transform->GetWorldPosition());
		// 	SimpleMath::Matrix T = SimpleMath::Matrix::CreateTranslation(SimpleMath::Vector3(l.PositionWS));
		// 	SimpleMath::Matrix S = SimpleMath::Matrix::CreateScale(l.Radius);
		// 	SimpleMath::Matrix W = (S * T).Transpose();
		// 	m_GBuffer->GetGPassPSO()->SetWorldMatrices({{W}});
		// 	//m_PointLightPSO->SetObjectData({W});
		// 	m_GBuffer->GetGPassPSO()->Apply(*commandList);
		//
		// 	m_SphereLightVolumeMesh->Draw(*commandList);
		// }

		for (auto& it : meshes)
		{
			auto owners = it->GetOwners();
			if (owners.empty())
				continue;

			std::vector<DirectX::SimpleMath::Matrix> worldMats;
			worldMats.reserve(owners.size());
			for (auto weakOwner : owners)
			{
				auto owner = weakOwner.lock();
				auto transform = owner->GetComponent<TransformComponent>();
				if (!transform)
					continue;

				auto ttr = transform->GetWorldMatrix().Transpose();
				worldMats.push_back(ttr);
			}

			m_GBuffer->GetGPassPSO()->SetWorldMatrices(worldMats);
			it->OnRender(geometryPass);
		}
		
		commandQueue.ExecuteCommandList(commandList);
	}

	void DXRenderingContext::DeferredLightingPass()
	{
		DirectionalLightsPass();
		PointLightsPass();
	}

	void DXRenderingContext::DirectionalLightsPass()
	{
		auto& commandQueue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

		auto commandList = commandQueue.GetCommandList();


		float clearValue[4];
		clearValue[0] = 0.0;
		clearValue[1] = 0.0;
		clearValue[2] = 0.0;
		clearValue[3] = 1.f;
		commandList->ClearTexture(m_RenderTarget.GetTexture(dx12lib::AttachmentPoint::Color0), clearValue);
		commandList->ClearDepthStencilTexture(m_RenderTarget.GetTexture(dx12lib::AttachmentPoint::DepthStencil), D3D12_CLEAR_FLAG_DEPTH, 1.0f);

		commandList->SetViewport(m_ScreenViewport);
		commandList->SetScissorRect(m_ScissorRect);
		commandList->SetRenderTarget(m_RenderTarget);
		
		auto& dirLightComponents = ComponentManager::Get().GetComponents<DirectionalLightComponent>();
		for (auto& dl : dirLightComponents)
		{
			auto owner = dl->GetOwner();
			if (!owner)
				continue;
		
			auto transform = owner->GetComponent<TransformComponent>();
			if (!transform)
				continue;
		
			auto& d = dl->GetDirectionalLight();
			d.DirectionWS = SimpleMath::Vector4(transform->GetWorldForwardVector());

			m_DirLightPSO->SetDirectionalLight(d);
			m_DirLightPSO->Apply(*commandList);
			commandList->SetVertexBuffer(0, m_FullQuadVertexBuffer);
			commandList->Draw(4);
			
			//dirLights.push_back(d);
		}
		commandQueue.ExecuteCommandList(commandList);
	}

	void DXRenderingContext::PointLightsPass()
	{
		auto& commandQueue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

		auto commandList = commandQueue.GetCommandList();
		commandList->SetViewport(m_ScreenViewport);
		commandList->SetScissorRect(m_ScissorRect);
		commandList->SetRenderTarget(m_RenderTarget);
		auto& pointLightComponents = ComponentManager::Get().GetComponents<PointLightComponent>();
		for (auto& pl : pointLightComponents)
		{
			auto owner = pl->GetOwner();
			if (!owner)
				continue;
		
			auto transform = owner->GetComponent<TransformComponent>();
			if (!transform)
				continue;
		
			auto l = pl->GetPointLight();
			l.PositionWS = SimpleMath::Vector4(transform->GetWorldPosition());
			SimpleMath::Matrix T = SimpleMath::Matrix::CreateTranslation(SimpleMath::Vector3(l.PositionWS));
			SimpleMath::Matrix S = SimpleMath::Matrix::CreateScale(l.Radius + 0.1f);
			SimpleMath::Matrix W = (S * T).Transpose();
			m_PointLightPSO->SetLightData(l);
			m_PointLightPSO->SetObjectData({W});
			m_PointLightPSO->Apply(*commandList);

			m_SphereLightVolumeMesh->Draw(*commandList);
		}
		commandQueue.ExecuteCommandList(commandList);
	}

	void DXRenderingContext::SpotLightsPass()
	{
	}
}

