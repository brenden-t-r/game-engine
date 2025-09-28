#if defined(PLATFORM_APPLE) and defined(BACKEND_METAL) and not defined(PLATFORM_IOS)
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <Carbon/Carbon.h>
#import <GameController/GameController.h>
#include "stb_image.h"

#include "../platform.h"

#include <cstdio>

#define MINIAUDIO_IMPLEMENTATION
#include "../../dependencies/miniaudio.h"
static ma_engine g_engine;

static void *runFuncContext;
static void (*runFunc)(void *);
static bool Running = false;

//region Input Helpers / Callbacks
static int GetAppleKey(KeyCode key) {
    switch (key) {
        case KeyCode::Up:       return kVK_UpArrow;
        case KeyCode::Down:     return kVK_DownArrow;
        case KeyCode::Left:     return kVK_LeftArrow;
        case KeyCode::Right:    return kVK_RightArrow;
        case KeyCode::W:        return kVK_ANSI_W;
        case KeyCode::A:        return kVK_ANSI_A;
        case KeyCode::S:        return kVK_ANSI_S;
        case KeyCode::D:        return kVK_ANSI_D;
        default:
            return -1;
    }
}
static KeyCode GetKeyCode(int appleKey) {
    switch (appleKey) {
        case kVK_UpArrow:       return KeyCode::Up;
        case kVK_DownArrow:     return KeyCode::Down;
        case kVK_LeftArrow:     return KeyCode::Left;
        case kVK_RightArrow:    return KeyCode::Right;
        case kVK_ANSI_W:        return KeyCode::W;
        case kVK_ANSI_A:        return KeyCode::A;
        case kVK_ANSI_S:        return KeyCode::S;
        case kVK_ANSI_D:        return KeyCode::D;
        default:                return KeyCode::Unknown;
    }
}
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
void static(*keyUpCallback)(KeyCode, void*);
static void* keyCallbackContext;
void static(*mouseUpCallback)(MouseButton, void*, vec3);
static void* mouseCallbackContext;
void static(*gamepadUpCallback)(GamepadButton, void*);
static void* gamepadCallbackContext;
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
            NSDictionary *options = @{
                    MTKTextureLoaderOptionSRGB : @NO // Needed this to fix "dark" sprites. May need to revisit.
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

//region: MetalView/MetalAppDelegate declarations
@interface MetalView : MTKView <MTKViewDelegate>
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLRenderPipelineState> trianglePSO;
@property (nonatomic, strong) id<MTLRenderPipelineState> texturePSO;
@property (strong) NSTrackingArea *trackingArea;
- (BOOL) IsMousePressed:(MouseButton) button;
- (BOOL) IsKeyPressed:(KeyCode) key;
- (BOOL)IsGamePadPressed:(GamepadButton)button;
@end
@interface MetalAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
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
        static const float metal_vertices[] = {
                vertices[0].x, vertices[0].y, 0.0f, 1.0f,  // Top vertex
                vertices[1].x, vertices[1].y, 0.0f, 1.0f,  // Bottom left vertex
                vertices[2].x, vertices[2].y, 0.0f, 1.0f   // Bottom right vertex
        };
        vertexBuffer = [metalDevice newBufferWithBytes:&metal_vertices length:sizeof(metal_vertices) options:MTLResourceStorageModeShared];
    }

    void Update() override {
        Triangle::Update();
        float metal_vertices[] = {
                vertices[0].x, vertices[0].y, 0.0f, 1.0f,  // Top vertex
                vertices[1].x, vertices[1].y, 0.0f, 1.0f,  // Bottom left vertex
                vertices[2].x, vertices[2].y, 0.0f, 1.0f   // Bottom right vertex
        };
        memcpy([vertexBuffer contents], metal_vertices, sizeof(metal_vertices));
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

class TextureMTL : public Texture{
public:
    explicit TextureMTL(id<MTLTexture> texture): texture(texture) {}
    ~TextureMTL() override {
        [texture release];
    }
    id<MTLTexture> texture;
};
class SpriteMetal : public Sprite {
public:
    ~SpriteMetal() {
        [vertexBuffer release];
    }
    SpriteMetal(
            id <MTLDevice> metalDevice,
            id <MTLRenderPipelineState> metalRenderPSO,
            TextureMTL* texture
    ){
        this->metalDevice = metalDevice;
        this->metalRenderPSO = metalRenderPSO;
        this->texture = texture;
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
        }

        vertexBuffer = [metalDevice newBufferWithBytes:&newVertices
                                                length:sizeof(newVertices)
                                               options:MTLResourceStorageModeShared];

        [renderCommandEncoder setRenderPipelineState:metalRenderPSO];
        [renderCommandEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [renderCommandEncoder setFragmentTexture:texture->texture atIndex:0];
        [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
    }

    Texture* GetTexture() override {
        return texture;
    }

    void SetRenderCommandEncoder(id<MTLRenderCommandEncoder> commandEncoder) {
        this->renderCommandEncoder = commandEncoder;
    }

private:
    id<MTLDevice> metalDevice;
    id<MTLBuffer> vertexBuffer;
    id<MTLRenderPipelineState> metalRenderPSO;
    id<MTLRenderCommandEncoder> renderCommandEncoder;
    TextureMTL* texture = nullptr;
};
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

//region: PlatformMetal
std::vector<TriangleMetal*> triangles = std::vector<TriangleMetal*>();
std::vector<SpriteMetal*> sprites = std::vector<SpriteMetal*>();
class PlatformMetal : public Platform {
public:
    void Init() override {
        printf("Hi from Init\n");

        // Init miniaudio
        ma_result result;
        result = ma_engine_init(nullptr, &g_engine);
        if (result != MA_SUCCESS) {
            printf("Failed to initialize audio engine.");
        }
        assert(result == MA_SUCCESS);
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
    TextureMTL* CreateTexture(const char* path) override {
        NSString *imageName = [NSString stringWithUTF8String:path];
        id<MTLTexture> texture = loadImageAsTextureFromBundle(imageName, metalAppDelegate.metalView.device);
        assert(texture != nullptr);
        return new TextureMTL(texture);
    }
    Sprite* CreateSprite(Texture* texture) override {
        auto gameObject = new SpriteMetal(
                metalAppDelegate.metalView.device, metalAppDelegate.metalView.texturePSO, (TextureMTL*)texture
        );
        sprites.push_back(gameObject);
        return gameObject;
    }
    Sprite* CreateSprite(const char* path) override {
        TextureMTL* texture = CreateTexture(path);
        return CreateSprite(texture);
    }
    Sound* CreateSound(const char* path) override {
        auto sound = new SoundMA();
        sound->Init(path);
        return sound;
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
    bool IsKeyPressed(KeyCode key) override {
        return [metalAppDelegate.metalView IsKeyPressed: key];
    }
    bool IsMousePressed(MouseButton button) override {
        return [metalAppDelegate.metalView IsMousePressed: button];
    }
    bool IsGamepadButtonPressed(GamepadButton button) override {
        return [metalAppDelegate.metalView IsGamePadPressed: button];
    }
    void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) override {
        keyUpCallback = func;
        keyCallbackContext = context;
    }
    void SetMouseReleasedCallback(void (*func)(MouseButton, void*, vec3), void* context) override {
        mouseUpCallback = func;
        mouseCallbackContext = context;
    }
    void SetGamepadReleasedCallback(void (*func)(GamepadButton, void*), void* context) override {
        gamepadUpCallback = func;
        gamepadCallbackContext = context;
    }
    virtual vec3 GetMousePos() override {
        NSPoint mouseLocationScreen = [NSEvent mouseLocation];
        NSPoint mouseLocationWindow = [metalAppDelegate.window convertPointFromScreen:mouseLocationScreen];
        NSPoint mouseLocationView = [metalAppDelegate.metalView convertPoint:mouseLocationWindow fromView:nil];
        NSRect viewBounds = [metalAppDelegate.metalView bounds];
        float ndcX = (mouseLocationView.x / viewBounds.size.width) * 2.0f - 1.0f;
        float ndcY = (mouseLocationView.y / viewBounds.size.height) * 2.0f - 1.0f;
        return {ndcX, ndcY, 1};
    }
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
        self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
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
    if (!self.texturePSO) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
    }

    [triangleDesc release];
    [textureDesc release];

    // Print all resources in the main bundle
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *resourcePath = [mainBundle resourcePath];
    NSLog(@"Resource path: %@", resourcePath);
    listFilesInDirectory(resourcePath, 0);

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
    if (!Running) return;

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

    runFunc(runFuncContext);

    [renderCommandEncoder endEncoding];
    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
}
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size __attribute__((swift_attr("@UIActor"))) {
    NSLog(@"resized");
}
- (BOOL)acceptsFirstResponder {
    return YES;
}
- (BOOL)IsMousePressed:(MouseButton)button {
    NSUInteger pressed = [NSEvent pressedMouseButtons];
    switch (button) {
        case MouseButton::Unknown:
            return false;
        case MouseButton::Left:
            return pressed & (1 << 0);
        case MouseButton::Middle:
            return pressed & (1 << 1);
        case MouseButton::Right:
            return pressed & (1 << 2);
    }
}
- (BOOL)IsKeyPressed:(KeyCode) key {
    auto appleKey = GetAppleKey(key);
    return CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, appleKey);
}
- (void)keyDown:(NSEvent *)event {
    // Just implement to avoid "beep" sound
}
- (void)keyUp:(NSEvent *)event {
    auto key = GetKeyCode(event.keyCode);
    if (keyUpCallback != nullptr) {
        keyUpCallback(key, keyCallbackContext);
    }
}
- (void)mouseUp:(NSEvent *)event {
    NSPoint locInWindow = [event locationInWindow];
    NSPoint locInView = [self convertPoint:locInWindow fromView:nil];
    CGFloat width  = self.bounds.size.width;
    CGFloat height = self.bounds.size.height;
    float xNDC = (2.0f * locInView.x / width) - 1.0f;
    float yNDC = (2.0f * locInView.y / height) - 1.0f;

    if (mouseUpCallback != nil) {
        mouseUpCallback(MouseButton::Left, mouseCallbackContext, vec3{xNDC, yNDC, 0});
    }
}
- (void)rightMouseUp:(NSEvent *)event {
    NSPoint locInWindow = [event locationInWindow];
    NSPoint locInView = [self convertPoint:locInWindow fromView:nil];
    CGFloat width  = self.bounds.size.width;
    CGFloat height = self.bounds.size.height;
    float xNDC = (2.0f * locInView.x / width) - 1.0f;
    float yNDC = (2.0f * locInView.y / height) - 1.0f;

    if (mouseUpCallback != nil) {
        mouseUpCallback(MouseButton::Right, mouseCallbackContext, vec3{xNDC, yNDC, 0});
    }
}
- (void)otherMouseUp:(NSEvent *)event {
    NSPoint locInWindow = [event locationInWindow];
    NSPoint locInView = [self convertPoint:locInWindow fromView:nil];
    CGFloat width  = self.bounds.size.width;
    CGFloat height = self.bounds.size.height;
    float xNDC = (2.0f * locInView.x / width) - 1.0f;
    float yNDC = (2.0f * locInView.y / height) - 1.0f;

    if (event.buttonNumber == 2) {
        if (mouseUpCallback != nil) {
            mouseUpCallback(MouseButton::Middle, mouseCallbackContext, vec3{xNDC, yNDC, 0});
        }
    }
}
- (void)mouseMoved:(NSEvent *)event {
    NSPoint point = [self convertPoint:event.locationInWindow fromView:nil];
}
- (void)updateTrackingAreas {
    [super updateTrackingAreas];

    if (self.trackingArea) {
        [self removeTrackingArea:self.trackingArea];
    }

    self.trackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                     options:(NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect)
                                                       owner:self
                                                    userInfo:nil];
    [self addTrackingArea:self.trackingArea];
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
@end
//endregion

//region: AppDelegate
@implementation MetalAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    NSLog(@"applicationDidFinishLaunching");
    NSRect frame = NSMakeRect(100, 100, WINDOW_WIDTH, WINDOW_HEIGHT);
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                                         NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
                                                backing:NSBackingStoreBuffered
                                                  defer:NO
    ];
    [self.window setTitle:@"Metal Triangle"];
    self.window.delegate = self;
    [self.window setLevel:NSNormalWindowLevel];
    [self.window makeKeyAndOrderFront:nil];
    [self.window orderFrontRegardless];
    [NSApp activateIgnoringOtherApps:YES];

    MetalView *metalView = [[MetalView alloc] initWithFrame:frame];
    self.window.contentView = metalView;
    self.metalView = metalView;
    self.device = metalView.device;

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
        RealMainMetal(self, self.metalView);
    });
}
- (void)windowWillClose:(NSNotification *)notification {
    NSLog(@"Window is closing");
    [NSApp terminate:self]; // Optional: Quit the app when the window closes
    Running = false;
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