#ifndef GAMEPROJECT_MATERIAL_H
#define GAMEPROJECT_MATERIAL_H

#include "texture.h"

#include "cstring"
#include <vector>

#ifdef BACKEND_DIRECTX
#include "DirectXMath.h"
#include "d3d11shader.h"
#elif BACKEND_OPENGL
#include "GL/glew.h"
#elif BACKEND_METAL
#import <simd/simd.h>
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
    void BindConstantBuffer(GLuint shaderProgram) {
        GLint loc = glGetUniformLocation(shaderProgram, "Color");
        glUniform4f(loc, color[0], color[1], color[2], color[3]);
    }
#elif BACKEND_METAL
    struct ConstantBufferData {
        vector_float4 Color;
    };
    void* GetConstantBuffer() override {
        auto* buf = new ConstantBufferData();
        buf->Color = {color[0], color[1], color[2], color[3]};
        return buf;
    }
#else
    assert(false);
#endif
};

class MaterialWithUniformBuffer: public Material {
public:
    explicit MaterialWithUniformBuffer(Shader* shader): Material(shader){}
    enum UniformFieldType {
        FLOAT, FLOAT4
    };
    struct UniformField {
        UniformFieldType type;
        const char* name;
        union {
            float f;
            float f4[4]{0,0,0,0};
        };
    };
    std::vector<UniformField> fields;

#ifdef BACKEND_DIRECTX
    void BindConstantBuffer(ID3D11ShaderReflectionConstantBuffer* cb, uint8_t* dst) {
        for (auto f : fields) {
            auto var = cb->GetVariableByName(f.name);
            if (!var) {
                printf("Cannot find shader variable with name %s", f.name);
                continue;
            }
            D3D11_SHADER_VARIABLE_DESC varDesc;
            var->GetDesc(&varDesc);
            switch(f.type) {
                case FLOAT:
                    memcpy(dst + varDesc.StartOffset, &f.f, sizeof(float));
                    break;
                case FLOAT4:
                    memcpy(dst + varDesc.StartOffset, f.f4, sizeof(float) * 4);
                    break;
            }
        }
    }
#elif BACKEND_OPENGL
    void BindConstantBuffer(GLuint shaderProgram) {
        for (auto f : fields) {
            GLint loc = glGetUniformLocation(shaderProgram, f.name);
            switch(f.type) {
                case FLOAT:
                    glUniform1f(loc, f.f);
                    break;
                case FLOAT4:
                    glUniform4f(loc, f.f4[0], f.f4[1], f.f4[2], f.f4[3]);
                    break;
            }
        }
    }
#endif
};

class MaterialTwoColors : public Material {
public:
    float color1[4]{0,0,0,0};
    float color2[4]{0,0,0,0};
    explicit MaterialTwoColors(Shader* shader): Material(shader){}
    ~MaterialTwoColors() override {
        delete (ConstantBufferData*) constantBuffer;
    }

#ifdef BACKEND_DIRECTX
    struct ConstantBufferData {
        DirectX::XMFLOAT4 Color;
    };
    void BindConstantBuffer(ID3D11ShaderReflectionConstantBuffer* cb, uint8_t* dst) {
        auto var = cb->GetVariableByName("Color1");
        assert(var != nullptr);
        D3D11_SHADER_VARIABLE_DESC varDesc;
        var->GetDesc(&varDesc);
        DirectX::XMFLOAT4 c1{
                color1[0], color1[1], color1[2], color1[3]
        };
        memcpy(dst + varDesc.StartOffset, &c1, sizeof(c1));

        auto var1 = cb->GetVariableByName("Color2");
        assert(var1 != nullptr);
        D3D11_SHADER_VARIABLE_DESC varDesc1;
        var1->GetDesc(&varDesc1);
        DirectX::XMFLOAT4 c2{
                color2[0], color2[1], color2[2], color2[3]
        };
        memcpy(dst + varDesc1.StartOffset, &c2, sizeof(c2));
    }
#elif BACKEND_OPENGL
    struct ConstantBufferData {
        GLfloat color[4];
    };
    void* GetConstantBuffer() override {
        auto* buf = new ConstantBufferData();
        memcpy(buf->color, color1, sizeof(float) * 4);
        constantBuffer = buf;
        return constantBuffer;
    }
    void BindConstantBuffer(GLuint shaderProgram) {
        GLint loc = glGetUniformLocation(shaderProgram, "Color1");
        glUniform4f(loc, color1[0], color1[1], color1[2], color1[3]);
        GLint loc1 = glGetUniformLocation(shaderProgram, "Color2");
        glUniform4f(loc1, color2[0], color2[1], color2[2], color2[3]);
    }
#elif BACKEND_METAL
    struct ConstantBufferData {
        vector_float4 Color;
    };
    void* GetConstantBuffer() override {
        auto* buf = new ConstantBufferData();
        buf->Color = {color[0], color[1], color[2], color[3]};
        return buf;
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
#elif BACKEND_METAL
    struct ConstantBufferData {
        vector_float4 color;
    };
    void* GetConstantBuffer() override {
        auto* buf = new ConstantBufferData();
        return buf;
    }
#else
    assert(false);
#endif
};

#endif //GAMEPROJECT_MATERIAL_H
