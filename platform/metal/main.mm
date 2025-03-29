#if defined(PLATFORM_APPLE) and defined(BACKEND_METAL) and not defined(PLATFORM_IOS)
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "../platform.h"

#include <cstdio>

//region Shader Source
static const char *vertexShaderSrc = R"(
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
static const char *fragmentShaderSrc = R"(
#include <metal_stdlib>
using namespace metal;

fragment float4 fragment_main() {
    return float4(1.0, 0, 0.0, 1.0); // Red color
}
)";
//endregion

//region: MetalView/MetalAppDelegate declarations
@interface MetalView : MTKView <MTKViewDelegate>
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLRenderPipelineState> trianglePSO;
@property (nonatomic, assign) void *funcContext;
@property (nonatomic, assign) void (*func)(void *);
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
//endregion

//region: PlatformMetal
static bool Running = false;
std::vector<TriangleMetal*> triangles = std::vector<TriangleMetal*>();
class PlatformMetal : public Platform {
public:
    void Init() override {
        printf("Hi from Init\n");
    }
    void Run(void (*func)(void*), void* context) override {
        metalAppDelegate.metalView.func = func;
        metalAppDelegate.metalView.funcContext = context;
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
        return nullptr;
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

    // Compile shaders
    NSError *error = nil;
    MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
    NSString *shaderSource = [NSString stringWithFormat:@"%s\n%s", vertexShaderSrc, fragmentShaderSrc];
    id<MTLLibrary> library = [self.device newLibraryWithSource:shaderSource options:options error:&error];
    if (!library) {
        NSLog(@"Shader compilation error: %@", error.localizedDescription);
        return;
    } else {
        NSLog(@"Compiled library");
        NSLog(@"%@", library.functionNames[0]);
        NSLog(@"%@", library.functionNames[1]);
    }

    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];

    // Create pipeline state
    MTLRenderPipelineDescriptor *pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexFunction = vertexFunction;
    pipelineDesc.fragmentFunction = fragmentFunction;
    pipelineDesc.colorAttachments[0].pixelFormat = self.colorPixelFormat;

    self.trianglePSO = [self.device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (!self.trianglePSO) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
    }
}

- (void)drawInMTKView:(MTKView *)view {
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

    MTLRenderPassDescriptor *passDescriptor = view.currentRenderPassDescriptor;
    if (!passDescriptor) return;

    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.4, 0.4, 0.8, 1.0); // Black background
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

    id<MTLRenderCommandEncoder> renderCommandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];

    for (TriangleMetal* gameObject : triangles) {
        gameObject->SetRenderCommandEncoder(renderCommandEncoder);
    }

    self.func(self.funcContext);

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