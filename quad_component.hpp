#pragma once

#pragma once
#include "game_component.hpp"
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include <string>
#include <vector>

#include "SimpleMath.h"

namespace jd
{
	class QuadComponent : public GameComponent
	{
	public:
		QuadComponent(size_t instanceCount);

		void onInit() override;
		void onUpdate(double deltaTime) override;
		void onResize() override;
		void onReload() override;
		void onDestroy() override;
		void Draw() override;

		void setBounds(float left, float right, float top, float bottom);

		static void initSharedResources(const std::wstring& shader_path);
		static void updateProjection(float aspect, float top, float bottom);

	protected:
		virtual void updateTransforms() = 0; 
		virtual void updateInstanceBuffer();

		size_t getInstanceCount() const noexcept
		{ 
			return instances_.size(); 
		}

		struct InstanceData
		{
			DirectX::SimpleMath::Matrix world;
		};

		float leftBound_{};
		float rightBound_{};
		float topBound_{};
		float bottomBound_{};
		std::vector<InstanceData> instances_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> instanceBuffer_;

	private:
		void createInstanceBuffer();

		// Shared resources (static)
		inline static Microsoft::WRL::ComPtr<ID3D11VertexShader> s_vertexShader;
		inline static Microsoft::WRL::ComPtr<ID3D11PixelShader> s_pixelShader;
		inline static Microsoft::WRL::ComPtr<ID3D11InputLayout> s_inputLayout;
		inline static Microsoft::WRL::ComPtr<ID3D11Buffer> s_vertexBuffer;
		inline static Microsoft::WRL::ComPtr<ID3D11Buffer> s_indexBuffer;
		inline static Microsoft::WRL::ComPtr<ID3D11RasterizerState> s_rasterizerState;
		inline static Microsoft::WRL::ComPtr<ID3D11Buffer> s_projectionBuffer;
		inline static bool s_resourcesInitialized{ false };
	};
}