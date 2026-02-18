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
#include <unordered_map>
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
    struct UniformFieldOffset {
        const char* name;
        uint32_t offset;
    };
    // Called each frame just prior to submitting constant buffer data to GPU.
    // Useful for utility materials like MaterialColor, where internal properties
    // can overwrite the hidden uniformFields prior to GPU submission.
    virtual void PreBind() {};

#if BACKEND_METAL
    virtual void BindConstantBuffer(std::unordered_map<std::string, UniformFieldOffset> uniformFieldMap, uint8_t* dst)
    {
        for (auto& f : uniformFields)
        {
            auto it = uniformFieldMap.find(f.name);
            if (it == uniformFieldMap.end()) {
                printf("Cannot find shader variable with name %s", f.name);
                continue;
            }
            uint32_t offset = it->second.offset;
            switch (f.type)
            {
                case FLOAT:
                    memcpy(dst + offset, &f.f, sizeof(float));
                    break;

                case FLOAT4:
                    memcpy(dst + offset, f.f4, sizeof(float) * 4);
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
    void PreBind() override {
        UpdateColor();
    }

#if BACKEND_METAL
    void BindConstantBuffer(std::unordered_map<std::string, UniformFieldOffset> uniformFieldMap, uint8_t* dst) override {
        UpdateColor();
        Material::BindConstantBuffer(uniformFieldMap, dst);
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
