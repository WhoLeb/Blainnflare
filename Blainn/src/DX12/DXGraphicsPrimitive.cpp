#include "pch.h"
#include "DXGraphicsPrimitive.h"

namespace Blainn
{
	DXGraphicsPrimitive::DXGraphicsPrimitive(
		std::shared_ptr<DXResourceManager> resourceManager,
		const std::vector<Vertex>& vertices,
		const std::vector<UINT32>* indices)
		: m_ResourceManager(resourceManager)
	{
		m_VertexCount = vertices.size();
		assert(m_VertexCount >= 1 && "There should at least be 1 vertex!");

		UINT64 bufferSize = sizeof(Vertex) * m_VertexCount;
		UINT64 vertexSize = sizeof(Vertex);

		m_VertexBuffer = m_ResourceManager->CreateBuffer(
			vertices.data(),
			bufferSize,
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_COMMON
		);

		if (indices)
		{
			m_IndexCount = indices->size();

			m_bHasIndexBuffer = m_IndexCount > 0;
			if (!m_bHasIndexBuffer) return;

			bufferSize = sizeof(UINT32) * m_IndexCount;
			m_IndexBuffer = m_ResourceManager->CreateBuffer(
				indices->data(),
				bufferSize,
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_COMMON
			);
		}
	}
	void DXGraphicsPrimitive::Bind(std::shared_ptr<DXRenderingContext> renderingContext)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList = renderingContext->GetCommandList();

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

	void DXGraphicsPrimitive::Draw(std::shared_ptr<DXRenderingContext> renderingContext)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList = renderingContext->GetCommandList();
		if (m_bHasIndexBuffer)
			cmdList->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
		else
			cmdList->DrawInstanced(m_VertexCount, 1, 0, 0);
	}
}
