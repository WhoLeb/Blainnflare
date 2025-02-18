#pragma once

#include <d3d12.h>

namespace Blainn
{
	class DXDevice
	{
	public:
		DXDevice();

		inline Microsoft::WRL::ComPtr<ID3D12Device> Device () const { return m_Device; }

		bool IsFeatureLevelSupported(D3D_FEATURE_LEVEL featureLevel) const;

		/*void CreateBuffer(
			UINT64 size,
			D3D12_HEAP_TYPE heapProperties,
			D3D12_HEAP_FLAGS heapFlags,
			D3D12_RESOURCE_STATES initialResourceState,
			Microsoft::WRL::ComPtr<ID3D12Resource> buffer
		);*/

	private:
		IDXGIAdapter* SelectAdapter();
	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
	};
}
