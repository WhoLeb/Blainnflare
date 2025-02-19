#pragma once

#include "DXRenderingContext.h"

namespace Blainn
{
	template<typename T>
	class DXUploadBuffer
	{
	public:
		DXUploadBuffer(std::shared_ptr<DXRenderingContext> renderingContext, UINT elementCount, bool bIsConstantBuffer)
			: m_bIsConstantBuffer(bIsConstantBuffer)
		{
			m_ElementByteSize = sizeof(T);

			if (bIsConstantBuffer)
				m_ElementByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(T));

			Microsoft::WRL::ComPtr<ID3D12Device> device = renderingContext->GetDevice()->Device();
			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementCount),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_UploadBuffer)
			));

			ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)))
		}

		DXUploadBuffer(const DXUploadBuffer& rhs) = delete;
		DXUploadBuffer& operator=(const DXUploadBuffer& rhs) = delete;

		~DXUploadBuffer()
		{
			if (m_UploadBuffer)
				m_UploadBuffer->Unmap(0, nullptr);
			m_MappedData = nullptr;
		}

		ID3D12Resource* GetResource() const { return m_UploadBuffer.Get(); }

		void CopyData(UINT elementIndex, const T& data)
		{
			memcpy(&m_MappedData[elementIndex * m_ElementByteSize], &data, sizeof(T));
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
		BYTE* m_MappedData;

		bool m_bIsConstantBuffer;
		UINT m_ElementByteSize;
	};
}
