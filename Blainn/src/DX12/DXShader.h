#pragma once

#include <xstring>

namespace Blainn
{
	class DXShader
	{
	public:
		DXShader(
			const std::wstring& filenme,
			bool bCompileOnRuntime = false,
			D3D_SHADER_MACRO* defines = nullptr,
			const std::string& entryPoint = "",
			const std::string& target = ""
		);

		Microsoft::WRL::ComPtr<ID3DBlob> GetByteCode() const { return m_ByteCode; }

	private:
		HRESULT CompileShader(
			const std::wstring& filename,
			D3D_SHADER_MACRO* defines,
			const std::string& entrypoint,
			const std::string& target
		);

		HRESULT LoadBinary(const std::wstring& filename);
	private:
		Microsoft::WRL::ComPtr<ID3DBlob> m_ByteCode;
	};
}
