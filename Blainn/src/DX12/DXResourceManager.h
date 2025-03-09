#pragma once

#include "d3d12.h"
#include "wrl.h"

#include <queue>
#include <utility>

extern const int g_NumFrameResources;

namespace Blainn
{
	class DXResourceManager
	{
		struct UploadBatch
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdAlloc;
			UINT64 FenceValue = 0;
		};
	public:
		DXResourceManager(
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue
		);

		Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(
			UINT64 size,
			D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
			D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON
		);

		void* Map(Microsoft::WRL::ComPtr<ID3D12Resource> buffer)
		{
			void* mappedData;
			buffer->Map(0, nullptr, &mappedData);
			return mappedData;
		}
		void Unmap(Microsoft::WRL::ComPtr<ID3D12Resource> buffer)
		{
			buffer->Unmap(0, nullptr);
		}

		void StartUploadCommands();
		void EndUploadCommands();

		void WriteToUploadBuffer(void* mappedData, const void* data, UINT64 size, UINT64 offset);
		void WriteToDefaultBuffer(
			Microsoft::WRL::ComPtr<ID3D12Resource> buffer,
			const void* data, UINT64 size
		);

		void TryReleaseResources();
		void FlushUploadCommands();
		void WaitForFence(UINT64 fenceValue);

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		std::vector<UploadBatch> m_UploadAllocators;
		UINT m_AllocatorIndex = 0;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_UploadCommandList;

		std::queue<std::pair<UINT64, std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>>> m_UploadBufferBatches;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_PendingUploads;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_UploadFence;
		UINT64 m_CurrentFence;
	};
}
