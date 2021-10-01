Platform_Context *Platform; // for the extern

// Memory allocator functions
//
function MEMORY_ALLOCATOR_RESERVE(LinuxAllocatorReserve) {
    (void) context;
    (void) flags;

    void *result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    return result;
}

function MEMORY_ALLOCATOR_MODIFY(LinuxAllocatorNull) {
    (void) context;
    (void) base;
    (void) size;
}

function MEMORY_ALLOCATOR_MODIFY(LinuxAllocatorRelease) {
    (void) context;
    munmap(base, size);
}

// Platform context functions
//
function PLATFORM_GET_MEMORY_ALLOCATOR(LinuxGetMemoryAllocator) {
    Memory_Allocator *result = &linux_context->alloc;
    if (!result->Reserve) {
        result->context  = 0;

        // @Note: The mmap/munmap system doesn't allow you to explicitly commit/de-commit pages
        // so the Commit and Decommit pointers are just dummy calls that don't do anything.
        // It will only supply you with actual pages when you touch the memory
        //
        result->Reserve  = LinuxAllocatorReserve;
        result->Commit   = LinuxAllocatorNull;
        result->Decommit = LinuxAllocatorNull;
        result->Release  = LinuxAllocatorRelease;
    }

    return result;
}

function PLATFORM_GET_THREAD_CONTEXT(LinuxGetThreadContext) {
    Thread_Context *result = cast(Thread_Context *) pthread_getspecific(linux_context->tls_handle);
    return result;
}

// Initialisation
//
function void LinuxInitialiseThreadContext(Thread_Context *thread_context) {
    for (u32 it = 0; it < ArraySize(thread_context->scratch); ++it) {
        Initialise(&thread_context->scratch[it], Platform->GetMemoryAllocator(), Gigabytes(1));
    }
}

function b32 LinuxLoadXlibFunctions(Xlib_Context *xlib) {
    b32 result = false;

    xlib->so_handle = dlopen("libX11.so", RTLD_NOW);
    if (!xlib->so_handle) { return result; }

#define XLIB_LOAD_FUNCTION(name) xlib->_##name = (type_##name *) dlsym(xlib->so_handle, #name); do { if (!xlib->_##name) { return result; } } while (0)
    XLIB_LOAD_FUNCTION(XOpenDisplay);
    XLIB_LOAD_FUNCTION(XCreateWindow);
    XLIB_LOAD_FUNCTION(XInternAtom);
    XLIB_LOAD_FUNCTION(XSetWMProtocols);
    XLIB_LOAD_FUNCTION(XStoreName);
    XLIB_LOAD_FUNCTION(XMapWindow);
    XLIB_LOAD_FUNCTION(XPending);
    XLIB_LOAD_FUNCTION(XNextEvent);
#undef XLIB_LOAD_FUNCTION

    result = true;
    return result;
}

function b32 LinuxInitialise(Linux_Parameters *params) {
    b32 result = false;

    b32 open_window  = (params->init_flags & PlatformInit_OpenWindow);
    b32 enable_audio = (params->init_flags & PlatformInit_EnableAudio);

    Memory_Allocator alloc = {};
    alloc.context  = 0;
    alloc.Reserve  = LinuxAllocatorReserve;
    alloc.Commit   = LinuxAllocatorNull;
    alloc.Decommit = LinuxAllocatorNull;
    alloc.Release  = LinuxAllocatorRelease;

    linux_context = AllocInline(&alloc, Megabytes(64), Linux_Context, arena);

    // Setup platform context
    //
    Platform = cast(Platform_Context *) linux_context;

    Platform->GetMemoryAllocator = LinuxGetMemoryAllocator;
    Platform->GetThreadContext   = LinuxGetThreadContext;

    linux_context->arena.alloc = Platform->GetMemoryAllocator();

    linux_context->last_time = 0;

    if (pthread_key_create(&linux_context->tls_handle, 0) != 0) {
        return result;
    }

    linux_context->thread_count    = 1;
    linux_context->thread_contexts = AllocArray(&linux_context->arena, Thread_Context, linux_context->thread_count);

    LinuxInitialiseThreadContext(&linux_context->thread_contexts[0]);
    pthread_setspecific(linux_context->tls_handle, cast(void *) &linux_context->thread_contexts[0]);

    if (open_window) {
        Scratch_Memory scratch = GetScratch();

        Xlib_Context *xlib = &linux_context->xlib;
        if (!LinuxLoadXlibFunctions(xlib)) { return result; }

        // Open a connection to X11 display
        //
        xlib->display = XOpenDisplay(0);
        if (!xlib->display) { return result; }

        // Create the window
        //
        XSetWindowAttributes window_attrs = {};
        window_attrs.event_mask =
            StructureNotifyMask |
            ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask |
            KeyPressMask | KeyReleaseMask;

        xlib->window = XCreateWindow(xlib->display, DefaultRootWindow(xlib->display), 0, 0, params->window_dim.w, params->window_dim.h, 1, CopyFromParent, InputOutput, CopyFromParent, CWEventMask, &window_attrs);

        if (!xlib->window) { return result; }

        xlib->closed = XInternAtom(xlib->display, "WM_DELETE_WINDOW", False);

        Atom client_events[] = { xlib->closed };
        XSetWMProtocols(xlib->display, xlib->window, client_events, ArraySize(client_events));

        const char *window_name = CopyZ(scratch.arena, params->window_title);
        XStoreName(xlib->display, xlib->window, window_name);

        XMapWindow(xlib->display, xlib->window);
    }

    result = true;
    return result;
}
