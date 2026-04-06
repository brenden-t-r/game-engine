#ifndef GAMEPROJECT_MATERIAL_H
#define GAMEPROJECT_MATERIAL_H

#include "texture.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

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
class Shader {
public:
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
    std::vector<UniformFieldOffset> uniformFieldOffsets;
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
    Shader* shader;
    // Called each frame just prior to submitting constant buffer data to GPU.
    // Useful for utility materials like MaterialColor, where internal properties
    // can overwrite the hidden uniformFields prior to GPU submission.
    virtual void PreBind() {};
    void BindConstantBuffer(uint8_t* dst) {
        PreBind();
        for (auto f : uniformFields) {
            uint32_t offset = 0;
            bool found = false;
            for (auto& s : shader->uniformFieldOffsets)
            {
                if (strcmp(s.name, f.name) == 0)
                {
                    offset = s.offset;
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("Cannot find shader variable with name %s", f.name);
                continue;
            }
            switch(f.type) {
                case Shader::FLOAT:
                    memcpy(dst + offset, &f.f, sizeof(float));
                    break;
                case Shader::FLOAT4:
                    memcpy(dst + offset, f.f4, sizeof(float) * 4);
                    break;
            }
        }
    }

    std::vector<Shader::UniformField> uniformFields;
protected:
    std::vector<TextureBuffer> textures;
};

class MaterialColor : public Material {
public:
    explicit MaterialColor(Shader* shader): Material(shader){
        Shader::UniformField field{};
        field.name = "Color";
        field.type = Shader::UniformFieldType::FLOAT4;
        uniformFields.push_back(field);
    }
    ~MaterialColor() override = default;
    float color[4]{1.0,1.0,1.0,1.0};
    void PreBind() override {
        UpdateColor();
    }

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
    MaterialSprite(Shader* shader, Texture* texture) : MaterialColor(shader), texture(texture) {
        textures = std::vector<TextureBuffer>{};
        textures.push_back(TextureBuffer{texture, 0});
    }
    Texture* texture;
};

#endif //GAMEPROJECT_MATERIAL_H
