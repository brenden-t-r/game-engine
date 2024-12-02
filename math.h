#ifndef GAMEENGINE_MATH
#define GAMEENGINE_MATH

#include <DirectXMath.h>

// Vertex structure
struct VertexBasic
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

// Vertex structure
struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texCoord;
};

#endif //GAMEENGINE_MATH