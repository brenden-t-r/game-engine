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
        FLOAT, FLOAT4, BOOL
    };
    struct UniformField {
        UniformFieldType type;
        const char* name;
        union {
            float f;
            float f4[4]{0,0,0,0};
            bool b;
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
                printf("Cannot find shader variable with name %s\n", f.name);
                continue;
            }
            switch(f.type) {
                case Shader::FLOAT:
                    memcpy(dst + offset, &f.f, sizeof(float));
                    break;
                case Shader::FLOAT4:
                    memcpy(dst + offset, f.f4, sizeof(float) * 4);
                    break;
                case Shader::BOOL:
                    uint32_t v = f.b ? 1 : 0;
                    memcpy(dst + offset, &v, sizeof(uint32_t));
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

protected:
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

class MaterialFont : public MaterialSprite {
public:
    float outlineColor[4]{1.0,0.0,0.0,0.5};
    float outlineWidth = 0;
    float pxRange = 16;
    bool isMSDF = false;
    MaterialFont(Shader* shader, Texture* texture) : MaterialSprite(shader, texture) {
        Shader::UniformField fieldOutlineColor{};
        fieldOutlineColor.name = "OutlineColor";
        fieldOutlineColor.type = Shader::UniformFieldType::FLOAT4;
        uniformFields.push_back(fieldOutlineColor);
        Shader::UniformField fieldOutlineWidth{};
        fieldOutlineWidth.name = "OutlineWidth";
        fieldOutlineWidth.type = Shader::UniformFieldType::FLOAT;
        uniformFields.push_back(fieldOutlineWidth);
        Shader::UniformField fieldPxRange{};
        fieldPxRange.name = "PxRange";
        fieldPxRange.type = Shader::UniformFieldType::FLOAT;
        uniformFields.push_back(fieldPxRange);
        Shader::UniformField fieldIsMSDF{};
        fieldIsMSDF.name = "IsMSDF";
        fieldIsMSDF.type = Shader::UniformFieldType::BOOL;
        uniformFields.push_back(fieldIsMSDF);
    }
    void PreBind() override {
        MaterialColor::UpdateColor();
        uniformFields[1].f4[0] = outlineColor[0];
        uniformFields[1].f4[1] = outlineColor[1];
        uniformFields[1].f4[2] = outlineColor[2];
        uniformFields[1].f4[3] = outlineColor[3];
        uniformFields[2].f = outlineWidth;
        uniformFields[3].f = pxRange;
        uniformFields[4].b = isMSDF;
    }
};


#endif //GAMEPROJECT_MATERIAL_H
