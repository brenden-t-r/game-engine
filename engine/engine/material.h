#ifndef GAMEPROJECT_MATERIAL_H
#define GAMEPROJECT_MATERIAL_H

#include "texture.h"

#include "cstring"
#include <vector>
#ifdef BACKEND_DIRECTX
#include "DirectXMath.h"
#elif BACKEND_OPENGL
#include "GL/glew.h"
#endif

enum class ShaderType {
    COLOR, TEXTURE, FONT
};

enum class InputLayoutType {
    POSITION, POSITION_TEXCOORD
};

class Shader {
public:
    InputLayoutType inputLayoutType;
};

class TextureBuffer {
public:
    Texture* texture;
    int index;
};

class Material {
public:
    explicit Material(Shader* shader): shader(shader){}
    virtual ~Material() {}
    virtual void* GetConstantBuffer() {
        return nullptr;
    }
    virtual std::vector<TextureBuffer> GetTextures() {
        return textures;
    }

    Shader* shader;
protected:
    // Consider vector - is there value in multiple constant buffers?
    void* constantBuffer{};
    std::vector<TextureBuffer> textures;
};

class MaterialColor : public Material {
public:
    float color[4]{0,0,0,0};
    explicit MaterialColor(Shader* shader): Material(shader){}
    ~MaterialColor() override {
        delete (ConstantBufferData*) constantBuffer;
    }

#ifdef BACKEND_DIRECTX
    struct ConstantBufferData {
        DirectX::XMFLOAT4 Color;
    };
    void* GetConstantBuffer() override {
        auto* constantBufferData = new ConstantBufferData{
                DirectX::XMFLOAT4(color)
        };
        return constantBufferData;
    }
#elif BACKEND_OPENGL
    struct ConstantBufferData {
        GLfloat color[4];
    };
    void* GetConstantBuffer() override {
        auto* buf = new ConstantBufferData();
        memcpy(buf->color, color, sizeof(float) * 4);
        constantBuffer = buf;
        return constantBuffer;
    }
#else
    assert(false);
#endif
};

class MaterialSprite : public Material {
public:
    Texture* texture;
    MaterialSprite(Shader *shader, Texture* texture) : Material(shader), texture(texture) {
        textures = std::vector<TextureBuffer>{};
        textures.push_back(TextureBuffer{texture, 0});
    }
    std::vector<TextureBuffer> GetTextures() override {
        textures[0].texture = texture;
        return textures;
    }
};

class MaterialFont : public Material {
public:
    float color[4]{0,0,0,0};
    float outlineColor[4]{0,0,0,0};
    Texture* textureBitmap;
    Texture* textureSDF;
    MaterialFont(Shader *shader, Texture* textureBitmap, Texture* textureSDF):
        Material(shader), textureBitmap(textureBitmap), textureSDF(textureSDF) {
        textures = std::vector<TextureBuffer>{};
        textures.push_back(TextureBuffer{nullptr, 0});
        textures.push_back(TextureBuffer{nullptr, 1});
    }
    ~MaterialFont() override {
        delete (ConstantBufferData*) constantBuffer;
    }
    std::vector<TextureBuffer> GetTextures() override {
        textures[0].texture = textureBitmap;
        textures[0].texture = textureSDF;
        return textures;
    }

#ifdef BACKEND_DIRECTX
    struct ConstantBufferData {
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT4 outlineColor;
    };
    void* GetConstantBuffer() override {
        auto* constantBufferData = new ConstantBufferData{
                DirectX::XMFLOAT4(color), DirectX::XMFLOAT4(outlineColor)
        };
       return constantBufferData;
    }
#elif BACKEND_OPENGL
    struct ConstantBufferData {
        GLfloat color[4];
        GLfloat outlineColor[4];
    };
    void* GetConstantBuffer() override {
        auto* buf = new ConstantBufferData();
        memcpy(buf->color, color, sizeof(float) * 4);
        memcpy(buf->outlineColor, outlineColor, sizeof(float) * 4);
        constantBuffer = buf;
        return constantBuffer;
    }
#else
    assert(false);
#endif
};

#endif //GAMEPROJECT_MATERIAL_H
