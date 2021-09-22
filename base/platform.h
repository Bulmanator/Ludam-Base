#if !defined(BASE_PLATFORM_H_)
#define BASE_PLATFORM_H_

#define PLATFORM_GET_MEMORY_ALLOCATOR(name) Memory_Allocator *name(void)
typedef PLATFORM_GET_MEMORY_ALLOCATOR(Platform_Get_Memory_Allocator);

struct Platform_Context {
    Platform_Get_Memory_Allocator *GetMemoryAllocator;
};

extern Platform_Context *Platform;

#endif  // BASE_PLATFORM_H_
