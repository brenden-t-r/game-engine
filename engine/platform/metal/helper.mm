#if defined(PLATFORM_APPLE)

#define MINIAUDIO_IMPLEMENTATION
#include "../../dependencies/miniaudio.h"
static ma_engine g_engine;

//region Static helper functions
static void listFilesInDirectory(NSString *directoryPath, int indent) {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error = nil;

    NSArray *contents = [fileManager contentsOfDirectoryAtPath:directoryPath error:&error];
    if (error) {
        NSLog(@"Error reading directory: %@", error);
        return;
    }

    NSString *indentString = [@"" stringByPaddingToLength:indent withString:@" " startingAtIndex:0];

    for (NSString *item in contents) {
        NSString *fullPath = [directoryPath stringByAppendingPathComponent:item];
        BOOL isDirectory = NO;

        [fileManager fileExistsAtPath:fullPath isDirectory:&isDirectory];

        NSLog(@"%@%@ %@", indentString, isDirectory ? @"📁" : @"📄", item);

        if (isDirectory) {
            listFilesInDirectory(fullPath, indent + 2);
        }
    }
}
static id<MTLTexture> loadImageAsTextureFromBundle(NSString *imageName, id<MTLDevice> device, TextureSettings settings) {
    // Get the path to the image in the app bundle (including extension)
    NSString *imagePath = [[NSBundle mainBundle] pathForResource:imageName ofType:nil];

    if (imagePath) {
        // Read the image data from the file path
        NSData *imageData = [NSData dataWithContentsOfFile:imagePath];

        if (imageData) {
            MTKTextureLoader *textureLoader = [[MTKTextureLoader alloc] initWithDevice:device];
            NSError *error = nil;
            // Load the texture from the image data
            auto mipsEnabled = settings.mipMapsEnabled ? @YES : @NO;
            NSDictionary *options = @{
                    MTKTextureLoaderOptionSRGB : @NO, // Needed this to fix "dark" sprites. May need to revisit.
                    MTKTextureLoaderOptionAllocateMipmaps: mipsEnabled,
                    MTKTextureLoaderOptionGenerateMipmaps: mipsEnabled
            };
            id<MTLTexture> texture = [textureLoader newTextureWithData:imageData options:options error:&error];
            if (texture) {
                NSLog(@"Texture loaded successfully from %@", imageName);
                return texture;
            } else {
                NSLog(@"Failed to load texture: %@", error.localizedDescription);
            }
        } else {
            NSLog(@"Failed to read data from image file: %@", imageName);
        }
    } else {
        NSLog(@"Image not found in bundle: %@", imageName);
    }

    return nil;
}
static NSString* loadTextFileFromBundleAsString(NSString *fileName)
{
    NSString *filePath = [[NSBundle mainBundle] pathForResource:fileName ofType:nil];
    if (!filePath)
    {
        NSLog(@"Text file not found in bundle: %@", fileName);
        return {};
    }
    NSError *error = nil;
    NSString *fileContents = [NSString stringWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:&error];
    if (!fileContents)
    {
        NSLog(@"Failed to read text file: %@ (%@)", fileName, error.localizedDescription);
        return {};
    }
    return fileContents;
}
//endregion

//region Shader Source
struct VertexData {
    simd::float4 position;
    simd::float2 textureCoordinate;
};
static const char* vertexShaderSrc = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
};

vertex VertexOut vertex_main(uint vertexID [[vertex_id]],
                             constant float4 *vertices [[buffer(0)]]) {
    VertexOut out;
    out.position = vertices[vertexID];
    return out;
}
)";
static const char* fragmentShaderSrc = R"(
#include <metal_stdlib>
using namespace metal;

struct ConstantBufferData {
    float4 Color;
};

fragment float4 fragment_main(constant ConstantBufferData& uniforms [[ buffer(0) ]]) {
    return uniforms.Color;
}
)";
static const char* textureVertexShaderSrc = R"(
#include <metal_stdlib>
using namespace metal;

#include <simd/simd.h>
using namespace simd;

struct VertexData {
    float4 position;
    float2 textureCoordinate;
};

struct VertexOut {
    float4 position [[position]];
    float2 textureCoordinate;
};

vertex VertexOut vertex_main(uint vertexID [[vertex_id]],
                              constant VertexData* vertexData) {
    VertexOut out;
    out.position = vertexData[vertexID].position;
    out.textureCoordinate = vertexData[vertexID].textureCoordinate;
    return out;
}
)";
static const char* textureFragmentShaderSrc = R"(
#include <metal_stdlib>
using namespace metal;

#include <simd/simd.h>
using namespace simd;

struct ConstantBufferData {
    float4 Color;
};

fragment float4 fragment_main(VertexOut in [[stage_in]],
                               texture2d<float> colorTexture [[texture(0)]],
                                sampler textureSampler  [[sampler(0)]],
                                constant ConstantBufferData& uniforms [[buffer(0)]]
) {
    float4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    colorSample *= uniforms.Color;
    return colorSample;
}
)";
class ShaderMTL : public Shader {
public:
    id<MTLRenderPipelineState> renderPipelineState;
    int bufferDataSize;
};
//endregion

//region Game Objects
//region Triangle
class TriangleMetal : public Triangle {
public:
    ~TriangleMetal() {
        [vertexBuffer release];
    }
    TriangleMetal(
            id <MTLDevice> metalDevice
    ){
        this->metalDevice = metalDevice;
        static const float metal_vertices[] = {
                vertices[0].x, vertices[0].y, 0.0f, 1.0f,  // Top vertex
                vertices[1].x, vertices[1].y, 0.0f, 1.0f,  // Bottom left vertex
                vertices[2].x, vertices[2].y, 0.0f, 1.0f   // Bottom right vertex
        };
        vertexBuffer = [metalDevice newBufferWithBytes:&metal_vertices length:sizeof(metal_vertices) options:MTLResourceStorageModeShared];
    }

    void SetMaterial(Material* mat) override {
        [constantBuffer release];
        material = mat;
        auto shader = (ShaderMTL*)material->shader;
        constantBuffer = [metalDevice newBufferWithLength:shader->bufferDataSize options:MTLResourceStorageModeShared];
    }

    void Update() override {
        Triangle::Update();
        float metal_vertices[] = {
                vertices[0].x, vertices[0].y, 0.0f, 1.0f,  // Top vertex
                vertices[1].x, vertices[1].y, 0.0f, 1.0f,  // Bottom left vertex
                vertices[2].x, vertices[2].y, 0.0f, 1.0f   // Bottom right vertex
        };
        memcpy([vertexBuffer contents], metal_vertices, sizeof(metal_vertices));

        // Bind constant buffer
        auto shader = (ShaderMTL*)material->shader;
        uint8_t* dst = (uint8_t*)constantBuffer.contents;
        material->BindConstantBuffer(dst);

        [renderCommandEncoder setRenderPipelineState:shader->renderPipelineState];
        [renderCommandEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [renderCommandEncoder setFragmentBuffer:constantBuffer offset:0 atIndex:0];
        [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    }

    void SetRenderCommandEncoder(id<MTLRenderCommandEncoder> commandEncoder) {
        this->renderCommandEncoder = commandEncoder;
    }

private:
    id<MTLDevice> metalDevice;
    id<MTLBuffer> vertexBuffer;
    id<MTLBuffer> constantBuffer;
    id<MTLRenderCommandEncoder> renderCommandEncoder;
};
//endregion
//region Texture
class TextureMTL : public Texture{
public:
    TextureMTL(const char *path, id <MTLTexture> texture, TextureSettings settings, id<MTLSamplerState> samplerState)
            : Texture(path, settings), texture(texture), samplerState(samplerState) {}
    ~TextureMTL() override {
        [texture release];
    }
    id<MTLTexture> texture;
    id<MTLSamplerState> samplerState;
};
//endregion
//region Sprite
class SpriteMetal : public Sprite {
public:
    ~SpriteMetal() {
        [vertexBuffer release];
    }
    SpriteMetal(
            id <MTLDevice> metalDevice,
            TextureMTL* texture
    ){
        this->metalDevice = metalDevice;
        this->texture = texture;
    }

    void SetMaterial(Material* mat) override {
        [constantBuffer release];
        material = mat;
        auto shader = (ShaderMTL*)material->shader;
        constantBuffer = [metalDevice newBufferWithLength:shader->bufferDataSize options:MTLResourceStorageModeShared];
    }

    void Update() override {
        Sprite::Update();
        VertexData newVertices[]{
                {{vertices[0].x, vertices[0].y, 0, 1}, {0.0f, 0.0f}}, // Top left
                {{vertices[3].x, vertices[3].y, 0, 1}, {0.0f, 1.0f}}, // Bottom left
                {{vertices[2].x, vertices[2].y, 0, 1}, {1.0f, 1.0f}}, // Bottom right
                {{vertices[0].x, vertices[0].y, 0, 1}, {0.0f, 0.0f}}, // Top left
                {{vertices[2].x, vertices[2].y, 0, 1}, {1.0f, 1.0f}}, // Bottom right
                {{vertices[1].x, vertices[1].y, 0, 1}, {1.0f, 0.0f}}  // Top right
        };

        int row = atlasRow;
        if (useAtlas && !useGlyph) {
            newVertices[0].textureCoordinate.x = atlasCellSize * (float)atlasColumn; // Top-left
            newVertices[0].textureCoordinate.y = atlasCellSize * (float)atlasRow;
            newVertices[1].textureCoordinate.x = atlasCellSize * (float)atlasColumn; // Bottom left
            newVertices[1].textureCoordinate.y = atlasCellSize * (float)atlasRow + atlasCellSize;
            newVertices[2].textureCoordinate.x = atlasCellSize * (float)atlasColumn + atlasCellSize; // Bottom right
            newVertices[2].textureCoordinate.y = atlasCellSize * (float)atlasRow + atlasCellSize;;
            newVertices[3].textureCoordinate.x = atlasCellSize * (float)atlasColumn; // Top-left
            newVertices[3].textureCoordinate.y = atlasCellSize * (float)atlasRow;
            newVertices[4].textureCoordinate.x = atlasCellSize * (float)atlasColumn + atlasCellSize; // Bottom right
            newVertices[4].textureCoordinate.y = atlasCellSize * (float)atlasRow + atlasCellSize;
            newVertices[5].textureCoordinate.x = atlasCellSize * (float)atlasColumn + atlasCellSize; // Top right
            newVertices[5].textureCoordinate.y = atlasCellSize * (float)atlasRow;
        }
        if (useAtlas && useGlyph) {
            float modifier = 0.0f;
            float u0 = (glyphX + modifier) / (float)atlasWidth;
            float v0 = (glyphY + modifier) / (float)atlasHeight;
            float u1 = (glyphX + glyphW - modifier) / (float)atlasWidth;
            float v1 = (glyphY + glyphH - modifier) / (float)atlasHeight;
            newVertices[0].textureCoordinate = {u0, v0}; // Top-left
            newVertices[1].textureCoordinate = {u0, v1}; // Bottom-left
            newVertices[2].textureCoordinate = {u1, v1}; // Bottom-right
            newVertices[3].textureCoordinate = {u0, v0}; // Top-left  (second triangle)
            newVertices[4].textureCoordinate = {u1, v1}; // Bottom-right
            newVertices[5].textureCoordinate = {u1, v0}; // Top-right
        }

        vertexBuffer = [metalDevice newBufferWithBytes:&newVertices
                                                length:sizeof(newVertices)
                                               options:MTLResourceStorageModeShared];

        // Bind constant buffer
        auto shader = (ShaderMTL*)material->shader;
        uint8_t* dst = (uint8_t*)constantBuffer.contents;
        material->BindConstantBuffer(dst);

        // Bind texture
        auto tex = (TextureMTL*)((MaterialSprite*)material)->texture;

        [renderCommandEncoder setRenderPipelineState:shader->renderPipelineState];
        [renderCommandEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [renderCommandEncoder setFragmentTexture:tex->texture atIndex:0];
        [renderCommandEncoder setFragmentBuffer:constantBuffer offset:0 atIndex:0];
        [renderCommandEncoder setFragmentSamplerState:texture->samplerState atIndex:0];

        [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
    }

    Texture* GetTexture() override {
        return (TextureMTL*)((MaterialSprite*)material)->texture;
    }

    void SetRenderCommandEncoder(id<MTLRenderCommandEncoder> commandEncoder) {
        this->renderCommandEncoder = commandEncoder;
    }

private:
    id<MTLDevice> metalDevice;
    id<MTLBuffer> vertexBuffer;
    id<MTLRenderCommandEncoder> renderCommandEncoder;
    TextureMTL* texture = nullptr;
    id<MTLBuffer> constantBuffer;
};
//endregion
//region Sound
class SoundMA : public Sound {
public:
    ~SoundMA(){
        ma_sound_uninit(&sound);
    };

    void Init(const char* filePath) {
        NSError *error = nil;
        NSString *file = [NSString stringWithUTF8String:filePath];
        NSString *path = [[NSBundle mainBundle] pathForResource:file ofType:nil];
        ma_result result = ma_sound_init_from_file(&g_engine, [path UTF8String], MA_SOUND_FLAG_DECODE, nullptr, nullptr, &sound);
        if (result != MA_SUCCESS) {
            printf("Failed to initialize audio sound.");
        }
        assert(result == MA_SUCCESS);
    }

    void Play() {
        ma_sound_start(&sound);
    }

    void Stop() {
        ma_sound_stop(&sound);
    }

    void Reset() {
        ma_sound_seek_to_pcm_frame(&sound, 0);
    }

    ma_sound sound{};
};
//endregion
//endregion

#endif