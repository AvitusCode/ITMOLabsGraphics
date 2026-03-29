#include "shader_manager.hpp"
#include "game.hpp"
#include "exception.hpp"
#include <d3dcompiler.h>

namespace jd
{
	ShaderResoures ShaderManager::LoadShaders(const std::wstring& shaderPath)
	{
		auto device = Game::getGame().getDevice();

		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

		HRESULT hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf());
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				std::string errorMsg(static_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
				OutputDebugStringA(errorMsg.c_str());
			}
			ThrowIfFailed(hr);
		}

		hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf());
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				std::string errorMsg(static_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
				OutputDebugStringA(errorMsg.c_str());
			}
			ThrowIfFailed(hr);
		}

		Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;

		ThrowIfFailed(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vs.GetAddressOf()));
		ThrowIfFailed(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, ps.GetAddressOf()));

		return { std::move(vs), std::move(ps), std::move(vsBlob) };
	}

	Microsoft::WRL::ComPtr<ID3D11InputLayout> ShaderManager::CreateInputLayout(ID3DBlob* vertexShaderBlob, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements)
	{
		if (!vertexShaderBlob)
		{
			ThrowIfFailedMsg(E_INVALIDARG, "Vertex shader blob is null");
		}

		auto device = Game::getGame().getDevice();

		Microsoft::WRL::ComPtr<ID3D11InputLayout> layout;
		ThrowIfFailed(device->CreateInputLayout(layoutDesc, numElements, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), layout.GetAddressOf()));
		return layout;
	}
}