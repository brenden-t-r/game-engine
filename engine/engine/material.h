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
    virtual ~Material() = default;
    virtual std::vector<TextureBuffer> GetTextures() {
        return textures;
    }
    Shader* shader;
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

#ifdef BACKEND_DIRECTX
    virtual void BindConstantBuffer(ID3D11ShaderReflectionConstantBuffer* cb, uint8_t* dst){}
#elif BACKEND_OPENGL
    virtual void BindConstantBuffer(GLuint shaderProgram){}
#endif

protected:
    std::vector<TextureBuffer> textures;
};

class MaterialWithUniformBuffer: public Material {
public:
    explicit MaterialWithUniformBuffer(Shader* shader): Material(shader){}
    std::vector<UniformField> uniformFields;

#ifdef BACKEND_DIRECTX
    void BindConstantBuffer(ID3D11ShaderReflectionConstantBuffer* cb, uint8_t* dst) override {
        for (auto f : uniformFields) {
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
    void BindConstantBuffer(GLuint shaderProgram) override {
        for (auto f : uniformFields) {
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

class MaterialColor : public MaterialWithUniformBuffer {
public:
    explicit MaterialColor(Shader* shader): MaterialWithUniformBuffer(shader){
        UniformField field{};
        field.name = "Color";
        field.type = UniformFieldType::FLOAT4;
        uniformFields.push_back(field);
    }
    ~MaterialColor() override = default;
    float color[4]{1.0,1.0,1.0,1.0};

#ifdef BACKEND_DIRECTX
    void BindConstantBuffer(ID3D11ShaderReflectionConstantBuffer* cb, uint8_t* dst) override {
        UpdateColor();
        MaterialWithUniformBuffer::BindConstantBuffer(cb, dst);
    }
#elif BACKEND_OPENGL
    void BindConstantBuffer(GLuint shaderProgram) override {
        UpdateColor();
        MaterialWithUniformBuffer::BindConstantBuffer(shaderProgram);
    }
#endif

private:
    void UpdateColor() {
        uniformFields[0].f4[0] = color[0];
        uniformFields[0].f4[1] = color[1];
        uniformFields[0].f4[2] = color[2];
        uniformFields[0].f4[3] = color[3];
    }
};

class MaterialSprite : public Material {
public:
    MaterialSprite(Shader *shader, Texture* texture) : Material(shader), texture(texture) {
        textures = std::vector<TextureBuffer>{};
        textures.push_back(TextureBuffer{texture, 0});
    }
    Texture* texture;

    std::vector<TextureBuffer> GetTextures() override {
        textures[0].texture = texture;
        return textures;
    }
};

/*class MaterialFont : public Material {
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
};*/

struct ShaderDef {
    // Path to a consolidated vertex + fragment shader
    const char* path;
    // Path to a vertex shader
    const char* vertexPath;
    // Path to a fragment shader
    const char* fragmentPath;
    InputLayoutType inputLayoutType;
};

#endif //GAMEPROJECT_MATERIAL_H
