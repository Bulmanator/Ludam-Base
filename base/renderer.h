#if !defined(BASE_RENDERER_H_)
#define BASE_RENDERER_H_

struct Renderer_Context;
struct Renderer_Parameters;

#define RENDERER_INITIALISE(name) Renderer_Context *name(Renderer_Parameters *params)
#define RENDERER_SHUTDOWN(name) void name(Renderer_Context *renderer)

typedef RENDERER_INITIALISE(Renderer_Initialise);
typedef RENDERER_SHUTDOWN(Renderer_Shutdown);

enum Renderer_Context_Flags {
    RendererContext_Initialised = (1 << 0),
};

struct Renderer_Context {
    u32 flags;

    Renderer_Initialise *Initialise;
    Renderer_Shutdown   *Shutdown;
};

struct Renderer_Parameters {
    uptr command_buffer_size;
    u32  max_immediate_quads;

    // @Note: These don't need to be filled out by the user
    //
    void *platform_data[2];
    Memory_Allocator *platform_alloc;
};

#endif  // BASE_RENDERER_H_
