#include <X11/Xlib.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <base/types.h>
#include <base/memory.h>
#include <base/string.h>

#include <base/renderer.h>

#include <base/memory.cpp>
#include <base/string.cpp>

#include "opengl.h"
#include "opengl.cpp"

#define GLX_LOAD_FUNCTION(name) name = (type_##name *) glXGetProcAddress((const GLubyte *) #name)

// Provided by GLX_ARB_create_context or GLX_ARB_create_context_profile
//
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#define GLX_CONTEXT_FLAGS_ARB         0x2094
#define GLX_CONTEXT_PROFILE_MASK_ARB  0x9126

#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

typedef GLXContext type_glXCreateContextAttribsARB(Display *, GLXFBConfig, GLXContext, Bool, const int *);

// Provided by GLX_EXT_swap_control
//
#define GLX_SWAP_INTERVAL_EXT     0x20F1
#define GLX_MAX_SWAP_INTERVAL_EXT 0x20F2

typedef void type_glXSwapIntervalEXT(Display *, GLXDrawable, int);

// GLX function pointers
//
global type_glXCreateContextAttribsARB *glXCreateContextAttribsARB;
global type_glXSwapIntervalEXT         *glXSwapIntervalEXT;

function b32 GLXInitialise(OpenGL_Context *gl, Display *display, Window window) {
    b32 result = false;

    str8 extensions = WrapZ(cast(u8 *) glXQueryExtensionsString(display, DefaultScreen(display)));
    while (extensions.count != 0) {
        str8 extension;
        extension.data  = extensions.data;
        extension.count = 0;

        while (extension.count <= extensions.count) {
            if (extension.data[extension.count] == ' ') { break; }
            extension.count += 1;
        }

        if (StringsEqual(extension, WrapConst("GLX_ARB_framebuffer_sRGB"))) {
            gl->info.srgb_supported = true;
        }
        else if (StringsEqual(extension, WrapConst("GLX_EXT_framebuffer_sRGB"))) {
            gl->info.srgb_supported = true;
        }
        else if (StringsEqual(extension, WrapConst("GLX_ARB_multisample"))) {
            gl->info.multisample_supported = true;
        }

        extensions = Advance(extensions, extension.count + 1);
    }

    // @Todo: This is such a mess. The official Nvidia drivers break completley when passing the colour sizes as
    // frambuffer attributes. It searches and finds framebuffer configs just fine, it loads the context create function and
    // even makes the context with these framebuffer configurations but as soon as it tries to make the context current it
    // throws a BadMatch error and crashes the program without recovery.
    //
    // I don't really have the will or paitence to deal with this right now so I am just going
    // to exclude the colour sizes from the attributes
    //
    // Works fine with the colour sizes on nouveau
    //

    int attrib_count = 16;
    int framebuffer_attrs[32] = {
        GLX_X_RENDERABLE,  True,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
#if 0
        GLX_RED_SIZE,      8,
        GLX_BLUE_SIZE,     8,
        GLX_GREEN_SIZE,    8,
        GLX_ALPHA_SIZE,    8,
#endif
        GLX_DEPTH_SIZE,    24,
        GLX_STENCIL_SIZE,  8,
        GLX_DOUBLEBUFFER,  True,
        0
    };

    if (gl->info.multisample_supported) {
        framebuffer_attrs[attrib_count + 0] = GLX_SAMPLE_BUFFERS_ARB;
        framebuffer_attrs[attrib_count + 1] = 1;

        framebuffer_attrs[attrib_count + 2] = GLX_SAMPLES_ARB;
        framebuffer_attrs[attrib_count + 3] = 4;

        attrib_count += 4;
    }

    if (gl->info.srgb_supported) {
        framebuffer_attrs[attrib_count + 0] = GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB;
        framebuffer_attrs[attrib_count + 1] = True;

        attrib_count += 2;
    }

    framebuffer_attrs[attrib_count] = 0;

    int config_count = 0;
    GLXFBConfig *configs = 0;

    configs = glXChooseFBConfig(display, DefaultScreen(display), framebuffer_attrs, &config_count);
    if (!configs || config_count < 1) {
        if (gl->info.srgb_supported) {
            // @Hack: There are legitimatly drivers that _say_ they support GLX_ARB_framebuffer_sRGB or
            // GLX_EXT_framebuffer_sRGB, but then when you go to actually request an sRGB framebuffer it doesn't
            // find any valid configurations. So if it fails and we find that sRGB is supposed to be supported
            // but failed then disable it.
            //
            // Buuuuutttt, don't actually disable (i.e. don't set gl->info.srgb_supported to false) beacause
            // even though when you request it to be there and it fails to find a config just enabling
            // GL_FRAMEBUFFER_SRGB works fine showing that IT GAVE YOU AN sRGB FRAMEBUFFER ANYWAY..........
            //
            framebuffer_attrs[attrib_count - 2] = 0;

            configs = glXChooseFBConfig(display, DefaultScreen(display), framebuffer_attrs, &config_count);
            if (!configs || config_count < 0) {
                return result;
            }
        }
        else {
            return result;
        }
    }

    GLX_LOAD_FUNCTION(glXCreateContextAttribsARB);

    int context_flags = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

    const int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_FLAGS_ARB,         context_flags,
        GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    GLXContext context = glXCreateContextAttribsARB(display, configs[0], 0, True, context_attribs);
    if (!context) {
        return result;
    }

    result = glXMakeCurrent(display, window, context);

    if (result) {
        GLX_LOAD_FUNCTION(glGenVertexArrays);
        GLX_LOAD_FUNCTION(glGenBuffers);
        GLX_LOAD_FUNCTION(glBindVertexArray);
        GLX_LOAD_FUNCTION(glBindBuffer);
        GLX_LOAD_FUNCTION(glVertexAttribPointer);
        GLX_LOAD_FUNCTION(glEnableVertexAttribArray);
        GLX_LOAD_FUNCTION(glBufferData);
        GLX_LOAD_FUNCTION(glShaderSource);
        GLX_LOAD_FUNCTION(glCompileShader);
        GLX_LOAD_FUNCTION(glGetShaderiv);
        GLX_LOAD_FUNCTION(glGetShaderInfoLog);
        GLX_LOAD_FUNCTION(glDeleteShader);
        GLX_LOAD_FUNCTION(glAttachShader);
        GLX_LOAD_FUNCTION(glLinkProgram);
        GLX_LOAD_FUNCTION(glGetProgramiv);
        GLX_LOAD_FUNCTION(glGetProgramInfoLog);
        GLX_LOAD_FUNCTION(glDeleteProgram);
        GLX_LOAD_FUNCTION(glUseProgram);
        GLX_LOAD_FUNCTION(glUniformMatrix4fv);
        GLX_LOAD_FUNCTION(glDrawElementsBaseVertex);
        GLX_LOAD_FUNCTION(glGenerateMipmap);

        GLX_LOAD_FUNCTION(glMapBuffer);

        GLX_LOAD_FUNCTION(glUnmapBuffer);

        GLX_LOAD_FUNCTION(glGetUniformLocation);

        GLX_LOAD_FUNCTION(glCreateProgram);
        GLX_LOAD_FUNCTION(glCreateShader);

        GLX_LOAD_FUNCTION(glXSwapIntervalEXT);
    }

    return result;
}


function RENDERER_BEGIN_FRAME(GLXBeginFrame) {
    Renderer_Buffer *result = 0;

    OpenGL_Context *gl = cast(OpenGL_Context *) renderer;
    result = OpenGLBeginFrame(gl, window_dim, render_region);

    return result;
}

function RENDERER_SUBMIT_FRAME(GLXSubmitFrame) {
    OpenGL_Context *gl = cast(OpenGL_Context *) renderer;
    OpenGLSubmitFrame(gl);

    Display *display    = glXGetCurrentDisplay();
    GLXDrawable window  = glXGetCurrentDrawable();

    glXSwapBuffers(display, window);
}

function RENDERER_SHUTDOWN(LinuxOpenGLShutdown) {
    OpenGL_Context *gl = cast(OpenGL_Context *) renderer;
    if (gl) {
        Display *display   = glXGetCurrentDisplay();
        GLXContext context = glXGetCurrentContext();

        glXMakeCurrent(0, 0, 0);
        glXDestroyContext(display, context);

        Reset(&gl->arena, true);
        gl = 0;
    }
}

extern "C" RENDERER_INITIALISE(LinuxOpenGLInitialise) {
    Renderer_Context *result = 0;

    OpenGL_Context *gl = AllocInline(params->platform_alloc, Megabytes(512), OpenGL_Context, arena);
    if (!gl) { return result; }

    result             = &gl->renderer;
    result->flags      = 0;

    result->Initialise = LinuxOpenGLInitialise;
    result->Shutdown   = LinuxOpenGLShutdown;

    Display *display = cast(Display *) params->platform_data[0];
    Window   window  = *cast(Window *) &params->platform_data[1];

    if (!GLXInitialise(gl, display, window)) {
        return result;
    }

    if (glXSwapIntervalEXT) {
        // @Todo: There are two of them!
        //
    }

    if (!OpenGLInitialise(gl, params)) {
        return result;
    }

    result->BeginFrame   = GLXBeginFrame;
    result->SubmitFrame  = GLXSubmitFrame;
    result->flags       |= RendererContext_Initialised;

    return result;
}

