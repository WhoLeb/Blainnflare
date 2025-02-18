#pragma once

#include "DXRenderingContext.h"
#include "DXResourceManager.h"

namespace Blainn
{

	class DXGraphicsPrimitive
	{
	public:
		struct Vertex
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT4 Color;
			DirectX::XMFLOAT2 UV;

			static D3D12_INPUT_LAYOUT_DESC GetLayoutDescription()
			{
				D3D12_INPUT_ELEMENT_DESC vertexDesc[] =
				{
					{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, offsetof(Vertex, Position),	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
					{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, offsetof(Vertex, Normal),	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
					{"COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, offsetof(Vertex, Color),		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
					{"UV",			0, DXGI_FORMAT_R32G32_FLOAT,	0, offsetof(Vertex, UV),		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				};

				D3D12_INPUT_LAYOUT_DESC ld = {};
				ld.NumElements = 4;
				ld.pInputElementDescs = vertexDesc;
				return ld;
			}
		};

		DXGraphicsPrimitive(
			std::shared_ptr<DXResourceManager> resourceManager,
			const std::vector<Vertex>& vertices,
			const std::vector<UINT32>* indices = nullptr
		);

		void Bind(std::shared_ptr<DXRenderingContext> renderingContext);
		void Draw(std::shared_ptr<DXRenderingContext> renderingContext);

	private:
		std::shared_ptr<DXResourceManager> m_ResourceManager;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
		UINT32 m_VertexCount;

		bool m_bHasIndexBuffer = false;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
		UINT32 m_IndexCount;
	};
}
