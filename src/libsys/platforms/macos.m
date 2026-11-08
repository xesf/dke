#include "gamma.h"
#include "envfx.h"
#include <Cocoa/Cocoa.h>
#include <AudioToolbox/AudioQueue.h>
#include <mach-o/dyld.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "../system.h"
#include "../common.h"
#include "../audio/audio.h"
// #include "platform.h"

/* =========================================================================
 * CoreAudio AudioQueue streaming backend
 * =========================================================================
 *
 * Each push allocates ONE AudioQueue buffer sized to the whole clip, enqueues
 * it, and returns immediately (non-blocking). aq_callback frees the buffer
 * when CoreAudio finishes playing it. aq_inflight bounds outstanding buffers.
 * This avoids the old fixed-pool design that blocked the render thread for a
 * clip's entire duration (dialogue voice froze the frame).
 * ========================================================================= */

static AudioQueueRef        aq_queue  = NULL;
static pthread_mutex_t      aq_mutex  = PTHREAD_MUTEX_INITIALIZER;

static b8 aq_open = false;

/* Count of buffers currently enqueued (allocated on demand, freed here).
 * Bounds memory if a caller pushes faster than playback drains. */
static i32                  aq_inflight = 0;
/* Bytes enqueued and not yet handed back by the callback (the buffer being
 * played counts in full). */
static i32                  aq_queued_bytes = 0;
#define AQ_MAX_INFLIGHT     64

/* AudioQueue callback — called from CoreAudio's internal thread.
 * Buffers are allocated per-push (whole clip in one buffer) and FREED here,
 * so a push never blocks waiting on a fixed pool. */
static void aq_callback(void *user_data,
                        AudioQueueRef queue,
                        AudioQueueBufferRef buf)
{
    (void)user_data;
    i32 bytes = (i32)buf->mAudioDataByteSize;
    AudioQueueFreeBuffer(queue, buf);
    pthread_mutex_lock(&aq_mutex);
    if (aq_inflight > 0) aq_inflight--;
    aq_queued_bytes -= bytes;
    if (aq_queued_bytes < 0) aq_queued_bytes = 0;
    pthread_mutex_unlock(&aq_mutex);
}

b8 ls_platform_audio_open(i32 sample_rate, i32 channels, i32 bit_depth)
{
    if (aq_open) return true;   /* already open */

    AudioStreamBasicDescription fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.mSampleRate       = (Float64)sample_rate;
    fmt.mFormatID         = kAudioFormatLinearPCM;
    fmt.mFormatFlags      = kAudioFormatFlagIsSignedInteger
                          | kAudioFormatFlagIsPacked;
    /* Little-endian on all Apple silicon / x86 hosts — no endian flag needed */
    fmt.mBitsPerChannel   = (UInt32)bit_depth;
    fmt.mChannelsPerFrame = (UInt32)channels;
    fmt.mBytesPerFrame    = (UInt32)(channels * (bit_depth / 8));
    fmt.mFramesPerPacket  = 1;
    fmt.mBytesPerPacket   = fmt.mBytesPerFrame;

    OSStatus err = AudioQueueNewOutput(&fmt, aq_callback, NULL,
                                       NULL, NULL, 0, &aq_queue);
    if (err != noErr) {
        printf("ls_platform_audio_open: AudioQueueNewOutput failed (%d)\n", (int)err);
        aq_queue = NULL;
        return false;
    }

    /* Buffers are allocated on demand per push (see ls_platform_audio_push);
     * no fixed pool / silence prefill. */
    pthread_mutex_lock(&aq_mutex);
    aq_inflight = 0;
    pthread_mutex_unlock(&aq_mutex);

    err = AudioQueueStart(aq_queue, NULL);
    if (err != noErr) {
        printf("ls_platform_audio_open: AudioQueueStart failed (%d)\n", (int)err);
        AudioQueueDispose(aq_queue, true);
        aq_queue = NULL;
        return false;
    }

    aq_open = true;
    printf("ls_platform_audio_open: CoreAudio queue started (%d Hz, %d ch, %d bit)\n",
           sample_rate, channels, bit_depth);
    return true;
}

i32 ls_platform_audio_push(const i16 *pcm, i32 bytes)
{
    if (!aq_open || !aq_queue || !pcm || bytes <= 0) return 0;

    /* Non-blocking: allocate ONE AudioQueue buffer sized to the whole clip,
     * copy, enqueue, return immediately. CoreAudio plays it on its own thread
     * and aq_callback frees it. The render thread never waits on playback,
     * so dialogue voice no longer stalls the frame. */
    pthread_mutex_lock(&aq_mutex);
    if (aq_inflight >= AQ_MAX_INFLIGHT) {   /* backpressure — drop, don't block */
        pthread_mutex_unlock(&aq_mutex);
        return 0;
    }
    aq_inflight++;
    pthread_mutex_unlock(&aq_mutex);

    AudioQueueBufferRef buf = NULL;
    OSStatus err = AudioQueueAllocateBuffer(aq_queue, (UInt32)bytes, &buf);
    if (err != noErr || !buf) {
        pthread_mutex_lock(&aq_mutex); aq_inflight--; pthread_mutex_unlock(&aq_mutex);
        return 0;
    }
    memcpy(buf->mAudioData, pcm, (size_t)bytes);
    buf->mAudioDataByteSize = (UInt32)bytes;

    pthread_mutex_lock(&aq_mutex); aq_queued_bytes += bytes; pthread_mutex_unlock(&aq_mutex);
    err = AudioQueueEnqueueBuffer(aq_queue, buf, 0, NULL);
    if (err != noErr) {
        AudioQueueFreeBuffer(aq_queue, buf);
        pthread_mutex_lock(&aq_mutex); aq_inflight--; aq_queued_bytes -= bytes; pthread_mutex_unlock(&aq_mutex);
        return 0;
    }
    return bytes;
}

i32 ls_platform_audio_queued(void)
{
    if (!aq_open) return -1;
    pthread_mutex_lock(&aq_mutex);
    i32 q = aq_queued_bytes;
    pthread_mutex_unlock(&aq_mutex);
    return q;
}

void ls_platform_audio_close(void)
{
    if (!aq_open || !aq_queue) return;
    AudioQueueStop(aq_queue, true);   /* true = synchronous drain */
    AudioQueueDispose(aq_queue, true);
    aq_queue = NULL;
    aq_open  = false;
    pthread_mutex_lock(&aq_mutex);
    aq_inflight = 0;
    aq_queued_bytes = 0;
    pthread_mutex_unlock(&aq_mutex);
    printf("ls_platform_audio_close: CoreAudio queue stopped.\n");
}


static NSWindow *window = NULL;
static u8 *surface = NULL;
static NSAutoreleasePool *pool = NULL;


static void handle_events(system_t *system, i32 type, i32 key) {
    // Letters also set their VK code (actions['A'..'Z'], what process_input_command
    // tests); 'P' = actions[80] is the pause key and the cheat words typed on the
    // pause screen use the rest. macOS key codes are laid out by keyboard position.
    static const char letter_of_key[128] = {
        [0x00]='A', [0x0B]='B', [0x08]='C', [0x02]='D', [0x0E]='E', [0x03]='F', [0x05]='G', [0x04]='H',
        [0x22]='I', [0x26]='J', [0x28]='K', [0x25]='L', [0x2E]='M', [0x2D]='N', [0x1F]='O', [0x23]='P',
        [0x0C]='Q', [0x0F]='R', [0x01]='S', [0x11]='T', [0x20]='U', [0x09]='V', [0x0D]='W', [0x07]='X',
        [0x10]='Y', [0x06]='Z' };
    if (key >= 0 && key < 128 && letter_of_key[key] && letter_of_key[key] != 'C') system->actions[(int)letter_of_key[key]] = type;
    // if (key > 0 && key < 256) {
    //     return;
    // }
    // https://web.archive.org/web/20100501161453/http://www.classicteck.com/rbarticles/mackeyboard.php
    switch(key) {
        // w
        case 0x03:
            system->actions[119] = type;
            break;
        // s
        case 0x01:
            system->actions[115] = type;
            break;
        // a
        case 0x00:
            system->actions[97] = type;
            break;
        // d
        case 0x02:
            system->actions[100] = type;
            break;
        // q
        case 0x0C:
            system->actions[113] = type;
            break;
        // e
        case 0x0E:
            system->actions[101] = type;
            break;
        // r
        case 0x0F:
            system->actions[114] = type;
            break;
        // t
        case 0x11:
            system->actions[116] = type;
            break;
        // l
        case 0x25:
            system->actions[108] = type;
            break;
        /* Navigation keys — wired for menu_update (pass 2) */
        // Return / Enter (keyCode 36) → actions[13] (ECX=0x0D in original asm)
        case 0x24:
            system->actions[13] = type;
            break;
        // Escape (keyCode 53) → actions[27] (ECX=0x1B in original asm)
        case 0x35:
            system->actions[27] = type;
            break;
        // Up arrow (keyCode 126) → actions[38] (ECX=0x25 / VK_UP)
        case 0x7E:
            system->actions[38] = type;
            break;
        // Down arrow (keyCode 125) → actions[40] (ECX=0x27 / VK_DOWN)
        case 0x7D:
            system->actions[40] = type;
            break;
        // Left arrow (keyCode 123) → actions[37] (VK_LEFT)
        case 0x7B:
            system->actions[37] = type;
            break;
        // Right arrow (keyCode 124) → actions[39] (VK_RIGHT)
        case 0x7C:
            system->actions[39] = type;
            break;
        // Tab (0x30) → VK_TAB: special mode; c (0x08) → 'C': draw / sheathe
        case 0x30: system->actions[9] = type; break;
        case 0x08: system->actions[0x43] = type; break;
        // i (keyCode 0x22) → actions[105]: inventory
        case 0x22: system->actions[105] = type; break;
        // Backspace (keyCode 0x33) → actions[8] (VK_BACK)
        case 0x33: system->actions[8] = type; break;
        // Digits 1..9, 0 → actions['1'..'9'], actions['0'] (inventory hotkeys)
        case 0x12: system->actions['1'] = type; break;
        case 0x13: system->actions['2'] = type; break;
        case 0x14: system->actions['3'] = type; break;
        case 0x15: system->actions['4'] = type; break;
        case 0x17: system->actions['5'] = type; break;
        case 0x16: system->actions['6'] = type; break;
        case 0x1A: system->actions['7'] = type; break;
        case 0x1C: system->actions['8'] = type; break;
        case 0x19: system->actions['9'] = type; break;
        case 0x1D: system->actions['0'] = type; break;
        // Page Up / Page Down / F1 → VK_PRIOR / VK_NEXT / VK_F1
        case 0x74: system->actions[33] = type; break;
        case 0x79: system->actions[34] = type; break;
        case 0x7A: system->actions[112] = type; break;
        // F2 / F3 → actions[121] / [122]: in-game save / load screens
        case 0x78: system->actions[121] = type; break;
        case 0x63: system->actions[122] = type; break;
        // Space (keyCode 49) → actions[32] (VK_SPACE: action / talk key)
        case 0x31:
            system->actions[32] = type;
            break;
        // F12 (keyCode 111) → actions[111] for screenshot
        case 0x6F:
            system->actions[111] = type;
            break;
        // [ (keyCode 0x21) → actions[91] : manual camera prev (debug)
        case 0x21:
            system->actions[91] = type;
            break;
        // ] (keyCode 0x1E) → actions[93] : manual camera next (debug)
        case 0x1E:
            system->actions[93] = type;
            break;
        default:
            break;
    }
}

static void handle_mouse_events(system_t *system, i32 type, i32 x, i32 y) {
    switch(type) {
        case 0:
            break;
        case 1:
            system->motion_x = x;
            system->motion_y = y;
            break;
        default:
            break;
    }
}

@interface WindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation WindowDelegate {
    system_t *system;
}

- (instancetype)initWithSystem:(system_t *)sys {
    self = [super init];
    if (self != nil) {
        system = sys;
    }
    return self;
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    system->quit = 1;
    return NO;
}

@end



@interface ContentView : NSView
@end

@implementation ContentView {
    system_t *system;
}

- (instancetype)initWithSystem:(system_t *)sys {
    self = [super init];
    if (self != nil) {
        system = sys;
    }
    return self;
}

- (BOOL)acceptsFirstResponder {
    return YES; // keyboard events
}

- (void)drawRect:(NSRect)dirtyRect {
    NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:&(surface)
                          pixelsWide:system->width
                          pixelsHigh:system->height
                       bitsPerSample:8
                     samplesPerPixel:3
                            hasAlpha:NO
                            isPlanar:NO
                      colorSpaceName:NSCalibratedRGBColorSpace
                         bytesPerRow:system->width * system->bpp
                        bitsPerPixel:32] autorelease];
    NSImage *nsimage = [[[NSImage alloc] init] autorelease];
    [nsimage addRepresentation:rep];
    (void)dirtyRect;
    [nsimage drawInRect:[self bounds]];   /* whole frame, even on partial redraws */
}

- (void)keyDown:(NSEvent *)event {
    handle_events(system, 1, [event keyCode]);
}

- (void)keyUp:(NSEvent *)event {
    handle_events(system, 0, [event keyCode]);
}

/* Modifier keys arrive as flag changes: Shift / Control / Option ->
 * VK_SHIFT / VK_CONTROL / VK_MENU (run, attack, alt fire). */
- (void)flagsChanged:(NSEvent *)event {
    NSEventModifierFlags f = [event modifierFlags];
    system->actions[0x10] = (f & NSEventModifierFlagShift) ? 1 : 0;
    system->actions[0x11] = (f & NSEventModifierFlagControl) ? 1 : 0;
    system->actions[0x12] = (f & NSEventModifierFlagOption) ? 1 : 0;
}

- (void)mouseDown:(NSEvent *)event {
    /* Update motion_x/y at click time so hitbox test uses current position.
     * Cocoa y-origin is bottom-left; flip to match top-left framebuffer origin. */
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    system->motion_x = (i32)point.x;
    system->motion_y = system->height - (i32)point.y;
    /* Edge = 2 (pressed this frame); ls_update_events downgrades to 1 next tick. */
    system->mouse_left = 2;
}

- (void)mouseUp:(NSEvent *)event {
    (void)event;
    system->mouse_left = 0;
}

- (void)rightMouseDown:(NSEvent *)event {
    (void)event;
    system->mouse_right = 2;
}

- (void)rightMouseUp:(NSEvent *)event {
    (void)event;
    system->mouse_right = 0;
}

- (void)otherMouseDown:(NSEvent *)event {
    (void)event;
    system->mouse_middle = 2;
}

- (void)otherMouseUp:(NSEvent *)event {
    (void)event;
    system->mouse_middle = 0;
}

- (void)scrollWheel:(NSEvent *)event {
    // float offset = (float)[event scrollingDeltaY];
    // if ([event hasPreciseScrollingDeltas]) {
    //     offset *= 0.1f;
    // }
}

- (void)updateTrackingAreas {
    [self initTrackingArea];
}

- (void) initTrackingArea {
    NSTrackingAreaOptions options = (NSTrackingActiveAlways | NSTrackingInVisibleRect |
                                NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved);

    NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                        options:options
                                                            owner:self
                                                        userInfo:nil];

    [self addTrackingArea:area];
}

- (void)mouseMoved:(NSEvent *)event {
    [super mouseMoved: event];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    /* Cocoa y-origin is bottom-left; framebuffer y-origin is top-left.
     * Flip y so menu hitbox tests (top-down) work correctly. */
    i32 flipped_y = system->height - (i32)point.y;
    handle_mouse_events(system, 1, (i32)point.x, flipped_y);
}

@end
    

static void create_window(system_t *system) {
    NSRect rect;
    NSUInteger mask;
    WindowDelegate *delegate;
    ContentView *view;
    
    delegate = [[WindowDelegate alloc] initWithSystem:system];
    view = [[[ContentView alloc] initWithSystem:system] autorelease];

    rect = NSMakeRect(0, 0, system->width, system->height);
    mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    window = [[NSWindow alloc] initWithContentRect:rect
                                         styleMask:mask
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
    [window setDelegate:delegate];
    [window setContentView:view];
    [window makeFirstResponder:view];
    [window makeKeyAndOrderFront:nil];

    [window setTitle:[NSString stringWithUTF8String:system->title]];
    [window setColorSpace:[NSColorSpace genericRGBColorSpace]];
    [window center];
}


i32 ls_init_instance(system_t *system, i32 width, i32 height, i32 bpp) {
    /* Line-buffer stdout so print() output reaches the shell when the app
     * runs backgrounded (Cocoa apps otherwise leave stdout fully buffered). */
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    system->quit = false;
    system->width = width;
    system->height = height;
    system->bpp = bpp;

    surface = (u8 *)malloc(width * height * bpp);

    pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    
    NSMenu *menu, *app_menu;
    NSMenuItem *app_menu_item, *quit_menu_item;
    NSString *app_name, *quit_title;

    menu = [[[NSMenu alloc] init] autorelease];
    [NSApp setMainMenu:menu];

    app_menu_item = [[[NSMenuItem alloc] init] autorelease];
    [menu addItem:app_menu_item];

    app_menu = [[[NSMenu alloc] init] autorelease];
    [app_menu_item setSubmenu:app_menu];

    app_name = [[NSProcessInfo processInfo] processName];
    quit_title = [@"Quit " stringByAppendingString:app_name];
    quit_menu_item = [[[NSMenuItem alloc] initWithTitle:quit_title action:@selector(terminate:) keyEquivalent:@"q"] autorelease];

    [app_menu addItem:quit_menu_item];

    [NSApp finishLaunching];

    create_window(system);

    return TRUE;
}

void ls_update_events(system_t *system) {
    /* Downgrade pressed-edge (2) → held (1) so consumers can distinguish
     * a click frame from subsequent held frames. */
    if (system->mouse_left   == 2) system->mouse_left   = 1;
    if (system->mouse_right  == 2) system->mouse_right  = 1;
    if (system->mouse_middle == 2) system->mouse_middle = 1;

    NSEvent* ev;
    do {
        ev = [NSApp nextEventMatchingMask: NSEventMaskAny
                                untilDate: nil
                                    inMode: NSDefaultRunLoopMode
                                    dequeue: YES];
        if (ev) {
            [NSApp sendEvent: ev];
        }
    } while (ev);
    [pool drain];
    pool = [[NSAutoreleasePool alloc] init];
}

static pthread_once_t tick_timebase_once = PTHREAD_ONCE_INIT;
static mach_timebase_info_data_t tick_timebase;
static b8 tick_timebase_valid;

static void tick_timebase_init(void) {
    kern_return_t result = mach_timebase_info(&tick_timebase);
    if (result != KERN_SUCCESS || tick_timebase.denom == 0) {
        printf("ls_tick: mach_timebase_info failed (%d)\n", (int)result);
        return;
    }
    tick_timebase_valid = true;
}

inline u32 ls_tick() {
    pthread_once(&tick_timebase_once, tick_timebase_init);
    if (!tick_timebase_valid) return 0;

    u64 ticks = mach_absolute_time();
    u64 ns = (ticks / tick_timebase.denom) * tick_timebase.numer
           + ((ticks % tick_timebase.denom) * tick_timebase.numer) / tick_timebase.denom;
    return (u32)(ns / 1000000);
}

inline void ls_delay(u32 delay) {
    [NSThread sleepForTimeInterval:delay / 1000.0];
}

void ls_set_title(system_t *system, c8 *title) {
    [window setTitle:[NSString stringWithUTF8String:title]];
}

void ls_blit(system_t *system, u32 *front_buffer) {
    memcpy(surface, (u8*)front_buffer, system->width * system->height * system->bpp);
    envfx_apply_pixels((u32 *)surface, system->width * system->height);   /* envfx.h */
    gamma_apply_pixels((u32 *)surface, system->width * system->height);   /* gamma.h */
}

void ls_flip(system_t *system) {
    [[window contentView] setNeedsDisplay:YES];
}

void ls_release(system_t *system) {
    [window orderOut:nil];

    [[window delegate] release];
    [window close];

    [pool drain];
    pool = [[NSAutoreleasePool alloc] init];

    free(surface);
    surface = NULL;

    /* `window` is an NSWindow, not a malloc'd block. free()ing it aborted in
     * libsystem_malloc ("pointer being freed was not allocated"); releasing it
     * here over-releases, because -close already dropped the window's own
     * reference (releasedWhenClosed defaults to YES) and the pool drain
     * reclaimed it. Just forget the pointer. */
    window = NULL;
}
