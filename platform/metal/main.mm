#if defined(PLATFORM_APPLE)
#define GLFW_INCLUDE_NONE
#import <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#import <GLFW/glfw3native.h>

#include <Metal/Metal.hpp>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>
#include <cstdio>
#include <vector>

#include "../platform.h"
#include "../../constants.h"

#include "stb_image.h"

struct VertexData {
    simd::float4 position;
    simd::float2 textureCoordinate;
};

class Texture {
public:
    Texture(const char* filepath, MTL::Device* metalDevice) {
        device = metalDevice;

        stbi_set_flip_vertically_on_load(true);
        unsigned char* image = stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha);
        assert(image != NULL);

        MTL::TextureDescriptor* textureDescriptor = MTL::TextureDescriptor::alloc()->init();
        textureDescriptor->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
        textureDescriptor->setWidth(width);
        textureDescriptor->setHeight(height);

        texture = device->newTexture(textureDescriptor);

        MTL::Region region = MTL::Region(0, 0, 0, width, height, 1);
        NS::UInteger bytesPerRow = 4 * width;

        texture->replaceRegion(region, 0, image, bytesPerRow);

        textureDescriptor->release();
        stbi_image_free(image);
    }

    ~Texture() {
        texture->release();
    }

    MTL::Texture* texture;
    int width, height, channels;

private:
    MTL::Device* device;
};

void encodeRenderCommandTriangle(MTL::RenderCommandEncoder* renderCommandEncoder, MTL::RenderPipelineState* metalRenderPSO, MTL::Buffer* vertexBuffer) {
    renderCommandEncoder->setRenderPipelineState(metalRenderPSO);
    renderCommandEncoder->setVertexBuffer(vertexBuffer, 0, 0);
    MTL::PrimitiveType typeTriangle = MTL::PrimitiveTypeTriangle;
    NS::UInteger vertexStart = 0;
    NS::UInteger vertexCount = 3;
    renderCommandEncoder->drawPrimitives(typeTriangle, vertexStart, vertexCount);
}

void encodeRenderCommandQuad(MTL::RenderCommandEncoder* renderCommandEncoder, MTL::RenderPipelineState* metalRenderPSO, MTL::Buffer* vertexBuffer, Texture* texture) {
    renderCommandEncoder->setRenderPipelineState(metalRenderPSO);
    renderCommandEncoder->setVertexBuffer(vertexBuffer, 0, 0);
    MTL::PrimitiveType typeTriangle = MTL::PrimitiveTypeTriangle;
    NS::UInteger vertexStart = 0;
    NS::UInteger vertexCount = 6;
    renderCommandEncoder->setFragmentTexture(texture->texture, 0);
    renderCommandEncoder->drawPrimitives(typeTriangle, vertexStart, vertexCount);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        printf("key %d pressed\n", key);
    } else if (action == GLFW_RELEASE) {
        printf("Key %d released\n", key);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        printf("mouse button %d pressed\n", button);
    }
}

void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos) {
    printf("Cursor position: (%.2f, %.2f)\n", xpos, ypos);
}



class PlatformMetal : public Platform {
public:
    void Init() override {

        // Init Metal device
        metalDevice = MTL::CreateSystemDefaultDevice();

        // Init GLFW and create window
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Metal Engine", NULL, NULL);
        if (!glfwWindow) {
            printf("uhoh\n");
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        printf("GLFW initialized\n");

        int width, height;
        glfwGetFramebufferSize(glfwWindow, &width, &height);

        metalWindow = glfwGetCocoaWindow(glfwWindow);
        metalLayer = [CAMetalLayer layer];
        metalLayer.device = (__bridge id<MTLDevice>)metalDevice;
        metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        metalLayer.drawableSize = CGSizeMake(width, height);
        metalWindow.contentView.layer = metalLayer;
        metalWindow.contentView.wantsLayer = YES;
        printf("Metal initialized\n");

        glfwSetKeyCallback(glfwWindow, keyCallback);
        glfwSetMouseButtonCallback(glfwWindow, mouseButtonCallback);
        glfwSetCursorPosCallback(glfwWindow, mouseCursorCallback);

        // Create command queue
        metalCommandQueue = metalDevice->newCommandQueue();
    }

    void LoadShaders() override {
        // Triangle
        NS::String* nsPath = NS::String::string("assets/shaders/triangle.metallib", NS::UTF8StringEncoding);
        if (!nsPath) {
            printf("error with path\n");
            return;
        }
        trianglePipelineDescriptor = LoadShader(nsPath);

        // Sprite
        NS::String* nsPathSq = NS::String::string("assets/shaders/square.metallib", NS::UTF8StringEncoding);
        if (!nsPathSq) {
            printf("error with path\n");
            return;
        }
        texturePipelineDescriptor = LoadShader(nsPathSq);

        NS::Error* error = nullptr;
        error = nullptr;
        triangleRenderPSO = metalDevice->newRenderPipelineState(trianglePipelineDescriptor, &error);
        error = nullptr;
        textureRenderPSO = metalDevice->newRenderPipelineState(texturePipelineDescriptor, &error);

        trianglePipelineDescriptor->release();
        texturePipelineDescriptor->release();
    }

    void Run(void (*func)(void*), void* ctx) override{
        Texture* bgTexture = new Texture("assets/sprites/background.png", metalDevice);
        auto sprite = new SpriteMetal(metalDevice, textureRenderPSO, bgTexture);

        // Game loop
        while (!glfwWindowShouldClose(glfwWindow)) {
            @autoreleasepool {
                metalDrawable = (__bridge CA::MetalDrawable*)[metalLayer nextDrawable];

                metalCommandBuffer = metalCommandQueue->commandBuffer();
                renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();

                MTL::RenderPassColorAttachmentDescriptor* cd = renderPassDescriptor->colorAttachments()->object(0);
                cd->setTexture(metalDrawable->texture());
                cd->setLoadAction(MTL::LoadActionClear);
                cd->setClearColor(MTL::ClearColor(41.0f/255.0f, 42.0f/255.0f, 48.0f/255.0f, 1.0));
                cd->setStoreAction(MTL::StoreActionStore);

                MTL::RenderCommandEncoder* renderCommandEncoder = metalCommandBuffer->renderCommandEncoder(renderPassDescriptor);

                for (TriangleMetal* gameObject : gameObjects) {
                    gameObject->SetRenderCommandEncoder(renderCommandEncoder);
                }

                // Quad
                sprite->SetRenderCommandEncoder(renderCommandEncoder);
                sprite->Update();

                // Triangle
                func(ctx);

                renderCommandEncoder->endEncoding();
                metalCommandBuffer->presentDrawable(metalDrawable);
                metalCommandBuffer->commit();
                metalCommandBuffer->waitUntilCompleted();

                renderPassDescriptor->release();
            }
            glfwPollEvents();
        }
    }

    virtual bool IsKeyPressed(KeyCode key) override {return false; }
    virtual bool IsMousePressed(MouseButton button) override {return false;}
    virtual bool IsMouseReleased(MouseButton button) override {return false;}
    virtual vec3 GetMousePos() override {return {};}
    Sprite* CreateSprite(const char* path) override { return nullptr; }

    class SpriteMetal : public GameObject {
    public:
         ~SpriteMetal() {
            vertexBuffer->release();
        }
        SpriteMetal(
            MTL::Device* metalDevice,
            MTL::RenderPipelineState* metalRenderPSO,
            Texture* texture
        ){
            this->metalDevice = metalDevice;
            this->metalRenderPSO = metalRenderPSO;
            this->texture = texture;
        }

        void Update() override {
            VertexData vertices[] {
                {{-1, -1,  0, 1}, {0.0f, 0.0f}}, // Top left
                {{-1,  1,  0, 1}, {0.0f, 1.0f}}, // Bottom left
                {{ 1,  1,  0, 1}, {1.0f, 1.0f}}, // Bottom right
                {{-1, -1,  0, 1}, {0.0f, 0.0f}}, // Top left
                {{ 1,  1,  0, 1}, {1.0f, 1.0f}}, // Bottom right
                {{ 1, -1,  0, 1}, {1.0f, 0.0f}}  // Top right
            };
            vertexBuffer = metalDevice->newBuffer(&vertices, sizeof(vertices), MTL::ResourceStorageModeShared);
            renderCommandEncoder->setRenderPipelineState(metalRenderPSO);
            renderCommandEncoder->setVertexBuffer(vertexBuffer, 0, 0);
            renderCommandEncoder->setFragmentTexture(texture->texture, 0);
            renderCommandEncoder->drawPrimitives(typeTriangle, vertexStart, vertexCount);
        }

        void SetRenderCommandEncoder(MTL::RenderCommandEncoder* commandEncoder) {
            this->renderCommandEncoder = commandEncoder;
        }

    private:
        MTL::Device* metalDevice;
        MTL::Buffer* vertexBuffer;
        MTL::RenderPipelineState* metalRenderPSO;
        MTL::RenderCommandEncoder* renderCommandEncoder;
        Texture* texture;

        MTL::PrimitiveType typeTriangle = MTL::PrimitiveTypeTriangle;
        NS::UInteger vertexStart = 0;
        NS::UInteger vertexCount = 6;
    };

    class TriangleMetal : public Triangle {
    public:
        ~TriangleMetal() {
            vertexBuffer->release();
        }
        TriangleMetal(
            MTL::Device* metalDevice,
            MTL::RenderPipelineState* metalRenderPSO
        ){
            this->metalDevice = metalDevice;
            this->metalRenderPSO = metalRenderPSO;
            simd::float3 vertices[3] = {
                {-0.5f, -0.5f, 0.0f},
                { 0.5f, -0.5f, 0.0f},
                { 0.0f,  0.5f, 0.0f}
            };
            vertexBuffer = metalDevice->newBuffer(&vertices, sizeof(vertices), MTL::ResourceStorageModeShared);
        }

        void Update() override {
            simd::float3 vertices[3] = {
                {transform.pos.x - 0.5f, -0.5f, 0.0f},
                {transform.pos.x + 0.5f, -0.5f, 0.0f},
                {transform.pos.x,  0.5f, 0.0f}
            };
            vertexBuffer = metalDevice->newBuffer(&vertices, sizeof(vertices), MTL::ResourceStorageModeShared);
            renderCommandEncoder->setRenderPipelineState(metalRenderPSO);
            renderCommandEncoder->setVertexBuffer(vertexBuffer, 0, 0);
            renderCommandEncoder->drawPrimitives(typeTriangle, vertexStart, vertexCount);
        }

        void SetRenderCommandEncoder(MTL::RenderCommandEncoder* commandEncoder) {
            this->renderCommandEncoder = commandEncoder;
        }

    private:
        MTL::Device* metalDevice;
        MTL::Buffer* vertexBuffer;
        MTL::RenderPipelineState* metalRenderPSO;
        MTL::RenderCommandEncoder* renderCommandEncoder;

        MTL::PrimitiveType typeTriangle = MTL::PrimitiveTypeTriangle;
        NS::UInteger vertexStart = 0;
        NS::UInteger vertexCount = 3;
    };

    GameObject* CreateTriangle() override {
        auto gameObject = new TriangleMetal(metalDevice, triangleRenderPSO);
        gameObjects.push_back(gameObject);
        return gameObject;
    }

    void Shutdown() override {
        renderPassDescriptor->release();
    }

private:
        MTL::Device* metalDevice;
        GLFWwindow* glfwWindow;
        NSWindow* metalWindow;
        CAMetalLayer* metalLayer;
        CA::MetalDrawable* metalDrawable;
        MTL::Library* metalLibrary = nullptr;
        MTL::CommandQueue* metalCommandQueue;
        MTL::CommandBuffer* metalCommandBuffer;
        MTL::RenderPipelineState* metalRenderPSO;
        MTL::Buffer* triangleVertexBuffer;
        MTL::Buffer* squareVertexBuffer;
        MTL::RenderPassDescriptor* renderPassDescriptor;

        MTL::RenderPipelineDescriptor* trianglePipelineDescriptor;
        MTL::RenderPipelineDescriptor* texturePipelineDescriptor;
        MTL::RenderPipelineState* triangleRenderPSO;
        MTL::RenderPipelineState* textureRenderPSO;

        std::vector<TriangleMetal*> gameObjects = {};

        MTL::RenderPipelineDescriptor* LoadShader(NS::String* path) {
            NS::Error* error = nullptr;
            metalLibrary = metalDevice->newLibrary(path, &error);
            if (error) {
                printf("Error: %s\n", error->localizedDescription()->utf8String());
            } else if (!metalLibrary) {
                printf("failed to load shader library\n");
            } else {
                printf("shader library loaded successfully\n");
            }

            // Create render pipeline
            MTL::Function* vertexShader = metalLibrary->newFunction(NS::String::string("vertexShader", NS::ASCIIStringEncoding));
            assert(vertexShader);
            MTL::Function* fragmentShader = metalLibrary->newFunction(NS::String::string("fragmentShader", NS::ASCIIStringEncoding));
            assert(fragmentShader);

            MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
            renderPipelineDescriptor->setLabel(NS::String::string("Triangle Rendering Pipeline", NS::ASCIIStringEncoding));
            renderPipelineDescriptor->setVertexFunction(vertexShader);
            renderPipelineDescriptor->setFragmentFunction(fragmentShader);
            assert(renderPipelineDescriptor);
            MTL::PixelFormat pixelFormat = (MTL::PixelFormat)metalLayer.pixelFormat;
            renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);

            return renderPipelineDescriptor;
        }
};

int RealMain(Platform* platform);

int main() {
	printf("Hello world\n");
    auto platform = new PlatformMetal();
    return RealMain(platform);
}
#endif