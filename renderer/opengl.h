#if !defined(RENDERER_OPENGL_H_)
#define RENDERER_OPENGL_H_

struct OpenGL_Info {
    str8 vendor;
    str8 renderer;

    b32 srgb_supported;
};

struct OpenGL_Context {
    Renderer_Context renderer;
    Memory_Arena arena;

    OpenGL_Info info;
};

#endif  // RENDERER_OPENGL_H_
