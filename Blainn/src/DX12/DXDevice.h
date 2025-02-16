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

	private:
		IDXGIAdapter* SelectAdapter();
	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
	};
}
