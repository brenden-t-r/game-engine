#if defined(PLATFORM_APPLE) and defined(BACKEND_METAL) and not defined(PLATFORM_IOS)

//region Imports
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <Carbon/Carbon.h>
#import <GameController/GameController.h>
#include "stb_image.h"

#include "../platform.h"
#include "helper.mm"

#include <cstdio>
#include "unordered_map"
//endregion

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
static void *runFuncContext;
static void (*runFunc)(void *);
static bool Running = false;
//endregion

//region MetalView/MetalAppDelegate declarations
@interface MetalView : MTKView <MTKViewDelegate>
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, assign) ShaderMTL* colorShader;
@property (nonatomic, assign) ShaderMTL* textureShader;
@property (nonatomic, assign) id<MTLSamplerState> linearSampler;
@property (nonatomic, assign) id<MTLSamplerState> pointSampler;
@property (strong) NSTrackingArea *trackingArea;
- (void) LoadShaders;
- (Shader*) LoadShader:(ShaderDef)shaderDef;
- (BOOL) IsMousePressed:(MouseButton) button;
- (BOOL) IsKeyPressed:(KeyCode) key;
- (BOOL) IsGamePadPressed:(GamepadButton)button;
@end
@interface MetalAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property (strong, nonatomic) NSWindow *window;
@property (strong, nonatomic) MetalView *metalView;
@property (strong, nonatomic) id<MTLDevice> device;
@end
//endregion

//region PlatformMetal
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
            sleep(1000);
        }
        printf("Run end");
    }
    void LoadShaders() override {
        [metalAppDelegate.metalView LoadShaders];
    }
    Shader* LoadShader(ShaderDef shaderDef) override {
        return [metalAppDelegate.metalView LoadShader:shaderDef];
    }
    GameObject* CreateGameObject() override { return new GameObject(); };
    GameObject* CreateTriangle() override {
        auto gameObject = new TriangleMetal(metalAppDelegate.device);
        gameObject->material = new MaterialColor([metalAppDelegate.metalView colorShader]);
        gameObject->SetMaterial(gameObject->material);
        triangles.push_back(gameObject);
        return gameObject;
    }
    TextureMTL* CreateTexture(const char* path, TextureSettings settings) override {
        NSString *imageName = [NSString stringWithUTF8String:path];
        id<MTLTexture> texture = loadImageAsTextureFromBundle(imageName, metalAppDelegate.metalView.device, settings);
        assert(texture != nullptr);
        auto sampler = settings.filter == TextureFilter::LINEAR
                ? [metalAppDelegate.metalView linearSampler]
                : [metalAppDelegate.metalView pointSampler];
        return new TextureMTL(path, texture, settings, sampler);
    }
    Sprite* CreateSprite(Texture* texture) override {
        auto gameObject = new SpriteMetal(
                metalAppDelegate.metalView.device, (TextureMTL*)texture
        );
        gameObject->material = new MaterialSprite([metalAppDelegate.metalView textureShader], texture);
        gameObject->SetMaterial(gameObject->material);
        sprites.push_back(gameObject);
        return gameObject;
    }
    Sprite* CreateSprite(const char* path) override {
        TextureMTL* texture = CreateTexture(path, DEFAULT_TEXTURE_SETTINGS);
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
    void RemoveAllCallbacks() override {
        gamepadUpCallback = nullptr;
        mouseUpCallback = nullptr;
        keyUpCallback = nullptr;
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
    const char* LoadFileData(const char* path) override {
        auto pathNS = [NSString stringWithUTF8String:path];
        auto result = loadTextFileFromBundleAsString(pathNS);
        return result.cString;
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

//region MetalView
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
-(void)LoadShaders {
    self.colorShader = [self LoadShader:InputLayoutType::POSITION vertexSrc:vertexShaderSrc fragmentSrc:fragmentShaderSrc];
    self.textureShader = [self LoadShader:InputLayoutType::POSITION_TEXCOORD vertexSrc:textureVertexShaderSrc fragmentSrc:textureFragmentShaderSrc];
}
-(Shader*)LoadShader:(ShaderDef)shaderDef {
    NSString* vertPath = [NSString stringWithUTF8String:shaderDef.vertexPath];
    NSString* fragPath = [NSString stringWithUTF8String:shaderDef.fragmentPath];
    NSString* vert = loadTextFileFromBundleAsString(vertPath);
    NSString* frag = loadTextFileFromBundleAsString(fragPath);
    return [self LoadShader:shaderDef.inputLayoutType vertexSrc:[vert cString] fragmentSrc:[frag cString]];
}
-(ShaderMTL*)LoadShader:(InputLayoutType)inputLayoutType vertexSrc:(const char*)vertexSrc fragmentSrc:(const char*)fragmentSrc {
    MTLRenderPipelineDescriptor* desc = [self loadShaderLibrary:vertexSrc frag:fragmentSrc];

    switch(inputLayoutType) {
        case InputLayoutType::POSITION:
        case InputLayoutType::POSITION_TEXCOORD:
            MTLRenderPipelineColorAttachmentDescriptor *attachment = desc.colorAttachments[0];
            attachment.pixelFormat = MTLPixelFormatBGRA8Unorm;
            attachment.blendingEnabled = YES;
            attachment.rgbBlendOperation = MTLBlendOperationAdd;
            attachment.alphaBlendOperation = MTLBlendOperationAdd;
            attachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
            attachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            attachment.sourceAlphaBlendFactor = MTLBlendFactorOne;
            attachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            desc.colorAttachments[0] = attachment;
            break;
    }

    MTLRenderPipelineReflection* reflection = nil;
    NSError *error = nil;
    id<MTLRenderPipelineState> pipelineState =
            [self.device newRenderPipelineStateWithDescriptor:desc
                                                 options:MTLPipelineOptionArgumentInfo |
                                                         MTLPipelineOptionBufferTypeInfo |
                                                         MTLPipelineOptionBindingInfo
                                              reflection:&reflection
                                                   error:&error];
    if (!pipelineState) {
        NSLog(@"Pipeline creation error: %@", error.localizedDescription);
        assert(false);
    }
    [desc release];

    ShaderMTL* shader = new ShaderMTL();
    shader->renderPipelineState = pipelineState;

    // Find uniform buffer at index 0
    MTLArgument *buffer0 = nil;
    for (MTLArgument *arg in reflection.fragmentArguments)
    {
        if (arg.type == MTLArgumentTypeBuffer && arg.index == 0)
        {
            buffer0 = arg;
            break;
        }
    }
    // Set the buffer data size
    shader->bufferDataSize = buffer0.bufferDataSize;
    // Enumerate constant buffer properties and offsets
    for (MTLStructMember *member in buffer0.bufferStructType.members)
    {
        Shader::UniformFieldOffset field;
        field.name = member.name.UTF8String;
        field.offset = (uint32_t)member.offset;
        shader->uniformFieldOffsets.push_back(field);
    }

    return shader;
}

- (void)setupPipeline {
    self.commandQueue = [self.device newCommandQueue];

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

    // Create sampler states
    MTLSamplerDescriptor *linearDesc = [[MTLSamplerDescriptor alloc] init];
    linearDesc.minFilter = MTLSamplerMinMagFilterLinear;
    linearDesc.magFilter = MTLSamplerMinMagFilterLinear;
    linearDesc.mipFilter = MTLSamplerMipFilterLinear;
    self.linearSampler = [self.device newSamplerStateWithDescriptor:linearDesc];

    MTLSamplerDescriptor *pointDesc = [[MTLSamplerDescriptor alloc] init];
    pointDesc.minFilter = MTLSamplerMinMagFilterNearest;
    pointDesc.magFilter = MTLSamplerMinMagFilterNearest;
    pointDesc.mipFilter = MTLSamplerMipFilterNearest;
    self.pointSampler = [self.device newSamplerStateWithDescriptor:pointDesc];
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
//    printf("%lu\n",static_cast<unsigned long>(pressed));
    switch (button) {
        case MouseButton::Unknown:
            return false;
        case MouseButton::Left:
            return pressed & (1 << 0);
        case MouseButton::Right:
            return pressed & (1 << 1);
        case MouseButton::Middle:
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

//region AppDelegate
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