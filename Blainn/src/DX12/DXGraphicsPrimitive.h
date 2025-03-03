#pragma once

#include "DXRenderingContext.h"
#include "DXResourceManager.h"

#include "SimpleMath.h"
#include <d3d12.h>
#include <vector>

namespace Blainn
{

	class DXGraphicsPrimitive
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
					{"COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, offsetof(Vertex, Color),		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
					{"UV",			0, DXGI_FORMAT_R32G32_FLOAT,	0, offsetof(Vertex, UV),		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				};
				return vertexDesc;
			}
		};

		DXGraphicsPrimitive(
			const std::vector<Vertex>& vertices,
			const std::vector<UINT32>* indices = nullptr
		);

		void Draw();

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader;
		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader;

	private:
		std::shared_ptr<DXResourceManager> m_ResourceManager;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
		UINT32 m_VertexCount;

		bool m_bHasIndexBuffer = false;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
		UINT32 m_IndexCount;
	};
}
