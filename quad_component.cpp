#include "quad_component.hpp"

#include "game.hpp"
#include "exception.hpp"
#include <d3dcompiler.h>

using namespace DirectX;
using namespace SimpleMath;

namespace jd
{
	
	QuadComponent::QuadComponent(size_t instanceCount)
		: instances_(instanceCount)
	{
	}

	void QuadComponent::setBounds(float left, float right, float top, float bottom) {
		leftBound_ = left;
		rightBound_ = right;
		topBound_ = top;
		bottomBound_ = bottom;
	}

	void QuadComponent::updateProjection(float aspect, float top, float bottom)
	{
		if (!s_resourcesInitialized) [[unlikely]] {
			return;
		}

		Matrix proj = Matrix::CreateOrthographicOffCenter(-aspect, aspect, bottom, top, 0.0f, 1.0f);
		auto context = Game::getGame().getContext();
		context->UpdateSubresource(s_projectionBuffer.Get(), 0, nullptr, &proj, 0, 0);
	}

	void QuadComponent::initSharedResources(const std::wstring& shader_path)
	{
		if (s_resourcesInitialized) {
			return;
		}

		auto device = Game::getGame().getDevice();

		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

		ThrowIfFailed(D3DCompileFromFile(shader_path.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf()));
		ThrowIfFailed(D3DCompileFromFile(shader_path.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf()));

		ThrowIfFailed(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, s_vertexShader.GetAddressOf()));
		ThrowIfFailed(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, s_pixelShader.GetAddressOf()));

		// Vertex buffer (quad)
		Vector4 vertices[] = {
			Vector4(-0.5f, -0.5f, 0.0f, 1.0f),
			Vector4(0.5f, -0.5f, 0.0f, 1.0f),
			Vector4(0.5f, 0.5f, 0.0f, 1.0f),
			Vector4(-0.5f, 0.5f, 0.0f, 1.0f)
		};
		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = sizeof(vertices);
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA init = {};
		init.pSysMem = vertices;
		ThrowIfFailed(device->CreateBuffer(&bd, &init, s_vertexBuffer.GetAddressOf()));

		UINT indices[] = { 0, 1, 2, 0, 2, 3 };
		bd.ByteWidth = sizeof(indices);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		init.pSysMem = indices;
		ThrowIfFailed(device->CreateBuffer(&bd, &init, s_indexBuffer.GetAddressOf()));

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
		ThrowIfFailed(device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), s_inputLayout.GetAddressOf()));

		D3D11_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthClipEnable = TRUE;
		ThrowIfFailed(device->CreateRasterizerState(&rasterDesc, s_rasterizerState.GetAddressOf()));

		bd = {};
		bd.ByteWidth = sizeof(Matrix);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(device->CreateBuffer(&bd, nullptr, s_projectionBuffer.GetAddressOf()));

		s_resourcesInitialized = true;
	}

	void QuadComponent::createInstanceBuffer()
	{
		if (instanceBuffer_) {
			instanceBuffer_->Release();
		}

		auto device = Game::getGame().getDevice();
		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = static_cast<UINT>(instances_.size() * sizeof(InstanceData));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ThrowIfFailed(device->CreateBuffer(&bd, nullptr, instanceBuffer_.GetAddressOf()));
	}

	void QuadComponent::updateInstanceBuffer()
	{
		auto context = Game::getGame().getContext();
		D3D11_MAPPED_SUBRESOURCE mapped;
		ThrowIfFailed(context->Map(instanceBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
		memcpy(mapped.pData, instances_.data(), instances_.size() * sizeof(InstanceData));
		context->Unmap(instanceBuffer_.Get(), 0);
	}

	void QuadComponent::onInit()
	{
		createInstanceBuffer();
		updateTransforms();
		QuadComponent::updateInstanceBuffer();
	}

	void QuadComponent::onUpdate(double)
	{
		// pass
	}

	void QuadComponent::onResize() 
	{
		const auto& display = Game::getGame().getDisplay();
		setBounds(-display.aspect, display.aspect, 1.0f, -1.0f);
		updateTransforms();
	}

	void QuadComponent::onReload() 
	{
		onInit();
	}

	void QuadComponent::onDestroy() 
	{
		// pass
	}

	void QuadComponent::Draw()
	{
		if (!s_resourcesInitialized) [[unlikely]] {
			return;
		}

		auto context = Game::getGame().getContext();

		context->IASetInputLayout(s_inputLayout.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT stride = sizeof(Vector4);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, s_vertexBuffer.GetAddressOf(), &stride, &offset);

		stride = sizeof(InstanceData);
		context->IASetVertexBuffers(1, 1, instanceBuffer_.GetAddressOf(), &stride, &offset);

		context->IASetIndexBuffer(s_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		context->VSSetShader(s_vertexShader.Get(), nullptr, 0);
		context->PSSetShader(s_pixelShader.Get(), nullptr, 0);

		context->VSSetConstantBuffers(0, 1, s_projectionBuffer.GetAddressOf());

		context->RSSetState(s_rasterizerState.Get());

		context->DrawIndexedInstanced(6, static_cast<UINT>(instances_.size()), 0, 0, 0);
	}
}