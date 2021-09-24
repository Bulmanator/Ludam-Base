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

function PLATFORM_GET_THREAD_CONTEXT(WindowsGetThreadContext) {
    Thread_Context *result = cast(Thread_Context *) TlsGetValue(windows_context.tls_handle);
    return result;
}

function void WindowsInitialiseThreadContext(Thread_Context *tctx) {
    for (u32 it = 0; it < ArraySize(tctx->scratch); ++it) {
        Initialise(&tctx->scratch[it], Platform->GetMemoryAllocator(), Gigabytes(1));
    }
}

function b32 WindowsInitialise() {
    b32 result = false;

    // Setup platform context
    //
    Platform = &windows_context.platform;

    Platform->GetMemoryAllocator = WindowsGetMemoryAllocator;
    Platform->GetThreadContext   = WindowsGetThreadContext;

    // Initialise the Windows memory arena for permanent platform side allocations
    //
    Memory_Allocator *alloc = Platform->GetMemoryAllocator();
    Initialise(&windows_context.arena, alloc, Megabytes(64));

    // Allocate thread local storage handle
    //
    windows_context.tls_handle = TlsAlloc();
    if (windows_context.tls_handle == TLS_OUT_OF_INDEXES) {
        return result;
    }

    // Allocate and initialise thread contexts
    //
    // @Todo: Allow for more threads
    //
    windows_context.thread_count    = 1;
    windows_context.thread_contexts = AllocArray(&windows_context.arena, Thread_Context, windows_context.thread_count);

    WindowsInitialiseThreadContext(&windows_context.thread_contexts[0]);
    TlsSetValue(windows_context.tls_handle, cast(LPVOID) &windows_context.thread_contexts[0]);

    result = true;
    return result;
}
