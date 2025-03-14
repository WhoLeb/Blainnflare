#include "pch.h"
#include "DXDevice.h"

#include "Core/Application.h"
#include "Util/ComboboxSelector.h"
#include "Util/Util.h"

#include <dxgi.h>

namespace Blainn
{
	DXDevice::DXDevice()
	{
#if defined (DEBUG) || defined (_DEBUG)
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			Microsoft::WRL::ComPtr<ID3D12Debug1> debugController1;
			if (SUCCEEDED(debugController.As(&debugController1)))
			{
				debugController1->SetEnableGPUBasedValidation(true);
			}
		}
#endif
		IDXGIAdapter* adapter = SelectAdapter();
		HRESULT hr = D3D12CreateDevice(
			adapter,
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&m_Device)
		);

		if (FAILED(hr))
			assert(false && "Failed to create D3D12 device");
	}

	bool DXDevice::IsFeatureLevelSupported(D3D_FEATURE_LEVEL featureLevel) const
	{
		D3D12_FEATURE_DATA_FEATURE_LEVELS fl = {};
		static const D3D_FEATURE_LEVEL s_FeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};
		fl.NumFeatureLevels = _countof(s_FeatureLevels);
		fl.pFeatureLevelsRequested = s_FeatureLevels;

		if (FAILED(m_Device->CheckFeatureSupport(
			D3D12_FEATURE_FEATURE_LEVELS,
			&fl,
			sizeof(fl)
		)))
			return false;

		return fl.MaxSupportedFeatureLevel >= featureLevel;
	}

	IDXGIAdapter* DXDevice::SelectAdapter()
	{
		Microsoft::WRL::ComPtr<IDXGIFactory4> tmpDxgiFac;

		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&tmpDxgiFac)));

		UINT i = 0;

		IDXGIAdapter* adapter = nullptr;
		std::vector<IDXGIAdapter*> adapters;
		std::vector<std::wstring> adapterNames;

		while (tmpDxgiFac->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			adapters.push_back(adapter);
			adapterNames.push_back(desc.Description);
			++i;
		}

		ComboBoxSelector cmbb(GetModuleHandle(nullptr), Application::Get().GetWindow().GetNativeWindow(), adapterNames);
		int selectedAdapterIndex = cmbb.ShowModal();

		if (selectedAdapterIndex >= 0)
			return adapters[selectedAdapterIndex];
		else
		{
			MessageBox(nullptr, L"You didn't select an adapter. Falling back to default adapter.", L"Warning", MB_OK);
			return nullptr;
		}

		return nullptr;
	}
}
