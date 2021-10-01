#include <sys/mman.h>
#include <dlfcn.h>
#include <pthread.h>

#if defined(function)
#    undef function
#endif

#include <X11/Xlib.h>

#define function static

typedef Display *type_XOpenDisplay(_Xconst char *);
typedef Window type_XCreateWindow(Display *, Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int, Visual *, unsigned long, XSetWindowAttributes *);
typedef Atom type_XInternAtom(Display *, _Xconst char *, Bool);
typedef Status type_XSetWMProtocols(Display *, Window, Atom *, int);
typedef int type_XStoreName(Display *display, Window, _Xconst char *);
typedef int type_XMapWindow(Display *, Window);
typedef int type_XPending(Display *);
typedef int type_XNextEvent(Display *, XEvent *);

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
};

#define XOpenDisplay    linux_context->xlib._XOpenDisplay
#define XCreateWindow   linux_context->xlib._XCreateWindow
#define XInternAtom     linux_context->xlib._XInternAtom
#define XSetWMProtocols linux_context->xlib._XSetWMProtocols
#define XStoreName      linux_context->xlib._XStoreName
#define XMapWindow      linux_context->xlib._XMapWindow
#define XPending        linux_context->xlib._XPending
#define XNextEvent      linux_context->xlib._XNextEvent

#define XCreateSimpleWindow linux_context->xlib._XCreateSimpleWindow

struct Linux_Context {
    Platform_Context platform;

    Xlib_Context xlib;

    b32 running;

    Memory_Allocator alloc;
    Memory_Arena arena;

    pthread_key_t tls_handle;
    u32 thread_count;
    Thread_Context *thread_contexts;

    u64 last_time;
};

global Linux_Context *linux_context;

struct Linux_Parameters {
    u32 init_flags;

    str8 window_title;
    v2u  window_dim;
};

function b32 LinuxInitialise(Linux_Parameters *params);
