#if defined(PLATFORM_IOS)
#include <Metal/Metal.h>
#include <UIKit/UIKit.h>
#include <MetalKit/MetalKit.h>

#include "../platform.h"

static void* runFuncContext;
static void (*runFunc)(void *);

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

//region: MetalViewController/UIAppDelegate declarations
@interface MetalViewController : UIViewController<MTKViewDelegate>
@property (nonatomic, strong) MTKView *metalView;
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLRenderPipelineState> trianglePSO;
@property (nonatomic, strong) id<MTLBuffer> vertexBuffer;
@end
@interface MetalAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) MetalViewController *viewController;
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

//region: PlatformIOS
static bool Running = false;
std::vector<TriangleMetal*> triangles = std::vector<TriangleMetal*>();
class PlatformIOS : public Platform {
public:
    void Init() override {}

    void Run(void (*func)(void*), void* context) override {
        printf("Hi from Run\n");
        runFunc = func;
        runFuncContext = context;
        Running = true;
        while (Running) {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
        }
        printf("Run end");
    }

    void LoadShaders() override {}

    GameObject* CreateGameObject() override {
        return new GameObject();
    };
    GameObject* CreateTriangle() override {
        auto gameObject = new TriangleMetal(metalAppDelegate.viewController.device, metalAppDelegate.viewController.trianglePSO);
        triangles.push_back(gameObject);
        return gameObject;
    }
    Sprite* CreateSprite(const char* path) override { return nullptr; }

    bool IsKeyPressed(KeyCode key) override { return false; }
    bool IsMousePressed(MouseButton button) override { return false; }
    bool IsGamepadButtonPressed(GamepadButton button) override { return false; }

    void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) override {}
    void SetMouseReleasedCallback(void (*func)(MouseButton, void*), void* context) override {}
    void SetGamepadReleasedCallback(void (*func)(GamepadButton, void*), void* context) override {}

    vec3 GetMousePos() override { return {}; }
    void Shutdown() override {}

    MetalAppDelegate* metalAppDelegate = nullptr;
};
int RealMain(Platform* platform);
static void RealMainMetal(MetalAppDelegate* app) {
    auto platform = new PlatformIOS();
    platform->metalAppDelegate = app;
    RealMain(platform);
}
//endregion

//region: MetalViewController
@implementation MetalViewController
- (void)viewDidLoad {
    [super viewDidLoad];

    // Set up Metal device
    self.device = MTLCreateSystemDefaultDevice();
    if (!self.device) {
        NSLog(@"Metal is not supported on this device.");
        return;
    }

    // Set up MTKView
    self.metalView = [[MTKView alloc] initWithFrame:self.view.bounds device:self.device];
    self.metalView.delegate = self;
    self.metalView.enableSetNeedsDisplay = NO;
    self.metalView.preferredFramesPerSecond = 60;
    self.metalView.framebufferOnly = NO; // Allow read/write operations
    self.metalView.clearColor = MTLClearColorMake(0.4, 0.4, 0.8, 1.0); // Set initial clear color

    [self.view addSubview:self.metalView];

    // Create command queue
    self.commandQueue = [self.device newCommandQueue];

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

    self.trianglePSO = [self.device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (!self.trianglePSO) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
    }
}

- (void)drawInMTKView:(MTKView *)view {
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

    MTLRenderPassDescriptor *passDescriptor = view.currentRenderPassDescriptor;
    if (!passDescriptor) return;

    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.4, 0.4, 0.8, 1.0);
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;

//    float vertices[] = {
//            -0.5, -0.5, 0.0f, 1.0f,
//            0.5f, -0.5f, 0.0f, 1.0f,
//            0.0f,  0.5f, 0.0f, 1.0f
//    };
//    self.vertexBuffer = [self.metalView.device newBufferWithBytes:vertices length:sizeof(vertices) options:MTLResourceStorageModeShared];

//    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
//    [encoder setRenderPipelineState:self.trianglePSO];
//    [encoder setVertexBuffer:self.vertexBuffer offset:0 atIndex:0];
//    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];


    id<MTLRenderCommandEncoder> renderCommandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];

    for (TriangleMetal* gameObject : triangles) {
        gameObject->SetRenderCommandEncoder(renderCommandEncoder);
    }

    runFunc(runFuncContext);

    [renderCommandEncoder endEncoding];
    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    NSLog(@"resized");
}
@end
//endregion

//region: AppDelegate
@implementation MetalAppDelegate
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    NSLog(@"didFinishLaunchingWithOptions");
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.viewController = [[MetalViewController alloc] init];
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
        RealMainMetal(self);
    });
    return YES;
}
@end
//endregion

//region: Main
int RealMain(Platform* platform);
int main(int argc, char * argv[]) {
    NSLog(@"Hello world");
    @autoreleasepool {
        UIApplicationMain(argc, argv, nil, NSStringFromClass([MetalAppDelegate class]));
    }
    return 0;
}
//endregion

#endif
