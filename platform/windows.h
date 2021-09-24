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

    HWND window;
};

global Windows_Context windows_context;

struct Windows_Parameters {
    u32 init_flags;

    int show_cmd;

    str8 window_title;
    v2u  window_dim;
};

function b32 WindowsInitialise(Windows_Parameters *params);

#endif  // PLATFORM_WINDOWS_H_
