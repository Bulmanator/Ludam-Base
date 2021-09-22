Platform_Context *Platform; // for the extern

// Memory allocation functions
//
function MEMORY_ALLOCATOR_RESERVE(WindowsAllocatorReserve) {
    (void) context;
    (void) flags;

    void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
    return result;
}

function MEMORY_ALLOCATOR_MODIFY(WindowsAllocatorCommit) {
    (void) context;

    VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE);
}

function MEMORY_ALLOCATOR_MODIFY(WindowsAllocatorDecommit) {
    (void) context;

    VirtualFree(base, size, MEM_DECOMMIT);
}

function MEMORY_ALLOCATOR_MODIFY(WindowsAllocatorRelease) {
    (void) context;
    (void) size; // VirtualFree requies size to be 0 when releasing

    VirtualFree(base, 0, MEM_RELEASE);
}

// Platform context functions
//
function PLATFORM_GET_MEMORY_ALLOCATOR(WindowsGetMemoryAllocator) {
    Memory_Allocator *result = &windows_context.alloc;

    if (!result->Reserve) {
        result->context = 0;

        result->Reserve  = WindowsAllocatorReserve;
        result->Commit   = WindowsAllocatorCommit;
        result->Decommit = WindowsAllocatorDecommit;
        result->Release  = WindowsAllocatorRelease;
    }

    return result;
}

function b32 WindowsInitialise() {
    b32 result = true;

    // Setup platform context
    //
    Platform = &windows_context.platform;

    Platform->GetMemoryAllocator = WindowsGetMemoryAllocator;

    return result;
}
