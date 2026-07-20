#if defined(PLATFORM_IOS)

//region Imports
#include <Metal/Metal.h>
#include <UIKit/UIKit.h>
#include <MetalKit/MetalKit.h>
#include <AVFoundation/AVFoundation.h>
#include <GameController/GameController.h>

#include "../platform.h"
#include "../metal/helper.mm"

#include "unordered_map"
//endregion

// region Input Helpers / Callbacks
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
void static(*mouseUpCallback)(MouseButton, void*, vec3);
static void* mouseCallbackContext;
void static(*gamepadUpCallback)(GamepadButton, void*);
static void* gamepadCallbackContext;
//endregion

//region Interface declarations
@interface MetalViewController : UIViewController<MTKViewDelegate>
@property (nonatomic, strong) MTKView *metalView;
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, assign) ShaderMTL* colorShader;
@property (nonatomic, assign) ShaderMTL* textureShader;
@property (nonatomic, assign) id<MTLSamplerState> linearSampler;
@property (nonatomic, assign) id<MTLSamplerState> pointSampler;
@property (nonatomic, strong) NSMutableSet<UITouch *> *activeTouches;
@property (nonatomic, strong) GCVirtualController *virtualController;
- (void) LoadShaders;
- (Shader*) LoadShader:(ShaderDef)shaderDef;
- (BOOL) IsGamePadPressed:(GamepadButton)button;
@end
@interface MetalAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) MetalViewController *viewController;
@end
//endregion

//region Sound
AVAudioEngine *engine = nullptr;
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
    void LoadShaders() override {
        [metalAppDelegate.viewController LoadShaders];
    }
    Shader* LoadShader(ShaderDef shaderDef) override {
        return [metalAppDelegate.viewController LoadShader:shaderDef];
    }
    GameObject* CreateGameObject() override { return new GameObject(); };
    GameObject* CreateTriangle() override {
        auto gameObject = new TriangleMetal(metalAppDelegate.viewController.device);
        gameObject->material = new MaterialColor([metalAppDelegate.viewController colorShader]);
        gameObject->SetMaterial(gameObject->material);
        triangles.push_back(gameObject);
        return gameObject;
    }
    TextureMTL* CreateTexture(const char* path, TextureSettings settings) override {
        NSString *imageName = [NSString stringWithUTF8String:path];
        id<MTLTexture> texture = loadImageAsTextureFromBundle(imageName, metalAppDelegate.viewController.device, settings);
        assert(texture != nullptr);
        auto sampler = settings.filter == TextureFilter::LINEAR
                       ? [metalAppDelegate.viewController linearSampler]
                       : [metalAppDelegate.viewController pointSampler];
        return new TextureMTL(path, texture, settings, sampler);
    }
    Sprite* CreateSprite(Texture* texture) override {
        auto gameObject = new SpriteMetal(
                metalAppDelegate.viewController.device, (TextureMTL*)texture
        );
        gameObject->material = new MaterialSprite([metalAppDelegate.viewController textureShader], texture);
        gameObject->SetMaterial(gameObject->material);
        sprites.push_back(gameObject);
        return gameObject;
    }
    Sprite* CreateSprite(const char* path) override {
        TextureMTL* texture = CreateTexture(path, DEFAULT_TEXTURE_SETTINGS);
        return CreateSprite(texture);
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
        if (metalAppDelegate.viewController.activeTouches.count > 0) {
            //NSLog(@"activeTouches:%lu", static_cast<unsigned long>(metalAppDelegate.viewController.activeTouches.count));
        }
        if (button == MouseButton::Left) {
            return metalAppDelegate.viewController.activeTouches.count == 1;
        } else if (button == MouseButton::Right) {
            return metalAppDelegate.viewController.activeTouches.count == 2;
        } else return false;
    }
    bool IsGamepadButtonPressed(GamepadButton button) override { return [metalAppDelegate.viewController IsGamePadPressed: button]; }
    void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) override {}
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
    }
    vec3 GetMousePos() override { return {}; }
    std::string LoadFileData(const char* path) override {
        auto pathNS = [NSString stringWithUTF8String:path];
        auto result = loadTextFileFromBundleAsString(pathNS);
        return result.cString;
    }
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
    self.view.multipleTouchEnabled = YES;
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
#ifdef IOS_VIRTUAL_CONTROLLER
    if (!_virtualController) {
        GCVirtualControllerConfiguration *config = [[GCVirtualControllerConfiguration alloc] init];
        config.elements = [NSSet setWithArray:@[
                GCInputDirectionalDpad,
                GCInputButtonA,
                GCInputButtonY,
//                GCInputButtonB,
//                GCInputButtonX,
//                GCInputLeftThumbstick,
//                GCInputRightThumbstick
        ]];
        _virtualController = [[GCVirtualController alloc] initWithConfiguration:config];
    }
    if (GCController.controllers.count == 0 && _virtualController != nil) {
        [_virtualController connectWithReplyHandler:nil];
    }
#endif
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
    MTLRenderPipelineDescriptor *desc = [self loadShaderLibrary:vertexSrc frag:fragmentSrc];

    switch (inputLayoutType) {
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
- (void)setupPipeline {
    self.commandQueue = [self.device newCommandQueue];

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
#ifdef IOS_FORCE_PORTRAIT
    return UIInterfaceOrientationMaskPortrait;
#elif defined(IOS_FORCE_LANDSCAPE)
    return UIInterfaceOrientationMaskLandscape;
#else
    return UIInterfaceOrientationMaskAll;
#endif

}
- (void)handleTap:(UITapGestureRecognizer *)gesture {
    CGPoint location = [gesture locationInView:self.view];
    CGSize size = self.view.bounds.size;
    float ndcX = (2.0f * (float)location.x / (float)size.width) - 1.0f;
    float ndcY = 1.0f - (float)(2.0f * location.y / size.height); // flip Y
    NSLog(@"Tap at NDC: (%f, %f)", ndcX, ndcY);
    if (mouseUpCallback != nil) {
        mouseUpCallback(MouseButton::Left, mouseCallbackContext, vec3{ndcX, ndcY, 0});
    }
}
- (void)handleTwoFingerTap:(UITapGestureRecognizer *)gesture {
    if (gesture.numberOfTouches == 2) {
        CGPoint touch1 = [gesture locationOfTouch:0 inView:self.view];
        CGPoint touch2 = [gesture locationOfTouch:1 inView:self.view];
        CGPoint midpoint = CGPointMake((touch1.x + touch2.x) / 2, (touch1.y + touch2.y) / 2);
        NSLog(@"Two-finger tap midpoint: (%f, %f)", midpoint.x, midpoint.y);
        CGSize size = self.view.bounds.size;
        float ndcX = (2.0f * (float)midpoint.x / (float)size.width) - 1.0f;
        float ndcY = 1.0f - (float)(2.0f * midpoint.y / size.height); // flip Y
        NSLog(@"Two-finger tap at NDC: (%f, %f)", ndcX, ndcY);
        if (mouseUpCallback != nil) {
            mouseUpCallback(MouseButton::Right, mouseCallbackContext, vec3{ndcX, ndcY, 0});
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
    [self updateTouchesFromEvent:event];
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self updateTouchesFromEvent:event];
}
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self updateTouchesFromEvent:event];
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self updateTouchesFromEvent:event];
}
- (void)updateTouchesFromEvent:(UIEvent *)event {
    [self.activeTouches removeAllObjects];
    for (UITouch *touch in event.allTouches) {
        if (touch.phase == UITouchPhaseBegan ||
            touch.phase == UITouchPhaseMoved ||
            touch.phase == UITouchPhaseStationary) {
            [self.activeTouches addObject:touch];
        }
    }
//    NSLog(@"Active touches: %lu", (unsigned long)self.activeTouches.count);
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

    if (_virtualController != nil) {
        BOOL hasPhysicalController = NO;
        for (GCController *ctrl in GCController.controllers) {
            if (ctrl != _virtualController.controller) {
                hasPhysicalController = YES;
                break;
            }
        }
        if (hasPhysicalController) {
            [_virtualController disconnect];
        }
    }
}
- (void)controllerDisconnected:(NSNotification *)notification {
    GCController *controller = notification.object;
    NSLog(@"Controller disconnected: %@", controller.vendorName);
    [self showVirtualController];
}
- (void)showVirtualController {
    if (!self.virtualController.controller.isAttachedToDevice) {
        [self.virtualController connectWithReplyHandler:^(NSError * _Nullable error) {
            if (error) {
                NSLog(@"Error showing virtual controller: %@", error);
            }
        }];
    }
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
