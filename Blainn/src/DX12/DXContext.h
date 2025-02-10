#pragma once

#include "DXDevice.h"
#include <wrl.h>
#include "Renderer/RenderContext.h"

namespace Blainn
{
	class DXContext : public RendererContext 
	{
	public:
		DXContext();
		virtual ~DXContext();

		virtual void Init() override;

		std::shared_ptr<DXDevice> GetDevice();

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_DXGIFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> m_D3DDevice;
	};
}
