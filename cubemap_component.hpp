#pragma once
#include "game_component.hpp"
#include <wrl.h>
#include "SimpleMath.h"
#include <d3d11.h>
#include <string>

namespace jd
{
	class CubeMapComponent : public GameComponent
	{
	public:
		CubeMapComponent(std::wstring shaderPath, std::wstring cubeMapPath);

		void onInit() override;
		void onUpdate(double deltaTime) override;
		void onResize() override;
		void onReload() override;
		void onDestroy() override;
		void Draw() override;

		void updateCameraMatrices(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& projection);

	private:
		const std::wstring shaderPath_;
		const std::wstring cubeMapPath_;

		Microsoft::WRL::ComPtr<ID3D11VertexShader>       vertexShader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>        pixelShader_;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>        inputLayout_;
		Microsoft::WRL::ComPtr<ID3D11Buffer>             vertexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer>             indexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer>             constantBuffer_;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMapSRV_;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>    rasterizerState_;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>  dsState_;

		struct ConstantBuffer
		{
			DirectX::SimpleMath::Matrix view;
			DirectX::SimpleMath::Matrix projection;
		};

		void createGeometry();
		void loadShaders();
		void loadCubeMapFromSixFiles();
	};
}