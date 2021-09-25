#if !defined(PLATFORM_WINDOWS_H_)
#define PLATFORM_WINDOWS_H_

#if defined(function)
#   undef function
#endif

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#define function static

struct Windows_Context {
    Platform_Context platform;

    b32 running;

    Memory_Allocator alloc;
    Memory_Arena arena;

    u32 thread_count; // @Note: Count includes the main thread
    Thread_Context *thread_contexts;

    DWORD tls_handle;

    f64 performance_freq;
    u64 last_time;

    b32 fullscreen;
    WINDOWPLACEMENT placement;

    HWND window;
    v2u window_dim;
};

global Windows_Context windows_context;

struct Windows_Parameters {
    u32 init_flags;

    int show_cmd;

    str8 window_title;
    v2u  window_dim;
};

// This must be called before uinsg any platform layer code
//
function b32 WindowsInitialise(Windows_Parameters *params);

// Will process keyboard and mouse input, get frame delta time and handle quit events
//
function void WindowsHandleInput(Input *input);

// Set the window as fullscreen or toggle to switch between windowed and fullscreen
//
// @Todo: I think the current implementation is more of a "borderless windowed" fullscreen than exclusive
// fullscreen. Work out how to do exclusive fullscreen
//
function void WindowsSetFullscreen(b32 fullscreen);
function void WindowsToggleFullscreen();

#endif  // PLATFORM_WINDOWS_H_
