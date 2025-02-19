#pragma once

#include "DXRenderingContext.h"

namespace Blainn
{
	struct PipelineDesc
	{
		
	};

	class Pipeline
	{
	public:
		Pipeline(std::shared_ptr<DXRenderingContext> renderingContext, PipelineDesc& pipelineDesc);

	private:
		void BuildRootSignature();
	private:
		std::shared_ptr<DXRenderingContext> m_RenderingContext;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	};
}
