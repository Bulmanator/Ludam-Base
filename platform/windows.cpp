Platform_Context *Platform; // for the extern

// Utiltiy functions
//
function str8 WindowsToUTF8(Memory_Arena *arena, WCHAR *data, u32 count) {
    str8 result;
    result.count = WideCharToMultiByte(CP_UTF8, 0, data, count, 0, 0, 0, 0);
    result.data  = AllocArray(arena, u8, result.count);

    WideCharToMultiByte(CP_UTF8, 0, data, count, cast(LPSTR) result.data, cast(u32) result.count, 0, 0);

    return result;
}

function WCHAR *WindowsToUTF16(Memory_Arena *arena, str8 str, uptr *result_count) {
    WCHAR *result = 0;
    u32 count = MultiByteToWideChar(CP_UTF8, 0, cast(LPSTR) str.data, cast(u32) str.count, 0, 0);
    result = AllocArray(arena, WCHAR, count);

    MultiByteToWideChar(CP_UTF8, 0, cast(LPSTR) str.data, cast(u32) str.count, result, count);

    if (result_count) { *result_count = count; }

    return result;
}

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
    Memory_Allocator *result = &windows_context->alloc;

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
    Thread_Context *result = cast(Thread_Context *) TlsGetValue(windows_context->tls_handle);
    return result;
}

function PLATFORM_GET_PATH(WindowsGetPath) {
    str8 result = {};

    Scratch_Memory scratch = GetScratch();

    switch (path) {
        case PlatformPath_Executable: {
            if (!IsValid(windows_context->exe_path)) {
                WCHAR *exe_path;
                DWORD  exe_path_count = 4096;

                // Get the executable path
                //
                while (true) {
                    exe_path = AllocArray(scratch.arena, WCHAR, exe_path_count);
                    exe_path_count = GetModuleFileNameW(0, exe_path, exe_path_count);

                    DWORD err = GetLastError();
                    if (err != ERROR_INSUFFICIENT_BUFFER) { break; }

                    exe_path_count *= 2;
                }

                // Remove the executable filename from the end of the string
                //
                DWORD last = 0;
                for (DWORD it = 0; it < exe_path_count; ++it) {
                    if (exe_path[it] == L'\\') {
                        last = it;
                    }
                }

                exe_path[last] = 0;
                exe_path_count = last;

                // Convert the UTF-16 string to UTF-8 for use in the game code
                //
                windows_context->exe_path = WindowsToUTF8(&windows_context->arena, exe_path, last);
            }

            result = windows_context->exe_path;
        }
        break;

        case PlatformPath_User: {
            if (!IsValid(windows_context->user_path)) {
                DWORD  user_path_count = 0;
                WCHAR *user_path = AllocArray(scratch.arena, WCHAR, 2 * MAX_PATH);

                // This function call doesn't give us any information about the returned string and won't
                // even say it 'failed' if the buffer is too small. The only thing the documentation says
                // is the buffer passed to it has to be at least MAX_PATH in characters.
                //
                if (!SHGetSpecialFolderPathW(0, user_path, CSIDL_APPDATA, TRUE)) {
                    return result;
                }

                LPWSTR appends[] = {
                    L"\\ametentia",
                    L"\\ludum dare"
                };

                for (u32 it = 0; it < ArraySize(appends); ++it) {
                    PathCchAppend(user_path, 2 * MAX_PATH, appends[it]);

                    // Check if the directory exists
                    //
                    DWORD attrs = GetFileAttributesW(user_path);
                    if ((attrs == INVALID_FILE_ATTRIBUTES) || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                        if (!CreateDirectoryW(user_path, 0)) {
                            break;
                        }
                    }
                }

                // The path appending doesn't give us the length so find the null-terminating byte
                //
                while (user_path[user_path_count] != 0) { user_path_count += 1; }

                // Convert from UTF-16 to UTF-8 for use in game code
                //
                windows_context->user_path = WindowsToUTF8(&windows_context->arena, user_path, user_path_count);
            }

            result = windows_context->user_path;
        }
        break;

        case PlatformPath_Working: {
            if (!IsValid(windows_context->working_path)) {
                WCHAR *working_path = AllocArray(scratch.arena, WCHAR, MAX_PATH);
                DWORD working_path_count = GetCurrentDirectoryW(MAX_PATH, working_path);
                if (working_path_count > MAX_PATH) {
                    working_path = AllocArray(scratch.arena, WCHAR, working_path_count);

                    working_path_count = GetCurrentDirectoryW(working_path_count, working_path);
                }

                // Convert UTF-16 to UTF-8 for use in the game code
                //
                windows_context->working_path = WindowsToUTF8(&windows_context->arena, working_path, working_path_count);
            }

            result = windows_context->working_path;
        }
        break;
    }

    return result;
}

function PLATFORM_LIST_PATH(WindowsListPath) {
    Path_List result = {};

    Scratch_Memory scratch = GetScratch(&arena, 1);

    u32 count = MultiByteToWideChar(CP_UTF8, 0, cast(LPSTR) path.data, cast(u32) path.count, 0, 0) + 6;

    WCHAR *wpath = AllocArray(scratch.arena, WCHAR, count);
    MultiByteToWideChar(CP_UTF8, 0, cast(LPSTR) path.data, cast(u32) path.count, wpath, count);

    PathCchAppend(wpath, count, L"\\*.*");

    WIN32_FIND_DATAW find_data = {};
    HANDLE find_handle = FindFirstFileW(wpath, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE) { return result; }

    for (; FindNextFileW(find_handle, &find_data);) {
        WCHAR *filename = find_data.cFileName;

        // @Note: We don't care about the relative paths to the current directory or the previous
        // directory so just skip them. It *seems* like they are always the first two entries to
        // show up when using FindFirstFile/FindNextFile but I couldn't find anything concrete to
        // support this. Instead of just skipping the first two entries we make sure the names match
        //
        if (filename[0] == L'.' && filename[1] == 0) { continue; } // Ignore '.'
        if (filename[0] == L'.' && filename[1] == L'.' && filename[2] == 0) { continue; } // Ignore '..'

        Path_Entry_Type type;

        // @Note: Windows marks all normal files as "ready for archiving" for some reason which causes
        // their attributes to come up as FILE_ATTRIBUTE_ARCHIVE instead of FILE_ATTRIBUTE_NORMAL so
        // we have to test for both.. I guess?
        //
        if (find_data.dwFileAttributes == FILE_ATTRIBUTE_NORMAL ||
            find_data.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE)
        {
            type = PathEntry_File;
        }
        else if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            type = PathEntry_Directory;
        }
        else {
            continue;
        }

        u32 filename_count = 0;
        while (filename[filename_count] != 0) { filename_count += 1; }

        FILETIME *mtime = &find_data.ftLastWriteTime;

        Path_Entry *entry = AllocType(arena, Path_Entry);

        entry->type = type;
        entry->size = (cast(uptr) find_data.nFileSizeHigh << 32) | cast(uptr) find_data.nFileSizeLow;
        entry->time = (cast(u64)  mtime->dwHighDateTime   << 32) | cast(u32) mtime->dwLowDateTime;

        // @Todo: basename could use the same memory as full_path
        //
        entry->basename  = WindowsToUTF8(arena, filename, filename_count);
        entry->full_path = FormatStr(arena, "%.*s\\%.*s", str8_unpack(path), str8_unpack(entry->basename));

        entry->next = 0;

        if (!result.first) {
            result.first = entry;
            result.last  = entry;
        }
        else {
            result.last->next = entry;
            result.last       = entry;
        }

        result.entry_count += 1;
    }


    return result;
}

function PLATFORM_OPEN_FILE(WindowsOpenFile) {
    File_Handle result = {};

    DWORD access   = 0;
    DWORD creation = 0;

    if (access_flags & FileAccess_Read) {
        access   |= GENERIC_READ;
        creation  = OPEN_EXISTING;
    }

    if (access_flags & FileAccess_Write) {
        access   |= GENERIC_WRITE;
        creation  = OPEN_ALWAYS;
    }

    Scratch_Memory scratch = GetScratch();

    u32 count = MultiByteToWideChar(CP_UTF8, 0, cast(LPSTR) path.data, cast(u32) path.count, 0, 0);
    WCHAR *wpath = AllocArray(scratch.arena, WCHAR, count + 1);

    MultiByteToWideChar(CP_UTF8, 0, cast(LPSTR) path.data, cast(u32) path.count, wpath, count);
    wpath[count] = 0;

    HANDLE handle = CreateFileW(wpath, access, FILE_SHARE_READ, 0, creation, 0, 0);
    if (handle == INVALID_HANDLE_VALUE) { result.errors = true; }

    *cast(HANDLE *) &result.platform = handle;

    return result;
}

function PLATFORM_CLOSE_FILE(WindowsCloseFile) {
    HANDLE win_handle = *cast(HANDLE *) &handle->platform;
    if (win_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(win_handle);
    }
}

function PLATFORM_ACCESS_FILE(WindowsReadFile) {
    if (handle->errors) { return; }
    HANDLE win_handle = *cast(HANDLE *) &handle->platform;

    // @Todo: Should probably read in multiple chunks but probably not going to be reading > 4GiB at a time
    // so whatever
    //
    DWORD to_read = cast(DWORD) size;
    DWORD bytes_read = 0;

    OVERLAPPED overlapped = {};
    overlapped.Offset     = cast(DWORD) (offset >>  0);
    overlapped.OffsetHigh = cast(DWORD) (offset >> 32);

    if (!ReadFile(win_handle, data, to_read, &bytes_read, &overlapped)) {
        handle->errors = true;
    }
}

function PLATFORM_ACCESS_FILE(WindowsWriteFile) {
    if (handle->errors) { return; }
    HANDLE win_handle = *cast(HANDLE *) &handle->platform;

    // @Todo: Should probably write in multiple chunks but probably not going to be writing > 4GiB at a time
    // so whatever
    //
    DWORD to_write = cast(DWORD) size;
    DWORD bytes_written = 0;

    OVERLAPPED overlapped = {};
    overlapped.Offset     = cast(DWORD) (offset >>  0);
    overlapped.OffsetHigh = cast(DWORD) (offset >> 32);

    if (!WriteFile(win_handle, data, to_write, &bytes_written, &overlapped)) {
        handle->errors = true;
    }
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
    f64 result = cast(f64) (end - start) / windows_context->performance_freq;
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
            windows_context->running = false;
        }
        break;

        case WM_SIZE: {
            windows_context->window_dim.w = LOWORD(lparam);
            windows_context->window_dim.h = HIWORD(lparam);
        }
        break;

        case WM_DPICHANGED: {
            LPRECT rect = cast(LPRECT) lparam;
            SetWindowPos(window, 0, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, SWP_NOOWNERZORDER | SWP_NOZORDER);
        }
        break;

        case WM_PAINT: {
            PAINTSTRUCT paint;
            BeginPaint(windows_context->window, &paint);
            EndPaint(windows_context->window, &paint);
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
    input->delta_time  = WindowsGetElapsedTime(windows_context->last_time, current_time);
    input->time       += input->delta_time;

    windows_context->last_time = current_time;

    MSG msg;
    while (PeekMessageA(&msg, windows_context->window, 0, 0, PM_REMOVE)) {
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
                mouse_clip.x = -1.0f + (2.0f * (cast(f32) point.x / cast(f32) windows_context->window_dim.x));
                mouse_clip.y = -1.0f + (2.0f * (cast(f32) point.y / cast(f32) windows_context->window_dim.y));

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

    input->requested_quit = !windows_context->running;
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

    // Enable High DPI support
    //
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    b32 open_window  = (params->init_flags & PlatformInit_OpenWindow);

    // @Todo: Enable audio
    //
    b32 enable_audio = (params->init_flags & PlatformInit_EnableAudio);
    (void) enable_audio;

    Memory_Allocator alloc;
    alloc.context  = 0;
    alloc.Reserve  = WindowsAllocatorReserve;
    alloc.Commit   = WindowsAllocatorCommit;
    alloc.Decommit = WindowsAllocatorDecommit;
    alloc.Release  = WindowsAllocatorRelease;

    windows_context = AllocInline(&alloc, Megabytes(64), Windows_Context, arena);

    // Setup platform context
    //
    Platform = cast(Platform_Context *) windows_context;

    Platform->GetMemoryAllocator = WindowsGetMemoryAllocator;
    Platform->GetThreadContext   = WindowsGetThreadContext;

    Platform->GetPath            = WindowsGetPath;
    Platform->ListPath           = WindowsListPath;

    Platform->OpenFile           = WindowsOpenFile;
    Platform->CloseFile          = WindowsCloseFile;

    Platform->ReadFile           = WindowsReadFile;
    Platform->WriteFile          = WindowsWriteFile;

    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
        return result;
    }

    windows_context->performance_freq = cast(f64) freq.QuadPart;

    // Allocate thread local storage handle
    //
    windows_context->tls_handle = TlsAlloc();
    if (windows_context->tls_handle == TLS_OUT_OF_INDEXES) {
        return result;
    }

    // Allocate and initialise thread contexts
    //
    // @Todo: Allow for more threads
    //
    windows_context->thread_count    = 1;
    windows_context->thread_contexts = AllocArray(&windows_context->arena, Thread_Context, windows_context->thread_count);

    WindowsInitialiseThreadContext(&windows_context->thread_contexts[0]);
    TlsSetValue(windows_context->tls_handle, cast(LPVOID) &windows_context->thread_contexts[0]);

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

        windows_context->window = CreateWindowExA(0, class_name, window_title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, window_dim.w, window_dim.h, 0, 0, instance, 0);
        if (!windows_context->window) {
            return result;
        }

        ShowWindow(windows_context->window, params->show_cmd);
    }

    windows_context->running = true;

    result = true;
    return result;
}

function void WindowsSetFullscreen(b32 fullscreen) {
    DWORD style = GetWindowLong(windows_context->window, GWL_STYLE);
    if (fullscreen) {
        if (GetWindowPlacement(windows_context->window, &windows_context->placement)) {
            HMONITOR monitor = MonitorFromWindow(windows_context->window, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO monitor_info = {};
            monitor_info.cbSize = sizeof(MONITORINFO);

            if (GetMonitorInfoA(monitor, &monitor_info)) {
                v2s pos = V2S(monitor_info.rcMonitor.left, monitor_info.rcMonitor.top);

                v2s dim;
                dim.w = (monitor_info.rcMonitor.right - monitor_info.rcMonitor.left);
                dim.h = (monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top);

                SetWindowLongA(windows_context->window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(windows_context->window, HWND_TOP, pos.x, pos.y, dim.w, dim.h, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            }
        }
    }
    else {
        SetWindowLongA(windows_context->window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(windows_context->window, &windows_context->placement);
        SetWindowPos(windows_context->window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

function void WindowsToggleFullscreen() {
    DWORD style = GetWindowLong(windows_context->window, GWL_STYLE);
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

    windows_context->renderer_dll = renderer_dll;

    Renderer_Initialise *Initialise = cast(Renderer_Initialise *) GetProcAddress(renderer_dll, "WindowsOpenGLInitialise");
    if (!Initialise) {
        return result;
    }

    // Set the platform data required for initialisation
    //
    *cast(HINSTANCE *) &params->platform_data[0] = GetModuleHandle(0);
    *cast(HWND *)      &params->platform_data[1] = windows_context->window;

    params->platform_alloc = Platform->GetMemoryAllocator();

    result = Initialise(params);
    return result;
}

function v2u WindowsGetWindowDim() {
    v2u result = V2U(0, 0);

    RECT client_rect;
    if (GetClientRect(windows_context->window, &client_rect)) {
        result.w = (client_rect.right - client_rect.left);
        result.h = (client_rect.bottom - client_rect.top);
    }

    return result;
}
