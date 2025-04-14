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
	class CommandList;
	class CommandQueue;
	class DescriptorAllocator;
	class DescriptorAllocation;
	class Device;
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
			std::shared_ptr<dx12lib::Device> device
		);

		D3D12MA::Allocator* GetResourceAllocator() const { return m_Allocator.Get(); }

		Microsoft::WRL::ComPtr<D3D12MA::Allocation> CreateAllocation(
			const CD3DX12_RESOURCE_DESC& resourceDesc,
			D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
			D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON
		);

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
		std::shared_ptr<dx12lib::Device> m_Device;

		Microsoft::WRL::ComPtr<ID3D12Device> m_D3DDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		//dx12lib::CommandQueue& m_CommandQueue;

		std::vector<UploadBatch> m_UploadAllocators;
		UINT m_AllocatorIndex = 0;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_UploadCommandList;

		//std::shared_ptr<dx12lib::CommandList> m_UploadCommandList;

		std::queue<std::pair<UINT64, std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>>>> m_UploadBufferBatches;
		std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> m_PendingUploads;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_UploadFence;
		UINT64 m_CurrentFence;
	};
}
