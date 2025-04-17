#include "pch.h"
#include "DXRenderingContext.h"

#include "CascadeShadowMaps.h"
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

using Microsoft::WRL::ComPtr;
using namespace DirectX;

extern const int g_NumFrameResources = 3;
extern const UINT32 g_NumObjects = 10000;

namespace Blainn
{
	DXRenderingContext::~DXRenderingContext()
	{
		m_Device->GetCommandQueue().Flush();
	}


	void DXRenderingContext::Init(std::shared_ptr<Window> wnd)
	{
		//CD3DX12_RESOURCE_DESC::Tex2D();

		m_Device = dx12lib::Device::Create();
		m_SwapChain = m_Device->CreateSwapChain(wnd->GetNativeWindow(), m_BackBufferFormat);

		m_ScreenViewport = CD3DX12_VIEWPORT(0.f, 0.f, float(wnd->GetWidth()), float(wnd->GetHeight()));
		m_ScissorRect = { 0, 0, LONG_MAX, LONG_MAX };

		m_bIsInitialized = true;
	}


	void DXRenderingContext::CreateResources()
	{
		auto vertexShader = DXShader(L"src\\Shaders\\BasicVS.hlsl", true, nullptr, "main", "vs_5_1");
		auto pixelShader = DXShader(L"src\\Shaders\\LitPS.hlsl", true, nullptr, "main", "ps_5_1");

		m_PSOs["Opaque"] = std::make_shared<EffectPSO>(
			m_Device,
			vertexShader.GetByteCode(), pixelShader.GetByteCode(),
			true, false);
		pixelShader = DXShader(L"src\\Shaders\\UnlitPS.hlsl", true, nullptr, "main", "ps_5_1");
		m_PSOs["Unlit"] = std::make_shared<EffectPSO>(
			m_Device,
			vertexShader.GetByteCode(), pixelShader.GetByteCode(),
			false, false);

		vertexShader = DXShader(L"src\\Shaders\\ShadowMap.hlsl", true, nullptr, "main", "vs_5_1");
		pixelShader = DXShader(L"src\\Shaders\\ShadowMap.hlsl", true, nullptr, "mainPS", "ps_5_1");
		m_SMPSO = std::make_shared<ShadowMapPSO>(
			m_Device,
			vertexShader.GetByteCode(), pixelShader.GetByteCode()
		);

		DXGI_SAMPLE_DESC sampleDesc = m_Device->GetMultisampleQualityLevels(m_BackBufferFormat);

		UINT width = UINT(m_ScreenViewport.Width);
		UINT height = UINT(m_ScreenViewport.Height);
		auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_BackBufferFormat, width, height, 1, 1,
			sampleDesc.Count, sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		D3D12_CLEAR_VALUE colorClearValue;
		colorClearValue.Format = colorDesc.Format;
		colorClearValue.Color[0] = 0.274509817f;
		colorClearValue.Color[1] = 0.509803951f;
		colorClearValue.Color[2] = 0.705882370f;
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

		width = 4096, height = 4096;

		m_CascadeShadowMaps = std::make_shared<CascadeShadowMaps>();

		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthClearValue.DepthStencil = { 1.0f, 0 };

		for (int i = CascadeSlice::Slice0; i < CascadeSlice::NumSlices; ++i)
		{
			depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, width, height, 1, 1,
				1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
			width = height = width / 2;

			auto cascadeTexture = m_Device->CreateTexture(depthDesc, &depthClearValue);
			std::wstring name = L"Cascade " + std::to_wstring(i) + L" texture";
			cascadeTexture->SetName(name);

			m_CascadeShadowMaps->AttachShadowMap(CascadeSlice(i), cascadeTexture);
		}
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
		EffectPSO::PerPassData passCB;

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

		auto pointLightComponents = ComponentManager::Get().GetComponents<PointLightComponent>();
		std::vector<PointLight> pointLights;
		pointLights.reserve(pointLightComponents.size());
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
			l.PositionVS = SimpleMath::Vector4::Transform(l.PositionWS, view);

			pointLights.push_back(l);
		}

		for (auto& pso : m_PSOs)
		{
			pso.second->SetPerPassData(passCB);
			pso.second->SetShadowMap(m_CascadeShadowMaps);

			pso.second->SetPointLights(pointLights);
		}
	}


	void DXRenderingContext::Draw()
	{
		//m_SwapChain->WaitForSwapChain();

		const auto& meshes = ComponentManager::Get().GetComponents<StaticMeshComponent>();//scene.GetRenderObjects();

		auto& commandQueue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

		std::vector<std::shared_ptr<dx12lib::CommandList>> shadowCommandLists(m_CascadeShadowMaps->GetSlices().size());
		for(int32_t i = 0; i < shadowCommandLists.size(); ++i)
		{
			auto commandList = commandQueue.GetCommandList();
			shadowCommandLists[i] = commandList;

			ShadowVisitor shadowPass(*commandList, *m_SMPSO);

			auto mpPD = m_PSOs["Opaque"]->GetPerPassData();
			ShadowMapPSO::PerPassData smPassData;
			smPassData.View = mpPD.View;
			smPassData.Proj = mpPD.Proj;
			smPassData.ViewProj = mpPD.ViewProj;

			m_SMPSO->SetPerPassData(smPassData);

			auto& rt = m_CascadeShadowMaps->GetRenderTarget(CascadeSlice(i));

			commandList->ClearDepthStencilTexture(
				m_RenderTarget.GetTexture(dx12lib::AttachmentPoint::DepthStencil),
				D3D12_CLEAR_FLAG_DEPTH);

			commandList->SetRenderTarget(rt);

			for (auto& it : meshes)
			{
				auto owner = it->GetOwner();
				if (!owner)
					continue;

				auto transform = owner->GetComponent<TransformComponent>();
				if (!transform)
					continue;

				auto wm = transform->GetWorldMatrix();
				it->GetModel()->GetScene()->GetRootNode()->SetLocalTransform(wm);

				it->OnRender(shadowPass);
			}
		}


		commandQueue.ExecuteCommandLists(shadowCommandLists);

		auto commandList = commandQueue.GetCommandList();

		{
			FLOAT clearColor[4] = { 0.274509817f, 0.509803951f, 0.705882370f, 1.f };
			commandList->ClearTexture(m_RenderTarget.GetTexture(dx12lib::AttachmentPoint::Color0), clearColor);

			commandList->ClearDepthStencilTexture(
				m_RenderTarget.GetTexture(dx12lib::AttachmentPoint::DepthStencil),
				D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL
			);

		}

		commandList->SetViewport(m_ScreenViewport);
		commandList->SetScissorRect(m_ScissorRect);
		commandList->SetRenderTarget(m_RenderTarget);

		SceneVisitor opaquePass(*commandList, *m_PSOs["Opaque"], false);
		SceneVisitor unlitPass(*commandList, *m_PSOs["Unlit"], false);

		m_CascadeShadowMaps->TransitionTo(commandList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		for (auto& it : meshes)
		{
			auto owner = it->GetOwner();
			if (!owner)
				continue;

			auto transform = owner->GetComponent<TransformComponent>();
			if (!transform)
				continue;

			auto wm = transform->GetWorldMatrix();
			it->GetModel()->GetScene()->GetRootNode()->SetLocalTransform(wm);

			it->OnRender(opaquePass);
		}

		auto swapChainBackBuffer = m_SwapChain->GetRenderTarget().GetTexture(dx12lib::AttachmentPoint::Color0);
		auto msaaRenderTarget = m_RenderTarget.GetTexture(dx12lib::AttachmentPoint::Color0);

		commandList->ResolveSubresource(swapChainBackBuffer, msaaRenderTarget);

		commandQueue.ExecuteCommandList(commandList);

		m_SwapChain->Present();
	}


	void DXRenderingContext::Resize(int newWidth, int newHeight)
	{
		m_SwapChain->Resize(newWidth, newHeight);

		m_ScreenViewport = CD3DX12_VIEWPORT(0.f, 0.f, float(newWidth), float(newHeight));
		m_RenderTarget.Resize(newWidth, newHeight);
	}

}

