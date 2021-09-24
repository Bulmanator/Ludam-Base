#if !defined(BASE_PLATFORM_H_)
#define BASE_PLATFORM_H_

enum Platform_Init_Flags {
    PlatformInit_OpenWindow  = (1 << 0),
    PlatformInit_EnableAudio = (1 << 1),
};

#define PLATFORM_GET_MEMORY_ALLOCATOR(name) Memory_Allocator *name(void)
typedef PLATFORM_GET_MEMORY_ALLOCATOR(Platform_Get_Memory_Allocator);

struct Thread_Context {
    // @Todo: We can add more things here if need be
    //
    Memory_Arena scratch[4];
};

#define PLATFORM_GET_THREAD_CONTEXT(name) Thread_Context *name(void)
typedef PLATFORM_GET_THREAD_CONTEXT(Platform_Get_Thread_Context);

struct Platform_Context {
    Platform_Get_Memory_Allocator *GetMemoryAllocator;
    Platform_Get_Thread_Context   *GetThreadContext;

};

extern Platform_Context *Platform;

#endif  // BASE_PLATFORM_H_
