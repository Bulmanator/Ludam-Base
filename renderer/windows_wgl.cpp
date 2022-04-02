#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include <gl/gl.h>

#include <base/types.h>
#include <base/memory.h>
#include <base/string.h>

#include <base/renderer.h>

#include <base/memory.cpp>
#include <base/string.cpp>

#include "opengl.h"
#include "opengl.cpp"

global HGLRC global_gl_context;
global HDC   global_dc;

#define WGL_LOAD_FUNCTION(name) name = (type_##name *) wglGetProcAddress(#name)

// Provided by WGL_ARB_pixel_format
//
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB   0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB  0x2011
#define WGL_PIXEL_TYPE_ARB     0x2013
#define WGL_COLOR_BITS_ARB     0x2014
#define WGL_ALPHA_BITS_ARB     0x201B
#define WGL_DEPTH_BITS_ARB     0x2022
#define WGL_STENCIL_BITS_ARB   0x2023

#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB         0x202B

typedef BOOL type_wglChoosePixelFormatARB(HDC, const int *, const FLOAT *, UINT, int *, UINT *);

// Provided by WGL_ARB_create_context or WGL_ARB_create_context_profile
//
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB   0x2093
#define WGL_CONTEXT_FLAGS_ARB         0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB              0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

typedef HGLRC type_wglCreateContextAttribsARB(HDC, HGLRC, const int *);

// Provided by WGL_ARB_framebuffer_sRGB or WGL_EXT_framebuffer_sRGB
//
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20A9

// Provided by WGL_ARB_extensions_string
//
typedef const char *type_wglGetExtensionsStringARB(HDC);

// Provided by WGL_EXT_swap_control
//
typedef BOOL type_wglSwapIntervalEXT(int);

// Provided by WGL_ARB_multisample
//
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB        0x2042

// WGL function pointers
//
global type_wglChoosePixelFormatARB    *wglChoosePixelFormatARB;
global type_wglCreateContextAttribsARB *wglCreateContextAttribsARB;
global type_wglGetExtensionsStringARB  *wglGetExtensionsStringARB;
global type_wglSwapIntervalEXT         *wglSwapIntervalEXT;

function b32 WGLSetPixelFormat(HDC dc, b32 enable_srgb, b32 enable_multisample) {
    b32 result = false;

    int format_index;

    if (wglChoosePixelFormatARB) {
        int attrib_count = 18;
        int format_attribs[32] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_COLOR_BITS_ARB,     32,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            WGL_STENCIL_BITS_ARB,   8,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB
        };

        if (enable_srgb) {
            format_attribs[attrib_count + 0] = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
            format_attribs[attrib_count + 1] = GL_TRUE;

            attrib_count += 2;
        }

        if (enable_multisample) {
            format_attribs[attrib_count + 0] = WGL_SAMPLE_BUFFERS_ARB;
            format_attribs[attrib_count + 1] = 1;

            format_attribs[attrib_count + 2] = WGL_SAMPLES_ARB;
            format_attribs[attrib_count + 3] = 4;

            attrib_count += 4;
        }

        format_attribs[attrib_count] = 0;

        UINT format_count;
        if (!wglChoosePixelFormatARB(dc, format_attribs, 0, 1, &format_index, &format_count)) {
            return result;
        }
        else if (format_count == 0) {
            return result;
        }
    }
    else {
        PIXELFORMATDESCRIPTOR desired_format = {};
        desired_format.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
        desired_format.nVersion     = 1;
        desired_format.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        desired_format.iPixelType   = PFD_TYPE_RGBA;
        desired_format.cColorBits   = 32;
        desired_format.cAlphaBits   = 8;
        desired_format.cDepthBits   = 24;
        desired_format.cStencilBits = 8;

        format_index = ChoosePixelFormat(dc, &desired_format);
        if (!format_index) {
            return result;
        }
    }

    result = SetPixelFormat(dc, format_index, 0);
    return result;
}

function b32 WGLInitialise(OpenGL_Context *gl, HINSTANCE instance, HWND window) {
    b32 result = false;

    // To initialise a modern OpenGL context using WGL we have to first load some extension
    // functions, this requires setting the pixel format on our window. You can only set the pixel
    // format on a window once so to initially load these extension functions we have to make a dummy
    // window, set the pixel format on that, make a old OpenGL context, load the extension functions
    // and then setup the modern OpenGL context on our actual window.
    //
    WNDCLASSA dummy_class = {};
    dummy_class.style         = 0;
    dummy_class.lpfnWndProc   = DefWindowProcA;
    dummy_class.cbClsExtra    = 0;
    dummy_class.cbWndExtra    = 0;
    dummy_class.hInstance     = instance;
    dummy_class.hIcon         = 0;
    dummy_class.hbrBackground = 0;
    dummy_class.lpszMenuName  = 0;
    dummy_class.lpszClassName = "dummy_wgl_class";

    if (!RegisterClassA(&dummy_class)) {
        return result;
    }

    HWND dummy_window;
    HDC dummy_dc;
    HGLRC dummy_context;

    dummy_window = CreateWindowExA(0, dummy_class.lpszClassName, "dummy wgl window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);
    if (!dummy_window) { return result; }

    dummy_dc = GetDC(dummy_window);
    if (!WGLSetPixelFormat(dummy_dc, false, false)) { return result; }

    dummy_context = wglCreateContext(dummy_dc);
    if (!dummy_dc) { return result; }

    if (!wglMakeCurrent(dummy_dc, dummy_context)) { return result; }

    WGL_LOAD_FUNCTION(wglChoosePixelFormatARB);
    WGL_LOAD_FUNCTION(wglCreateContextAttribsARB);
    WGL_LOAD_FUNCTION(wglGetExtensionsStringARB);
    WGL_LOAD_FUNCTION(wglSwapIntervalEXT);

    str8 extensions = WrapZ(cast(u8 *) wglGetExtensionsStringARB(dummy_dc));
    while (extensions.count != 0) {
        str8 extension;
        extension.data  = extensions.data;
        extension.count = 0;

        while (extension.count <= extensions.count) {
            if (extension.data[extension.count] == ' ') { break; }
            extension.count += 1;
        }

        // Some drivers expose the ARB extension while others expose the EXT extension. As the ARB one
        // is supposed to supercede the EXT one if drivers were actually conformant we would only
        // have to check for ARB
        //
        if (StringsEqual(extension, WrapConst("WGL_ARB_framebuffer_sRGB"))) {
            gl->info.srgb_supported = true;
        }
        else if (StringsEqual(extension, WrapConst("WGL_EXT_framebuffer_sRGB"))) {
            gl->info.srgb_supported = true;
        }
        else if (StringsEqual(extension, WrapConst("WGL_ARB_multisample"))) {
            gl->info.multisample_supported = true;
        }

        extensions = Advance(extensions, extension.count + 1);
    }

    wglMakeCurrent(0, 0);
    wglDeleteContext(dummy_context);

    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);
    UnregisterClass(dummy_class.lpszClassName, instance);

    global_dc = GetDC(window);
    if (!WGLSetPixelFormat(global_dc, gl->info.srgb_supported, gl->info.multisample_supported)) {
        return result;
    }

    int context_flags = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

    int context_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB,         context_flags,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    global_gl_context = wglCreateContextAttribsARB(global_dc, 0, context_attribs);
    if (!global_gl_context) { return result; }

    result = wglMakeCurrent(global_dc, global_gl_context);

    WGL_LOAD_FUNCTION(glGenVertexArrays);
    WGL_LOAD_FUNCTION(glGenBuffers);
    WGL_LOAD_FUNCTION(glBindVertexArray);
    WGL_LOAD_FUNCTION(glBindBuffer);
    WGL_LOAD_FUNCTION(glVertexAttribPointer);
    WGL_LOAD_FUNCTION(glEnableVertexAttribArray);
    WGL_LOAD_FUNCTION(glBufferData);
    WGL_LOAD_FUNCTION(glShaderSource);
    WGL_LOAD_FUNCTION(glCompileShader);
    WGL_LOAD_FUNCTION(glGetShaderiv);
    WGL_LOAD_FUNCTION(glGetShaderInfoLog);
    WGL_LOAD_FUNCTION(glDeleteShader);
    WGL_LOAD_FUNCTION(glAttachShader);
    WGL_LOAD_FUNCTION(glLinkProgram);
    WGL_LOAD_FUNCTION(glGetProgramiv);
    WGL_LOAD_FUNCTION(glGetProgramInfoLog);
    WGL_LOAD_FUNCTION(glDeleteProgram);
    WGL_LOAD_FUNCTION(glUseProgram);
    WGL_LOAD_FUNCTION(glUniformMatrix4fv);
    WGL_LOAD_FUNCTION(glDrawElementsBaseVertex);
    WGL_LOAD_FUNCTION(glGenerateMipmap);
    WGL_LOAD_FUNCTION(glGenFramebuffers);
    WGL_LOAD_FUNCTION(glBindFramebuffer);
    WGL_LOAD_FUNCTION(glFramebufferTexture2D);
    WGL_LOAD_FUNCTION(glActiveTexture);
    WGL_LOAD_FUNCTION(glUniform1i);
    WGL_LOAD_FUNCTION(glUniform1f);

    WGL_LOAD_FUNCTION(glMapBuffer);

    WGL_LOAD_FUNCTION(glUnmapBuffer);

    WGL_LOAD_FUNCTION(glGetUniformLocation);

    WGL_LOAD_FUNCTION(glCreateProgram);
    WGL_LOAD_FUNCTION(glCreateShader);
    WGL_LOAD_FUNCTION(glCheckFramebufferStatus);

    return result;
}

function RENDERER_BEGIN_FRAME(WGLBeginFrame) {
    Renderer_Buffer *result = 0;

    OpenGL_Context *gl = cast(OpenGL_Context *) renderer;
    result = OpenGLBeginFrame(gl, window_dim, render_region);

    return result;
}

function RENDERER_SUBMIT_FRAME(WGLSubmitFrame) {
    OpenGL_Context *gl = cast(OpenGL_Context *) renderer;
    OpenGLSubmitFrame(gl);

    SwapBuffers(global_dc);
}

function RENDERER_SHUTDOWN(WindowsOpenGLShutdown) {
    OpenGL_Context *gl = cast(OpenGL_Context *) renderer;
    if (gl) {
        wglMakeCurrent(0, 0);
        wglDeleteContext(global_gl_context);

        Reset(&gl->arena, true);
        gl = 0;
    }
}

extern "C" __declspec(dllexport) RENDERER_INITIALISE(WindowsOpenGLInitialise) {
    Renderer_Context *result = 0;

    OpenGL_Context *gl = AllocInline(params->platform_alloc, Megabytes(512), OpenGL_Context, arena);
    if (!gl) { return result; }

    result             = &gl->renderer;
    result->flags      = 0;

    result->Initialise = WindowsOpenGLInitialise;
    result->Shutdown   = WindowsOpenGLShutdown;

    HINSTANCE instance = *cast(HINSTANCE *) &params->platform_data[0];
    HWND window        = *cast(HWND *) &params->platform_data[1];

    if (!WGLInitialise(gl, instance, window)) {
        return result;
    }

    if (wglSwapIntervalEXT) { wglSwapIntervalEXT(1); }

    if (!OpenGLInitialise(gl, params)) {
        return result;
    }

    result->BeginFrame   = WGLBeginFrame;
    result->SubmitFrame  = WGLSubmitFrame;
    result->flags       |= RendererContext_Initialised;

    return result;
}
