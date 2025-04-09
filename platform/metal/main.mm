#if defined(PLATFORM_APPLE) and defined(BACKEND_METAL) and not defined(PLATFORM_IOS)
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include "stb_image.h"

#include "../platform.h"
#include "../../engine/audio.h"

#include <cstdio>

static void *runFuncContext;
static void (*runFunc)(void *);
static bool Running = false;

struct VertexData {
    simd::float4 position;
    simd::float2 textureCoordinate;
};

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
@property (strong) NSTrackingArea *trackingArea;
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

        int row = atlasNumRows - atlasRow - 1;
        if (useAtlas) {
            newVertices[0].textureCoordinate.x = atlasCellSize * (float)atlasColumn; // Top-left
            newVertices[0].textureCoordinate.y = atlasCellSize * (float)row + atlasCellSize;
            newVertices[1].textureCoordinate.x = atlasCellSize * (float)atlasColumn; // Bottom left
            newVertices[1].textureCoordinate.y = atlasCellSize * (float)row;
            newVertices[2].textureCoordinate.x = atlasCellSize * (float)atlasColumn + atlasCellSize; // Bottom right
            newVertices[2].textureCoordinate.y = atlasCellSize * (float)row;
            newVertices[3].textureCoordinate.x = atlasCellSize * (float)atlasColumn; // Top-left
            newVertices[3].textureCoordinate.y = atlasCellSize * (float)row + atlasCellSize;
            newVertices[4].textureCoordinate.x = atlasCellSize * (float)atlasColumn + atlasCellSize; // Bottom right
            newVertices[4].textureCoordinate.y = atlasCellSize * (float)row;
            newVertices[5].textureCoordinate.x = atlasCellSize * (float)atlasColumn + atlasCellSize; // Top right
            newVertices[5].textureCoordinate.y = atlasCellSize * (float)row+ atlasCellSize;
        }

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
        audioWrapper = new AudioWrapper();
        audioWrapper->Init();
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
        NSString *imageName = [NSString stringWithUTF8String:path];
        id<MTLTexture> texture = loadImageAsTextureFromBundle(imageName, metalAppDelegate.metalView.device);
        if (texture) {
            NSLog(@"Texture loaded successfully!");
        } else {
            NSLog(@"Failed to load texture ☹\uFE0F");
        }
        auto gameObject = new SpriteMetal(
                metalAppDelegate.metalView.device, metalAppDelegate.metalView.texturePSO, texture
        );
        sprites.push_back(gameObject);
        return gameObject;
    }
    Sound* CreateSound(const char* path) override {
        auto sound = new SoundMA();
        sound->Init(path);
        return sound;
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
    AudioWrapper* audioWrapper;
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

    NSUInteger pressed = [NSEvent pressedMouseButtons];

    if (pressed & (1 << 0)) {
        NSLog(@"Left button is down");
    }
    if (pressed & (1 << 1)) {
        NSLog(@"Right button is down");
    }
    if (pressed & (1 << 2)) {
        NSLog(@"Middle button is down");
    }


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

- (void)keyDown:(NSEvent *)event {
    NSLog(@"Key pressed: %@, keyCode: %hu", event.characters, event.keyCode);
}

- (void)mouseDown:(NSEvent *)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    NSLog(@"Left click at: (%f, %f)", location.x, location.y);
}

- (void)mouseUp:(NSEvent *)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    NSLog(@"Left released at: (%f, %f)", location.x, location.y);
}

- (void)rightMouseDown:(NSEvent *)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    NSLog(@"Right click at: (%f, %f)", location.x, location.y);
}

- (void)rightMouseUp:(NSEvent *)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    NSLog(@"Right released at: (%f, %f)", location.x, location.y);
}

- (void)otherMouseDown:(NSEvent *)event {
    if (event.buttonNumber == 2) { // middle mouse button
        NSLog(@"Middle click at: %@", NSStringFromPoint([self convertPoint:event.locationInWindow fromView:nil]));
    }
}

- (void)otherMouseUp:(NSEvent *)event {
    if (event.buttonNumber == 2) { // middle mouse button
        NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
        NSLog(@"Middle released at: (%f, %f)", location.x, location.y);
    }
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint point = [self convertPoint:event.locationInWindow fromView:nil];
    NSLog(@"Mouse moved to: (%f, %f)", point.x, point.y);
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

@end
//endregion

//region: AppDelegate
@implementation MetalAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    NSLog(@"applicationDidFinishLaunching");
    NSRect frame = NSMakeRect(100, 100, 1280, 720);
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