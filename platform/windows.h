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

    Memory_Allocator alloc;
};

global Windows_Context windows_context;

function b32 WindowsInitialise();

#endif  // PLATFORM_WINDOWS_H_
