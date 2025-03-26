#pragma once

#include "d3d12.h"
#include "wrl.h"

#include <memory>
#include <queue>
#include <utility>

#include "D3D12MemAlloc.h"

extern const int g_NumFrameResources;

namespace dx12lib
{
	class DescriptorAllocator;
	class DescriptorAllocation;
	class RootSignature;
}

//namespace D3D12MA
//{
//	class Allocator;
//	class Allocation;
//}

namespace Blainn
{
	class DXDevice;
	class DynamicDescriptorHeap;

	class DXResourceManager
	{
		struct UploadBatch
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdAlloc;
			UINT64 FenceValue = 0;
		};
	public:
		DXResourceManager(
			std::shared_ptr<DXDevice> device,
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue
		);

		D3D12MA::Allocator* GetResourceAllocator() const { return m_Allocator.Get(); }

		void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,const std::shared_ptr<dx12lib::RootSignature>& rootSignature);

		void SetDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* descriptorHeap);
		void StageDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, UINT rootParameterIndex, UINT descriptorOffset, UINT numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptor);
		void CommitStagedDescriptorHeaps(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

		Microsoft::WRL::ComPtr<D3D12MA::Allocation> CreateAllocation(
			const CD3DX12_RESOURCE_DESC& resourceDesc,
			D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
			D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON
		);

		dx12lib::DescriptorAllocation AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT32 numDescriptors = 1);

		void* Map  (Microsoft::WRL::ComPtr<D3D12MA::Allocation> buffer);
		void  Unmap(Microsoft::WRL::ComPtr<D3D12MA::Allocation> buffer);

		void StartUploadCommands();
		void EndUploadCommands();

		void WriteToUploadBuffer(
			void* mappedData,
			const void* data,
			UINT64 size,
			UINT64 offset
		);

		void WriteToAllocation(
			Microsoft::WRL::ComPtr<D3D12MA::Allocation>
			allocation,
			UINT32 firstSubresource,
			UINT32 numSubresources,
			D3D12_SUBRESOURCE_DATA* data,
			D3D12_RESOURCE_STATES finalState = D3D12_RESOURCE_STATE_GENERIC_READ
		);

		void TryReleaseResources();
		void FlushUploadCommands();
		void WaitForFence(UINT64 fenceValue);

	private:
		Microsoft::WRL::ComPtr<D3D12MA::Allocator> m_Allocator = nullptr;
		std::shared_ptr<DXDevice> m_Device;

		std::shared_ptr<dx12lib::DescriptorAllocator> m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		std::shared_ptr<DynamicDescriptorHeap> m_DynamicDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		ID3D12DescriptorHeap* m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		ID3D12RootSignature* m_CurrentRootSignature;

		Microsoft::WRL::ComPtr<ID3D12Device> m_D3DDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		std::vector<UploadBatch> m_UploadAllocators;
		UINT m_AllocatorIndex = 0;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_UploadCommandList;

		std::queue<std::pair<UINT64, std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>>>> m_UploadBufferBatches;
		std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> m_PendingUploads;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_UploadFence;
		UINT64 m_CurrentFence;
	};
}
