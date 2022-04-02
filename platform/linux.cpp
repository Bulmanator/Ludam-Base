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

function void LinuxHandleButtonEvent(Input_Button *button, b32 is_pressed) {
    if (button->pressed != is_pressed) {
        button->pressed      = is_pressed;
        button->transitions += 1;
    }
}

function Key_Code TranslateKeyCode(SDL_Keycode code) {
    Key_Code result = Key_Unknown;
    if (code >= SDLK_a && code <= SDLK_z) {
        result = cast(Key_Code) (Key_A + (code - SDLK_a));
    }
    else if (code >= SDLK_F1 && code <= SDLK_F12) {
        result = cast(Key_Code) (Key_F1 + (code - SDLK_F1));
    }
    else if (code >= SDLK_0 && code <= SDLK_9) {
        result = cast(Key_Code) (Key_0 + (code - SDLK_0));
    }
    else {
        switch (code) {
            case SDLK_SPACE:        { result = Key_Space;     } break;
            case SDLK_BACKQUOTE:    { result = Key_Grave;     } break;
            case SDLK_MINUS:        { result = Key_Minus;     } break;
            case SDLK_EQUALS:       { result = Key_Equals;    } break;
            case SDLK_LEFTBRACKET:  { result = Key_LBracket;  } break;
            case SDLK_RIGHTBRACKET: { result = Key_RBracket;  } break;
            case SDLK_SEMICOLON:    { result = Key_Semicolon; } break;
            case SDLK_QUOTE:        { result = Key_Quote;     } break;
            case SDLK_HASH:         { result = Key_Hash;      } break;
            case SDLK_BACKSLASH:    { result = Key_Backslash; } break;
            case SDLK_COMMA:        { result = Key_Comma;     } break;
            case SDLK_PERIOD:       { result = Key_Period;    } break;
            case SDLK_SLASH:        { result = Key_Slash;     } break;

            case SDLK_UP:    { result = Key_Up;    } break;
            case SDLK_DOWN:  { result = Key_Down;  } break;
            case SDLK_LEFT:  { result = Key_Left;  } break;
            case SDLK_RIGHT: { result = Key_Right; } break;

            case SDLK_LSHIFT:
            case SDLK_RSHIFT:    { result = Key_Shift;     } break;
            case SDLK_LCTRL:
            case SDLK_RCTRL:     { result = Key_Ctrl;      } break;
            case SDLK_LALT:
            case SDLK_RALT:      { result = Key_Alt;       } break;
            case SDLK_LGUI:
            case SDLK_RGUI:      { result = Key_Super;     } break;
            case SDLK_TAB:       { result = Key_Tab;       } break;
            case SDLK_RETURN:    { result = Key_Enter;     } break;
            case SDLK_BACKSPACE: { result = Key_Backspace; } break;
            case SDLK_ESCAPE:    { result = Key_Esc;       } break;

            default: {} break;
        }
    }

    return result;
}

function void LinuxHandleInput(Input *input) {
    LinuxResetInput(input);

    u64 current_time   = LinuxGetTicks();
    input->delta_time  = LinuxGetElapsedTime(linux_context->last_time, current_time);
    input->time       += input->delta_time;
    input->ticks       = current_time;

    linux_context->last_time = current_time;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT: {
                input->requested_quit = true;
                linux_context->running = false;
            }
            break;

            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                if (!e.key.repeat) {
                    Key_Code kc = TranslateKeyCode(e.key.keysym.sym);
                    if (kc != Key_Unknown) {
                        LinuxHandleButtonEvent(&input->keys[kc], e.type == SDL_KEYDOWN);
                    }
                }
            }
            break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                if (e.button.button < Mouse_Count) {
                    Mouse_Button mb = cast(Mouse_Button) (cast(s8) e.button.button - 1);
                    if (mb != Mouse_Unknown) {
                        LinuxHandleButtonEvent(&input->mouse_buttons[mb], e.type == SDL_MOUSEBUTTONDOWN);
                    }
                }
            }
            break;

            case SDL_MOUSEMOTION: {
                v2 mouse_p = V2(e.motion.x, e.motion.y);

                v2 window_dim = V2(LinuxGetWindowDim());

                v2 mouse_clip;
                mouse_clip.x = -1.0f + (2.0f * (mouse_p.x / window_dim.w));
                mouse_clip.y = -1.0f + (2.0f * (mouse_p.y / window_dim.h));

                v2 mouse_delta = input->mouse_clip - mouse_clip;

                input->mouse_clip      = mouse_clip;
                input->mouse_delta.xy += mouse_delta;
            }
            break;

            case SDL_MOUSEWHEEL: {
                input->mouse_delta.z += e.wheel.y;
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
        linux_context->window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, params->window_dim.w, params->window_dim.h, SDL_WINDOW_OPENGL);
        if (!linux_context->window) { return result; }
    }

    if (enable_audio) {
        SDL_AudioSpec desired = {};
        desired.freq     = 48000;
        desired.format   = AUDIO_S16LSB;
        desired.channels = 2;
        desired.samples  = 512;
        desired.callback = 0;
        desired.userdata = 0;

        SDL_AudioSpec got = {};

        linux_context->audio_device = SDL_OpenAudioDevice(0, SDL_FALSE, &desired, &got, 0);

        linux_context->max_sample_count = 12000;
        linux_context->sound_samples    = AllocArray(&linux_context->arena, s16, linux_context->max_sample_count);
    }

    linux_context->running = true;

    result = true;
    return result;
}

function v2u LinuxGetWindowDim() {
    v2u result = V2U(0, 0);
    if (!linux_context->window) { return result; }

    SDL_GetWindowSize(linux_context->window, (s32 *) &result.w, (s32 *) &result.h);
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

    params->platform_data[0] = cast(void *) linux_context->window;
    params->platform_alloc   = Platform->GetMemoryAllocator();

    result = Initialise(params);
    return result;
}

function void LinuxGetAudioBuffer(Audio_Buffer *buffer) {
    uptr frame_sample_size = (linux_context->max_sample_count * sizeof(s16)) - SDL_GetQueuedAudioSize(linux_context->audio_device);

    buffer->samples      = linux_context->sound_samples;
    buffer->sample_count = frame_sample_size / sizeof(s16);
}

function void LinuxSubmitAudioBuffer(Audio_Buffer *buffer) {
    if (linux_context->audio_device != 0) {
        uptr frame_sample_size = buffer->sample_count * sizeof(s16);
        SDL_QueueAudio(linux_context->audio_device, buffer->samples, frame_sample_size);
    }
}
