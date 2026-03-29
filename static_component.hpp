#pragma once

#include "jd_types.hpp"
#include "game_component.hpp"
#include <d3d11.h>
#include <wrl/client.h>
#include "SimpleMath.h"

#include <functional>
#include <vector>

#include "pick_event.hpp"

namespace jd
{
	class CubeMapComponent;
	class StaticComponent;

	struct StaticInfo
	{
		uint32_t indx;
		const StaticComponent* comp_ptr;
	};

	class StaticComponent : public GameComponent
	{
	public:
		using init_func_t = std::function<DirectX::SimpleMath::Matrix(size_t)>;

		StaticComponent(size_t instanceCount);

		void onInit() override;
		void onUpdate(double deltaTime) override;
		void onResize() override;
		void onReload() override;
		void onDestroy() override;
		void Draw() override;

		StaticInfo pickHandle(const event::SimplePickEvent& ev);
		DirectX::SimpleMath::Vector3 getPos(uint32_t indx) const noexcept;
		float getRadius(uint32_t indx) const noexcept;

		static void initStaticResources();
		static void updateCameraMatrices(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& projection);
		static void setCubeMap(sptr<CubeMapComponent> cubeMap);

	protected:
		virtual ID3D11VertexShader* getVertexShader() const = 0;
		virtual ID3D11PixelShader* getPixelShader() const = 0;
		virtual ID3D11InputLayout* getInputLayout() const = 0;
		virtual ID3D11Buffer* getVertexBuffer() const = 0;
		virtual ID3D11Buffer* getIndexBuffer() const = 0;
		virtual UINT getIndexCount() const noexcept = 0;

		virtual void updateTransforms() = 0;
		virtual void updateInstanceBuffer();

		size_t getInstanceCount() const noexcept { return instances_.size(); }

		struct InstanceData
		{
			DirectX::SimpleMath::Matrix world;
			DirectX::SimpleMath::Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		};

		std::vector<InstanceData> instances_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> instanceBuffer_;

	private:
		void createInstanceBuffer();

		struct CameraBuffer
		{
			DirectX::SimpleMath::Matrix view;
			DirectX::SimpleMath::Matrix projection;
		};

		inline static Microsoft::WRL::ComPtr<ID3D11Buffer> s_cameraBuffer;
		inline static Microsoft::WRL::ComPtr<ID3D11Buffer> s_projectionBuffer;
		inline static Microsoft::WRL::ComPtr<ID3D11RasterizerState> s_rasterizerState;
		inline static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> s_dsState;
		inline static sptr<CubeMapComponent> cubeMap_;
		inline static bool s_staticResourcesInitialized{ false };
	};
}