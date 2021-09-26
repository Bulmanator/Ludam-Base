#if !defined(BASE_RENDERER_H_)
#define BASE_RENDERER_H_

struct Renderer_Context;
struct Renderer_Parameters;

#define RENDERER_INITIALISE(name) Renderer_Context *name(Renderer_Parameters *params)
#define RENDERER_SHUTDOWN(name) void name(Renderer_Context *renderer)

typedef RENDERER_INITIALISE(Renderer_Initialise);
typedef RENDERER_SHUTDOWN(Renderer_Shutdown);

struct Renderer_Buffer;

#define RENDERER_BEGIN_FRAME(name) Renderer_Buffer *name(Renderer_Context *renderer, v2u window_dim, rect2 render_region)
#define RENDERER_SUBMIT_FRAME(name) void name(Renderer_Context *renderer)

typedef RENDERER_BEGIN_FRAME(Renderer_Begin_Frame);
typedef RENDERER_SUBMIT_FRAME(Renderer_Submit_Frame);

enum Renderer_Context_Flags {
    RendererContext_Initialised = (1 << 0),
};

struct Renderer_Context {
    u32 flags;

    Renderer_Initialise *Initialise;
    Renderer_Shutdown   *Shutdown;

    Renderer_Begin_Frame    *BeginFrame;
    Renderer_Submit_Frame   *SubmitFrame;
};

struct Renderer_Parameters {
    uptr command_buffer_size;
    u32  max_immediate_quads;

    // @Note: These don't need to be filled out by the user
    //
    void *platform_data[2];
    Memory_Allocator *platform_alloc;
};

enum Render_Command_Type {
    RenderCommand_Render_Command_Clear = 0,
    RenderCommand_Render_Command_Camera_Transform,
    RenderCommand_Render_Command_Quad_Batch
};

struct Render_Command_Clear {
    v4 colour;
    f32 depth;
};

struct Render_Command_Camera_Transform {
    m4x4 transform;
};

struct Render_Command_Quad_Batch {
    u32 vertex_offset;
    u32 vertex_count;

    u32 index_offset;
    u32 index_count;
};

struct Renderer_Setup {
    v2u   window_dim;
    rect2 render_region;

    b32 vsync_enabled;
};

struct Renderer_Buffer {
    Renderer_Setup setup;

    u8  *buffer_base;
    uptr buffer_size;
    uptr buffer_used;

    vert3 *immediate_vertices;
    u32    max_immediate_vertices;
    u32    num_immediate_vertices;

    u16  *immediate_indices;
    u32   max_immediate_indices;
    u32   num_immediate_indices;
};

#endif  // BASE_RENDERER_H_
