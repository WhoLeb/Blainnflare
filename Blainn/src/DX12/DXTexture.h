#pragma once

#include "Core/Application.h"
#include "DXDevice.h"
#include "DXRenderingContext.h"
#include "DXResourceManager.h"

#include <filesystem>

#include "DirectXTex.h"

#include "D3D12MemAlloc.h"
#include "dx12lib/DescriptorAllocation.h"

namespace Blainn
{
	class DXTexture
	{
	public:
		DXTexture(const std::filesystem::path& filepath)
			: uuid()
			, FilePath(filepath)
		{
			//m_Allocator = Application::Get().GetResourceManager()->GetResourceAllocator();
			auto resManager = Application::Get().GetResourceManager();
			auto d3dDevice = Application::Get().GetRenderingContext()->GetDevice()->Device();
			resManager->StartUploadCommands();

			DirectX::TexMetadata metadata;
			DirectX::ScratchImage image;
			DirectX::LoadFromDDSFile(
				filepath.c_str(),
				DirectX::DDS_FLAGS_FORCE_RGB,
				&metadata,
				image
			);
			metadata.format = DirectX::MakeSRGB(metadata.format);

			auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
				metadata.format,
				(UINT64)metadata.width,
				(UINT)metadata.height,
				(UINT16)metadata.arraySize,
				(UINT16)metadata.mipLevels
			);

			m_TexAllocation = resManager->CreateAllocation(texDesc);
			m_TexDescAllocation = std::make_shared<dx12lib::DescriptorAllocation>
				(resManager->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1));
			

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = metadata.format;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = (UINT)metadata.mipLevels;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dDevice->CreateShaderResourceView(
					m_TexAllocation->GetResource(),
					&srvDesc,
					m_TexDescAllocation->GetDescriptorHandle()
				);
			std::vector<D3D12_SUBRESOURCE_DATA> subresources(image.GetImageCount());
			const DirectX::Image* pImages = image.GetImages();
			for (int i = 0; i < image.GetImageCount(); ++i)
			{
				auto& subresource = subresources[i];
				subresource.RowPitch = pImages[i].rowPitch;
				subresource.SlicePitch = pImages[i].slicePitch;
				subresource.pData = pImages[i].pixels;
			}

			resManager->WriteToAllocation(
				m_TexAllocation,
				0, (UINT32)subresources.size(),
				subresources.data(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			);

			resManager->EndUploadCommands();
		}
		UUID uuid;

		void Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList)
		{
			auto resManager = Application::Get().GetResourceManager();
			auto allocPage = m_TexDescAllocation->GetDescriptorAllocatorPage();
			m_TexDescAllocation->GetDescriptorHandle();

			resManager->StageDescriptorHeap(
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				0, 0, 1, m_TexDescAllocation->GetDescriptorHandle());
			resManager->CommitStagedDescriptorHeaps(cmdList);
		}
		
		std::filesystem::path FilePath;
	private:
		Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_TexAllocation;
		std::shared_ptr<dx12lib::DescriptorAllocation> m_TexDescAllocation;

	};
}
