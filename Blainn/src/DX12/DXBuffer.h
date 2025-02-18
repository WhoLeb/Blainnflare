#pragma once

#include "DXResourceManager.h"

namespace Blainn
{
	class DXBuffer
	{
	public:
		DXBuffer(
			DXResourceManager& resourceManager,
			UINT64 instanceSize,
			UINT64 instanceCount,
			D3D12_HEAP_TYPE heapType,
			D3D12_HEAP_FLAGS heapFlags,
			D3D12_RESOURCE_STATES initialState,
			UINT64 minOffsetAlignment = 1
			);
		~DXBuffer();


	private:
		UINT64 GetAlignment(UINT64 instanceSize, UINT64 minOffsetAlignment);

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Buffer;
		DXResourceManager& m_ResourceManager;

		UINT64 m_BufferSize;

		UINT64 m_InstanceSize;
		UINT64 m_InstanceCount;
		D3D12_HEAP_TYPE m_HeapType;
		D3D12_HEAP_FLAGS m_HeapFlags;
		UINT64 m_MinOffsetAlignment;
	};
}
