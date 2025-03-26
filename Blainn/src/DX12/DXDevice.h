#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include <wrl.h>

namespace D3D12MA
{
	class Allocator;
}

namespace Blainn
{
	class DXDevice
	{
	public:
		DXDevice();
		~DXDevice();

		inline Microsoft::WRL::ComPtr<ID3D12Device> Device () const { return m_Device; }
		inline IDXGIAdapter* Adapter() const { return m_Adapter; }

		operator Microsoft::WRL::ComPtr<ID3D12Device>() { return m_Device; }

		bool IsFeatureLevelSupported(D3D_FEATURE_LEVEL featureLevel) const;

	private:
		IDXGIAdapter* SelectAdapter();
	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		IDXGIAdapter* m_Adapter;
	};

}
