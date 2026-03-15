#include "pong_component.hpp"
#include "game.hpp"

#include <mmsystem.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "logger.hpp"
#include "exception.hpp"
#include "input_device.hpp"

#pragma comment(lib, "winmm.lib")

using namespace DirectX;

namespace
{
    static constexpr float paddleSpeed    = 2.0f;
    static constexpr float paddleWidth    = 0.05f;
    static constexpr float paddleHeight   = 0.3f;
    static constexpr float ballSize       = 0.03f;
    static constexpr float lineSquareSize = 0.02f;
    static constexpr float velFactor      = 1.1f;

	static constexpr uint32_t TOTAL_SEGMENTS = 28;

    void playSound()
    {
        BOOL success = PlaySound(L"./resources/sounds/pong_hit.wav", nullptr, SND_FILENAME | SND_ASYNC);
        if (!success) [[unlikely]] {
            DLOG(JERROR) << "Problems with sound!";
        }
    }
}

namespace jd
{
    PongGameComponent::PongGameComponent()
        : ballPos_{ 0.0f, 0.0f }
        , ballVel_{ 0.5f, 0.3f }
        , lineCount_{ 30 }
    {
		instances_.resize(INST_LINE_START + lineCount_ + TOTAL_SEGMENTS);

		const size_t segmentStart = INST_LINE_START + lineCount_;
		for (size_t i = segmentStart; i < instances_.size(); ++i) {
			XMStoreFloat4x4(&instances_[i].world, XMMatrixScaling(0.0f, 0.0f, 0.0f));
		}

    }

    void PongGameComponent::onInit()
    {
        if (not compileShaders()) {
            DLOG(JERROR) << "PongGameComponent creation failed";
            return;
        }
        createBuffers();
        createInputLayout();
        createRasterizerState();

        srand(static_cast<unsigned>(time(nullptr)));

        DLOG(INFO) << "PongGameComponent creation sucessed";
        is_ok_ = true;
    }

    void PongGameComponent::updateFieldBounds(float aspect)
    {
        left_ = -aspect;
        right_ = aspect;
        top_ = 1.0f;
        bottom_ = -1.0f;

        const float step = (top_ - bottom_) / static_cast<float>(lineCount_ + 1);
        for (uint16_t i = 0; i < lineCount_; ++i)
        {
            float y = bottom_ + static_cast<float>(i + 1) * step;
            auto world = XMMatrixScaling(lineSquareSize, lineSquareSize, 1.0f) * XMMatrixTranslation(0.0f, y, 0.0f);
            XMStoreFloat4x4(&instances_[INST_LINE_START + i].world, world);
        }

        updateScoreDisplay();

        XMMATRIX proj = XMMatrixOrthographicOffCenterLH(-aspect, aspect, bottom_, top_, 0.0f, 1.0f);
        Game::getGame().getContext()->UpdateSubresource(projectionBuffer_.Get(), 0, nullptr, &proj, 0, 0);
    }

    void PongGameComponent::clampPositions()
    {
        const float halfPaddle = paddleHeight * 0.5f;
        paddle1Y_ = std::clamp(paddle1Y_, bottom_ + halfPaddle, top_ - halfPaddle);
        paddle2Y_ = std::clamp(paddle2Y_, bottom_ + halfPaddle, top_ - halfPaddle);

        const float halfBall = ballSize * 0.5f;
        ballPos_.x = std::clamp(ballPos_.x, left_ + halfBall, right_ - halfBall);
        ballPos_.y = std::clamp(ballPos_.y, bottom_ + halfBall, top_ - halfBall);
    }

    void PongGameComponent::ñheckPaddleCollision()
    {
        const float halfBall = ballSize * 0.5f;
        const float halfPaddleX = paddleWidth * 0.5f;
        const float halfPaddleY = paddleHeight * 0.5f;
		static constexpr float offset = 0.05f;

        float paddle1X = left_ + halfPaddleX + offset;
        if (ballPos_.x - halfBall < paddle1X + halfPaddleX &&
            ballPos_.x + halfBall > paddle1X - halfPaddleX &&
            ballPos_.y - halfBall < paddle1Y_ + halfPaddleY &&
            ballPos_.y + halfBall > paddle1Y_ - halfPaddleY)
        {
            if (ballVel_.x < 0)
            {
                playSound();
				XMStoreFloat2(&ballPos_, XMVectorSetX(XMLoadFloat2(&ballPos_), paddle1X + halfPaddleX + halfBall));
				XMStoreFloat2(&ballVel_, XMVectorScale(XMVectorSetX(XMLoadFloat2(&ballVel_), -ballVel_.x), velFactor));
            }
        }

        float paddle2X = right_ - halfPaddleX - offset;
        if (ballPos_.x - halfBall < paddle2X + halfPaddleX &&
            ballPos_.x + halfBall > paddle2X - halfPaddleX &&
            ballPos_.y - halfBall < paddle2Y_ + halfPaddleY &&
            ballPos_.y + halfBall > paddle2Y_ - halfPaddleY)
        {
            if (ballVel_.x > 0)
            {
                playSound();
                XMStoreFloat2(&ballPos_, XMVectorSetX(XMLoadFloat2(&ballPos_), paddle2X - halfPaddleX - halfBall));
                XMStoreFloat2(&ballVel_, XMVectorScale(XMVectorSetX(XMLoadFloat2(&ballVel_), -ballVel_.x), velFactor));
            }
        }
    }

    void PongGameComponent::updateScoreDisplay()
    {
		static constexpr float digitWidth = 0.12f;
        static constexpr float digitHeight = 0.18f;
        static constexpr float segmentThickness = 0.02f;
        static constexpr float digitSpacing = digitWidth * 1.2f;
        const float digitY = top_ - digitHeight * 1.2f;

        struct DigitSegmentsGeo {
            float offsetX;
            float offsetY;
            float scaleX;
            float scaleY;
        };

		static constexpr DigitSegmentsGeo segGeom[7] = {
			{ 0.0f,  (digitHeight - segmentThickness) * 0.5f, digitWidth - 2.0f * segmentThickness, segmentThickness },
			{ (digitWidth - segmentThickness) * 0.5f,  digitHeight * 0.25f, segmentThickness, digitHeight * 0.5f - segmentThickness },
			{ (digitWidth - segmentThickness) * 0.5f, -digitHeight * 0.25f, segmentThickness, digitHeight * 0.5f - segmentThickness },
			{ 0.0f, -(digitHeight - segmentThickness) * 0.5f, digitWidth - 2.0f * segmentThickness, segmentThickness },
			{ -(digitWidth - segmentThickness) * 0.5f, -digitHeight * 0.25f, segmentThickness, digitHeight * 0.5f - segmentThickness },
			{ -(digitWidth - segmentThickness) * 0.5f,  digitHeight * 0.25f, segmentThickness, digitHeight * 0.5f - segmentThickness },
			{ 0.0f, 0.0f, digitWidth - 2.0f * segmentThickness, segmentThickness }
		};

		static const bool digitSegments[10][7] = {
			{1,1,1,1,1,1,0}, // 0
			{0,1,1,0,0,0,0}, // 1
			{1,1,0,1,1,0,1}, // 2
			{1,1,1,1,0,0,1}, // 3
			{0,1,1,0,0,1,1}, // 4
			{1,0,1,1,0,1,1}, // 5
			{1,0,1,1,1,1,1}, // 6
			{1,1,1,0,0,0,0}, // 7
			{1,1,1,1,1,1,1}, // 8
			{1,1,1,1,0,1,1}  // 9
		};

		float leftCenterX = left_ * 0.5f;
		float rightCenterX = right_ * 0.5f;

		float leftTensX = leftCenterX - digitSpacing * 0.5f;
		float leftUnitsX = leftCenterX + digitSpacing * 0.5f;
		float rightTensX = rightCenterX - digitSpacing * 0.5f;
		float rightUnitsX = rightCenterX + digitSpacing * 0.5f;

		int leftTens = static_cast<int>(score1_) / 10;
		int leftUnits = static_cast<int>(score1_) % 10;
		int rightTens = static_cast<int>(score2_) / 10;
		int rightUnits = static_cast<int>(score2_) % 10;

		auto setDigit = [this](int digit, float centerX, float centerY, size_t baseIdx) noexcept {
			for (size_t segment = 0; segment < 7; ++segment) {
                const size_t instIdx = baseIdx + segment;
				if (digitSegments[digit][segment]) {
					XMMATRIX world =
						XMMatrixScaling(segGeom[segment].scaleX, segGeom[segment].scaleY, 1.0f) *
						XMMatrixTranslation(segGeom[segment].offsetX, segGeom[segment].offsetY, 0.0f) *
						XMMatrixTranslation(centerX, centerY, 0.0f);
					XMStoreFloat4x4(&instances_[instIdx].world, world);
				}
				else {
					XMStoreFloat4x4(&instances_[instIdx].world, XMMatrixScaling(0.0f, 0.0f, 0.0f));
				}
			}
			};

        const size_t segmentStart = INST_LINE_START + lineCount_;
		setDigit(leftTens, leftTensX, digitY, segmentStart);
		setDigit(leftUnits, leftUnitsX, digitY, segmentStart + 7);
		setDigit(rightTens, rightTensX, digitY, segmentStart + 14);
		setDigit(rightUnits, rightUnitsX, digitY, segmentStart + 21);
    }

    void PongGameComponent::resetBall()
    {
        ballPos_ = XMFLOAT2{ 0.0f, 0.0f };
        ballVel_.x = (rand() % 2 == 0) ? 0.5f : -0.5f;
        ballVel_.y = (rand() % 2 == 0) ? 0.3f : -0.3f;
    }

    void PongGameComponent::onUpdate(double deltaTime)
    {
        if (!is_ok_) [[unlikely]] {
            return;
        }

        if (const float aspect = Game::getGame().getDisplay().aspect; aspect != previousAspect_) [[unlikely]]
        {
            previousAspect_ = aspect;
            updateFieldBounds(aspect);
            clampPositions();
        }

        const float delta = static_cast<float>(deltaTime);
        {
            const auto& input = Game::getGame().getInputDevice();
            if (input.isKeyDown(Keys::W)) {
                paddle1Y_ += paddleSpeed * delta;
            }
            if (input.isKeyDown(Keys::S)) {
                paddle1Y_ -= paddleSpeed * delta;
            }

            if (input.isKeyDown(Keys::Up)) {
                paddle2Y_ += paddleSpeed * delta;
            }
            if (input.isKeyDown(Keys::Down)) {
                paddle2Y_ -= paddleSpeed * delta;
            }
        }

        float halfPaddle = paddleHeight * 0.5f;
        paddle1Y_ = std::clamp(paddle1Y_, bottom_ + halfPaddle, top_ - halfPaddle);
        paddle2Y_ = std::clamp(paddle2Y_, bottom_ + halfPaddle, top_ - halfPaddle);

        XMStoreFloat2(&ballPos_, XMVectorAdd(XMLoadFloat2(&ballPos_), XMVectorSet(ballVel_.x * delta, ballVel_.y * delta, 0.0f, 0.0f)));

        const float halfBall = ballSize * 0.5f;
        if (ballPos_.y + halfBall > top_)
        {
            playSound();
            XMStoreFloat2(&ballPos_, XMVectorSetY(XMLoadFloat2(&ballPos_), top_ - halfBall));
            XMStoreFloat2(&ballVel_, XMVectorSetY(XMLoadFloat2(&ballVel_), -ballVel_.y));
        }
        else if (ballPos_.y - halfBall < bottom_)
        {
            playSound();
			XMStoreFloat2(&ballPos_, XMVectorSetY(XMLoadFloat2(&ballPos_), bottom_ + halfBall));
			XMStoreFloat2(&ballVel_, XMVectorSetY(XMLoadFloat2(&ballVel_), -ballVel_.y));
        }

        ñheckPaddleCollision();

        bool collide{ false };
        if (ballPos_.x - halfBall > right_)
        {
            ++score1_;
            collide = true;
        }
        else if (ballPos_.x + halfBall < left_)
        {
            ++score2_;
            collide = true;
        }

        if (collide)
        {
            updateScoreDisplay();
            resetBall();
        }

        updateInstanceBuffer();

        auto context = Game::getGame().getContext();
        D3D11_MAPPED_SUBRESOURCE mapped;
        ThrowIfFailed(context->Map(instanceBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        memcpy(mapped.pData, instances_.data(), instances_.size() * sizeof(InstanceData));
        context->Unmap(instanceBuffer_.Get(), 0);
    }

    void PongGameComponent::onReload()
    {
        compileShaders();
        createInputLayout();
    }

    void PongGameComponent::onDestroy()
    {
        // pass
    }

    void PongGameComponent::Draw()
    {
        if (!is_ok_) [[unlikely]] {
            return;
        }

        auto context = Game::getGame().getContext();

        context->IASetInputLayout(inputLayout_.Get());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        UINT stride = sizeof(XMFLOAT4);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);

        stride = sizeof(InstanceData);
        context->IASetVertexBuffers(1, 1, instanceBuffer_.GetAddressOf(), &stride, &offset);

        context->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

        context->VSSetShader(vertexShader_.Get(), nullptr, 0);
        context->PSSetShader(pixelShader_.Get(), nullptr, 0);

        context->VSSetConstantBuffers(0, 1, projectionBuffer_.GetAddressOf());

        context->RSSetState(rasterizerState_.Get());

        context->DrawIndexedInstanced(6, static_cast<UINT>(instances_.size()), 0, 0, 0);
    }

    void PongGameComponent::createBuffers()
    {
        auto device = Game::getGame().getDevice();

        {
            XMFLOAT4 vertices[] = {
                XMFLOAT4(-0.5f, -0.5f, 0.0f, 1.0f),
                XMFLOAT4(0.5f, -0.5f, 0.0f, 1.0f),
                XMFLOAT4(0.5f, 0.5f, 0.0f, 1.0f),
                XMFLOAT4(-0.5f, 0.5f, 0.0f, 1.0f)
            };
            D3D11_BUFFER_DESC bd{};
            bd.ByteWidth = sizeof(vertices);
            bd.Usage = D3D11_USAGE_IMMUTABLE;
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.CPUAccessFlags = 0;
            D3D11_SUBRESOURCE_DATA init{};
            init.pSysMem = vertices;
            ThrowIfFailed(device->CreateBuffer(&bd, &init, vertexBuffer_.GetAddressOf()));
        }

        {
            // Indexes for 2 triangles (0,1,2 and 0,2,3)
            UINT indices[] = { 0, 1, 2, 0, 2, 3 };
            D3D11_BUFFER_DESC bd{};
            bd.ByteWidth = sizeof(indices);
            bd.Usage = D3D11_USAGE_IMMUTABLE;
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            D3D11_SUBRESOURCE_DATA init{};
            init.pSysMem = indices;
            ThrowIfFailed(device->CreateBuffer(&bd, &init, indexBuffer_.GetAddressOf()));
        }

        {
            D3D11_BUFFER_DESC bd = {};
            bd.ByteWidth = static_cast<UINT>(instances_.size() * sizeof(InstanceData));
            bd.Usage = D3D11_USAGE_DYNAMIC;
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bd.MiscFlags = 0;
            ThrowIfFailed(device->CreateBuffer(&bd, nullptr, instanceBuffer_.GetAddressOf()));
        }

        {
            D3D11_BUFFER_DESC bd = {};
            bd.ByteWidth = sizeof(XMFLOAT4X4);
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;
            ThrowIfFailed(device->CreateBuffer(&bd, nullptr, projectionBuffer_.GetAddressOf()));
        }
    }

    void PongGameComponent::updateInstanceBuffer()
    {
        XMMATRIX scale, trans, world;

        scale = XMMatrixScaling(paddleWidth, paddleHeight, 1.0f);
        float paddle1X = left_ + paddleWidth * 0.5f + 0.05f;
        trans = XMMatrixTranslation(paddle1X, paddle1Y_, 0.0f);
        world = scale * trans;
        XMStoreFloat4x4(&instances_[INST_PADDLE_LEFT].world, world);

        float paddle2X = right_ - paddleWidth * 0.5f - 0.05f;
        trans = XMMatrixTranslation(paddle2X, paddle2Y_, 0.0f);
        world = scale * trans;
        XMStoreFloat4x4(&instances_[INST_PADDLE_RIGHT].world, world);

        // LOG(INFO) << "( " << paddle1X << ", " << paddle1Y_ << " ); ( " << paddle2X << ", " << paddle2Y_ << ");";

        scale = XMMatrixScaling(ballSize, ballSize, 1.0f);
        trans = XMMatrixTranslation(ballPos_.x, ballPos_.y, 0.0f);
        world = scale * trans;
        XMStoreFloat4x4(&instances_[INST_BALL].world, world);
    }

    bool PongGameComponent::compileShaders()
    {
        const std::wstring shaderPath{ L"./resources/shaders/pongShader.hlsl" };

        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        auto device = Game::getGame().getDevice();

        HRESULT hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, vertexShaderByteCode_.GetAddressOf(), errorBlob.GetAddressOf());
        if (FAILED(hr) && errorBlob)
        {
            LOG(JERROR) << "Vertex Shader compilation has failed";
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
            return false;
        }

        ThrowIfFailed(device->CreateVertexShader(vertexShaderByteCode_->GetBufferPointer(), vertexShaderByteCode_->GetBufferSize(), nullptr, vertexShader_.GetAddressOf()));

        hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, pixelShaderByteCode_.GetAddressOf(), errorBlob.GetAddressOf());
        if (FAILED(hr) && errorBlob)
        {
            LOG(JERROR) << "Pixel Shader compilation has failed";
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
            return false;
        }

        ThrowIfFailed(device->CreatePixelShader(pixelShaderByteCode_->GetBufferPointer(), pixelShaderByteCode_->GetBufferSize(), nullptr, pixelShader_.GetAddressOf()));

        return true;
    }

    void PongGameComponent::createInputLayout()
    {
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        };

        ThrowIfFailed(Game::getGame().getDevice()->CreateInputLayout(layout, ARRAYSIZE(layout),
            vertexShaderByteCode_->GetBufferPointer(),
            vertexShaderByteCode_->GetBufferSize(),
            inputLayout_.GetAddressOf()));
    }

    void PongGameComponent::createRasterizerState()
    {
        D3D11_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.CullMode = D3D11_CULL_NONE;
        rasterDesc.FrontCounterClockwise = FALSE;
        rasterDesc.DepthClipEnable = TRUE;

        ThrowIfFailed(Game::getGame().getDevice()->CreateRasterizerState(&rasterDesc, rasterizerState_.GetAddressOf()));
    }
}