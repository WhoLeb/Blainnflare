#include "pch.h"
#include "DXStaticMesh.h"

#include "Core/Application.h"

namespace Blainn
{
	DXStaticMesh::DXStaticMesh(
		const std::vector<Vertex>& vertices,
		const std::vector<UINT32>* indices)
	{
		auto resourceManager = Application::Get().GetResourceManager();
		m_VertexCount = UINT32(vertices.size());
		assert(m_VertexCount >= 1 && "There should at least be 1 vertex!");

		UINT64 bufferSize = sizeof(Vertex) * m_VertexCount;
		UINT64 vertexSize = sizeof(Vertex);

		m_VertexBuffer = resourceManager->CreateBuffer(
			bufferSize,
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_COMMON
		);
		resourceManager->WriteToDefaultBuffer(m_VertexBuffer, vertices.data(), bufferSize);

		if (indices)
		{
			m_IndexCount = UINT32(indices->size());

			m_bHasIndexBuffer = m_IndexCount > 0;
			if (!m_bHasIndexBuffer) return;

			bufferSize = sizeof(UINT32) * m_IndexCount;
			m_IndexBuffer = resourceManager->CreateBuffer(
				bufferSize,
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_COMMON
			);
			resourceManager->WriteToDefaultBuffer(m_IndexBuffer, indices->data(), bufferSize);
		}

	}

	void DXStaticMesh::Bind()
	{
		assert(m_VertexBuffer);
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList = Application::Get().GetRenderingContext()->GetCommandList();

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
		vbv.StrideInBytes = sizeof(Vertex);
		vbv.SizeInBytes = m_VertexCount * sizeof(Vertex);

		D3D12_VERTEX_BUFFER_VIEW vertexBuffers[1] = { vbv };
		cmdList->IASetVertexBuffers(0, 1, vertexBuffers);

		if (m_bHasIndexBuffer)
		{
			D3D12_INDEX_BUFFER_VIEW ibv = {};
			ibv.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
			ibv.Format = DXGI_FORMAT_R32_UINT;
			ibv.SizeInBytes = m_IndexCount * sizeof(UINT32);

			cmdList->IASetIndexBuffer(&ibv);
		}
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void DXStaticMesh::Draw()
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList = Application::Get().GetRenderingContext()->GetCommandList();

		if (m_bHasIndexBuffer)
			cmdList->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
		else
			cmdList->DrawInstanced(m_VertexCount, 1, 0, 0);
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreateCube(float side, const DirectX::SimpleMath::Color& color)
	{
		using namespace DirectX;
		std::vector<Blainn::DXStaticMesh::Vertex> vertices =
		{
			{ SimpleMath::Vector3(-side / 2, -side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(-side / 2, +side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(+side / 2, +side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(+side / 2, -side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(-side / 2, -side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(-side / 2, +side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(+side / 2, +side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(+side / 2, -side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)}
		};

		std::vector<UINT32> indices =
		{
			// front face
			0, 1, 2,
			0, 2, 3,
			// back face
			4, 6, 5,
			4, 7, 6,
			// left face
			4, 5, 1,
			4, 1, 0,
			// right face
			3, 2, 6,
			3, 6, 7,
			// top face
			1, 5, 6,
			1, 6, 2,
			// bottom face
			4, 0, 3,
			4, 3, 7
		};

		Application::Get().GetResourceManager()->StartUploadCommands();
		auto mesh = std::make_shared<Blainn::DXStaticMesh>(vertices, &indices);
		Application::Get().GetResourceManager()->EndUploadCommands();
		return mesh;
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreateSphere(float radius, UINT sliceCount, UINT stackCount, const DirectX::SimpleMath::Color& color)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		std::vector<DXStaticMesh::Vertex> vertices;
		std::vector<UINT32> indices;

		vertices.push_back({
				.Position = {0.f, radius, 0},
				.Normal = {0.f, 1.f, 0.f},
				.Color = color,
				.UV = {0.f, 0.f}
			});

		float phiStep = XM_PI / stackCount;
		float thetaStep = XM_2PI / sliceCount;

		for (UINT i = 1; i < stackCount; ++i)
		{
			float phi = i * phiStep;
			for (UINT j = 0; j <= sliceCount; ++j)
			{
				float theta = j * thetaStep;

				Vertex v;

				v.Position.x = radius * sinf(phi) * cosf(theta);
				v.Position.y = radius * cosf(phi);
				v.Position.z = radius * sinf(phi) * sin(theta);

				v.Normal = v.Position / radius;

				v.Color = color;

				v.UV.x = theta / (XM_2PI);
				v.UV.y = phi / (XM_PI);

				vertices.push_back(v);
			}
		}

		vertices.push_back({
				.Position = {0.f, -radius, 0},
				.Normal = {0.f, -1.f, 0.f},
				.Color = color,
				.UV = {0.f, 1.f}
			});

		UINT baseIndex = 1;
		UINT ringVertexCount = sliceCount + 1;

		for (UINT i = 1; i <= sliceCount; ++i)
		{
			indices.push_back(i + 1);
			indices.push_back(i);
			indices.push_back(0);
		}

		for (UINT i = 0; i < stackCount - 2; ++i)
		{
			for (UINT j = 0; j < sliceCount; ++j)
			{
				indices.push_back(baseIndex + i * ringVertexCount + j);
				indices.push_back(baseIndex + i * ringVertexCount + j + 1);
				indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

				indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
				indices.push_back(baseIndex + i * ringVertexCount + j + 1);
				indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
			}
		}

		UINT southPoleIndex = (UINT)vertices.size() - 1;
		baseIndex = southPoleIndex - ringVertexCount;

		for (UINT i = 0; i < sliceCount; ++i)
		{
			indices.push_back(southPoleIndex);
			indices.push_back(baseIndex + i);
			indices.push_back(baseIndex + i + 1);
		}

		Application::Get().GetResourceManager()->StartUploadCommands();
		auto mesh = std::make_shared<DXStaticMesh>(vertices, &indices);
		Application::Get().GetResourceManager()->EndUploadCommands();
		return mesh;
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreateTorus(float majorRadius, float minorRadius, UINT majorSegments, UINT minorSegments, const DirectX::SimpleMath::Color& color)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;
		std::vector<DXStaticMesh::Vertex> vertices;
		std::vector<UINT32> indices;

		float majorStep = XM_2PI / majorSegments;
		float minorStep = XM_2PI / minorSegments;

		for (UINT i = 0; i <= majorSegments; ++i) {
			float majorAngle = i * majorStep;

			float cosMajor = cosf(majorAngle);
			float sinMajor = sinf(majorAngle);

			Vector3 center = { majorRadius * cosMajor, 0, majorRadius * sinMajor };
			Matrix rotation = Matrix::CreateFromAxisAngle({ 0.f, 1.f, 0.f }, majorAngle);

			for (UINT j = 0; j <= minorSegments; ++j) {
				float minorAngle = j * minorStep;
				float cosMinor = cosf(minorAngle);
				float sinMinor = sinf(minorAngle);


				Vector3 pos = {
					(majorRadius + minorRadius * cosMinor) * cosMajor,
					minorRadius * sinMinor,
					(majorRadius + minorRadius * cosMinor) * sinMajor
				};


				Vertex v;
				v.Position = pos;

				v.Normal = {
					cosMinor * cosMajor,
					sinMajor,
					cosMinor * sinMajor
				};

				v.Color = color;

				v.UV = { (float)i / majorSegments, (float)j / minorSegments };

				vertices.push_back(v);
			}
		}

		UINT ringVertexCount = minorSegments + 1;
		for (UINT i = 0; i < majorSegments; ++i) {
			for (UINT j = 0; j < minorSegments; ++j) {
				UINT a = i * ringVertexCount + j;
				UINT b = (i + 1) * ringVertexCount + j;
				UINT c = (i + 1) * ringVertexCount + j + 1;
				UINT d = i * ringVertexCount + j + 1;

				// Correct CCW winding order
				indices.insert(indices.end(), { a, d, b });
				indices.insert(indices.end(), { d, c, b });
			}
		}

		Application::Get().GetResourceManager()->StartUploadCommands();
		auto mesh = std::make_shared<DXStaticMesh>(vertices, &indices);
		Application::Get().GetResourceManager()->EndUploadCommands();
		return mesh;
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreatePyramid(float width, float height, const DirectX::SimpleMath::Color& color)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;
		std::vector<DXStaticMesh::Vertex> vertices;
		std::vector<UINT32> indices;

		float halfWidth = width / 2.0f;
		float halfHeight = height / 2.0f;

		vertices = {
			{{ -halfWidth, -halfHeight, -halfWidth }, {0,-1,0}, color, {0,1}},
			{{ halfWidth, -halfHeight, -halfWidth }, {0,-1,0}, color, {1,1}},
			{{ halfWidth, -halfHeight, halfWidth }, {0,-1,0}, color, {1,0}},
			{{ -halfWidth, -halfHeight, halfWidth }, {0,-1,0}, color, {0,0}},
			{{ 0, halfHeight, 0 }, {0,1,0}, color, {0.5f,0.5f}}
		};

		indices.insert(indices.end(), { 0,1,2, 0,2,3 });

		indices.insert(indices.end(), { 0,4,1 });
		indices.insert(indices.end(), { 1,4,2 });
		indices.insert(indices.end(), { 2,4,3 });
		indices.insert(indices.end(), { 3,4,0 });

		Application::Get().GetResourceManager()->StartUploadCommands();
		auto mesh = std::make_shared<DXStaticMesh>(vertices, &indices);
		Application::Get().GetResourceManager()->EndUploadCommands();
		return mesh;
	}

}
