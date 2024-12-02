#include "sprite.h"

class Scene {
public:
    Scene(ID3D11Device* d3dDevice,
          ID3D11DeviceContext* d3dContext,
          ID3D11VertexShader* vertexShader,
          ID3D11PixelShader* pixelShader) {
        this->d3dDevice = d3dDevice;
        this->d3dContext = d3dContext;
        this->vertexShader = vertexShader;
        this->pixelShader = pixelShader;
    }
    virtual ~Scene() = default;
    virtual void Init() = 0;
    virtual void Update() = 0;

protected:
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
};
