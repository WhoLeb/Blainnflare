#include "pch.h"
#include "DXBuffer.h"

#include "DXResourceManager.h"


namespace Blainn
{
	DXBuffer::DXBuffer(
		DXResourceManager& resourceManager,
		UINT64 instanceSize,
		UINT64 instanceCount,
		D3D12_HEAP_TYPE heapType,
		D3D12_HEAP_FLAGS heapFlags,
		D3D12_RESOURCE_STATES initialState,
		UINT64 minOffsetAlignment
	) 
		: m_ResourceManager(resourceManager)
		, m_InstanceSize(instanceSize)
		, m_InstanceCount(instanceCount)
		, m_HeapType(heapType)
		, m_HeapFlags(heapFlags)
	{
		//m_MinOffsetAlignment = GetAlignment(instanceSize, minOffsetAlignment);
		//m_BufferSize = m_InstanceCount * m_MinOffsetAlignment;
		//resourceManager.CreateBuffer(m_BufferSize, heapType, heapFlags, initialState, m_Buffer);
	}

	DXBuffer::~DXBuffer()
	{
	}

	UINT64 DXBuffer::GetAlignment(UINT64 instanceSize, UINT64 minOffsetAlignment)
	{
		if (minOffsetAlignment > 0)
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		return instanceSize;
	}

}
