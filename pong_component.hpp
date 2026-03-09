// PongGameComponent.h
#pragma once
#include "game_component.hpp"
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>

namespace jd
{
    class PongGameComponent : public GameComponent
    {
    public:
        PongGameComponent();

        void onInit() override;
        void onUpdate(double deltaTime) override;
        void onReload() override;
        void onDestroy() override;
        void Draw() override;

    private:
        struct InstanceData
        {
            DirectX::XMFLOAT4X4 world;
            DirectX::XMFLOAT4 color;
        };

        void createBuffers();
        void updateInstanceBuffer();
        bool compileShaders();
        void createInputLayout();
        void createRasterizerState();

        void updateFieldBounds(float aspect);
        void resetBall();
        void clampPositions();
        void ñheckPaddleCollision();

        enum InstanceIndex
        {
            INST_PADDLE_LEFT = 0,
            INST_PADDLE_RIGHT,
            INST_BALL,
            INST_LINE_START
        };

        float left_{};
        float right_{};
        float top_{};
        float bottom_{};
        float previousAspect_{};

        float paddle1Y_{}; // left puddle
        float paddle2Y_{}; // right puddle

        DirectX::XMFLOAT2 ballPos_;
        DirectX::XMFLOAT2 ballVel_;

        std::vector<InstanceData> instances_;
        UINT lineCount_;

        bool is_ok_{ false };

        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> instanceBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> projectionBuffer_;
        Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode_;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode_;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;
    };
}