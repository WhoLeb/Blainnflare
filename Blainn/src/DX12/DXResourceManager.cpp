#include "pch.h"
#include "DXResourceManager.h"

#include "DXDevice.h"
#include "DynamicDescriptorHeap.h"

#include "dx12lib/DescriptorAllocator.h"
#include "dx12lib/RootSignature.h"

using Microsoft::WRL::ComPtr;

namespace Blainn
{
	DXResourceManager::DXResourceManager(
		std::shared_ptr<DXDevice> device,
		ComPtr<ID3D12CommandQueue> commandQueue
	)
		: m_Device(device)
		, m_D3DDevice(device->Device())
		, m_CommandQueue(commandQueue)
		, m_CurrentFence(0)
	{

		using namespace D3D12MA;

		D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
		allocatorDesc.pAdapter = m_Device->Adapter();
		allocatorDesc.pDevice = m_Device->Device().Get();
		allocatorDesc.Flags = D3D12MA_RECOMMENDED_ALLOCATOR_FLAGS;

		ThrowIfFailed(D3D12MA::CreateAllocator(&allocatorDesc, &m_Allocator));

		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			m_DescriptorAllocators[i]
				= std::make_shared<dx12lib::DescriptorAllocator>(
					m_Device->Device(), static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i)
				);
			m_DynamicDescriptorHeaps[i]
				= std::make_shared<DynamicDescriptorHeap>(
					m_Device->Device(), static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i)
				);
			m_CurrentDescriptorHeaps[i] = nullptr;
		}

		ThrowIfFailed(m_D3DDevice->CreateFence(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_UploadFence)
		));

		m_UploadAllocators.resize(g_NumFrameResources);
		for (int i = 0; i < g_NumFrameResources; i++)
		{
			ThrowIfFailed(m_D3DDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&m_UploadAllocators[i].CmdAlloc)
			));
		}

		ThrowIfFailed(m_D3DDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_UploadAllocators[0].CmdAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(&m_UploadCommandList)
		));
		m_UploadCommandList->Close();
	}

	void DXResourceManager::SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, const std::shared_ptr<dx12lib::RootSignature>& rootSignature)
	{
		auto* d3d12RootSignature = rootSignature->GetD3D12RootSignature().Get();
		if (m_CurrentRootSignature != d3d12RootSignature)
		{
			m_CurrentRootSignature = d3d12RootSignature;

			for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
				m_DynamicDescriptorHeaps[i]->ParseRootSignature(rootSignature);

		}
		commandList->SetGraphicsRootSignature(m_CurrentRootSignature);
	}

	void DXResourceManager::SetDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* descriptorHeap)
	{
		if (true || m_CurrentDescriptorHeaps[heapType] != descriptorHeap)
		{
			m_CurrentDescriptorHeaps[heapType] = descriptorHeap;

			UINT                  numDescriptorHeaps = 0;
			ID3D12DescriptorHeap* descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

			for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
			{
				ID3D12DescriptorHeap* aDescriptorHeap = m_CurrentDescriptorHeaps[i];
				if (aDescriptorHeap)
				{
					descriptorHeaps[numDescriptorHeaps++] = aDescriptorHeap;
				}
			}

			commandList->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);
		}
	}

	void DXResourceManager::StageDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,UINT rootParameterIndex, UINT descriptorOffset, UINT numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptor)
	{
		m_DynamicDescriptorHeaps[descriptorType]->StageDescriptors(rootParameterIndex, descriptorOffset, numDescriptors, srcDescriptor);
	}

	void DXResourceManager::CommitStagedDescriptorHeaps(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
			m_DynamicDescriptorHeaps[i]->CommitStagedDescriptorsForDraw(commandList);
	}

	ComPtr<D3D12MA::Allocation> DXResourceManager::CreateAllocation(
		const CD3DX12_RESOURCE_DESC& resourceDesc,
		D3D12_HEAP_TYPE heapType,
		D3D12_HEAP_FLAGS heapFlags,
		D3D12_RESOURCE_STATES initialResourceState
	)
	{
		ComPtr<D3D12MA::Allocation> alloc;
		D3D12MA::ALLOCATION_DESC ad{};
		ad.HeapType = heapType;
		ad.ExtraHeapFlags = heapFlags;

		ThrowIfFailed(m_Allocator->CreateResource(
			&ad,
			&resourceDesc,
			initialResourceState,
			nullptr,
			&alloc,
			IID_NULL, nullptr
		));

		return alloc;
	}

	dx12lib::DescriptorAllocation DXResourceManager::AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT32 numDescriptors)
	{
		return m_DescriptorAllocators[type]->Allocate(numDescriptors);
	}

	inline void* DXResourceManager::Map(Microsoft::WRL::ComPtr<D3D12MA::Allocation> buffer)
	{
		void* mappedData;
		ID3D12Resource* bufferResource = buffer->GetResource();
		bufferResource->Map(0, nullptr, &mappedData);
		return mappedData;
	}

	inline void DXResourceManager::Unmap(Microsoft::WRL::ComPtr<D3D12MA::Allocation> buffer)
	{
		ID3D12Resource* bufferResource = buffer->GetResource();
		bufferResource->Unmap(0, nullptr);
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

	void DXResourceManager::WriteToAllocation(
		Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation,
		UINT32 firstSubresource,
		UINT32 numSubresources,
		D3D12_SUBRESOURCE_DATA* data,
		D3D12_RESOURCE_STATES finalState
	)
	{
		auto requiredSize = GetRequiredIntermediateSize(
			allocation->GetResource(),
			0, numSubresources
		);

		ComPtr<D3D12MA::Allocation> alloc;
		D3D12MA::ALLOCATION_DESC ad{};
		ad.HeapType = D3D12_HEAP_TYPE_UPLOAD;

		//auto resourceDesc = CD3DX12_RESOURCE_DESC(allocation->GetResource()->GetDesc());

		ThrowIfFailed(m_Allocator->CreateResource(
			&ad,
			&CD3DX12_RESOURCE_DESC::Buffer(requiredSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			&alloc,
			IID_NULL, nullptr
		));

		ID3D12Resource* uploadBuffer = alloc->GetResource();

		ID3D12Resource* defaultBuffer = allocation->GetResource();
		auto defDesc = defaultBuffer->GetDesc();
		m_UploadCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				defaultBuffer,
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_COPY_DEST
			));

		UpdateSubresources(m_UploadCommandList.Get(),
			defaultBuffer, uploadBuffer,
			0, 0, 1, data);

		m_UploadCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				defaultBuffer,
				D3D12_RESOURCE_STATE_COPY_DEST,
				finalState
			));

		m_PendingUploads.push_back(alloc);
	}

	void DXResourceManager::TryReleaseResources()
	{
		UINT64 completedFence = m_UploadFence->GetCompletedValue();
		while (!m_UploadBufferBatches.empty() &&
			m_UploadBufferBatches.front().first <= completedFence)
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
