#include <SDL2/SDL_video.h>
#include <GL/gl.h>

#include <base/types.h>
#include <base/memory.h>
#include <base/string.h>

#include <base/renderer.h>

#include <base/memory.cpp>
#include <base/string.cpp>

#include "opengl.h"
#include "opengl.cpp"

#define GLX_LOAD_FUNCTION(name) name = (type_##name *) SDL_GL_GetProcAddress(#name)

global SDL_Window *g_window;
global SDL_GLContext g_gl_context;

function b32 GLXInitialise(OpenGL_Context *gl, SDL_Window *window) {
    b32 result = false;

    g_window = window;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    g_gl_context = SDL_GL_CreateContext(window);

    if (g_gl_context) {
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
    }

    result = g_gl_context != 0;
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


    SDL_GL_SwapWindow(g_window);
}

function RENDERER_SHUTDOWN(LinuxOpenGLShutdown) {
    OpenGL_Context *gl = cast(OpenGL_Context *) renderer;
    if (gl) {
        SDL_GL_DeleteContext(g_gl_context);

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

    SDL_Window *window = cast(SDL_Window *) params->platform_data[0];

    if (!GLXInitialise(gl, window)) {
        return result;
    }

    if (!OpenGLInitialise(gl, params)) {
        return result;
    }

    result->BeginFrame   = GLXBeginFrame;
    result->SubmitFrame  = GLXSubmitFrame;
    result->flags       |= RendererContext_Initialised;

    return result;
}

