#include "pch.h"
#include "CascadeShadowMaps.h"

#include "ShadowMap.h"

#include <dx12lib/CommandList.h>
#include <dx12lib/Device.h>
#include <dx12lib/RenderTarget.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/Texture.h>

#include <limits>

#include "Core/Camera.h"

using namespace dx12lib;
using namespace Blainn;

CascadeShadowMaps::CascadeShadowMaps(std::shared_ptr<dx12lib::Device> device, DirectX::XMUINT2 size)
	: m_ShadowMaps(CascadeSlice::NumSlices)
	, m_Size(size)
{
	m_Viewport = { 0.f, 0.f, float(size.x), float(size.y), 0.f, 1.f };
	for (auto& rt : m_ShadowMaps)
	{
		rt = std::make_shared<ShadowMap>(device, size.x, size.y);
	}
}

const std::shared_ptr<dx12lib::Texture>& CascadeShadowMaps::GetSlice(CascadeSlice slice) const
{
	return m_ShadowMaps[slice]->GetTexture();
}

const dx12lib::RenderTarget& Blainn::CascadeShadowMaps::GetRenderTarget(CascadeSlice slice) const
{
	return m_ShadowMaps[slice]->GetRenderTarget();
}

dx12lib::RenderTarget& Blainn::CascadeShadowMaps::GetRenderTarget(CascadeSlice slice)
{
	return m_ShadowMaps[slice]->GetRenderTarget();
}

DirectX::XMUINT2 CascadeShadowMaps::GetSize() const
{
	return m_Size;
}

D3D12_VIEWPORT Blainn::CascadeShadowMaps::GetViewport() const
{
	return m_Viewport;
}

void CascadeShadowMaps::UpdateCascadeData(DirectX::SimpleMath::Matrix& invViewProj,
	DirectX::SimpleMath::Vector3 lightDirection)
{
	using namespace DirectX::SimpleMath;
	std::vector<Vector4> frustumCorners;
	frustumCorners.reserve(8);
	for (uint32_t z = 0; z < 2; ++z)
	{
		for (uint32_t y = 0; y < 2; ++y)
		{
			for (uint32_t x = 0; x < 2; ++x)
			{
				const Vector4 pt =
					Vector4::Transform(
						Vector4(
							2.f * x - 1.f,
							2.f * y - 1.f,
							z, 1.f),
						invViewProj);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}
	
	std::vector<Vector4> frustumEdges =
	{
		frustumCorners[4] - frustumCorners[0],
		frustumCorners[5] - frustumCorners[1],
		frustumCorners[6] - frustumCorners[2],
		frustumCorners[7] - frustumCorners[3]
	};

	for (int32_t cascade = 0; cascade < CASCADE_COUNT; ++cascade)
	{
		float dNear = m_CascadeViewWindows[cascade].first;
		float dFar = m_CascadeViewWindows[cascade].second;

		
		std::vector<Vector4> cascadeCorners;
		for (int8_t i = 0; i < 4; ++i)
		{
			//cascadeCorners.push_back(frustumCorners[i] + cascade * frustumEdges[i] / CASCADE_COUNT); // * m_CascadeViewWindows[cascade].first);
			cascadeCorners.push_back(frustumCorners[i] + frustumEdges[i] * dNear);
		}
		
		for (int8_t i = 0; i < 4; ++i)
		{
			//cascadeCorners.push_back(frustumCorners[i] + (cascade + 1) * frustumEdges[i] / CASCADE_COUNT); // * m_CascadeViewWindows[cascade].second);
			cascadeCorners.push_back(frustumCorners[i] + frustumEdges[i] * dFar);
		}

		Vector3 center = Vector3::Zero;
		for (const auto& v : cascadeCorners)
		{
			center += Vector3(v);
		}
		center /= cascadeCorners.size();

		const auto lightView = Matrix::CreateLookAt(
			center - lightDirection,
			center,
			Vector3::Up
		);
	
		float minX = FLT_MAX;
		float minY = FLT_MAX;
		float minZ = FLT_MAX;
		float maxX = FLT_MIN;
		float maxY = FLT_MIN;
		float maxZ = FLT_MIN;

		for (const auto& v : cascadeCorners)
		{
			const auto trf = Vector4::Transform(v, lightView);
			minX = std::min<float>(minX, trf.x);
			minY = std::min<float>(minY, trf.y);
			minZ = std::min<float>(minZ, trf.z);
			maxX = std::max<float>(maxX, trf.x);
			maxY = std::max<float>(maxY, trf.y);
			maxZ = std::max<float>(maxZ, trf.z);
		}

		constexpr float zMult = 10.f;
		minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
		maxZ = (maxZ < 0) ? maxZ / zMult : maxZ * zMult;

		auto lightProjection = Matrix::CreateOrthographicOffCenter(minX, maxX, minY, maxY, minZ, maxZ);
		m_CascadeData.viewProjMats[cascade] = (lightView * lightProjection).Transpose();
		m_CascadeData.distances[cascade] = 100 * dFar;
	}
}

void CascadeShadowMaps::UpdateCascadeMatrices(const Blainn::Camera& camera, const DirectX::SimpleMath::Vector3& lightDirection)
{
	using namespace DirectX;
	using namespace DirectX::SimpleMath;
	
	bool hasDistances = false;
	for (auto& i : m_CascadeData.distances)
		hasDistances |= i > FLT_EPSILON;
	if (!hasDistances)
	{
		m_CascadeData.distances[0] = camera.GetFarPlane() * 0.1f;
		m_CascadeData.distances[1] = camera.GetFarPlane() * 0.3f;
		m_CascadeData.distances[2] = camera.GetFarPlane() * 0.5f;
		m_CascadeData.distances[3] = camera.GetFarPlane();
	}
	
	if (m_CascadeData.distances)
	for (int32_t cascade = 0; cascade < CASCADE_COUNT; ++cascade)
	{
		float nearPlane = cascade == 0 ? camera.GetNearPlane() : m_CascadeData.distances[cascade - 1];
		float farPlane = m_CascadeData.distances[cascade];

		auto projMat = Matrix::CreatePerspectiveFieldOfView(
				camera.GetFieldOfView(),
				camera.GetAspectRatio(),
				nearPlane, farPlane
			);

		auto viewProj = camera.GetViewMatrix() * projMat;
		auto invViewProj = viewProj.Invert();

		std::vector<Vector4> frustumCorners;
		frustumCorners.reserve(8);
		for (int32_t z = 0; z < 2; ++z)
		{
			for (int32_t y = 0; y < 2; ++y)
			{
				for (int32_t x = 0; x < 2; ++x)
				{
					const Vector4 pt =
						Vector4::Transform(
							Vector4(
								2.f * x - 1.f,
								2.f * y - 1.f,
								z, 1.f),
							invViewProj);
					frustumCorners.push_back(pt / pt.w);
				}
			}
		}

		Vector3 center = Vector3::Zero;
		for (const auto& v : frustumCorners)
		{
			center += Vector3(v);
		}
		center /= frustumCorners.size();

		const auto lightView = Matrix::CreateLookAt(
			center - lightDirection,
			center,
			Vector3::Up
		);
	
		float minX = FLT_MAX;
		float minY = FLT_MAX;
		float minZ = FLT_MAX;
		float maxX = FLT_MIN;
		float maxY = FLT_MIN;
		float maxZ = FLT_MIN;

		for (const auto& v : frustumCorners)
		{
			const auto trf = Vector4::Transform(v, lightView);
			minX = std::min<float>(minX, trf.x);
			minY = std::min<float>(minY, trf.y);
			minZ = std::min<float>(minZ, trf.z);
			maxX = std::max<float>(maxX, trf.x);
			maxY = std::max<float>(maxY, trf.y);
			maxZ = std::max<float>(maxZ, trf.z);
		}

		constexpr float zMult = 10.f;
		minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
		maxZ = (maxZ < 0) ? maxZ / zMult : maxZ * zMult;

		auto lightProjection = Matrix::CreateOrthographicOffCenter(minX, maxX, minY, maxY, minZ, maxZ);
		m_CascadeData.viewProjMats[cascade] = (lightView * lightProjection).Transpose();
	}
}

void CascadeShadowMaps::UpdateCascadeDistances(const std::vector<float>& distances)
{
	for (int i = 0; i < distances.size() && i < CASCADE_COUNT; ++i)
		m_CascadeData.distances[i] = distances[i];
}

CascadeData& CascadeShadowMaps::GetCascadeData()
{
	return m_CascadeData;
}

std::vector<DXGI_FORMAT> CascadeShadowMaps::GetShadowMapFormats() const
{
	std::vector<DXGI_FORMAT> smFormats(CascadeSlice::NumSlices);
	for (int i = CascadeSlice::Slice0; i < CascadeSlice::NumSlices; ++i)
	{
		auto texture = m_ShadowMaps[i]->GetTexture();
		if (texture)
			smFormats[i] = texture->GetD3D12ResourceDesc().Format;
	}

	return smFormats;
}

void CascadeShadowMaps::Reset()
{
	m_ShadowMaps = ShadowMapList(CascadeSlice::NumSlices);
}

ShadowMapPSO::ShadowMapPSO(
	std::shared_ptr<dx12lib::Device> device,
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType
)
	: m_Device(device)
	, m_DirtyFlags(DF_All)
	, m_PrimitiveTopologyType(primitiveTopologyType)
{
	CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
	rootParameters[RootParameters::PerObjectDataSB	].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[RootParameters::PerPassDataCB	].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(
        RootParameters::NumRootParameters,
        rootParameters,
        0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

	m_RootSignature = m_Device->CreateRootSignature(rootSignatureDescription.Desc_1_1);

    // Setup the pipeline state.
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
    } pipelineStateStream;

    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 0;

    CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);

    pipelineStateStream.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.RasterizerState = rasterizerState;
    pipelineStateStream.InputLayout = VertexPositionNormalTangentBitangentTexture::InputLayout;
    pipelineStateStream.PrimitiveTopologyType = primitiveTopologyType;
    pipelineStateStream.DSVFormat = depthBufferFormat;
    pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.SampleDesc = { 1, 0 };

    m_PipelineStateObject = m_Device->CreatePipelineStateObject(pipelineStateStream);
}

void ShadowMapPSO::Apply(dx12lib::CommandList& commandList)
{
	commandList.SetPipelineState(m_PipelineStateObject);
	commandList.SetGraphicsRootSignature(m_RootSignature);

    if (m_DirtyFlags & DF_PerObjectData)
    {
        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::PerObjectDataSB, m_ObjectData);
    }

    if (m_DirtyFlags & DF_PerPassData)
    {
        commandList.SetGraphicsDynamicConstantBuffer(RootParameters::PerPassDataCB, m_PassData);
    }

	m_DirtyFlags = DF_None;
}
