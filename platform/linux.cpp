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

function PLATFORM_GET_PATH(LinuxGetPath) {
    str8 result = {};

    Scratch_Memory scratch = GetScratch();

    switch (path) {
        case PlatformPath_Executable: {
            if (!IsValid(linux_context->exe_path)) {
                str8 path;
                path.count = 4096;

                while (true) {
                    path.data  = AllocArray(scratch.arena, u8, path.count);

                    sptr count = readlink("/proc/self/exe", cast(char *) path.data, path.count);
                    if (count < 0) { return result; }

                    if (cast(uptr) count < path.count) {
                        path.count = count;
                        break;
                    }

                    path.count *= 2;
                }

                uptr last = 0;
                for (uptr it = 0; it < path.count; ++it) {
                    if (path.data[it] == '/') { last = it; }
                }

                path.count = last;

                linux_context->exe_path = CopyStr(&linux_context->arena, path);
            }

            result = linux_context->exe_path;
        }
        break;

        case PlatformPath_User: {
            if (!IsValid(linux_context->user_path)) {
                const char *home = getenv("HOME");
                if (!home) { return result; }

                str8 ametentia_dir = FormatStr(scratch.arena, "%s/.local/share/ametentia", home);
                str8 path = FormatStr(scratch.arena, "%s/.local/share/ametentia/ludum dare", home);

                char *check = cast(char *) ametentia_dir.data;

                struct stat info;
                if (stat(check, &info) != 0) {
                    mkdir(check, 0755);
                }
                else if (!S_ISDIR(info.st_mode)) {
                    return result;
                }

                check = cast(char *) path.data;
                if (stat(check, &info) != 0) {
                    mkdir(check, 0755);
                }
                else if (!S_ISDIR(info.st_mode)) {
                    return result;
                }

                linux_context->user_path = CopyStr(&linux_context->arena, path);
            }

            result = linux_context->user_path;
        }
        break;

        case PlatformPath_Working: {
            if (!IsValid(linux_context->working_path)) {
                str8 path;
                path.count = 4096;
                path.data  = AllocArray(scratch.arena, u8, path.count);

                while (!getcwd(cast(char *) path.data, path.count)) {
                    if (errno != ERANGE) {  return result; }

                    path.count *= 2;
                    path.data   = AllocArray(scratch.arena, u8, path.count);
                }

                linux_context->working_path = CopyStr(&linux_context->arena, path);
            }

            result = linux_context->working_path;
        }
        break;
    }

    return result;
}

function PLATFORM_LIST_PATH(LinuxListPath) {
    Path_List result = {};

    Scratch_Memory scratch = GetScratch(&arena, 1);

    const char *zpath = CopyZ(scratch.arena, path);

    DIR *dir = opendir(zpath);
    if (!dir) { return result; }

    for (struct dirent *it = readdir(dir); it != 0; it = readdir(dir)) {
        char *filename = it->d_name;

        // Ignore relative path entries
        //
        if (filename[0] == '.' && filename[1] == 0) { continue; }
        if (filename[0] == '.' && filename[1] == '.' && filename[2] == 0) { continue; }

        Path_Entry_Type type;
        if (it->d_type == DT_REG) {
            type = PathEntry_File;
        }
        else if (it->d_type == DT_DIR) {
            type = PathEntry_Directory;
        }
        else {
            continue;
        }

        Path_Entry *entry = AllocType(arena, Path_Entry);

        entry->type      = type;

        entry->basename  = CopyStr(arena, WrapZ(filename));
        entry->full_path = FormatStr(arena, "%.*s/%.*s", path, entry->basename);

        // @Leak: This leaks a bit of memory for the names and path entry if this fails
        //
        struct stat info;
        if (stat(cast(char *) entry->full_path.data, &info) != 0) { continue; }

        entry->size = info.st_size;
        entry->time = (info.st_mtim.tv_sec * 1000000000) + info.st_mtim.tv_nsec;

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

function PLATFORM_OPEN_FILE(LinuxOpenFile) {
    File_Handle result = {};

    u32 open_mode = 0;
    u32 mode      = 0644;

    if (access_flags & FileAccess_Read) {
        open_mode = O_RDONLY;
    }

    if (access_flags & FileAccess_Write) {
        if (open_mode == O_RDONLY) {
            open_mode = O_RDWR;
        }
        else {
            open_mode = O_WRONLY;
        }

        open_mode |= O_CREAT;
    }

    Scratch_Memory scratch = GetScratch();

    const char *zpath = CopyZ(scratch.arena, path);

    int fd = open(zpath, open_mode, mode);
    if (fd < 0) {
        result.errors = true;
    }
    else {
        *cast(int *) &result.platform = fd;
    }

    return result;
}

function PLATFORM_CLOSE_FILE(LinuxCloseFile) {
    int fd = *cast(int *) &handle->platform;
    if (fd > 2) {
        close(fd);
    }

    handle->platform = 0;
    handle->errors   = true;
}

function PLATFORM_ACCESS_FILE(LinuxReadFile) {
    if (handle->errors) { return; }

    int fd = *cast(int *) &handle->platform;

    lseek(fd, offset, SEEK_SET);

    sptr success = read(fd, data, size);
    if (success < 0 || cast(uptr) success != size) {
        handle->errors = true;
    }
}

function PLATFORM_ACCESS_FILE(LinuxWriteFile) {
    if (handle->errors) { return; }

    int fd = *cast(int *) &handle->platform;

    lseek(fd, offset, SEEK_SET);

    sptr success = write(fd, data, size);
    if (success < 0 || cast(uptr) success != size) {
        handle->errors = true;
    }
}

// Input handling
//
function u64 LinuxGetTicks() {
    u64 result;

    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    result = time.tv_nsec + (time.tv_sec * 1000000000);
    return result;
}

function f64 LinuxGetElapsedTime(u64 start, u64 end) {
    f64 result = (end - start) / 1000000000.0;
    return result;
}

function void LinuxResetInput(Input *input) {
    if (linux_context->last_time == 0) { linux_context->last_time = LinuxGetTicks(); }

    for (u32 it = 0; it < Key_Count; ++it) {
        input->keys[it].transitions = 0;
    }

    for (u32 it = 0; it < Mouse_Count; ++it) {
        input->mouse_buttons[it].transitions = 0;
    }

    input->mouse_delta = V3(0, 0, 0);
    input->delta_time  = 0;
}

function b32 LinuxIgnoreKeyRepeat(Display *display, XEvent *event) {
    b32 result = false;

    if ((event->type == KeyRelease) && XEventsQueued(display, QueuedAfterReading)) {
        XEvent next;
        XPeekEvent(display, &next);

        result = (next.type == KeyPress) && (next.xkey.time == event->xkey.time) && (next.xkey.keycode == event->xkey.keycode);
        if (result) { XNextEvent(display, &next); }
    }

    return result;
}

function Key_Code LinuxTranslateKeysym(XKeyEvent *xkey) {
    Key_Code result = Key_Unknown;

    KeySym keysym = XLookupKeysym(xkey, 0);

    if (keysym >= 'a' && keysym <= 'z') {
        result = cast(Key_Code) (Key_A + (keysym - 'a'));
    }
    else if (keysym >= '0' && keysym <= '9') {
        result = cast(Key_Code) (Key_0 + (keysym - '0'));
    }
    else if (keysym >= XK_F1 && keysym <= XK_F12) {
        result = cast(Key_Code) (Key_F1 + (keysym - XK_F1));
    }
    else {
        switch (keysym) {
            // Punctuation
            //
            case ' ':  { result = Key_Space;        } break;
            case '`':  { result = Key_Grave;        } break;
            case '-':  { result = Key_Minus;        } break;
            case '=':  { result = Key_Equals;       } break;
            case '[':  { result = Key_LeftBracket;  } break;
            case ']':  { result = Key_RightBracket; } break;
            case ';':  { result = Key_Semicolon;    } break;
            case '\'': { result = Key_Quote;        } break;
            case '#':  { result = Key_Hash;         } break;
            case '\\': { result = Key_Backslash;    } break;
            case ',':  { result = Key_Comma;        } break;
            case '.':  { result = Key_Peroid;       } break;
            case '/':  { result = Key_Slash;        } break;

            // Arrow keys
            //
            case XK_Up:    { result = Key_Up;    } break;
            case XK_Down:  { result = Key_Down;  } break;
            case XK_Left:  { result = Key_Left;  } break;
            case XK_Right: { result = Key_Right; } break;

            // Other keys
            //
            case XK_Shift_L:
            case XK_Shift_R: { result = Key_Shift;  } break;

            case XK_Control_L:
            case XK_Control_R: { result = Key_Control; } break;

            case XK_Alt_L:
            case XK_Alt_R: { result = Key_Alt; } break;

            case XK_Super_L:
            case XK_Super_R: { result = Key_Super; } break;

            case XK_Tab:       { result = Key_Tab;       } break;
            case XK_Return:    { result = Key_Enter;     } break;
            case XK_BackSpace: { result = Key_Backspace; } break;
            case XK_Escape:    { result = Key_Escape;    } break;
        }
    }

    return result;
}

function Mouse_Button LinuxTranslateMouseButton(s32 button) {
    Mouse_Button result = Mouse_Unknown;
    switch (button) {
        case Button1: { result = Mouse_Left;   } break;
        case Button2: { result = Mouse_Middle; } break;
        case Button3: { result = Mouse_Right;  } break;
        case Button8: { result = Mouse_X1;     } break;
        case Button9: { result = Mouse_X2;     } break;
    }

    return result;
}

function void LinuxHandleButtonEvent(Input_Button *button, b32 is_pressed) {
    if (button->pressed != is_pressed) {
        button->pressed      = is_pressed;
        button->transitions += 1;
    }
}

function void LinuxHandleInput(Input *input) {
    LinuxResetInput(input);

    u64 current_time   = LinuxGetTicks();
    input->delta_time  = LinuxGetElapsedTime(linux_context->last_time, current_time);
    input->time       += input->delta_time;
    input->ticks       = current_time;

    linux_context->last_time = current_time;

    Xlib_Context *xlib = &linux_context->xlib;
    while (XPending(xlib->display)) {
        XEvent event;
        XNextEvent(xlib->display, &event);

        switch (event.type) {
            case ClientMessage: {
                Atom message = cast(Atom) event.xclient.data.l[0];
                if (message == xlib->closed) {
                    linux_context->running = false;
                    input->requested_quit  = true;
                }
            }
            break;

            case ConfigureNotify: {
                linux_context->window_dim = V2U(event.xconfigure.width, event.xconfigure.height);
            }
            break;

            case KeyPress:
            case KeyRelease: {
                if (!LinuxIgnoreKeyRepeat(xlib->display, &event)) {
                    b32 is_pressed = (event.type == KeyPress);
                    Key_Code code  = LinuxTranslateKeysym(&event.xkey);

                    if (code != Key_Unknown) {
                        LinuxHandleButtonEvent(&input->keys[code], is_pressed);
                    }
                }
            }
            break;

            case ButtonPress:
            case ButtonRelease: {
                s32 button = event.xbutton.button;

                b32 is_pressed = (event.type == ButtonPress);
                Mouse_Button mbutton = LinuxTranslateMouseButton(button);

                if (mbutton != Mouse_Unknown) {
                    LinuxHandleButtonEvent(&input->mouse_buttons[mbutton], is_pressed);
                }
                else if (is_pressed) {
                    switch (button) {
                        case Button4: { input->mouse_delta.z += 1; } break;
                        case Button5: { input->mouse_delta.z -= 1; } break;
                    }
                }
            }
            break;

            case MotionNotify: {
                v2 mouse_clip;
                mouse_clip.x = -1.0f + (2.0f * (event.xmotion.x / cast(f32) linux_context->window_dim.x));
                mouse_clip.y =  1.0f - (2.0f * (event.xmotion.y / cast(f32) linux_context->window_dim.y));

                v2 old_clip = input->mouse_clip;

                input->mouse_clip      = mouse_clip;
                input->mouse_delta.xy += (mouse_clip - old_clip);
            }
            break;
        }
    }
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
    XLIB_LOAD_FUNCTION(XEventsQueued);
    XLIB_LOAD_FUNCTION(XPeekEvent);
    XLIB_LOAD_FUNCTION(XLookupKeysym);
#undef XLIB_LOAD_FUNCTION

    result = true;
    return result;
}

function b32 LinuxLoadPASimpleFunctions(Pulse_Context *pa) {
    b32 result = false;

    pa->so_handle = dlopen("libpulse-simple.so", RTLD_NOW);
    if (!pa->so_handle) { return result; }

#define PA_LOAD_FUNCTION(name) pa->_##name = (type_##name *) dlsym(pa->so_handle, #name); do { if (!pa->_##name) { return result; } } while (0)
    PA_LOAD_FUNCTION(pa_simple_new);
    PA_LOAD_FUNCTION(pa_simple_free);
    PA_LOAD_FUNCTION(pa_simple_write);
    PA_LOAD_FUNCTION(pa_simple_get_latency);
    PA_LOAD_FUNCTION(pa_simple_flush);
#undef PA_LOAD_FUNCTION

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

    Platform->GetPath  = LinuxGetPath;
    Platform->ListPath = LinuxListPath;

    Platform->OpenFile  = LinuxOpenFile;
    Platform->CloseFile = LinuxCloseFile;

    Platform->ReadFile  = LinuxReadFile;
    Platform->WriteFile = LinuxWriteFile;

    linux_context->arena.alloc = Platform->GetMemoryAllocator();

    linux_context->last_time = 0;

    if (pthread_key_create(&linux_context->tls_handle, 0) != 0) {
        return result;
    }

    linux_context->thread_count    = 1;
    linux_context->thread_contexts = AllocArray(&linux_context->arena, Thread_Context, linux_context->thread_count);

    LinuxInitialiseThreadContext(&linux_context->thread_contexts[0]);
    pthread_setspecific(linux_context->tls_handle, cast(void *) &linux_context->thread_contexts[0]);

    Scratch_Memory scratch = GetScratch();
    const char *window_name = CopyZ(scratch.arena, params->window_title);

    if (open_window) {
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

        XStoreName(xlib->display, xlib->window, window_name);

        XMapWindow(xlib->display, xlib->window);

        linux_context->window_dim = params->window_dim;
    }

    if (enable_audio) {
        Pulse_Context *pa = &linux_context->pa;
        if (LinuxLoadPASimpleFunctions(pa)) {
            pa->channel_count = 2;
            pa->sample_rate   = 48000;

            pa->max_audio_frames = 800;

            pa->sample_buffer    = AllocArray(&linux_context->arena, s16, pa->channel_count * pa->max_audio_frames);

            pa_sample_spec spec = {};
            spec.format   = PA_SAMPLE_S16LE;
            spec.channels = pa->channel_count;
            spec.rate     = pa->sample_rate;

            pa->handle = pa_simple_new(0, window_name, PA_STREAM_PLAYBACK, 0, "Game Audio", &spec, 0, 0, 0);
        }

        pa->enabled = (pa->handle != 0);
    }

    linux_context->running = true;

    result = true;
    return result;
}

function Renderer_Context *LinuxLoadRenderer(Renderer_Parameters *params) {
    Renderer_Context *result = 0;

    str8 exe_path = Platform->GetPath(PlatformPath_Executable);
    if (!IsValid(exe_path)) { return result; }

    Scratch_Memory scratch = GetScratch();

    str8 full_path = FormatStr(scratch.arena, "%.*s/renderer_glx.so", str8_unpack(exe_path));

    void *renderer_so = dlopen(cast(const char *) full_path.data, RTLD_NOW);
    if (!renderer_so) {
        return result;
    }

    Renderer_Initialise *Initialise = cast(Renderer_Initialise *) dlsym(renderer_so, "LinuxOpenGLInitialise");
    if (!Initialise) {
        return result;
    }

    params->platform_data[0] = cast(void *) linux_context->xlib.display;
    *cast(Window *) &params->platform_data[1] = linux_context->xlib.window;

    params->platform_alloc = Platform->GetMemoryAllocator();

    result = Initialise(params);
    return result;
}

function v2u LinuxGetWindowDim() {
    v2u result = linux_context->window_dim;
    return result;
}

// This setup "works" for me. It will only work properly at 60hz.
//
// @Todo: Use a different audio api, all of them are awful anyway
//
function void LinuxGetAudioBuffer(Audio_Buffer *buffer) {
    Pulse_Context *pa = &linux_context->pa;
    if (!pa->enabled) { return; }

    buffer->samples      = pa->sample_buffer;
    buffer->sample_count = pa->max_audio_frames;
}

function void LinuxSubmitAudioBuffer(Audio_Buffer *buffer) {
    Pulse_Context *pa = &linux_context->pa;
    if (!pa->enabled) { return; }
    if (!buffer->sample_count) { return; }

    pa_simple_write(pa->handle, buffer->samples, buffer->sample_count * pa->channel_count * sizeof(s16), 0);
}
