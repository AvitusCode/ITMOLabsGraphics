#include "cube_component.hpp"
#include "shader_manager.hpp"
#include "game.hpp"
#include "exception.hpp"
#include <DirectXMath.h>


using namespace DirectX;
using namespace SimpleMath;

namespace jd
{
	CubeComponent::CubeComponent(size_t instanceCount, const ShaderResoures& resource)
		: StaticComponent(instanceCount)
		, initialWorlds_(instanceCount, Matrix::Identity)
	{
		createGeometry();
		loadShaders(resource);
	}

	void CubeComponent::initilize(init_func_t init)
	{
		for (size_t i = 0; i < getInstanceCount(); ++i) {
			initialWorlds_[i] = init(i);
		}

		updateTransforms();
	}

	void CubeComponent::createGeometry()
	{
		auto device = Game::getGame().getDevice();

		struct Vertex
		{
			XMFLOAT4 position;
		};

		Vertex vertices[] = {
			{ { -1.0f, -1.0f, -1.0f, 1.0f} }, { {  1.0f, -1.0f, -1.0f, 1.0f } }, { {  1.0f, -1.0f,  1.0f, 1.0f } }, { { -1.0f, -1.0f,  1.0f, 1.0f } },
			{ { -1.0f,  1.0f, -1.0f, 1.0f } }, { {  1.0f,  1.0f, -1.0f, 1.0f } }, { {  1.0f,  1.0f,  1.0f, 1.0f } }, { { -1.0f,  1.0f,  1.0f, 1.0f } }
		};

		UINT indices[] = {
			0,1,2, 0,2,3,
			4,6,5, 4,7,6,
			0,5,1, 0,4,5,
			2,6,3, 3,6,7,
			1,2,6, 1,6,5,
			0,3,7, 0,7,4
		};
		indexCount_ = ARRAYSIZE(indices);

		D3D11_BUFFER_DESC bd = {};
		D3D11_SUBRESOURCE_DATA init = {};

		bd.ByteWidth = sizeof(vertices);
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		init.pSysMem = vertices;
		ThrowIfFailed(device->CreateBuffer(&bd, &init, vertexBuffer_.GetAddressOf()));

		bd.ByteWidth = sizeof(indices);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		init.pSysMem = indices;
		ThrowIfFailed(device->CreateBuffer(&bd, &init, indexBuffer_.GetAddressOf()));
	}

	void CubeComponent::loadShaders(const ShaderResoures& resource)
	{
		vertexShader_ = resource.vertexShader;
		pixelShader_ = resource.pixelShader;

		D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
		inputLayout_ = ShaderManager::CreateInputLayout(resource.vertexShaderBlob.Get(), layoutDesc, ARRAYSIZE(layoutDesc));
	}

	ID3D11VertexShader* CubeComponent::getVertexShader() const
	{
		return vertexShader_.Get();
	}

	ID3D11PixelShader* CubeComponent::getPixelShader() const
	{
		return pixelShader_.Get();
	}

	ID3D11InputLayout* CubeComponent::getInputLayout() const
	{
		return inputLayout_.Get();
	}

	ID3D11Buffer* CubeComponent::getVertexBuffer() const
	{
		return vertexBuffer_.Get();
	}

	ID3D11Buffer* CubeComponent::getIndexBuffer() const
	{
		return indexBuffer_.Get();
	}

	UINT CubeComponent::getIndexCount() const noexcept
	{
		return indexCount_;
	}

	void CubeComponent::onUpdate(double deltaTime)
	{
		static constexpr float RotationSpeed = 1.1f;
		static constexpr float BounceAmplitude = 1.0f;

		time_ += deltaTime;

		for (size_t i = 0; i < getInstanceCount(); ++i)
		{
			float angle = static_cast<float>(time_) * RotationSpeed;
			Matrix rotY = Matrix::CreateRotationY(angle);

			instances_[i].world = rotY * initialWorlds_[i];
			subscriber_.ExecuteIfBound(event::PosEvent{ 
				.world = initialWorlds_[i], 
				.indx = static_cast<uint32_t>(i), 
				.time = static_cast<float>(time_), 
				.radius = 5.0f 
			});
		}

		updateInstanceBuffer();
	}

	void CubeComponent::updateTransforms()
	{
		for (size_t i = 0; i < getInstanceCount(); ++i)
		{
			instances_[i].world = initialWorlds_[i];
		}
	}
}