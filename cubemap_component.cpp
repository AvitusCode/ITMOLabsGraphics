#include "cubemap_component.hpp"
#include "game.hpp"
#include "shader_manager.hpp"
#include "exception.hpp"

#include "WICTextureLoader.h" 

#pragma comment(lib, "comctl32.lib")

using namespace DirectX;
using namespace SimpleMath;

namespace jd
{
	CubeMapComponent::CubeMapComponent(std::wstring shaderPath, std::wstring cubeMapPath)
		: shaderPath_(std::move(shaderPath))
		, cubeMapPath_(std::move(cubeMapPath))
	{
	}

	void CubeMapComponent::onInit()
	{
		loadShaders();
		createGeometry();
		loadCubeMapFromSixFiles();

		auto device = Game::getGame().getDevice();

		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = sizeof(ConstantBuffer);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(device->CreateBuffer(&bd, nullptr, constantBuffer_.GetAddressOf()));

		D3D11_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthClipEnable = FALSE;
		ThrowIfFailed(device->CreateRasterizerState(&rasterDesc, rasterizerState_.GetAddressOf()));

		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = FALSE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsDesc.StencilEnable = FALSE;
		ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, dsState_.GetAddressOf()));
	}

	void CubeMapComponent::onUpdate(double)
	{
	}

	void CubeMapComponent::onResize()
	{
	}

	void CubeMapComponent::onReload()
	{
		onDestroy();
		onInit();
	}

	void CubeMapComponent::onDestroy()
	{
	}

	void CubeMapComponent::Draw()
	{
		auto context = Game::getGame().getContext();

		context->IASetInputLayout(inputLayout_.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT stride = sizeof(Vector3);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

		context->VSSetShader(vertexShader_.Get(), nullptr, 0);
		context->PSSetShader(pixelShader_.Get(), nullptr, 0);

		context->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
		context->PSSetShaderResources(0, 1, cubeMapSRV_.GetAddressOf());

		context->RSSetState(rasterizerState_.Get());
		context->OMSetDepthStencilState(dsState_.Get(), 0);

		context->DrawIndexed(36, 0, 0);
	}

	void CubeMapComponent::updateCameraMatrices(const Matrix& view, const Matrix& projection)
	{
		Matrix viewNoTranslation = view;
		viewNoTranslation._41 = 0.0f;
		viewNoTranslation._42 = 0.0f;
		viewNoTranslation._43 = 0.0f;

		ConstantBuffer cb;
		cb.view = viewNoTranslation;
		cb.projection = projection;

		auto context = Game::getGame().getContext();
		context->UpdateSubresource(constantBuffer_.Get(), 0, nullptr, &cb, 0, 0);
	}

	void CubeMapComponent::createGeometry()
	{
		Vector3 vertices[] = {
			{-1.0f, -1.0f, -1.0f},
			{-1.0f,  1.0f, -1.0f},
			{ 1.0f,  1.0f, -1.0f},
			{ 1.0f, -1.0f, -1.0f},
			{-1.0f, -1.0f,  1.0f},
			{-1.0f,  1.0f,  1.0f},
			{ 1.0f,  1.0f,  1.0f},
			{ 1.0f, -1.0f,  1.0f}
		};

		UINT indices[] = {
			0,1,2, 0,2,3,
			4,6,5, 4,7,6,
			4,5,1, 4,1,0,
			3,2,6, 3,6,7,
			1,5,6, 1,6,2,
			0,3,7, 0,7,4
		};

		auto device = Game::getGame().getDevice();

		D3D11_BUFFER_DESC vbd = {};
		vbd.ByteWidth = sizeof(vertices);
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA vInit = { vertices };
		ThrowIfFailed(device->CreateBuffer(&vbd, &vInit, vertexBuffer_.GetAddressOf()));

		D3D11_BUFFER_DESC ibd = {};
		ibd.ByteWidth = sizeof(indices);
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		D3D11_SUBRESOURCE_DATA iInit = { indices };
		ThrowIfFailed(device->CreateBuffer(&ibd, &iInit, indexBuffer_.GetAddressOf()));
	}

	void CubeMapComponent::loadShaders()
	{
		ShaderResoures shaderResources = ShaderManager::LoadShaders(shaderPath_);

		vertexShader_ = std::move(shaderResources.vertexShader);
		pixelShader_ = std::move(shaderResources.pixelShader);

		D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		inputLayout_ = ShaderManager::CreateInputLayout(shaderResources.vertexShaderBlob.Get(), layoutDesc, ARRAYSIZE(layoutDesc));
	}

	void CubeMapComponent::loadCubeMapFromSixFiles()
	{
		auto device = Game::getGame().getDevice();
		auto context = Game::getGame().getContext();

		constexpr UINT faceCount = 6;
		const std::wstring faceNames[faceCount] = {
	        L"right.png",   // +X
	        L"left.png",    // -X
	        L"top.png",     // +Y
	        L"bottom.png",  // -Y
	        L"back.png",    // +Z
	        L"front.png"    // -Z
		};

		Microsoft::WRL::ComPtr<ID3D11Texture2D> sourceTextures[faceCount];
		D3D11_TEXTURE2D_DESC desc = {};

		for (UINT i = 0; i < faceCount; ++i)
		{
			std::wstring fullPath = cubeMapPath_ + L"\\" + faceNames[i];

			Microsoft::WRL::ComPtr<ID3D11Resource> resource;
			ThrowIfFailed(CreateWICTextureFromFile(device.Get(), context.Get(), fullPath.c_str(), resource.GetAddressOf(), nullptr));

			ThrowIfFailed(resource.As(&sourceTextures[i]));

			D3D11_TEXTURE2D_DESC d;
			sourceTextures[i]->GetDesc(&d);

			if (i == 0)
			{
				desc = d;
			}
			else if (d.Width != desc.Width || d.Height != desc.Height || d.Format != desc.Format)
			{
				ThrowIfFailedMsg(E_FAIL, "All cube map faces must have the same size and format");
			}
		}

		D3D11_TEXTURE2D_DESC cubeDesc = {};
		cubeDesc.Width = desc.Width;
		cubeDesc.Height = desc.Height;
		cubeDesc.MipLevels = 1;
		cubeDesc.ArraySize = faceCount;
		cubeDesc.Format = desc.Format;
		cubeDesc.SampleDesc.Count = 1;
		cubeDesc.Usage = D3D11_USAGE_DEFAULT;
		cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMap;
		ThrowIfFailed(device->CreateTexture2D(&cubeDesc, nullptr, cubeMap.GetAddressOf()));

		for (UINT face = 0; face < faceCount; ++face)
		{
			D3D11_MAPPED_SUBRESOURCE mapped = {};
			Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;

			D3D11_TEXTURE2D_DESC stagingDesc = desc;
			stagingDesc.Usage = D3D11_USAGE_STAGING;
			stagingDesc.BindFlags = 0;
			stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

			ThrowIfFailed(device->CreateTexture2D(&stagingDesc, nullptr, staging.GetAddressOf()));
			context->CopyResource(staging.Get(), sourceTextures[face].Get());

			context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped);

			context->UpdateSubresource(cubeMap.Get(), D3D11CalcSubresource(0, face, 1), nullptr, mapped.pData, mapped.RowPitch, 0);

			context->Unmap(staging.Get(), 0);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = cubeDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = 1;

		ThrowIfFailed(device->CreateShaderResourceView(cubeMap.Get(), &srvDesc, cubeMapSRV_.GetAddressOf()));
	}
}