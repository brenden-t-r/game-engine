#if defined(PLATFORM_IOS)
#include <Metal/Metal.h>
#include <UIKit/UIKit.h>
#include <MetalKit/MetalKit.h>

#include "../platform.h"

static void* runFuncContext;
static void (*runFunc)(void *);

struct VertexData {
    simd::float4 position;
    simd::float2 textureCoordinate;
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

//region: Static helper functions
void listFilesInDirectory(NSString *directoryPath, int indent) {
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

static NSData *readPNGImageFromBundle(NSString *imageName) {
    // Get the path to the image in the app bundle
    NSString *imagePath = [[NSBundle mainBundle] pathForResource:imageName ofType:@"png"];

    // Check if the image exists in the bundle
    if (imagePath) {
        // Read the image data from the file path
        NSData *imageData = [NSData dataWithContentsOfFile:imagePath];

        if (imageData) {
            return imageData;
        } else {
            NSLog(@"Failed to read data from image file: %@", imageName);
        }
    } else {
        NSLog(@"Image not found in bundle: %@", imageName);
    }

    return nil;
}

// Static function to load any image as a Metal texture from app bundle
static id<MTLTexture> loadImageAsTextureFromBundle(NSString *imageName, id<MTLDevice> device) {
    // Get the path to the image in the app bundle (including extension)
    NSString *imagePath = [[NSBundle mainBundle] pathForResource:imageName ofType:nil];

    // Check if the image exists in the bundle
    if (imagePath) {
        // Read the image data from the file path
        NSData *imageData = [NSData dataWithContentsOfFile:imagePath];

        if (imageData) {
            // Create a MTKTextureLoader instance
            MTKTextureLoader *textureLoader = [[MTKTextureLoader alloc] initWithDevice:device];

            NSError *error = nil;

            // Load the texture from the image data
            id<MTLTexture> texture = [textureLoader newTextureWithData:imageData options:nil error:&error];

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

//endregion

//region: Interface declarations
@interface MetalViewController : UIViewController<MTKViewDelegate>
@property (nonatomic, strong) MTKView *metalView;
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLRenderPipelineState> trianglePSO;
@property (nonatomic, strong) id<MTLRenderPipelineState> texturePSO;
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
class SpriteMetal : public Sprite {
public:
    ~SpriteMetal() {
        [vertexBuffer release];
    }
    SpriteMetal(
            id <MTLDevice> metalDevice,
            id <MTLRenderPipelineState> metalRenderPSO,
            id <MTLTexture> texture
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
        [renderCommandEncoder setFragmentTexture:texture atIndex:0];
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
    id<MTLTexture> texture;
};
//endregion

//region: PlatformIOS
static bool Running = false;
std::vector<TriangleMetal*> triangles = std::vector<TriangleMetal*>();
std::vector<SpriteMetal*> sprites = std::vector<SpriteMetal*>();
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
    Sprite* CreateSprite(const char* path) override {
        NSString *imageName = @"assets/sprites/background.png";
        id<MTLTexture> texture = loadImageAsTextureFromBundle(imageName, metalAppDelegate.viewController.device);
        if (texture) {
            NSLog(@"Texture loaded successfully!");
        } else {
            NSLog(@"Failed to load texture ☹\uFE0F");
        }
        auto gameObject = new SpriteMetal(
                metalAppDelegate.viewController.device, metalAppDelegate.viewController.texturePSO, texture
        );
        sprites.push_back(gameObject);
        return gameObject;
    }

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
//    self.commandQueue = [self.device newCommandQueue];

    [self setupPipeline];

    // Print all resources in the main bundle
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *resourcePath = [mainBundle resourcePath];
    NSLog(@"Resource path: %@", resourcePath);
    listFilesInDirectory(resourcePath, 0);

//    [self loadImage];
//    [self loadTexture];
}

- (void)loadImage {
    NSString *imageName = @"assets/sprites/background";  // The name of the PNG image (without extension)
    NSData *imageData = readPNGImageFromBundle(imageName);

    if (imageData) {
        NSLog(@"Image data loaded successfully");
        // You can now use the imageData to create a UIImage or for other purposes
        UIImage *image = [UIImage imageWithData:imageData];
    }
}

- (void)loadTexture {
    // Get the device object (assuming it's set up somewhere)
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    // The name of the image (without extension), can be any supported image format (e.g., .png, .jpg, .gif, .tiff)
    NSString *imageName = @"assets/sprites/background.png";

    // Load the texture using the static function
    id<MTLTexture> texture = loadImageAsTextureFromBundle(imageName, device);

    if (texture) {
        NSLog(@"Texture loaded successfully!");
        // Use the texture for rendering or other purposes
    }
}

- (void)setupPipeline {
    self.commandQueue = [self.device newCommandQueue];

    // Triangle shader
    MTLRenderPipelineDescriptor* triangleDesc = [self loadShaderLibrary:vertexShaderSrc frag:fragmentShaderSrc];
    NSError *error = nil;
    self.trianglePSO = [self.device newRenderPipelineStateWithDescriptor:triangleDesc error:&error];
    if (!self.trianglePSO || error != nil) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
    }

    // Texture shader
    MTLRenderPipelineDescriptor* textureDesc = [self loadShaderLibrary:textureVertexShaderSrc frag:textureFragmentShaderSrc];
    error = nil;
    self.texturePSO = [self.device newRenderPipelineStateWithDescriptor:textureDesc error:&error];
    if (!self.texturePSO || error != nil) {
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
    pipelineDesc.colorAttachments[0].pixelFormat = self.metalView.colorPixelFormat;

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

    for (TriangleMetal* gameObject : triangles) {
        gameObject->SetRenderCommandEncoder(renderCommandEncoder);
    }
    for (SpriteMetal* gameObject : sprites) {
        gameObject->SetRenderCommandEncoder(renderCommandEncoder);
    }

    if (runFunc != nil) {
        runFunc(runFuncContext);
    }

    [renderCommandEncoder endEncoding];
    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
    
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
