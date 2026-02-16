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

enum class InputLayoutType {
    POSITION, POSITION_TEXCOORD
};
struct ShaderDef {
    // Path to a consolidated vertex + fragment shader
    const char* path;
    // Path to a vertex shader
    const char* vertexPath;
    // Path to a fragment shader
    const char* fragmentPath;
    InputLayoutType inputLayoutType;
};
class Shader {};
class TextureBuffer {
public:
    Texture* texture;
    int index;
};

class Material {
public:
    explicit Material(Shader* shader): shader(shader){}
    virtual ~Material() = default;
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
    virtual void BindConstantBuffer(ID3D11ShaderReflectionConstantBuffer* cb, uint8_t* dst) {
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
    virtual void BindConstantBuffer(GLuint shaderProgram) {
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

    std::vector<UniformField> uniformFields;
protected:
    std::vector<TextureBuffer> textures;
};

class MaterialColor : public Material {
public:
    explicit MaterialColor(Shader* shader): Material(shader){
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
        Material::BindConstantBuffer(cb, dst);
    }
#elif BACKEND_OPENGL
    void BindConstantBuffer(GLuint shaderProgram) override {
        UpdateColor();
        Material::BindConstantBuffer(shaderProgram);
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

class MaterialSprite : public MaterialColor {
public:
    MaterialSprite(Shader *shader, Texture* texture) : MaterialColor(shader), texture(texture) {
        textures = std::vector<TextureBuffer>{};
        textures.push_back(TextureBuffer{texture, 0});
    }
    Texture* texture;
};

#endif //GAMEPROJECT_MATERIAL_H
