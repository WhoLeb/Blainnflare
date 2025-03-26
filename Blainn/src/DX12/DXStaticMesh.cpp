#include "pch.h"
#include "DXStaticMesh.h"

#include "Core/Application.h"
#include "DXResourceManager.h"

#include "D3D12MemAlloc.h"

namespace Blainn
{
	DXStaticMesh::DXStaticMesh(
		const std::vector<Vertex>& vertices,
		const std::vector<UINT32>* indices)
	{
		auto resourceManager = Application::Get().GetResourceManager();
		resourceManager->StartUploadCommands();

		m_VertexCount = UINT32(vertices.size());
		assert(m_VertexCount >= 1 && "There should at least be 1 vertex!");

		UINT64 bufferSize = sizeof(Vertex) * m_VertexCount;
		UINT64 vertexSize = sizeof(Vertex);

		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		m_VBAlloc = resourceManager->CreateAllocation(
			resourceDesc,
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_COMMON
		);
		D3D12_SUBRESOURCE_DATA vertexData;
		vertexData.pData = vertices.data();
		vertexData.RowPitch = bufferSize;
		vertexData.SlicePitch = bufferSize;

		resourceManager->WriteToAllocation(m_VBAlloc, 0, 1, &vertexData);

		if (indices)
		{
			m_IndexCount = UINT32(indices->size());

			m_bHasIndexBuffer = m_IndexCount > 0;
			if (!m_bHasIndexBuffer) return;

			bufferSize = sizeof(UINT32) * m_IndexCount;
			resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
			m_IBAlloc = resourceManager->CreateAllocation(
				resourceDesc,
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_COMMON
			);

			D3D12_SUBRESOURCE_DATA indexData;
			indexData.pData = indices->data();
			indexData.RowPitch = bufferSize;
			indexData.SlicePitch = bufferSize;

			resourceManager->WriteToAllocation(m_IBAlloc, 0, 1, &indexData);
		}

		resourceManager->EndUploadCommands();
	}

	inline DXStaticMesh::~DXStaticMesh()
	{
		//m_VBAlloc->Release();
		//m_IBAlloc->Release();
	}

	void DXStaticMesh::Bind()
	{
		assert(m_VBAlloc);
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList = Application::Get().GetRenderingContext()->GetCommandList();

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = m_VBAlloc->GetResource()->GetGPUVirtualAddress();
		vbv.StrideInBytes = sizeof(Vertex);
		vbv.SizeInBytes = m_VertexCount * sizeof(Vertex);

		D3D12_VERTEX_BUFFER_VIEW vertexBuffers[1] = { vbv };
		cmdList->IASetVertexBuffers(0, 1, vertexBuffers);

		if (m_bHasIndexBuffer)
		{
			D3D12_INDEX_BUFFER_VIEW ibv = {};
			ibv.BufferLocation = m_IBAlloc->GetResource()->GetGPUVirtualAddress();
			ibv.Format = DXGI_FORMAT_R32_UINT;
			ibv.SizeInBytes = m_IndexCount * sizeof(UINT32);

			cmdList->IASetIndexBuffer(&ibv);
		}
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
			{ SimpleMath::Vector3(-side / 2, -side / 2, -side / 2), SimpleMath::Vector3(-0.5774f, -0.5774f, -0.5774f), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(-side / 2, +side / 2, -side / 2), SimpleMath::Vector3(-0.5774f, +0.5774f, -0.5774f), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(+side / 2, +side / 2, -side / 2), SimpleMath::Vector3(+0.5774f, +0.5774f, -0.5774f), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(+side / 2, -side / 2, -side / 2), SimpleMath::Vector3(+0.5774f, -0.5774f, -0.5774f), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(-side / 2, -side / 2, +side / 2), SimpleMath::Vector3(-0.5774f, -0.5774f, +0.5774f), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(-side / 2, +side / 2, +side / 2), SimpleMath::Vector3(-0.5774f, +0.5774f, +0.5774f), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(+side / 2, +side / 2, +side / 2), SimpleMath::Vector3(+0.5774f, +0.5774f, +0.5774f), color,	SimpleMath::Vector2(0, 0)},
			{ SimpleMath::Vector3(+side / 2, -side / 2, +side / 2), SimpleMath::Vector3(+0.5774f, -0.5774f, +0.5774f), color,	SimpleMath::Vector2(0, 0)}
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

		auto mesh = std::make_shared<Blainn::DXStaticMesh>(vertices, &indices);
		return mesh;
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreateSphere(float radius, UINT sliceCount, UINT stackCount, const DirectX::SimpleMath::Color& color)
	{
		return CreateCapsule(radius, 2 * radius, sliceCount, stackCount, 1, color);
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreateCapsule(float radius, float height, UINT sliceCount, UINT sphereStackCount, UINT cylinderStackCount, const DirectX::SimpleMath::Color& color)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;
		std::vector<DXStaticMesh::Vertex> vertices;
		std::vector<UINT32> indices;

		if (cylinderStackCount == 0)
		{
			cylinderStackCount = 1;
		}

		if (height < radius * 2.f)
			height = radius * 2.f;

		float halfHeight = (height - 2.f * radius) * 0.5f;
		float phiStep = XM_PI / (sphereStackCount + 1);
		float thetaStep = XM_2PI / sliceCount;

		UINT topPoleIndex = (UINT)vertices.size();
		vertices.push_back({ {0, halfHeight + radius, 0}, {0, 1, 0}, color, {0.5f, 0.0f} });

		UINT topRingIndex = (UINT)vertices.size();
		for (UINT i = 1; i <= sphereStackCount / 2; ++i)
		{
			float phi = i * phiStep;
			float y = halfHeight + radius * cosf(phi);
			float r = radius * sinf(phi);

			for (UINT j = 0; j < sliceCount; ++j)
			{
				float theta = j * thetaStep;

				float x = r * cosf(theta);
				float z = r * sinf(theta);

				Vector3 normal = Vector3(x, radius * cosf(phi), z);
				normal.Normalize();

				vertices.push_back({ {x, y, z}, normal, color, { (float)j / sliceCount, (float)i / sphereStackCount } });
			}
		}

		UINT cylinderStartIndex = (UINT)vertices.size() - sliceCount;
		for (UINT i = 1; i < cylinderStackCount; ++i)
		{
			float y = halfHeight - (i * (2.0f * halfHeight) / cylinderStackCount);

			for (UINT j = 0; j < sliceCount; ++j)
			{
				float theta = j * thetaStep;
				float x = radius * cosf(theta);
				float z = radius * sinf(theta);

				vertices.push_back({ {x, y, z}, {x, 0.0f, z}, color, { (float)j / sliceCount, (float)(i + cylinderStackCount / 2) / cylinderStackCount } });
			}
		}

		UINT bottomStartIndex = (UINT)vertices.size() - sliceCount;
		for (UINT i = sphereStackCount / 2; i <= sphereStackCount; ++i)
		{
			float phi = i * phiStep;
			float y = -halfHeight + radius * cosf(phi);
			float r = radius * sinf(phi);

			for (UINT j = 0; j < sliceCount; ++j)
			{
				float theta = j * thetaStep;

				float x = r * cosf(theta);
				float z = r * sinf(theta);

				Vector3 normal = Vector3(x, radius * cosf(phi), z);
				normal.Normalize();

				vertices.push_back({ {x, y, z}, normal, color, { (float)j / sliceCount, (float)i / sphereStackCount } });
			}
		}

		UINT bottomPoleIndex = (UINT)vertices.size();
		vertices.push_back({ {0, -halfHeight - radius, 0}, {0, -1, 0}, color, {0.5f, 0.5f} }); // Bottom center

		for (UINT j = 0; j < sliceCount; ++j)
			indices.insert(indices.end(), {
					topPoleIndex, topRingIndex + (j + 1) % sliceCount, topRingIndex + j
				});

		UINT ringVertexCount = sliceCount + 1;
		for (UINT i = 0; i < (sphereStackCount / 2) - 1; ++i)
		{
			for (UINT j = 0; j < sliceCount; ++j)
			{
				UINT a = topRingIndex + i * sliceCount + j;
				UINT b = topRingIndex + (i + 1) * sliceCount + j;
				UINT c = topRingIndex + (i + 1) * sliceCount + (j + 1) % sliceCount;
				UINT d = topRingIndex + i * sliceCount + (j + 1) % sliceCount;

				indices.insert(indices.end(), { d, b, a });
				indices.insert(indices.end(), { c, b, d });
			}
		}

		for (UINT i = 0; i < cylinderStackCount - 1; ++i)
		{
			for (UINT j = 0; j < sliceCount; ++j)
			{
				UINT a = cylinderStartIndex + i * sliceCount + j;
				UINT b = cylinderStartIndex + (i + 1) * sliceCount + j;
				UINT next_j = (j + 1) % sliceCount;
				UINT c = cylinderStartIndex + i * sliceCount + next_j;
				UINT d = cylinderStartIndex + (i + 1) * sliceCount + next_j;

				indices.insert(indices.end(), { c, b, a });
				indices.insert(indices.end(), { d, b, c });
			}
		}

		for (UINT i = 0; i <= sphereStackCount / 2 + 1; ++i)
		{
			for (UINT j = 0; j < sliceCount; ++j)
			{
				UINT a = bottomStartIndex + i * sliceCount + j;
				UINT b = bottomStartIndex + (i + 1) * sliceCount + j;
				UINT c = bottomStartIndex + (i + 1) * sliceCount + (j + 1) % sliceCount;
				UINT d = bottomStartIndex + i * sliceCount + (j + 1) % sliceCount;

				indices.insert(indices.end(), { d, b, a });
				indices.insert(indices.end(), { c, b, d });
			}
		}

		UINT lastRingStart = bottomPoleIndex - sliceCount;

		for (UINT j = 0; j < sliceCount; ++j)
			indices.insert(indices.end(), {
				bottomPoleIndex, lastRingStart + j, lastRingStart + (j+1) % sliceCount
				});

		auto mesh = std::make_shared<DXStaticMesh>(vertices, &indices);
		return mesh;
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreateTorus(float majorRadius, float minorRadius, UINT majorSegments, UINT minorSegments, const DirectX::SimpleMath::Color& color)
	{
		return CreateTorusKnot(1, 0, majorRadius, minorRadius, majorSegments, minorSegments, color);
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreateTorusKnot(int p, int q, float radius, float tubeRadius, UINT curveSegments, UINT tubeSegments, const DirectX::SimpleMath::Color& color)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;
		std::vector<DXStaticMesh::Vertex> vertices;
		std::vector<UINT32> indices;

		std::vector<Vector3> curvePoints;

		float phiStep = XM_2PI / curveSegments;

		for (UINT i = 0; i < curveSegments; ++i)
		{
			float phi = i * phiStep;

			float r = radius * (cosf(q * phi) + 2.f);
			float x = r * cosf(p * phi);
			float y = -sinf(q * phi);
			float z = r * sinf(p * phi);
			curvePoints.push_back({ x, y, z });
		}

		UINT ringVertexCount = tubeSegments + 1;

		for (UINT i = 0; i < curveSegments; ++i)
		{
			Vector3 tangent;

			auto next = curvePoints[(i + 1) % curvePoints.size()];
			auto curr = curvePoints[i];
			auto T = next - curr;
			T.Normalize();

			Vector3 N = T.Cross({ 0.f, 1.f, 0.f });
			N.Normalize();
			Vector3 B = N.Cross(T);

			for (UINT j = 0; j <= tubeSegments; ++j)
			{
				float theta = j * XM_2PI / tubeSegments;
				float cosTheta = cosf(theta);
				float sinTheta = cosf(theta);

				Vector3 radialOffset = N * (cosf(theta) * tubeRadius) + B * (sinf(theta) * tubeRadius);
				Vector3 pos = curr + radialOffset;

				radialOffset.Normalize();
				Vector2 texcoord = { float(i) / curveSegments, float(j) / tubeSegments };

				vertices.push_back({ pos, radialOffset, color, texcoord });
			}

		}

		for (UINT i = 0; i < curveSegments; ++i)
		{
			for (UINT j = 0; j < tubeSegments; ++j)
			{
				UINT a = i * ringVertexCount + j;
				UINT b = ((i + 1) % (curveSegments)) * ringVertexCount + j;
				UINT c = ((i + 1) % (curveSegments)) * ringVertexCount + j + 1;
				UINT d = i * ringVertexCount + j + 1;

				// Correct CCW winding order
				indices.insert(indices.end(), { a, b, d });
				indices.insert(indices.end(), { d, b, c });
			}
		}


		auto mesh = std::make_shared<DXStaticMesh>(vertices, &indices);
		return mesh;
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreatePyramid(float width, float height, const DirectX::SimpleMath::Color& color)
	{
		return CreateCylinder(width / 2.f, 0.f, height, 4, 1, color);
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreateCone(float bottomRadius, float height, UINT sliceCount, const DirectX::SimpleMath::Color& color)
	{
		return CreateCylinder(bottomRadius, 0.f, height, sliceCount, 1, color);
	}

	std::shared_ptr<Blainn::DXStaticMesh> DXStaticMesh::CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, const DirectX::SimpleMath::Color& color)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;
		std::vector<DXStaticMesh::Vertex> vertices;
		std::vector<UINT32> indices;

		float stackHeight = height / stackCount;
		float radiusStep = (topRadius - bottomRadius) / stackCount;
		UINT ringCount = stackCount + 1;

		float dTheta = XM_2PI / sliceCount;

		// Vertices
		for (UINT i = 0; i < ringCount; ++i)
		{
			float y = -0.5f * height + i * stackHeight;
			float r = bottomRadius + i * radiusStep;

			float dTheta = XM_2PI / sliceCount;

			for (UINT j = 0; j <= sliceCount; ++j)
			{
				float c = cosf(j * dTheta);
				float s = sinf(j * dTheta);

				Vertex vertex;

				vertex.Position = Vector3(r * c, y, r * s);
				vertex.Normal = Vector3(c, 0.0f, s);
				vertex.Color = color;
				vertex.UV.x = (float)j / sliceCount;
				vertex.UV.y = 1.0f - (float)i / stackCount;

				vertices.push_back(vertex);
			}
		}

		// Indices for side
		UINT ringVertexCount = sliceCount + 1;
		for (UINT i = 0; i < stackCount; ++i)
		{
			for (UINT j = 0; j < sliceCount; ++j)
			{
				indices.push_back(i * ringVertexCount + j);
				indices.push_back((i + 1) * ringVertexCount + j);
				indices.push_back((i + 1) * ringVertexCount + j + 1);

				indices.push_back(i * ringVertexCount + j);
				indices.push_back((i + 1) * ringVertexCount + j + 1);
				indices.push_back(i * ringVertexCount + j + 1);
			}
		}

		// Top Cap (Using existing top ring vertices)
		if (topRadius > 0.0f)
		{
			UINT baseIndex = (ringCount - 1) * ringVertexCount; // First vertex of top ring
			for (UINT i = 0; i < sliceCount; ++i)
			{
				indices.push_back(baseIndex + sliceCount); // Last vertex in the ring
				indices.push_back(baseIndex + (i + 1) % ringVertexCount);
				indices.push_back(baseIndex + i);
			}
		}

		// Bottom Cap (Using existing bottom ring vertices)
		if (bottomRadius > 0.0f)
		{
			UINT baseIndex = 0; // First vertex of bottom ring
			for (UINT i = 0; i < sliceCount; ++i)
			{
				indices.push_back(baseIndex + sliceCount); // Last vertex in the ring
				indices.push_back(baseIndex + i);
				indices.push_back(baseIndex + (i + 1) % ringVertexCount);
			}
		}

		auto mesh = std::make_shared<DXStaticMesh>(vertices, &indices);
		return mesh;
	}

}
