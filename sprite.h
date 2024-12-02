#ifndef GAMEENGINE_SPRITE
#define GAMEENGINE_SPRITE

#include <d3d11.h>
#include <directxmath.h>
#include "constants.h"
#include "math.h"
#include "file_util.h"

class Sprite {
public:
    Sprite() {}
    ~Sprite() {}

    Vertex vertices[4] = {
            { DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
            { DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    };

    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11ShaderResourceView* textureView = nullptr;

    void LoadTexture(ID3D11Device* d3dDevice, const WCHAR * file) {
        LoadTextureFromFile(d3dDevice, file, &textureView);
    }

    void SetPosition(float x, float y, float width, float height) {
        width = width / WINDOW_WIDTH * 2;
        height = height / WINDOW_HEIGHT * 2;
        vertices[0].position = DirectX::XMFLOAT3(x, y, 0.0f);
        vertices[1].position = DirectX::XMFLOAT3(x+width, y, 0.0f);
        vertices[2].position = DirectX::XMFLOAT3(x, y-height, 0.0f);
        vertices[3].position = DirectX::XMFLOAT3(x+width, y-height, 0.0);
    }

    void SetPositionCentered(float width, float height) {
        width = width / WINDOW_WIDTH;
        height = height / WINDOW_HEIGHT;
        vertices[0].position = DirectX::XMFLOAT3(- width/2, 0 + height/2, 0.0f);
        vertices[1].position = DirectX::XMFLOAT3(width/2, 0 + height/2, 0.0f);
        vertices[2].position = DirectX::XMFLOAT3(-width/2, 0 - height/2, 0.0f);
        vertices[3].position = DirectX::XMFLOAT3(width/2, 0 -height/2, 0.0);
    }

    void TranslateY(float y) {
        vertices[0].position.y += y;
        vertices[1].position.y += y;
        vertices[2].position.y += y;
        vertices[3].position.y += y;
    }

    void SetPositionPixels(float x, float y, float width, float height) {
        float w = width / WINDOW_WIDTH;
        float h = height / WINDOW_HEIGHT;
        float posX = -1 + x / WINDOW_WIDTH;
        float posY = 1 - y / WINDOW_HEIGHT;

        vertices[0].position = DirectX::XMFLOAT3(posX, posY, 0.0);
        vertices[1].position = DirectX::XMFLOAT3(posX+w, posY, 0.0);
        vertices[2].position = DirectX::XMFLOAT3(posX, posY-h, 0.0);
        vertices[3].position = DirectX::XMFLOAT3(posX+w, posY-h, 0.0);
    }

    void Rotate(float offset) {
        vertices[0].position = DirectX::XMFLOAT3(vertices[0].position.x+offset, vertices[0].position.y+offset, 0.0);
        vertices[1].position = DirectX::XMFLOAT3(vertices[1].position.x+offset, vertices[1].position.y-offset, 0.0);
        vertices[2].position = DirectX::XMFLOAT3(vertices[2].position.x-offset, vertices[2].position.y+offset, 0.0);
        vertices[3].position = DirectX::XMFLOAT3(vertices[3].position.x-offset, vertices[3].position.y-offset, 0.0);
    }

    void CreateBuffer(ID3D11Device* d3dDevice) {
        // Create the vertex buffer (same as before)
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(vertices);
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = vertices;

        d3dDevice->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
    }

    void Draw(ID3D11DeviceContext* d3dContext, ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader) {
        // Set the vertex buffer
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        // Set the shaders
        d3dContext->VSSetShader(vertexShader, nullptr, 0);
        d3dContext->PSSetShader(pixelShader, nullptr, 0);
        d3dContext->PSSetShaderResources(0, 1, &textureView);

        // Draw
        d3dContext->Draw(4, 0);
    }
};

#endif //GAMEENGINE_SPRITE