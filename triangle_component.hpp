#pragma once
#include "game_component.hpp"

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>
#include <string>

namespace jd
{
    class TriangleComponent final : public GameComponent
    {
    public:
        struct Vertex
        {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT4 color;
        };

        ~TriangleComponent() = default;

        TriangleComponent(std::vector<Vertex> vertices, std::wstring shaderPath);

        void onInit() override;
        void onUpdate(double deltaTime) override;
        void onReload() override;
        void onDestroy() override;
        void Draw() override;

    private:
        bool compileShaders();
        void createVertexBuffer();
        void createInputLayout();
        void createRasterizerState();

        std::vector<Vertex> vertices_;
        std::wstring shaderPath_;

        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;

        Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode_;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode_;

        bool is_ok_{ false };
    };

}