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
function u64 WindowsGetTicks() {
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);

    u64 result = ticks.QuadPart;
    return result;
}

function f64 WindowsGetElapsedTime(u64 start, u64 end) {
    f64 result = cast(f64) (end - start) / windows_context.performance_freq;
    return result;
}

function void WindowsResetInput(Input *input) {
    for (u32 it = 0; it < Key_Count; ++it) {
        input->keys[it].transitions = 0;
    }

    for (u32 it = 0; it < Mouse_Count; ++it) {
        input->mouse_buttons[it].transitions = 0;
    }

    input->mouse_delta = V3(0, 0, 0);
    input->delta_time  = 0;
}

function Key_Code WindowsTranslateVirtualKey(WPARAM wparam) {
    Key_Code result = Key_Unknown;

    if (wparam >= 'A' && wparam <= 'Z') {
        // Alphabet range
        //
        result = cast(Key_Code) (Key_A + (wparam - 'A'));
    }
    else if (wparam >= '0' && wparam <= '9') {
        // Numbers range
        //
        result = cast(Key_Code) (Key_0 + (wparam - '0'));
    }
    else if (wparam >= VK_F1 && wparam <= VK_F12) {
        // F key range
        //
        result = cast(Key_Code) (Key_F1 + (wparam - VK_F1));
    }
    else {
        switch (wparam) {
            // Punctuation
            //
            case VK_OEM_PLUS:   { result = Key_Equals;       } break;
            case VK_OEM_MINUS:  { result = Key_Minus;        } break;
            case VK_OEM_COMMA:  { result = Key_Comma;        } break;
            case VK_OEM_PERIOD: { result = Key_Peroid;       } break;

            // These can vary based on keyboard layout. I hate computers
            //
            case VK_OEM_1:      { result = Key_Semicolon;    } break;
            case VK_OEM_2:      { result = Key_Slash;        } break;
            case VK_OEM_4:      { result = Key_LeftBracket;  } break;
            case VK_OEM_6:      { result = Key_RightBracket; } break;
            case VK_OEM_5:      { result = Key_Backslash;    } break;

            // @Note: The documentation says this is typically the single quote/ double quote on US layout
            // but on my GB layout it is hash/ tilde
            // Likewise the documentation says that OEM_3 is the tilde key for US layout but on my GB layout it
            // is the quote/ at key
            //
            // so these two keys seem to be flipped around on my GB layout keyboard
            //
            case VK_OEM_7:      { result = Key_Hash;         } break;
            case VK_OEM_3:      { result = Key_Quote;        } break;

            // This key is literally marked "misc" on the documentation so who knows what it'll end up being
            // but it shows up as the 'grave' key (next to 1) on my GB layout keyboard
            //
            case VK_OEM_8:      { result = Key_Grave;        } break;

            // Other keys
            //
            case VK_SHIFT:   { result = Key_Shift;     } break;
            case VK_CONTROL: { result = Key_Control;   } break;
            case VK_LWIN:    { result = Key_Super;     } break;
            case VK_MENU:    { result = Key_Alt;       } break;
            case VK_RETURN:  { result = Key_Enter;     } break;
            case VK_BACK:    { result = Key_Backspace; } break;
            case VK_TAB:     { result = Key_Tab;       } break;
            case VK_ESCAPE:  { result = Key_Escape;    } break;

            default: {} break;
        }
    }

    return result;
}

function void WindowsHandleButtonEvent(Input_Button *button, b32 pressed) {
    if (button->pressed != pressed) {
        button->pressed      = pressed;
        button->transitions += 1;
    }
}

function void WindowsHandleMouseEvent(Input *input, WPARAM wparam, b32 pressed) {
    if (wparam & MK_LBUTTON)  { WindowsHandleButtonEvent(&input->mouse_buttons[Mouse_Left],   pressed); }
    if (wparam & MK_MBUTTON)  { WindowsHandleButtonEvent(&input->mouse_buttons[Mouse_Middle], pressed); }
    if (wparam & MK_RBUTTON)  { WindowsHandleButtonEvent(&input->mouse_buttons[Mouse_Right],  pressed); }
    if (wparam & MK_XBUTTON1) { WindowsHandleButtonEvent(&input->mouse_buttons[Mouse_X1],     pressed); }
    if (wparam & MK_XBUTTON2) { WindowsHandleButtonEvent(&input->mouse_buttons[Mouse_X2],     pressed); }
}

function LRESULT WindowsMainWindowMessageHandler(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    switch (message) {
        case WM_QUIT:
        case WM_CLOSE: {
            windows_context.running = false;
        }
        break;

        case WM_SIZE: {
            windows_context.window_dim.w = LOWORD(lparam);
            windows_context.window_dim.h = HIWORD(lparam);
        }
        break;

        case WM_DPICHANGED: {
            LPRECT rect = cast(LPRECT) lparam;
            SetWindowPos(window, 0, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, SWP_NOOWNERZORDER | SWP_NOZORDER);
        }
        break;

        default: {
            result = DefWindowProcA(window, message, wparam, lparam);
        }
        break;
    }

    return result;
}

function void WindowsHandleInput(Input *input) {
    WindowsResetInput(input);

    // Delta time calculation
    //
    u64 current_time   = WindowsGetTicks();
    input->delta_time  = WindowsGetElapsedTime(windows_context.last_time, current_time);
    input->time       += input->delta_time;

    windows_context.last_time = current_time;

    MSG msg;
    while (PeekMessageA(&msg, windows_context.window, 0, 0, PM_REMOVE)) {
        switch (msg.message) {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYUP:
            case WM_KEYDOWN: {
                b32 is_pressed = (msg.lParam & (1 << 31)) == 0;
                Key_Code code  = WindowsTranslateVirtualKey(msg.wParam);

                if (code != Key_Unknown) {
                    WindowsHandleButtonEvent(&input->keys[code], is_pressed);
                }
            }
            break;
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_XBUTTONDOWN: {
                WindowsHandleMouseEvent(input, msg.wParam, true);
            }
            break;
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            case WM_XBUTTONUP: {
                WindowsHandleMouseEvent(input, msg.wParam, false);
            }
            break;
            case WM_MOUSEMOVE: {
                POINTS point = MAKEPOINTS(msg.lParam);

                v2 mouse_clip;
                mouse_clip.x = -1.0f + (2.0f * (cast(f32) point.x / cast(f32) windows_context.window_dim.x));
                mouse_clip.y = -1.0f + (2.0f * (cast(f32) point.y / cast(f32) windows_context.window_dim.y));

                v2 old_clip = input->mouse_clip;

                input->mouse_clip      = mouse_clip;
                input->mouse_delta.xy += (mouse_clip - old_clip);
            }
            break;
            case WM_MOUSEWHEEL: {
                f32 wheel_delta = cast(f32) GET_WHEEL_DELTA_WPARAM(msg.wParam) / cast(f32) WHEEL_DELTA;
                input->mouse_delta.z += wheel_delta;
            }
            break;
            default: {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            break;
        }
    }

    input->requested_quit = !windows_context.running;
}

// Initialisation
//
function void WindowsInitialiseThreadContext(Thread_Context *tctx) {
    for (u32 it = 0; it < ArraySize(tctx->scratch); ++it) {
        Initialise(&tctx->scratch[it], Platform->GetMemoryAllocator(), Gigabytes(1));
    }
}

function b32 WindowsInitialise(Windows_Parameters *params) {
    b32 result = false;

    // Enable HighDPI support
    //
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    b32 open_window  = (params->init_flags & PlatformInit_OpenWindow);

    // @Todo: Enable audio
    //
    b32 enable_audio = (params->init_flags & PlatformInit_EnableAudio);
    (void) enable_audio;

    // Setup platform context
    //
    Platform = &windows_context.platform;

    Platform->GetMemoryAllocator = WindowsGetMemoryAllocator;
    Platform->GetThreadContext   = WindowsGetThreadContext;

    // Initialise the Windows memory arena for permanent platform side allocations
    //
    Memory_Allocator *alloc = Platform->GetMemoryAllocator();
    Initialise(&windows_context.arena, alloc, Megabytes(64));

    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
        return result;
    }

    windows_context.performance_freq = cast(f64) freq.QuadPart;

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

    windows_context.fullscreen = false;
    windows_context.running = true;

    result = true;
    return result;
}

function void WindowsSetFullscreen(b32 fullscreen) {
    DWORD style = GetWindowLong(windows_context.window, GWL_STYLE);
    if (fullscreen) {
        if (GetWindowPlacement(windows_context.window, &windows_context.placement)) {
            HMONITOR monitor = MonitorFromWindow(windows_context.window, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO monitor_info = {};
            monitor_info.cbSize = sizeof(MONITORINFO);

            if (GetMonitorInfoA(monitor, &monitor_info)) {
                v2s pos = V2S(monitor_info.rcMonitor.left, monitor_info.rcMonitor.top);

                v2s dim;
                dim.w = (monitor_info.rcMonitor.right - monitor_info.rcMonitor.left);
                dim.h = (monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top);

                SetWindowLongA(windows_context.window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(windows_context.window, HWND_TOP, pos.x, pos.y, dim.w, dim.h, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            }
        }
    }
    else {
        SetWindowLongA(windows_context.window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(windows_context.window, &windows_context.placement);
        SetWindowPos(windows_context.window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

function void WindowsToggleFullscreen() {
    DWORD style = GetWindowLong(windows_context.window, GWL_STYLE);
    WindowsSetFullscreen(style & WS_OVERLAPPEDWINDOW);
}

function Renderer_Context *WindowsLoadRenderer(Renderer_Parameters *params) {
    Renderer_Context *result = 0;

    // @Incomplete: This will not work if the executing directory is not the same as the exe directory
    //
    HMODULE renderer_dll = LoadLibraryA("renderer_wgl.dll");
    if (!renderer_dll) {
        return result;
    }

    windows_context.renderer_dll = renderer_dll;

    Renderer_Initialise *Initialise = cast(Renderer_Initialise *) GetProcAddress(renderer_dll, "WindowsOpenGLInitialise");
    if (!Initialise) {
        return result;
    }

    // Set the platform data required for initialisation
    //
    *cast(HINSTANCE *) &params->platform_data[0] = GetModuleHandle(0);
    *cast(HWND *)      &params->platform_data[1] = windows_context.window;

    params->platform_alloc = Platform->GetMemoryAllocator();

    result = Initialise(params);
    return result;
}

function v2u WindowsGetWindowDim() {
    v2u result = V2U(0, 0);

    RECT client_rect;
    if (GetClientRect(windows_context.window, &client_rect)) {
        result.w = (client_rect.right - client_rect.left);
        result.h = (client_rect.bottom - client_rect.top);
    }

    return result;
}
