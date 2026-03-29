#pragma once

#include "static_component.hpp"

#include "pos_event.hpp"
#include "delegates.hpp"

namespace jd
{
	struct ShaderResoures;

	class CubeComponent final : public StaticComponent
	{
	public:
		using delegate_t = Delegate<void, const event::PosEvent&>;

		CubeComponent(size_t instanceCount, const ShaderResoures& resource);
		void initilize(init_func_t init);

		void onUpdate(double deltaTime) override;
		void onRotate(const event::PosEvent& event);

		delegate_t& getSubscriber() noexcept { return subscriber_; }

	private:
		ID3D11VertexShader* getVertexShader() const override;
		ID3D11PixelShader* getPixelShader() const override;
		ID3D11InputLayout* getInputLayout() const override;
		ID3D11Buffer* getVertexBuffer() const override;
		ID3D11Buffer* getIndexBuffer() const override;
		UINT getIndexCount() const noexcept override;
		void updateTransforms() override;

	private:
		void createGeometry();
		void loadShaders(const ShaderResoures& resource);

		std::vector<DirectX::SimpleMath::Matrix> initialWorlds_;
		delegate_t subscriber_;
		double time_{ 0.0 };
		bool isRotating_{ false };

		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>  pixelShader_;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>  inputLayout_;
		Microsoft::WRL::ComPtr<ID3D11Buffer>       vertexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer>       indexBuffer_;
		UINT indexCount_{ 0 };
	};
}