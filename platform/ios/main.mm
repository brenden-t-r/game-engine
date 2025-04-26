#if defined(PLATFORM_IOS)
#include <Metal/Metal.h>
#include <UIKit/UIKit.h>
#include <MetalKit/MetalKit.h>
#include <AVFoundation/AVFoundation.h>
#include <GameController/GameController.h>

#include "../platform.h"

static void* runFuncContext;
static void (*runFunc)(void *);
static GCControllerButtonInput* GetAppleGamepadButton(GCExtendedGamepad *pad, GamepadButton btn) {
    switch (btn) {
        case GamepadButton::North: return pad.buttonY;
        case GamepadButton::South: return pad.buttonA;
        case GamepadButton::East: return pad.buttonB;
        case GamepadButton::West: return pad.buttonX;
        case GamepadButton::RB: return pad.rightShoulder;
        case GamepadButton::LB: return pad.leftShoulder;
        case GamepadButton::R3: return pad.rightThumbstickButton;
        case GamepadButton::L3: return pad.leftThumbstickButton;
        case GamepadButton::Start: return pad.buttonHome;
        case GamepadButton::Select: return pad.buttonMenu;
        case GamepadButton::DLeft: return pad.dpad.left;
        case GamepadButton::DRight: return pad.dpad.right;
        case GamepadButton::DUp: return pad.dpad.up;
        case GamepadButton::DDown: return pad.dpad.down;
        default: return nullptr;
    }
}
static GamepadButton GetGamepadButton(GCExtendedGamepad *pad, GCControllerElement* btn) {
    if (btn == pad.buttonY) return GamepadButton::North;
    if (btn == pad.buttonA) return GamepadButton::South;
    if (btn == pad.buttonB) return GamepadButton::East;
    if (btn == pad.buttonX) return GamepadButton::West;
    if (btn == pad.rightShoulder) return GamepadButton::RB;
    if (btn == pad.leftShoulder) return GamepadButton::LB;
    if (btn == pad.rightThumbstickButton) return GamepadButton::R3;
    if (btn == pad.leftThumbstickButton) return GamepadButton::L3;
    if (btn == pad.buttonHome) return GamepadButton::Start;
    if (btn == pad.buttonMenu) return GamepadButton::Select;
    else return GamepadButton::Unknown;
}
void static(*mouseUpCallback)(MouseButton, void*);
static void* mouseCallbackContext;
void static(*gamepadUpCallback)(GamepadButton, void*);
static void* gamepadCallbackContext;

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
static MTLRenderPipelineDescriptor* loadShaderLibrary(id <MTLDevice> device, const char *vertexSrc, const char *fragmentSrc) {
    // Compile shader
    NSError *error = nil;
    MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
    NSString *shaderSource = [NSString stringWithFormat:@"%s\n%s", vertexSrc, fragmentSrc];
    id <MTLLibrary> library = [device newLibraryWithSource:shaderSource options:options error:&error];
    if (!library) {
        NSLog(@"Shader compilation error: %@", error.localizedDescription);
        return nil;
    } else {
        NSLog(@"Compiled library");
        NSLog(@"%@", library.functionNames[0]);
        NSLog(@"%@", library.functionNames[1]);
    }
    id <MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
    id <MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];

    // Create pipeline state
    MTLRenderPipelineDescriptor *pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexFunction = vertexFunction;
    pipelineDesc.fragmentFunction = fragmentFunction;
    return pipelineDesc;
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
@property (nonatomic, strong) NSMutableSet *activeTouches;  // To store active touches

@end
@interface MetalAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) MetalViewController *viewController;
@end
//endregion

AVAudioEngine *engine = nullptr;

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
        [texture release];
    }
    SpriteMetal(
            id <MTLDevice> metalDevice,
            id <MTLRenderPipelineState> metalRenderPSO,
            id <MTLTexture> texture
    ){
        this->metalDevice = metalDevice;
        this->metalRenderPSO = metalRenderPSO;
        this->texture = texture;
        VertexData newVertices[]{
                {{vertex1.x, vertex1.y, 0, 1}, {0.0f, 0.0f}}, // Top left
                {{vertex4.x, vertex4.y, 0, 1}, {0.0f, 1.0f}}, // Bottom left
                {{vertex3.x, vertex3.y, 0, 1}, {1.0f, 1.0f}}, // Bottom right
                {{vertex1.x, vertex1.y, 0, 1}, {0.0f, 0.0f}}, // Top left
                {{vertex3.x, vertex3.y, 0, 1}, {1.0f, 1.0f}}, // Bottom right
                {{vertex2.x, vertex2.y, 0, 1}, {1.0f, 0.0f}}  // Top right
        };
        vertexBuffer = [metalDevice newBufferWithBytes:&newVertices
                                                length:sizeof(newVertices)
                                               options:MTLResourceStorageModeShared];
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

        if (useAtlas) {
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
            [vertexBuffer release];
            vertexBuffer = [metalDevice newBufferWithBytes:&newVertices
                                                length:sizeof(newVertices)
                                               options:MTLResourceStorageModeShared];
        } else {
            memcpy([vertexBuffer contents], newVertices, sizeof(newVertices));
        }

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
class SoundMetalAVAudioPlayer : public Sound {
public:
    ~SoundMetalAVAudioPlayer() {
        [player release];
    }
    SoundMetalAVAudioPlayer(const char* filePath) {
        NSString *file = [NSString stringWithUTF8String:filePath];
        NSString *path = [[NSBundle mainBundle] pathForResource:file ofType:nil];
        if (path) {
            NSURL *fileURL = [NSURL fileURLWithPath:path];
            NSError *error = nil;
            player = [[AVAudioPlayer alloc] initWithContentsOfURL:fileURL error:&error];
            if (error) {
                NSLog(@"Error initializing player: %@", error.localizedDescription);
            }
        } else {
            NSLog(@"File not found in bundle");
        }
        [player prepareToPlay];
    }

    void Play() override {
        if (player) {
            [player play];
        }
    }
    void Stop() override {
        if (player) {
            [player stop];
        }
    }
    void Reset() override {
        if (player) {
            player.currentTime = 0;
        }
    }
    AVAudioPlayer* player = nullptr;
};
class SoundMetalAVAudioBuffered : public Sound {
public:
    ~SoundMetalAVAudioBuffered() {
        [engine detachNode:player];
        [player release];
    }
    SoundMetalAVAudioBuffered(const char* filePath) {
        if (engine == nullptr) {
            engine = [[AVAudioEngine alloc] init];
        }

        // Load audio file
        NSError *error = nil;
        NSString *file = [NSString stringWithUTF8String:filePath];
        NSString *path = [[NSBundle mainBundle] pathForResource:file ofType:nil];
        NSURL *fileURL = [NSURL fileURLWithPath:path];
        NSLog(@"%s",filePath);
        audioFile = [[AVAudioFile alloc] initForReading:fileURL error:&error];
        if (error || !audioFile) {
            NSLog(@"Failed to load audio file: %@", error.localizedDescription);
        }

        // Buffer
        buffer = [[AVAudioPCMBuffer alloc]
                  initWithPCMFormat:audioFile.processingFormat
                  frameCapacity:(AVAudioFrameCount)audioFile.length];
        [audioFile readIntoBuffer:buffer error:&error];
        if (error) {
            NSLog(@"Failed to read audio buffer: %@", error.localizedDescription);
            return;
        }

        // Create player
        player = [[AVAudioPlayerNode alloc] init];
        [engine attachNode:player];

        AVAudioFormat *format = [engine.mainMixerNode outputFormatForBus:0];
        [engine connect:player to:engine.mainMixerNode format:format];

        [engine prepare];
        [engine startAndReturnError:&error];
        if (error) {
            NSLog(@"Failed to start audio engine: %@", error.localizedDescription);
        }
    }
    void Play() {
        if (!player.isPlaying) {
            [player play];
        }
        [player scheduleBuffer:buffer atTime:nil options:AVAudioPlayerNodeBufferInterrupts completionHandler:nil];
    }

    void Stop() {
        [player stop];
    }

    void Reset() {}

    AVAudioPlayerNode *player;
    AVAudioPCMBuffer *buffer;
    AVAudioFile *audioFile;
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
            sleep(1);
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
        NSString *imageName = [NSString stringWithUTF8String:path];
        id<MTLTexture> texture = loadImageAsTextureFromBundle(imageName, metalAppDelegate.viewController.device);
        assert(texture != nullptr);
        auto gameObject = new SpriteMetal(
                metalAppDelegate.viewController.device, metalAppDelegate.viewController.texturePSO, texture
        );
        sprites.push_back(gameObject);
        return gameObject;
    }
    Sound* CreateSound(const char* path) override {
        return new SoundMetalAVAudioBuffered(path);
    }
    void Delete(GameObject* object) override {
        for (auto it = triangles.begin(); it != triangles.end(); ++it) {
            if (*it == object) {
                triangles.erase(it);
                delete object;
                return;
            }
        }
        for (auto it = sprites.begin(); it != sprites.end(); ++it) {
            if (*it == object) {
                sprites.erase(it);
                delete object;
                return;
            }
        }
        delete object;
    }

    bool IsKeyPressed(KeyCode key) override { return false; }
    bool IsMousePressed(MouseButton button) override {
        if (button == MouseButton::Left) {
            return metalAppDelegate.viewController.activeTouches.count > 0;
        } else return false;
    }
    bool IsGamepadButtonPressed(GamepadButton button) override { return [metalAppDelegate.viewController IsGamePadPressed: button]; }
    void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) override {}
    void SetMouseReleasedCallback(void (*func)(MouseButton, void*), void* context) override {
        mouseUpCallback = func;
        mouseCallbackContext = context;
    }
    void SetGamepadReleasedCallback(void (*func)(GamepadButton, void*), void* context) override {
        gamepadUpCallback = func;
        gamepadCallbackContext = context;
    }
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
    self.metalView.clearColor = MTLClearColorMake(0.4, 0.4, 0.8, 1.0);
    self.metalView.frame = self.view.bounds;
    self.metalView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.view addSubview:self.metalView];
    [self setupPipeline];

    // Print all resources in the main bundle
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *resourcePath = [mainBundle resourcePath];
    NSLog(@"Resource path: %@", resourcePath);
    listFilesInDirectory(resourcePath, 0);

    // Input gesture setup
    self.activeTouches = [NSMutableSet set];
    UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTap:)];
    [tap setCancelsTouchesInView:false];
    [self.view addGestureRecognizer:tap];
    UITapGestureRecognizer *twoFingerTap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTwoFingerTap:)];
    twoFingerTap.numberOfTouchesRequired = 2;
    [twoFingerTap setCancelsTouchesInView:false];
    [self.view addGestureRecognizer:twoFingerTap];
    UILongPressGestureRecognizer *longPress = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleLongPress:)];
    [longPress setCancelsTouchesInView:false];
    [self.view addGestureRecognizer:longPress];
}
- (void)setupPipeline {
    self.commandQueue = [self.device newCommandQueue];

    // Triangle shader
    MTLRenderPipelineDescriptor* triangleDesc = loadShaderLibrary(self.device, vertexShaderSrc, fragmentShaderSrc);
    triangleDesc.colorAttachments[0].pixelFormat = self.metalView.colorPixelFormat;
    NSError *error = nil;
    self.trianglePSO = [self.device newRenderPipelineStateWithDescriptor:triangleDesc error:&error];
    if (!self.trianglePSO || error != nil) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
    }

    // Texture shader
    MTLRenderPipelineDescriptor* textureDesc = loadShaderLibrary(self.device, textureVertexShaderSrc, textureFragmentShaderSrc);
    MTLRenderPipelineColorAttachmentDescriptor *attachment = textureDesc.colorAttachments[0];
    attachment.pixelFormat = MTLPixelFormatBGRA8Unorm;
    attachment.blendingEnabled = YES;
    attachment.rgbBlendOperation = MTLBlendOperationAdd;
    attachment.alphaBlendOperation = MTLBlendOperationAdd;
    attachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    attachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    attachment.sourceAlphaBlendFactor = MTLBlendFactorOne;
    attachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    error = nil;
    self.texturePSO = [self.device newRenderPipelineStateWithDescriptor:textureDesc error:&error];
    if (!self.texturePSO || error != nil) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
    }

    [triangleDesc release];
    [textureDesc release];

    // Gamepad
    [GCController startWirelessControllerDiscoveryWithCompletionHandler:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(controllerConnected:)
                                                 name:GCControllerDidConnectNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(controllerDisconnected:)
                                                 name:GCControllerDidDisconnectNotification
                                               object:nil];
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
    NSLog(@"resized: %f, %f", size.width, size.height);
}
- (void)viewWillLayoutSubviews {
    [super viewWillLayoutSubviews];
    self.view.frame = UIScreen.mainScreen.bounds;
}
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
}
- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
#ifdef FORCE_PORTRAIT
    return UIInterfaceOrientationMaskPortrait;
#elif defined(FORCE_LANDSCAPE)
    return UIInterfaceOrientationMaskLandscape;
#else
    return UIInterfaceOrientationMaskAll;
#endif

}
- (void)handleTap:(UITapGestureRecognizer *)gesture {
//    CGPoint location = [gesture locationInView:self.view];
//    NSLog(@"Tap at: (%f, %f)", location.x, location.y);
    if (mouseUpCallback != nil) {
        mouseUpCallback(MouseButton::Left, mouseCallbackContext);
    }
}
- (void)handleTwoFingerTap:(UITapGestureRecognizer *)gesture {
    if (gesture.numberOfTouches == 2) {
//        CGPoint touch1 = [gesture locationOfTouch:0 inView:self.view];
//        CGPoint touch2 = [gesture locationOfTouch:1 inView:self.view];
//        CGPoint midpoint = CGPointMake((touch1.x + touch2.x) / 2, (touch1.y + touch2.y) / 2);
//        NSLog(@"Two-finger tap midpoint: (%f, %f)", midpoint.x, midpoint.y);
        if (mouseUpCallback != nil) {
            mouseUpCallback(MouseButton::Right, mouseCallbackContext);
        }
    }
}
- (void)handleLongPress:(UILongPressGestureRecognizer *)gesture {
    if (gesture.state == UIGestureRecognizerStateBegan) {
//        CGPoint point = [gesture locationInView:self.view];
//        NSLog(@"Long press at: (%f, %f)", point.x, point.y);
    }
}
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        [self.activeTouches addObject:touch];
//        NSLog(@"Touch began at: %@", NSStringFromCGPoint([touch locationInView:self.view]));
    }
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        if ([self.activeTouches containsObject:touch]) {
//            NSLog(@"Touch moved at: %@", NSStringFromCGPoint([touch locationInView:self.view]));
        }
    }
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        [self.activeTouches removeObject:touch];
//        NSLog(@"Touch ended at: %@", NSStringFromCGPoint([touch locationInView:self.view]));
    }
}
- (void)controllerConnected:(NSNotification *)notification {
    GCController *controller = notification.object;
    NSLog(@"Controller connected: %@", controller.vendorName);
    if (controller.extendedGamepad) {
        GCExtendedGamepad* gamepad = controller.extendedGamepad;
        gamepad.valueChangedHandler = ^(GCExtendedGamepad* pad, GCControllerElement* element) {
            for (int i = 0; i < pad.allButtons.count; i++) {
                GCControllerButtonInput *button = pad.allButtons.allObjects[i];
                if (element == button && !button.isPressed) {
                    auto btn = GetGamepadButton(pad, element);
                    if (btn != GamepadButton::Unknown && gamepadUpCallback != nullptr) {
                        gamepadUpCallback(btn, gamepadCallbackContext);
                    }
                }
            }
        };
        __block BOOL wasUpPressed = NO;
        __block BOOL wasDownPressed = NO;
        __block BOOL wasLeftPressed = NO;
        __block BOOL wasRightPressed = NO;
        gamepad.dpad.valueChangedHandler = ^(GCControllerDirectionPad *dpad, float xValue, float yValue) {
            BOOL isUpPressed = dpad.up.isPressed;
            BOOL isDownPressed = dpad.down.isPressed;
            BOOL isLeftPressed = dpad.left.isPressed;
            BOOL isRightPressed = dpad.right.isPressed;
            if (gamepadUpCallback != nullptr) {
                if (wasUpPressed && !isUpPressed) gamepadUpCallback(GamepadButton::DUp, gamepadCallbackContext);
                if (wasDownPressed && !isDownPressed) gamepadUpCallback(GamepadButton::DDown, gamepadCallbackContext);
                if (wasLeftPressed && !isLeftPressed) gamepadUpCallback(GamepadButton::DLeft, gamepadCallbackContext);
                if (wasRightPressed && !isRightPressed) gamepadUpCallback(GamepadButton::DRight, gamepadCallbackContext);
            }
            wasUpPressed = isUpPressed;
            wasDownPressed = isDownPressed;
            wasLeftPressed = isLeftPressed;
            wasRightPressed = isRightPressed;
        };
    }
}
- (void)controllerDisconnected:(NSNotification *)notification {
    GCController *controller = notification.object;
    NSLog(@"Controller disconnected: %@", controller.vendorName);
}
- (BOOL)IsGamePadPressed:(GamepadButton)button {
    GCController *controller = [GCController controllers].firstObject;
    if (controller.extendedGamepad) {
        GCExtendedGamepad* gamepad = controller.extendedGamepad;
        auto state = GetAppleGamepadButton(gamepad, button);
        if (state != nullptr) {
            return state.isPressed;
        } else return false;
    }
    return false;
}
#if 0
//    self.metalView.insetsLayoutMarginsFromSafeArea = NO;
//    self.edgesForExtendedLayout = UIRectEdgeAll;
//    self.modalPresentationStyle = UIModalPresentationFullScreen;
//- (UIRectEdge)preferredScreenEdgesDeferringSystemGestures {
//    return UIRectEdgeAll;
//}
[[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(handleOrientationChange:)
name:UIDeviceOrientationDidChangeNotification
        object:nil];
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    [self updateMetalViewForCurrentOrientation];
}
- (void)handleOrientationChange:(NSNotification *)notification {
    [self updateMetalViewForCurrentOrientation];
}
#endif
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
