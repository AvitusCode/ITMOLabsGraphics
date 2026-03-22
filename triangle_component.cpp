#include "triangle_component.hpp"

#include "jd_types.hpp"
#include <d3dcompiler.h>

#include "game.hpp"
#include "logger.hpp"

#include "exception.hpp"

using namespace Microsoft::WRL;

namespace jd
{
    TriangleComponent::TriangleComponent(std::vector<Vertex> vertices, std::wstring shaderPath)
        : vertices_{std::move(vertices)}
        , shaderPath_{std::move(shaderPath)}
    {
        if (vertices_.empty() || vertices_.size() % 3 != 0 || shaderPath_.empty()) {
            LOG(FATAL) << "Error conditions: vertices_size=" << vertices_.size();
        }
    }

    void TriangleComponent::onInit()
    {
        is_ok_ = compileShaders();
        if (!is_ok_) {
            return;
        }

        createInputLayout();
        createVertexBuffer();
        createRasterizerState();

        DLOG(INFO) << "Triangle component has created";
    }

    void TriangleComponent::onUpdate(double)
    {
        // pass
    }

    void TriangleComponent::onResize()
    {
        // pass
	}

    void TriangleComponent::onReload()
    {
        compileShaders();
        createInputLayout();
    }

    void TriangleComponent::onDestroy()
    {
        DLOG(INFO) << "Triangle component has destroyed";
    }

    void TriangleComponent::Draw()
    {
        // TODO: optimize
        if (is_ok_) [[likely]] {
            auto& game = Game::getGame();
            auto context = game.getContext();

            context->IASetInputLayout(inputLayout_.Get());

            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);

            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            context->VSSetShader(vertexShader_.Get(), nullptr, 0);
            context->PSSetShader(pixelShader_.Get(), nullptr, 0);

            context->RSSetState(rasterizerState_.Get());

            context->Draw(3, 0);
        }
    }

    bool TriangleComponent::compileShaders()
    {
        HRESULT hr;
        auto& game = Game::getGame();
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        hr = D3DCompileFromFile(shaderPath_.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, vertexShaderByteCode_.GetAddressOf(), errorBlob.GetAddressOf());
        if (FAILED(hr)) {
            if (errorBlob)
            {
                OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
            }
            return false;
        }

        ThrowIfFailed(game.getDevice()->CreateVertexShader(vertexShaderByteCode_->GetBufferPointer(), vertexShaderByteCode_->GetBufferSize(), nullptr, vertexShader_.GetAddressOf()));

        hr = D3DCompileFromFile(shaderPath_.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, pixelShaderByteCode_.GetAddressOf(), errorBlob.GetAddressOf());
        if (FAILED(hr)) {
            if (errorBlob)
            {
                OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
            }
            return false;
        }

        ThrowIfFailed(game.getDevice()->CreatePixelShader(pixelShaderByteCode_->GetBufferPointer(), pixelShaderByteCode_->GetBufferSize(), nullptr, pixelShader_.GetAddressOf()));

        return true;
    }

    void TriangleComponent::createVertexBuffer()
    {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = static_cast<UINT>(vertices_.size() * sizeof(Vertex));
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = vertices_.data();

        ThrowIfFailed(Game::getGame().getDevice()->CreateBuffer(&bufferDesc, &initData, vertexBuffer_.GetAddressOf()));
    }

    void TriangleComponent::createInputLayout()
    {
        D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        ThrowIfFailed(Game::getGame().getDevice()->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc),
            vertexShaderByteCode_->GetBufferPointer(),
            vertexShaderByteCode_->GetBufferSize(),
            inputLayout_.GetAddressOf()));
    }

    void TriangleComponent::createRasterizerState()
    {
        D3D11_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.CullMode = D3D11_CULL_BACK;
        rasterDesc.FrontCounterClockwise = FALSE;
        rasterDesc.DepthClipEnable = TRUE;

        ThrowIfFailed(Game::getGame().getDevice()->CreateRasterizerState(&rasterDesc, rasterizerState_.GetAddressOf()));
    }
}