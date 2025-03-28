#if defined(PLATFORM_IOS)
#include <Metal/Metal.h>
#include <UIKit/UIKit.h>
#include <MetalKit/MetalKit.h>

#include "../platform.h"

static void* funcContextSt;
static void (*funcSt)(void *);

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

//region: MetalViewController
@interface MetalViewController : UIViewController<MTKViewDelegate>
@property (nonatomic, strong) MTKView *metalView;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic, strong) id<MTLBuffer> vertexBuffer;
@end
@implementation MetalViewController
- (void)viewDidLoad {
    [super viewDidLoad];
    NSLog(@"viewDidLoad");

    self.metalView = [[MTKView alloc] initWithFrame:self.view.bounds device:MTLCreateSystemDefaultDevice()];
    self.metalView.delegate = self;
    self.metalView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.view addSubview:self.metalView];
    [self setupPipeline];
}

- (void)setupPipeline {
    self.commandQueue = [self.metalView.device newCommandQueue];

    // Compile shaders
    NSError *error = nil;
    MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
    NSString *shaderSource = [NSString stringWithFormat:@"%s\n%s", vertexShaderSrc, fragmentShaderSrc];
    id<MTLLibrary> library = [self.metalView.device newLibraryWithSource:shaderSource options:options error:&error];
    if (!library) {
        NSLog(@"Shader compilation error: %@", error.localizedDescription);
        return;
    }

    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];

    // Create pipeline state
    MTLRenderPipelineDescriptor *pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexFunction = vertexFunction;
    pipelineDesc.fragmentFunction = fragmentFunction;
    pipelineDesc.colorAttachments[0].pixelFormat = self.metalView.colorPixelFormat;

    self.pipelineState = [self.metalView.device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (!self.pipelineState) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
    }
}

- (void)drawInMTKView:(MTKView *)view {
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

    MTLRenderPassDescriptor *passDescriptor = view.currentRenderPassDescriptor;
    if (!passDescriptor) return;

    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.4, 0.4, 0.8, 1.0);
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;

    static const float vertices[] = {
            -0.5, -0.5, 0.0f, 1.0f,
            0.5f, -0.5f, 0.0f, 1.0f,
            0.0f,  0.5f, 0.0f, 1.0f
    };
    self.vertexBuffer = [self.metalView.device newBufferWithBytes:vertices length:sizeof(vertices) options:MTLResourceStorageModeShared];

    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    [encoder setRenderPipelineState:self.pipelineState];
    [encoder setVertexBuffer:self.vertexBuffer offset:0 atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    [encoder endEncoding];

    funcSt(funcContextSt);

    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    NSLog(@"resized");
}
@end
//endregion

//region: AppDelegate
static UIResponder <UIApplicationDelegate>* appDelegate = nullptr;
@interface MetalAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) MetalViewController *viewController;
@end
@implementation MetalAppDelegate
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    NSLog(@"didFinishLaunchingWithOptions");
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.viewController = [[MetalViewController alloc] init];
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];
    appDelegate = self;
    return YES;
}
@end
//endregion

//region: PlatformIOS
class PlatformIOS : public Platform {
public:
    void Init() override {}

    void Run(void (*func)(void*), void* context) override {
        printf("Hi from Run\n");
        funcSt = func;
        funcContextSt = context;
        int argc = 0;
        char **argv = nullptr;
        UIApplicationMain(argc, argv, nil, NSStringFromClass([MetalAppDelegate class]));
    }

    void LoadShaders() override {}

    GameObject* CreateGameObject() override {
        NSLog(@" %f", appDelegate.window.screen.bounds.size.width);
        auto width = appDelegate.window.screen.bounds.size.width;
        return new GameObject();
    };
    GameObject* CreateTriangle() override { return nullptr; }
    Sprite* CreateSprite(const char* path) override { return nullptr; }

    bool IsKeyPressed(KeyCode key) override { return false; }
    bool IsMousePressed(MouseButton button) override { return false; }
    bool IsGamepadButtonPressed(GamepadButton button) override { return false; }

    void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) override {}
    void SetMouseReleasedCallback(void (*func)(MouseButton, void*), void* context) override {}
    void SetGamepadReleasedCallback(void (*func)(GamepadButton, void*), void* context) override {}

    vec3 GetMousePos() override { return {}; }
    void Shutdown() override {}
};
//endregion

//region: Main
int RealMain(Platform* platform);
int main(int argc, char * argv[]) {
    @autoreleasepool {
        NSLog(@"Hello world");
        auto *platform = new PlatformIOS();
        RealMain(platform);
        return 0;
    }
}
//endregion

#endif
