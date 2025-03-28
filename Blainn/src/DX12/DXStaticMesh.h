#pragma once

#include "DXRenderingContext.h"

#include "SimpleMath.h"
#include <d3d12.h>
#include <vector>

namespace D3D12MA
{
	class Allocation;
}

namespace Blainn
{
	class DXResourceManager;

	class DXStaticMesh
	{
	public:
		struct Vertex
		{
			DirectX::SimpleMath::Vector3 Position;
			DirectX::SimpleMath::Vector3 Normal;
			DirectX::SimpleMath::Vector4 Color;
			DirectX::SimpleMath::Vector2 UV;

			static std::vector<D3D12_INPUT_ELEMENT_DESC> GetElementLayout()
			{
				std::vector<D3D12_INPUT_ELEMENT_DESC> vertexDesc =
				{
					{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, offsetof(Vertex, Position),	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
					{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, offsetof(Vertex, Normal),	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
					{"COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, offsetof(Vertex, Color),	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
					{"UV",			0, DXGI_FORMAT_R32G32_FLOAT,	0, offsetof(Vertex, UV),		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				};
				return vertexDesc;
			}
		};

		DXStaticMesh(
			const std::vector<Vertex>& vertices,
			const std::vector<UINT32>* indices = nullptr
		);
		~DXStaticMesh();

		void Bind();
		void Draw();
	private:
		Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_VBAlloc;
		UINT32 m_VertexCount;

		bool m_bHasIndexBuffer = false;
		Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_IBAlloc;
		UINT32 m_IndexCount;

	public:

		static std::shared_ptr<Blainn::DXStaticMesh> CreateCube(float side, const DirectX::SimpleMath::Color& color);

		static std::shared_ptr<Blainn::DXStaticMesh> CreateSphere(float radius, UINT sliceCount, UINT stackCount, const DirectX::SimpleMath::Color& color);
		static std::shared_ptr<Blainn::DXStaticMesh> CreateCapsule(float radius, float height, UINT sliceCount, UINT sphereStackCount, UINT cylinderStackCount, const DirectX::SimpleMath::Color& color);

		static std::shared_ptr<Blainn::DXStaticMesh> CreateTorus(float majorRadius, float minorRadius, UINT majorSegments, UINT minorSegments, const DirectX::SimpleMath::Color& color);
		static std::shared_ptr<Blainn::DXStaticMesh> CreateTorusKnot(int p, int q, float radius, float tubeRadius, UINT curveSegments, UINT tubeSegments, const DirectX::SimpleMath::Color& color);

		static std::shared_ptr<Blainn::DXStaticMesh> CreatePyramid(float width, float height, const DirectX::SimpleMath::Color& color);
		static std::shared_ptr<Blainn::DXStaticMesh> CreateCone(float bottomRadius, float height, UINT sliceCount, const DirectX::SimpleMath::Color& color);
		static std::shared_ptr<Blainn::DXStaticMesh> CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, const DirectX::SimpleMath::Color& color);

	};
}
