#pragma once

#include "d3d12.h"
#include "wrl.h"

namespace Blainn
{
	class DXResourceManager
	{
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

		void Map(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, void* mappedData)
		{
			buffer->Map(0, nullptr, &mappedData);
		}
		void Unmap(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, void* mappedData)
		{
			buffer->Unmap(0, nullptr);
		}

		void WriteToUploadBuffer(void* mappedData, const void* data, UINT64 size, UINT64 offset);
		void WriteToDefaultBuffer(
			Microsoft::WRL::ComPtr<ID3D12Resource> buffer,
			const void* data, UINT64 size,
			Microsoft::WRL::ComPtr<ID3D12Resource>& uploader
		);

		void FlushUploadCommands();

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_UploadCmdAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_UploadCommandList;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_UploadFence;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
		UINT64 m_CurrentFence;

	};
}
