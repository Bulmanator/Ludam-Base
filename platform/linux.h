#include <sys/mman.h>
#include <sys/stat.h>

#include <unistd.h>
#include <errno.h>

#include <dirent.h>
#include <fcntl.h>

#include <dlfcn.h>
#include <pthread.h>

#if defined(function)
#    undef function
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#define function static

// Xlib dynamic loading
//
#if !defined(Button8)
#    define Button8 8
#endif

#if !defined(Button9)
#    define Button9 9
#endif

typedef Display *type_XOpenDisplay(_Xconst char *);
typedef Window type_XCreateWindow(Display *, Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int, Visual *, unsigned long, XSetWindowAttributes *);
typedef Atom type_XInternAtom(Display *, _Xconst char *, Bool);
typedef Status type_XSetWMProtocols(Display *, Window, Atom *, int);
typedef int type_XStoreName(Display *display, Window, _Xconst char *);
typedef int type_XMapWindow(Display *, Window);
typedef int type_XPending(Display *);
typedef int type_XNextEvent(Display *, XEvent *);
typedef int type_XEventsQueued(Display *, int);
typedef int type_XPeekEvent(Display *, XEvent *);
typedef KeySym type_XLookupKeysym(XKeyEvent *, int);

struct Xlib_Context {
    void *so_handle;

    Display *display;
    Window   window;
    Atom     closed;

    type_XOpenDisplay    *_XOpenDisplay;
    type_XCreateWindow   *_XCreateWindow;
    type_XInternAtom     *_XInternAtom;
    type_XSetWMProtocols *_XSetWMProtocols;
    type_XStoreName      *_XStoreName;
    type_XMapWindow      *_XMapWindow;
    type_XPending        *_XPending;
    type_XNextEvent      *_XNextEvent;
    type_XEventsQueued   *_XEventsQueued;
    type_XPeekEvent      *_XPeekEvent;
    type_XLookupKeysym   *_XLookupKeysym;
};

#define XOpenDisplay    linux_context->xlib._XOpenDisplay
#define XCreateWindow   linux_context->xlib._XCreateWindow
#define XInternAtom     linux_context->xlib._XInternAtom
#define XSetWMProtocols linux_context->xlib._XSetWMProtocols
#define XStoreName      linux_context->xlib._XStoreName
#define XMapWindow      linux_context->xlib._XMapWindow
#define XPending        linux_context->xlib._XPending
#define XNextEvent      linux_context->xlib._XNextEvent
#define XEventsQueued   linux_context->xlib._XEventsQueued
#define XPeekEvent      linux_context->xlib._XPeekEvent
#define XLookupKeysym   linux_context->xlib._XLookupKeysym

// pa_simple dynamic loading
//
typedef pa_simple *type_pa_simple_new(const char *, const char *, pa_stream_direction_t, const char *, const char *, const pa_sample_spec *, const pa_channel_map *, const pa_buffer_attr *, int *);
typedef void type_pa_simple_free(pa_simple *);
typedef int type_pa_simple_write(pa_simple *, const void *, size_t, int *);
typedef pa_usec_t type_pa_simple_get_latency(pa_simple *, int *);
typedef int type_pa_simple_flush(pa_simple *, int *);

struct Pulse_Context {
    b32 enabled;
    void *so_handle;


    u32 channel_count;
    u32 sample_rate;

    u32 max_audio_frames;
    s16 *sample_buffer;

    pa_simple *handle;

    type_pa_simple_new   *_pa_simple_new;
    type_pa_simple_free  *_pa_simple_free;
    type_pa_simple_write *_pa_simple_write;
    type_pa_simple_get_latency *_pa_simple_get_latency;
    type_pa_simple_flush       *_pa_simple_flush;
};

#define pa_simple_new         linux_context->pa._pa_simple_new
#define pa_simple_free        linux_context->pa._pa_simple_free
#define pa_simple_write       linux_context->pa._pa_simple_write
#define pa_simple_get_latency linux_context->pa._pa_simple_get_latency
#define pa_simple_flush       linux_context->pa._pa_simple_flush

struct Linux_Context {
    Platform_Context platform;

    Xlib_Context xlib;
    Pulse_Context pa;

    v2u window_dim;

    b32 running;

    Memory_Allocator alloc;
    Memory_Arena arena;

    pthread_key_t tls_handle;
    u32 thread_count;
    Thread_Context *thread_contexts;

    u64 last_time;

    str8 exe_path;
    str8 user_path;
    str8 working_path;
};

global Linux_Context *linux_context;

struct Linux_Parameters {
    u32 init_flags;

    str8 window_title;
    v2u  window_dim;
};

// This must be called before using any platform layer code
//
function b32 LinuxInitialise(Linux_Parameters *params);

// Will process keyboard and mouse input, get frame delta time and handle quit events
//
function void LinuxHandleInput(Input *input);

// Loads the default renderer. OpenGL for now.
//
function Renderer_Context *LinuxLoadRenderer(Renderer_Parameters *parameters);

// Gets the current window size
//
function v2u LinuxGetWindowDim();

// Gets the required number of samples to output as well as a backing buffer to write those samples into
//
function void LinuxGetAudioBuffer(Audio_Buffer *buffer);

// Submits the audio buffer for playback
//
function void LinuxSubmitAudioBuffer(Audio_Buffer *buffer);
