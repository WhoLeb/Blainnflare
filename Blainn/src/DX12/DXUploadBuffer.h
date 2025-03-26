#pragma once

#include "DXResourceManager.h"

#include "Util/Util.h"

#include "D3D12MemAlloc.h"

namespace Blainn
{
	template<typename T>
	class DXUploadBuffer
	{
	public:
		DXUploadBuffer(std::shared_ptr<DXResourceManager> resourceManager, UINT elementCount, bool bIsConstantBuffer)
			: m_bIsConstantBuffer(bIsConstantBuffer)
		{
			m_ElementByteSize = sizeof(T);

			if (bIsConstantBuffer)
				m_ElementByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(T));

			CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementCount);

			m_UploadAllocation = resourceManager->CreateAllocation(
				resDesc,
				D3D12_HEAP_TYPE_UPLOAD,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_GENERIC_READ
				);
			m_UploadBuffer = m_UploadAllocation->GetResource();

			ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)))
		}

		DXUploadBuffer(const DXUploadBuffer& rhs) = delete;
		DXUploadBuffer& operator=(const DXUploadBuffer& rhs) = delete;

		~DXUploadBuffer()
		{
			if (m_UploadBuffer)
				m_UploadBuffer->Unmap(0, nullptr);
			m_MappedData = nullptr;

			m_UploadBuffer->Release();
			m_UploadAllocation->Release();
		}

		ID3D12Resource* GetResource() const { return m_UploadBuffer.Get(); }

		void CopyData(UINT elementIndex, const T& data)
		{
			memcpy(&m_MappedData[elementIndex * m_ElementByteSize], &data, sizeof(T));
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
		Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_UploadAllocation;
		BYTE* m_MappedData;

		bool m_bIsConstantBuffer;
		UINT m_ElementByteSize;
	};
}
