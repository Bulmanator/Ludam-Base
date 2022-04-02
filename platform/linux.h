#if defined(function)
#    undef function
#endif

#include <sys/mman.h>
#include <sys/stat.h>

#include <unistd.h>
#include <errno.h>

#include <dirent.h>
#include <fcntl.h>

#include <dlfcn.h>
#include <pthread.h>

#include <SDL2/SDL.h>

#define function static

struct Linux_Context {
    Platform_Context platform;

    SDL_Window *window;
    SDL_AudioDeviceID audio_device;

    u32 max_sample_count;
    s16 *sound_samples;

    b32 running;

    Memory_Allocator alloc;
    Memory_Arena arena;

    pthread_key_t tls_handle;
    u32 thread_count;
    Thread_Context *thread_contexts;

    u64 last_time;

    str8 exe_path;
    str8 user_path;
    str8 working_path;
};

global Linux_Context *linux_context;

struct Linux_Parameters {
    u32 init_flags;

    str8 window_title;
    v2u  window_dim;
};

// This must be called before using any platform layer code
//
function b32 LinuxInitialise(Linux_Parameters *params);

// Will process keyboard and mouse input, get frame delta time and handle quit events
//
function void LinuxHandleInput(Input *input);

// Loads the default renderer. OpenGL for now.
//
function Renderer_Context *LinuxLoadRenderer(Renderer_Parameters *parameters);

// Gets the current window size
//
function v2u LinuxGetWindowDim();

// Gets the required number of samples to output as well as a backing buffer to write those samples into
//
function void LinuxGetAudioBuffer(Audio_Buffer *buffer);

// Submits the audio buffer for playback
//
function void LinuxSubmitAudioBuffer(Audio_Buffer *buffer);
