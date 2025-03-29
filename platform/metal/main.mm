#if defined(PLATFORM_APPLE) and defined(BACKEND_METAL) and not defined(PLATFORM_IOS)
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include "stb_image.h"

#include "../platform.h"

#include <cstdio>

static void *runFuncContext;
static void (*runFunc)(void *);

struct VertexData {
    simd::float4 position;
    simd::float2 textureCoordinate;
};
class Texture {
public:
    Texture(const char* filepath, id<MTLDevice> metalDevice) {
        device = metalDevice;

        stbi_set_flip_vertically_on_load(true);
        unsigned char* image = stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha);
        assert(image != NULL);

        MTLTextureDescriptor* textureDescriptor = [[MTLTextureDescriptor alloc] init];
        [textureDescriptor setPixelFormat: MTLPixelFormatRGBA8Unorm];
        [textureDescriptor setWidth: width];
        [textureDescriptor setHeight: height];

        texture = [device newTextureWithDescriptor: textureDescriptor];

        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        NSUInteger bytesPerRow = 4 * width;

        [texture replaceRegion:region mipmapLevel:0 withBytes:image bytesPerRow:bytesPerRow];

        [textureDescriptor release];
        stbi_image_free(image);
    }

    ~Texture() {
        [texture release];
    }

    id<MTLTexture> texture;
    int width, height, channels;

private:
    id<MTLDevice> device;
};

//region Shader Source
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

fragment float4 fragment_main() {
    return float4(1.0, 0, 0.0, 1.0); // Red color
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

fragment float4 fragment_main(VertexOut in [[stage_in]],
                               texture2d<float> colorTexture [[texture(0)]]) {
    constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);
    const float4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    return colorSample;
}
)";
//endregion

//region: MetalView/MetalAppDelegate declarations
@interface MetalView : MTKView <MTKViewDelegate>
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLRenderPipelineState> trianglePSO;
@property (nonatomic, strong) id<MTLRenderPipelineState> texturePSO;
@end
@interface MetalAppDelegate : NSObject <NSApplicationDelegate>
@property (strong, nonatomic) NSWindow *window;
@property (strong, nonatomic) MetalView *metalView;
@property (strong, nonatomic) id<MTLDevice> device;
@end
//endregion

//region: Game Objects
class TriangleMetal : public Triangle {
public:
    ~TriangleMetal() {
        [vertexBuffer release];
    }
    TriangleMetal(
            id <MTLDevice> metalDevice,
            id <MTLRenderPipelineState> metalRenderPSO
    ){
        this->metalDevice = metalDevice;
        this->metalRenderPSO = metalRenderPSO;
        static const float vertices[] = {
                vertex1.x, vertex1.y, 0.0f, 1.0f,  // Top vertex
                vertex2.x, vertex2.y, 0.0f, 1.0f,  // Bottom left vertex
                vertex3.x, vertex3.y, 0.0f, 1.0f   // Bottom right vertex
        };
        vertexBuffer = [metalDevice newBufferWithBytes:&vertices length:sizeof(vertices) options:MTLResourceStorageModeShared];
    }

    void Update() override {
        Triangle::Update();
        float vertices[] = {
                vertex1.x, vertex1.y, 0.0f, 1.0f,  // Top vertex
                vertex2.x, vertex2.y, 0.0f, 1.0f,  // Bottom left vertex
                vertex3.x, vertex3.y, 0.0f, 1.0f   // Bottom right vertex
        };
        memcpy([vertexBuffer contents], vertices, sizeof(vertices));
        [renderCommandEncoder setRenderPipelineState:metalRenderPSO];
        [renderCommandEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    }

    void SetRenderCommandEncoder(id<MTLRenderCommandEncoder> commandEncoder) {
        this->renderCommandEncoder = commandEncoder;
    }

private:
    id<MTLDevice> metalDevice;
    id<MTLBuffer> vertexBuffer;
    id<MTLRenderPipelineState> metalRenderPSO;
    id<MTLRenderCommandEncoder> renderCommandEncoder;
};
class SpriteMetal : public Sprite {
public:
    ~SpriteMetal() {
        [vertexBuffer release];
    }
    SpriteMetal(
            id <MTLDevice> metalDevice,
            id <MTLRenderPipelineState> metalRenderPSO,
            Texture* texture
    ){
        this->metalDevice = metalDevice;
        this->metalRenderPSO = metalRenderPSO;
        this->texture = texture;
    }

    void Update() override {
        Sprite::Update();
        VertexData newVertices[]{
                {{vertex1.x, vertex1.y, 0, 1}, {0.0f, 0.0f}}, // Top left
                {{vertex4.x, vertex4.y, 0, 1}, {0.0f, 1.0f}}, // Bottom left
                {{vertex3.x, vertex3.y, 0, 1}, {1.0f, 1.0f}}, // Bottom right
                {{vertex1.x, vertex1.y, 0, 1}, {0.0f, 0.0f}}, // Top left
                {{vertex3.x, vertex3.y, 0, 1}, {1.0f, 1.0f}}, // Bottom right
                {{vertex2.x, vertex2.y, 0, 1}, {1.0f, 0.0f}}  // Top right
        };

        // TODO: Sprite atlas

        vertexBuffer = [metalDevice newBufferWithBytes:&newVertices
                                                length:sizeof(newVertices)
                                               options:MTLResourceStorageModeShared];

        [renderCommandEncoder setRenderPipelineState:metalRenderPSO];
        [renderCommandEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [renderCommandEncoder setFragmentTexture:texture->texture atIndex:0];
        [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
    }

    void SetRenderCommandEncoder(id<MTLRenderCommandEncoder> commandEncoder) {
        this->renderCommandEncoder = commandEncoder;
    }

private:
    id<MTLDevice> metalDevice;
    id<MTLBuffer> vertexBuffer;
    id<MTLRenderPipelineState> metalRenderPSO;
    id<MTLRenderCommandEncoder> renderCommandEncoder;
    Texture* texture;
};
//endregion

//region: PlatformMetal
static bool Running = false;
std::vector<TriangleMetal*> triangles = std::vector<TriangleMetal*>();
std::vector<SpriteMetal*> sprites = std::vector<SpriteMetal*>();
class PlatformMetal : public Platform {
public:
    void Init() override {
        printf("Hi from Init\n");
    }
    void Run(void (*_func)(void*), void* context) override {
        runFunc = _func;
        runFuncContext = context;
        Running = true;
        while (Running) {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
        }
        printf("Run end");
    }
    void LoadShaders() override {

    }
    GameObject* CreateGameObject() override { return new GameObject(); };
    GameObject* CreateTriangle() override {
        auto gameObject = new TriangleMetal(metalAppDelegate.device, metalAppDelegate.metalView.trianglePSO);
        triangles.push_back(gameObject);
        return gameObject;
    }
    Sprite* CreateSprite(const char* path) override {
        auto texture = new Texture(path, metalAppDelegate.device);
        auto sprite = new SpriteMetal(metalAppDelegate.device, metalAppDelegate.metalView.texturePSO, texture);
        sprites.push_back(sprite);
        return sprite;
    }
    bool IsKeyPressed(KeyCode key) override {
        return false;
    }
    bool IsMousePressed(MouseButton button) override {
        return false;
    }
    bool IsGamepadButtonPressed(GamepadButton button) override {
        return false;
    }
    void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) override {
        return;
    }
    void SetMouseReleasedCallback(void (*func)(MouseButton, void*), void* context) override {
        return;
    }
    void SetGamepadReleasedCallback(void (*func)(GamepadButton, void*), void* context) override {
        return;
    }
    virtual vec3 GetMousePos() override { return {}; }
    void Shutdown() override {
        Running = false;
    }

    MetalAppDelegate* metalAppDelegate;
};

int RealMain(Platform* platform);
static void RealMainMetal(MetalAppDelegate* app, MetalView* view) {
    auto platform = new PlatformMetal();
    platform->metalAppDelegate = app;
    RealMain(platform);
}
//endregion

//region: MetalView
@implementation MetalView
- (instancetype)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        self.device = MTLCreateSystemDefaultDevice();
        self.delegate = self;
        [self setupPipeline];
    }
    return self;
}

- (void)setupPipeline {
    self.commandQueue = [self.device newCommandQueue];

    // Triangle shader
    MTLRenderPipelineDescriptor* triangleDesc = [self loadShaderLibrary:vertexShaderSrc frag:fragmentShaderSrc];
    triangleDesc.colorAttachments[0].pixelFormat = self.colorPixelFormat;
    NSError *error = nil;
    self.trianglePSO = [self.device newRenderPipelineStateWithDescriptor:triangleDesc error:&error];
    if (!self.trianglePSO) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
    }

    // Texture shader
    MTLRenderPipelineDescriptor* textureDesc = [self loadShaderLibrary:textureVertexShaderSrc frag:textureFragmentShaderSrc];
    textureDesc.colorAttachments[0].pixelFormat = self.colorPixelFormat;
    error = nil;
    self.texturePSO = [self.device newRenderPipelineStateWithDescriptor:textureDesc error:&error];
    if (!self.texturePSO) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
    }

    [triangleDesc release];
    [textureDesc release];
}
- (MTLRenderPipelineDescriptor*)loadShaderLibrary:(const char *)vertexSrc frag:(const char *)fragmentSrc {
    // Compile shader
    NSError *error = nil;
    MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
    NSString *shaderSource = [NSString stringWithFormat:@"%s\n%s", vertexSrc, fragmentSrc];
    id<MTLLibrary> library = [self.device newLibraryWithSource:shaderSource options:options error:&error];
    if (!library) {
        NSLog(@"Shader compilation error: %@", error.localizedDescription);
        return nil;
    } else {
        NSLog(@"Compiled library");
        NSLog(@"%@", library.functionNames[0]);
        NSLog(@"%@", library.functionNames[1]);
    }
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];

    // Create pipeline state
    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexFunction = vertexFunction;
    pipelineDesc.fragmentFunction = fragmentFunction;
    pipelineDesc.colorAttachments[0].pixelFormat = self.colorPixelFormat;

    return pipelineDesc;
}

- (void)drawInMTKView:(MTKView *)view {
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

    MTLRenderPassDescriptor *passDescriptor = view.currentRenderPassDescriptor;
    if (!passDescriptor) return;

    [passDescriptor.colorAttachments[0] setTexture: view.currentDrawable.texture];
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.4, 0.4, 0.8, 1.0);
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

    id<MTLRenderCommandEncoder> renderCommandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];

//    for (TriangleMetal* gameObject : triangles) {
//        gameObject->SetRenderCommandEncoder(renderCommandEncoder);
//    }
    for (SpriteMetal* gameObject : sprites) {
        gameObject->SetRenderCommandEncoder(renderCommandEncoder);
    }

    runFunc(runFuncContext);

    [renderCommandEncoder endEncoding];
    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size __attribute__((swift_attr("@UIActor"))) {
    NSLog(@"resized");
}
@end
//endregion

//region: AppDelegate
@implementation MetalAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    NSLog(@"applicationDidFinishLaunching");
    NSRect frame = NSMakeRect(100, 100, 800, 600);
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                                         NSWindowStyleMaskResizable)
                                                backing:NSBackingStoreBuffered
                                                  defer:NO
    ];
    [self.window setTitle:@"Metal Triangle"];
    [self.window makeKeyAndOrderFront:nil];
    [self.window setLevel:NSFloatingWindowLevel];
    [NSApp activateIgnoringOtherApps:YES];

    MetalView *metalView = [[MetalView alloc] initWithFrame:frame];
    self.window.contentView = metalView;
    self.metalView = metalView;
    self.device = metalView.device;

    RealMainMetal(self, self.metalView);
}
@end
//endregion

//region: Main
int main(int argc, const char * argv[]) {
    printf("Hello world\n");
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        MetalAppDelegate *delegate = [[MetalAppDelegate alloc] init];
        [app setDelegate:delegate];
        [app finishLaunching];
        [app run];
    }
    return 0;
}
//endregions
#endif