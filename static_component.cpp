#include "static_component.hpp"
#include "game.hpp"
#include "exception.hpp"

#include "cubemap_component.hpp"

using namespace DirectX;
using namespace SimpleMath;

namespace jd
{
	StaticComponent::StaticComponent(size_t instanceCount)
		: instances_(instanceCount)
	{
	}

	void StaticComponent::initStaticResources()
	{
		if (s_staticResourcesInitialized) {
			return;
		}

		auto device = Game::getGame().getDevice();

		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = sizeof(CameraBuffer);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(device->CreateBuffer(&bd, nullptr, s_cameraBuffer.GetAddressOf()));

		D3D11_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthClipEnable = TRUE;
		ThrowIfFailed(device->CreateRasterizerState(&rasterDesc, s_rasterizerState.GetAddressOf()));

		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsDesc.StencilEnable = FALSE;
		ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, s_dsState.GetAddressOf()));

		s_staticResourcesInitialized = true;
	}

	StaticInfo StaticComponent::pickHandle(const event::SimplePickEvent& ev)
	{
		instances_[ev.indx].color = ev.color;
		return { ev.indx, this };
	}

	DirectX::SimpleMath::Vector3 StaticComponent::getPos(uint32_t indx) const noexcept
	{
		return instances_[indx].world.Translation();
	}

	float StaticComponent::getRadius(uint32_t indx) const noexcept
	{
		return 10.0f;
	}

	void StaticComponent::updateCameraMatrices(const Matrix& view, const Matrix& projection)
	{
		if (!s_staticResourcesInitialized) [[unlikely]] {
			return;
		}

		CameraBuffer cb;
		cb.view = view;
		cb.projection = projection;
		auto context = Game::getGame().getContext();
		context->UpdateSubresource(s_cameraBuffer.Get(), 0, nullptr, &cb, 0, 0);
		if (cubeMap_) {
			cubeMap_->updateCameraMatrices(view, projection);
		}
	}

	void StaticComponent::createInstanceBuffer()
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

	void StaticComponent::updateInstanceBuffer()
	{
		auto context = Game::getGame().getContext();
		D3D11_MAPPED_SUBRESOURCE mapped;
		ThrowIfFailed(context->Map(instanceBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
		memcpy(mapped.pData, instances_.data(), instances_.size() * sizeof(InstanceData));
		context->Unmap(instanceBuffer_.Get(), 0);
	}

	void StaticComponent::onInit()
	{
		createInstanceBuffer();
		updateTransforms();
		updateInstanceBuffer();
	}

	void StaticComponent::onUpdate(double)
	{
	}

	void StaticComponent::onResize()
	{
		updateTransforms();
	}

	void StaticComponent::onReload()
	{
		onInit();
	}

	void StaticComponent::onDestroy()
	{
	}

	void StaticComponent::Draw()
	{
		if (!s_staticResourcesInitialized) [[unlikely]] {
			return;
		}

		auto context = Game::getGame().getContext();

		context->IASetInputLayout(getInputLayout());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11Buffer* buffers[2] = { getVertexBuffer(), instanceBuffer_.Get() };
		UINT strides[2] = { sizeof(DirectX::SimpleMath::Vector4), sizeof(InstanceData) };
		UINT offsets[2] = { 0, 0 };
		context->IASetVertexBuffers(0, 2, buffers, strides, offsets);

		context->IASetIndexBuffer(getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		context->VSSetShader(getVertexShader(), nullptr, 0);
		context->PSSetShader(getPixelShader(), nullptr, 0);

		context->VSSetConstantBuffers(0, 1, s_cameraBuffer.GetAddressOf());

		context->RSSetState(s_rasterizerState.Get());
		context->OMSetDepthStencilState(s_dsState.Get(), 0);

		context->DrawIndexedInstanced(getIndexCount(), static_cast<UINT>(instances_.size()), 0, 0, 0);
	}

	void StaticComponent::setCubeMap(sptr<CubeMapComponent> cubeMap)
	{
		cubeMap_ = std::move(cubeMap);
	}
}