#pragma once

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
			const void* data,
			UINT64 size,
			D3D12_HEAP_TYPE heapType,
			D3D12_HEAP_FLAGS heapFlags,
			D3D12_RESOURCE_STATES initialResourceState
		);

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_UploadCmdAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_UploadCommandList;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_UploadFence;
		UINT64 m_CurrentFence;

	};
}
