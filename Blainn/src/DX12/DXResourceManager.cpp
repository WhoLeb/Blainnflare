#include "pch.h"
#include "DXResourceManager.h"

using Microsoft::WRL::ComPtr;

namespace Blainn
{
	DXResourceManager::DXResourceManager(
		ComPtr<ID3D12Device> device,
		ComPtr<ID3D12CommandQueue> commandQueue
	)
		: m_Device(device)
		, m_CommandQueue(commandQueue)
		, m_CurrentFence(0)
	{
		ThrowIfFailed(m_Device->CreateFence(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_UploadFence)
		));

		m_UploadAllocators.resize(g_NumFrameResources);
		for (int i = 0; i < g_NumFrameResources; i++)
		{
			ThrowIfFailed(m_Device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&m_UploadAllocators[i].CmdAlloc)
			));
		}

		ThrowIfFailed(m_Device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_UploadAllocators[0].CmdAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(&m_UploadCommandList)
		));
		m_UploadCommandList->Close();
	}

	ComPtr<ID3D12Resource> DXResourceManager::CreateBuffer(
		UINT64 size,
		D3D12_HEAP_TYPE heapType,
		D3D12_HEAP_FLAGS heapFlags,
		D3D12_RESOURCE_STATES initialResourceState
	)
	{
		ComPtr<ID3D12Resource> defaultBuffer;

		ThrowIfFailed(m_Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(heapType),
			heapFlags,
			&CD3DX12_RESOURCE_DESC::Buffer(size),
			initialResourceState,
			nullptr,
			IID_PPV_ARGS(&defaultBuffer)
		));

		return defaultBuffer;
	}

	void DXResourceManager::StartUploadCommands()
	{
		auto& allocator = m_UploadAllocators[m_AllocatorIndex];
		
		UINT64 completed = m_UploadFence->GetCompletedValue();
		if (allocator.FenceValue != 0 && completed < allocator.FenceValue)
			WaitForFence(allocator.FenceValue);

		ThrowIfFailed(allocator.CmdAlloc->Reset());
		ThrowIfFailed(m_UploadCommandList->Reset(allocator.CmdAlloc.Get(), nullptr));
	}

	void DXResourceManager::EndUploadCommands()
	{
		m_UploadCommandList->Close();
		ID3D12CommandList* cmdLists[] = { m_UploadCommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		m_CurrentFence++;
		ThrowIfFailed(m_CommandQueue->Signal(m_UploadFence.Get(), m_CurrentFence));

		m_UploadAllocators[m_AllocatorIndex].FenceValue = m_CurrentFence;
		m_AllocatorIndex = (m_AllocatorIndex + 1) % g_NumFrameResources;

		m_UploadBufferBatches.push({ m_CurrentFence, std::move(m_PendingUploads) });
		m_PendingUploads.clear();
	}

	void DXResourceManager::WriteToUploadBuffer(void* mappedData, const void* data, UINT64 size, UINT64 offset)
	{
		BYTE* byteMappedData = static_cast<BYTE*>(mappedData);
		memcpy(&byteMappedData[offset], data, size);
	}

	void DXResourceManager::WriteToDefaultBuffer(
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer,
		const void* data, UINT64 size
	)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
		ThrowIfFailed(m_Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(size),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuffer)
		));

		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = data;
		subResourceData.RowPitch = size;
		subResourceData.SlicePitch = size;

		m_UploadCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				buffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_COPY_DEST
			));

		UpdateSubresources<1>(m_UploadCommandList.Get(),
			buffer.Get(), uploadBuffer.Get(),
			0, 0, 1, &subResourceData);

		m_UploadCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				buffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_GENERIC_READ
			));

		m_PendingUploads.push_back(uploadBuffer);
	}

	void DXResourceManager::TryReleaseResources()
	{
		UINT64 completedFence = m_UploadFence->GetCompletedValue();
		while (!m_UploadBufferBatches.empty() && m_UploadBufferBatches.front().first <= completedFence)
			m_UploadBufferBatches.pop();
	}

	void DXResourceManager::FlushUploadCommands()
	{
		m_CurrentFence++;
		ThrowIfFailed(m_CommandQueue->Signal(m_UploadFence.Get(), m_CurrentFence));

		if (m_UploadFence->GetCompletedValue() < m_CurrentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

			ThrowIfFailed(m_UploadFence->SetEventOnCompletion(m_CurrentFence, eventHandle));

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	void DXResourceManager::WaitForFence(UINT64 fenceValue)
	{
		if (m_UploadFence->GetCompletedValue() < fenceValue)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

			ThrowIfFailed(m_UploadFence->SetEventOnCompletion(fenceValue, eventHandle));

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}
}
