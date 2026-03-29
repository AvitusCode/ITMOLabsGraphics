#include "sphere_component.hpp"
#include "shader_manager.hpp"
#include "game.hpp"
#include "exception.hpp"
#include <DirectXMath.h>
#include <vector>
#include <cmath>

using namespace DirectX;
using namespace SimpleMath;

namespace jd
{
	SphereComponent::SphereComponent(size_t instanceCount, const ShaderResoures& resource)
		: StaticComponent(instanceCount)
		, initialWorlds_(instanceCount, Matrix::Identity)
	{
		createGeometry();
		loadShaders(resource);
	}

	void SphereComponent::createGeometry()
	{
		auto device = Game::getGame().getDevice();

		constexpr uint32_t slices = 32u;
		constexpr uint32_t stacks = 32u;
		constexpr float radius = 1.0f;

		std::vector<Vector4> vertices;
		vertices.reserve(stacks * slices);

		for (uint32_t i = 0; i <= stacks; ++i)
		{
			float phi = XM_PI * static_cast<float>(i) / static_cast<float>(stacks);
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);

			for (uint32_t j = 0; j <= slices; ++j)
			{
				float theta = XM_2PI * static_cast<float>(j) / static_cast<float>(slices);
				float sinTheta = sinf(theta);
				float cosTheta = cosf(theta);

				float x = radius * sinPhi * cosTheta;
				float y = radius * cosPhi;
				float z = radius * sinPhi * sinTheta;

				vertices.emplace_back(x, y, z, 1.0f);
			}
		}

		std::vector<UINT> indices;
		indices.reserve(stacks * slices * 6);
		for (UINT i = 0; i < stacks; ++i)
		{
			for (UINT j = 0; j < slices; ++j)
			{
				UINT first = i * (slices + 1) + j;
				UINT second = first + slices + 1;

				indices.push_back(first);
				indices.push_back(second);
				indices.push_back(first + 1);

				indices.push_back(second);
				indices.push_back(second + 1);
				indices.push_back(first + 1);
			}
		}

		indexCount_ = static_cast<UINT>(indices.size());

		D3D11_BUFFER_DESC bd = {};
		D3D11_SUBRESOURCE_DATA init = {};

		bd.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vector4));
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		init.pSysMem = vertices.data();
		ThrowIfFailed(device->CreateBuffer(&bd, &init, vertexBuffer_.GetAddressOf()));

		bd.ByteWidth = static_cast<UINT>(indices.size() * sizeof(UINT));
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		init.pSysMem = indices.data();
		ThrowIfFailed(device->CreateBuffer(&bd, &init, indexBuffer_.GetAddressOf()));
	}

	void SphereComponent::loadShaders(const ShaderResoures& resource)
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

	ID3D11VertexShader* SphereComponent::getVertexShader() const
	{
		return vertexShader_.Get();
	}

	ID3D11PixelShader* SphereComponent::getPixelShader() const
	{
		return pixelShader_.Get();
	}

	ID3D11InputLayout* SphereComponent::getInputLayout() const
	{
		return inputLayout_.Get();
	}

	ID3D11Buffer* SphereComponent::getVertexBuffer() const
	{
		return vertexBuffer_.Get();
	}

	ID3D11Buffer* SphereComponent::getIndexBuffer() const
	{
		return indexBuffer_.Get();
	}

	UINT SphereComponent::getIndexCount() const noexcept
	{
		return indexCount_;
	}

	void SphereComponent::onUpdate(double deltaTime)
	{
		if (isRotating_) {
			updateInstanceBuffer();
			return;
		}

		updateInstanceBuffer();
	}

	void SphereComponent::onRotate(const event::PosEvent& ev)
	{
		static constexpr float RotationSpeed = 1.1f;
		static constexpr float orbitSpeed_ = 0.2f;
		static constexpr float additionalDistance_ = 5.0f;

		isRotating_ = true;
		Vector3 center = ev.world.Translation();

		float radius = ev.radius + additionalDistance_;
		float angle = orbitSpeed_ * ev.time;
		float x = center.x + radius * std::cos(angle);
		float z = center.z + radius * std::sin(angle);
		float y = center.y;

		float angle_rot = static_cast<float>(ev.time) * RotationSpeed;
		Matrix rotY = Matrix::CreateRotationY(angle_rot);

		instances_[ev.indx].world = rotY * Matrix::CreateTranslation(x, y, z);
	}

	void SphereComponent::updateTransforms()
	{
		for (size_t i = 0; i < getInstanceCount(); ++i)
		{
			instances_[i].world = initialWorlds_[i];
		}
	}
}