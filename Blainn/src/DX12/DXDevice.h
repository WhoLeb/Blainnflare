#pragma once

#include <d3d12.h>

namespace Blainn
{
	class DXDevice
	{
	public:
		DXDevice();

		inline ID3D12Device* Device () const { return m_Device; }
	private:
		ID3D12Device* m_Device;
	};
}
