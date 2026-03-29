#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

namespace jd
{
	struct ShaderResoures
	{
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
	};

	class ShaderManager
	{
	public:
		static ShaderResoures LoadShaders(const std::wstring& shaderPath);
		static Microsoft::WRL::ComPtr<ID3D11InputLayout> CreateInputLayout(ID3DBlob* vertexShaderBlob, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements);
	};
}