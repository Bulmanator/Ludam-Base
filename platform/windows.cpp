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

// Input handling
//
function LRESULT WindowsMainWindowMessageHandler(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    switch (message) {
        case WM_QUIT:
        case WM_CLOSE: {
            windows_context.running = false;
        }
        break;

        default: {
            result = DefWindowProcA(window, message, wparam, lparam);
        }
        break;
    }

    return result;
}

function void WindowsInitialiseThreadContext(Thread_Context *tctx) {
    for (u32 it = 0; it < ArraySize(tctx->scratch); ++it) {
        Initialise(&tctx->scratch[it], Platform->GetMemoryAllocator(), Gigabytes(1));
    }
}

function b32 WindowsInitialise(Windows_Parameters *params) {
    b32 result = false;

    b32 open_window  = (params->init_flags & PlatformInit_OpenWindow);
    b32 enable_audio = (params->init_flags & PlatformInit_EnableAudio);

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

    if (open_window) {
        HINSTANCE instance = GetModuleHandleA(0);

        Scratch_Memory scratch = GetScratch();

        LPCSTR window_title = CopyZ(scratch.arena, params->window_title);
        LPCSTR class_name   = 0;

        // @Todo: This can just be a FormatStr or something later
        //
        u8 *buffer = AllocArray(scratch.arena, u8, params->window_title.count + 7);

        CopySize(buffer, cast(void *) window_title, params->window_title.count);
        CopySize(&buffer[params->window_title.count], "_class", 7);

        class_name = cast(LPCSTR) buffer;

        WNDCLASSA wnd_class = {};
        wnd_class.style         = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        wnd_class.lpfnWndProc   = WindowsMainWindowMessageHandler;
        wnd_class.cbClsExtra    = 0;
        wnd_class.cbWndExtra    = 0;
        wnd_class.hInstance     = instance;
        wnd_class.hIcon         = 0; // @Todo: Allow icon
        wnd_class.hCursor       = LoadCursorA(instance, IDC_ARROW);
        wnd_class.hbrBackground = cast(HBRUSH) GetStockObject(BLACK_BRUSH);
        wnd_class.lpszMenuName  = 0;
        wnd_class.lpszClassName = class_name;

        if (!RegisterClassA(&wnd_class)) {
            return result;
        }

        RECT window_rect;
        window_rect.left   = 0;
        window_rect.top    = 0;
        window_rect.right  = params->window_dim.w;
        window_rect.bottom = params->window_dim.h;

        v2u window_dim;
        if (!AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE)) {
            window_dim = params->window_dim;
        }
        else {
            window_dim.w = (window_rect.right - window_rect.left);
            window_dim.h = (window_rect.bottom - window_rect.top);
        }

        windows_context.window = CreateWindowExA(0, class_name, window_title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, window_dim.w, window_dim.h, 0, 0, instance, 0);
        if (!windows_context.window) {
            return result;
        }

        ShowWindow(windows_context.window, params->show_cmd);
    }

    windows_context.running = true;

    result = true;
    return result;
}
