#include "pch.h"
#include "DXShader.h"

namespace Blainn
{
	DXShader::DXShader(const std::wstring& filename, bool bCompileOnRuntime, D3D_SHADER_MACRO* defines, const std::string& entryPoint, const std::string& target)
	{
		if (bCompileOnRuntime)
			CompileShader(filename, defines, entryPoint, target);
		else
			LoadBinary(filename);
	}

	HRESULT DXShader::CompileShader(const std::wstring& filename, D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
	{
		UINT compileFlags = 0;
#if defined (DEBUG) || defined (_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<ID3DBlob> errors;
		hr = D3DCompileFromFile(
			filename.c_str(),
			defines,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entrypoint.c_str(),
			target.c_str(),
			compileFlags, 0,
			&m_ByteCode,
			&errors
		);

		if (errors != nullptr)
			OutputDebugStringA((char*)errors->GetBufferPointer());
		return hr;
	}

	HRESULT DXShader::LoadBinary(const std::wstring& filename)
	{
		std::ifstream fin(filename, std::ios::binary);

		if (!fin.is_open())
			return S_FALSE;

		fin.seekg(0, std::ios_base::end);
		std::ifstream::pos_type size = (int)fin.tellg();
		fin.seekg(0, std::ios_base::beg);
		ThrowIfFailed(D3DCreateBlob(size, &m_ByteCode));

		fin.read((char*)m_ByteCode->GetBufferPointer(), size);
		fin.close();
		return S_OK;
	}
}
