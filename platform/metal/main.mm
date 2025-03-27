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

//region: MetalView
@interface MetalView : MTKView <MTKViewDelegate>
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic, strong) id<MTLBuffer> vertexBuffer;
@property (nonatomic, assign) void *funcContext;
@property (nonatomic, assign) void (*func)(void *);
@end
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

    self.pipelineState = [self.device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (!self.pipelineState) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
    }
}

- (void)drawInMTKView:(MTKView *)view {
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

    MTLRenderPassDescriptor *passDescriptor = view.currentRenderPassDescriptor;
    if (!passDescriptor) return;

    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.4, 0.4, 0.8, 1.0); // Black background
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;

    static const float vertices[] = {
            -0.5, -0.5, 0.0f, 1.0f,  // Top vertex
            0.5f, -0.5f, 0.0f, 1.0f,  // Bottom left vertex
            0.0f, 0.5f, 0.0f, 1.0f   // Bottom right vertex
    };
    self.vertexBuffer = [self.device newBufferWithBytes:&vertices length:sizeof(vertices) options:MTLResourceStorageModeShared];

    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    [encoder setRenderPipelineState:self.pipelineState];
    [encoder setVertexBuffer:self.vertexBuffer offset:0 atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    [encoder endEncoding];

    self.func(self.funcContext);

    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size __attribute__((swift_attr("@UIActor"))) {
    NSLog(@"resized");
}
@end
//endregion

//region: AppDelegate
@interface MetalAppDelegate : NSObject <NSApplicationDelegate>
@property (strong, nonatomic) NSWindow *window;
@property (nonatomic, assign) void *funcContext;
@property (nonatomic, assign) void (*func)(void *);
@end
@implementation MetalAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    NSRect frame = NSMakeRect(100, 100, 800, 600);
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                                         NSWindowStyleMaskResizable)
                                                backing:NSBackingStoreBuffered
                                                  defer:NO
    ];
    [self.window setTitle:@"Metal Triangle"];

    MetalView *metalView = [[MetalView alloc] initWithFrame:frame];
    self.window.contentView = metalView;
    metalView = metalView;
    metalView.func = self.func;
    metalView.funcContext = self.funcContext;

    [self.window makeKeyAndOrderFront:nil];
}
@end

class PlatformMetal : public Platform {
public:
    void Init() override {
        printf("Hi from Init\n");
    }
    void Run(void (*func)(void*), void* context) override {
        printf("Hi from Run\n");
        metalAppDelegate.func = func;
        metalAppDelegate.funcContext = context;
        [app run];
    }
    void LoadShaders() override {

    }
    GameObject* CreateGameObject() override { return new GameObject(); };
    GameObject* CreateTriangle() override {
            return nullptr;
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
    void Shutdown() override {}

    NSApplication* app;
    MetalAppDelegate* metalAppDelegate;
};
//endregion

//region: Main
int RealMain(Platform* platform);
int main(int argc, const char * argv[]) {
    printf("Hello world\n");
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        MetalAppDelegate *delegate = [[MetalAppDelegate alloc] init];
        [app setDelegate:delegate];
        auto platform = new PlatformMetal();
        platform->app = app;
        platform->metalAppDelegate = delegate;
        RealMain(platform);
    }
    return 0;
}
//endregions