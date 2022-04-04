#if !defined(RENDERER_OPENGL_H_)
#define RENDERER_OPENGL_H_

struct OpenGL_Framebuffer {
    GLuint handle;

    GLuint attachments[2];
};

struct OpenGL_Info {
    str8 vendor;
    str8 renderer;

    b32 srgb_supported;
    b32 multisample_supported;
};

struct OpenGL_Quad_Program {
    GLuint handle;

    GLint transform;    // mat4

    GLint image;        // sampler2D

    GLint is_circle;    // bool
    GLint circle_fade;  // float

    GLint use_mask;     // bool
    GLint reverse_mask; // bool

    GLint mask;         // sampler2D

    GLint screen_dim;   // vec2
};

struct OpenGL_Context {
    Renderer_Context renderer;
    Memory_Arena arena;

    OpenGL_Info info;

    Renderer_Buffer command_buffer;

    OpenGL_Quad_Program program;

    GLuint immediate_vao;
    GLuint immediate_vbo;
    GLuint immediate_ebo;

    OpenGL_Framebuffer framebuffers[4];
#if 0
    GLuint program;
    GLint  program_transform_loc;

    GLuint circle_program;
    GLint  circle_fade;
    GLint  circle_tx;

    OpenGL_Framebuffer mask_fb;
    OpenGL_Framebuffer masked_fb;

    GLuint fullscreen_vao;
    GLuint fullscreen_quad;

    GLuint mask_program;
    GLint  mask_texture_unit;
    GLint  masked_texture_unit;
    GLint  mask_reverse;
#endif

    // Texture stuff
    //
    GLenum texture_format;

    u32     max_texture_handles;
    GLuint *texture_handles;
    GLuint  err_texture_handle;
};

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

#define GL_FRAMEBUFFER_SRGB     0x8DB9
#define GL_SRGB8_ALPHA8_EXT     0x8C43
#define GL_MULTISAMPLE          0x809D
#define GL_ARRAY_BUFFER         0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STREAM_DRAW          0x88E0
#define GL_STATIC_DRAW          0x88E4
#define GL_DYNAMIC_DRAW         0x88E8
#define GL_FRAGMENT_SHADER      0x8B30
#define GL_VERTEX_SHADER        0x8B31
#define GL_COMPILE_STATUS       0x8B81
#define GL_LINK_STATUS          0x8B82
#define GL_BGRA                 0x80E1
#define GL_WRITE_ONLY           0x88B9
#define GL_CLAMP_TO_EDGE        0x812F
#define GL_FRAMEBUFFER          0x8D40
#define GL_COLOR_ATTACHMENT0    0x8CE0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_TEXTURE0             0x84C0
#define GL_TEXTURE1             0x84C1
#define GL_DEPTH_ATTACHMENT     0x8D00
#define GL_R8                   0x8229
#define GL_DEPTH24_STENCIL8     0x88F0
#define GL_UNSIGNED_INT_24_8    0x84FA
#define GL_DEPTH_STENCIL        0x84F9

typedef void type_glGenVertexArrays(GLsizei, GLuint *);
typedef void type_glGenBuffers(GLsizei, GLuint *);
typedef void type_glBindVertexArray(GLuint);
typedef void type_glBindBuffer(GLenum, GLuint);
typedef void type_glVertexAttribPointer(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
typedef void type_glEnableVertexAttribArray(GLuint);
typedef void type_glBufferData(GLenum, GLsizeiptr, const GLvoid *, GLenum);
typedef void type_glShaderSource(GLuint, GLsizei, const GLchar **, const GLint *);
typedef void type_glCompileShader(GLuint);
typedef void type_glGetShaderiv(GLuint, GLenum, GLint *);
typedef void type_glGetShaderInfoLog(GLuint, GLsizei, GLsizei *, GLchar *);
typedef void type_glDeleteShader(GLuint);
typedef void type_glAttachShader(GLuint, GLuint);
typedef void type_glLinkProgram(GLuint);
typedef void type_glGetProgramiv(GLuint, GLenum, GLint *);
typedef void type_glGetProgramInfoLog(GLuint, GLsizei, GLsizei *, GLchar *);
typedef void type_glDeleteProgram(GLuint);
typedef void type_glUseProgram(GLuint);
typedef void type_glUniformMatrix4fv(GLint, GLsizei, GLboolean, const GLfloat *);
typedef void type_glDrawElementsBaseVertex(GLenum, GLsizei, GLenum, void *, GLint);
typedef void type_glGenerateMipmap(GLenum);
typedef void type_glGenFramebuffers(GLsizei, GLuint *);
typedef void type_glBindFramebuffer(GLenum, GLuint);
typedef void type_glFramebufferTexture2D(GLenum, GLenum, GLenum, GLuint, GLint);
typedef void type_glActiveTexture(GLenum);
typedef void type_glUniform1i(GLint, GLint);
typedef void type_glUniform1f(GLint, GLfloat);
typedef void type_glUniform2f(GLint, GLfloat, GLfloat);
typedef void type_glDeleteFramebuffers(GLsizei, const GLuint *);

typedef void *type_glMapBuffer(GLenum, GLenum);

typedef GLboolean type_glUnmapBuffer(GLenum);

typedef GLint type_glGetUniformLocation(GLuint, const GLchar *);

typedef GLuint type_glCreateProgram();
typedef GLuint type_glCreateShader(GLenum);

typedef GLenum type_glCheckFramebufferStatus(GLenum);

#define GLOBAL_OPENGL_FUNCTION(name) static type_##name *name
GLOBAL_OPENGL_FUNCTION(glGenVertexArrays);
GLOBAL_OPENGL_FUNCTION(glGenBuffers);
GLOBAL_OPENGL_FUNCTION(glBindVertexArray);
GLOBAL_OPENGL_FUNCTION(glBindBuffer);
GLOBAL_OPENGL_FUNCTION(glVertexAttribPointer);
GLOBAL_OPENGL_FUNCTION(glEnableVertexAttribArray);
GLOBAL_OPENGL_FUNCTION(glBufferData);
GLOBAL_OPENGL_FUNCTION(glShaderSource);
GLOBAL_OPENGL_FUNCTION(glCompileShader);
GLOBAL_OPENGL_FUNCTION(glGetShaderiv);
GLOBAL_OPENGL_FUNCTION(glGetShaderInfoLog);
GLOBAL_OPENGL_FUNCTION(glDeleteShader);
GLOBAL_OPENGL_FUNCTION(glAttachShader);
GLOBAL_OPENGL_FUNCTION(glLinkProgram);
GLOBAL_OPENGL_FUNCTION(glGetProgramiv);
GLOBAL_OPENGL_FUNCTION(glGetProgramInfoLog);
GLOBAL_OPENGL_FUNCTION(glDeleteProgram);
GLOBAL_OPENGL_FUNCTION(glUseProgram);
GLOBAL_OPENGL_FUNCTION(glUniformMatrix4fv);
GLOBAL_OPENGL_FUNCTION(glDrawElementsBaseVertex);
GLOBAL_OPENGL_FUNCTION(glGenerateMipmap);
GLOBAL_OPENGL_FUNCTION(glGenFramebuffers);
GLOBAL_OPENGL_FUNCTION(glBindFramebuffer);
GLOBAL_OPENGL_FUNCTION(glFramebufferTexture2D);
GLOBAL_OPENGL_FUNCTION(glUniform1i);
GLOBAL_OPENGL_FUNCTION(glUniform1f);
GLOBAL_OPENGL_FUNCTION(glUniform2f);
GLOBAL_OPENGL_FUNCTION(glDeleteFramebuffers);

#if !defined(__linux__)
    GLOBAL_OPENGL_FUNCTION(glActiveTexture);
#endif

GLOBAL_OPENGL_FUNCTION(glMapBuffer);

GLOBAL_OPENGL_FUNCTION(glUnmapBuffer);

GLOBAL_OPENGL_FUNCTION(glGetUniformLocation);

GLOBAL_OPENGL_FUNCTION(glCreateProgram);
GLOBAL_OPENGL_FUNCTION(glCreateShader);

GLOBAL_OPENGL_FUNCTION(glCheckFramebufferStatus);
#undef GLOBAL_OPENGL_FUNCTION

function b32 OpenGLInitialise(OpenGL_Context *gl, Renderer_Parameters *params);
function b32 OpenGLBuildFramebuffers(OpenGL_Context *gl, v2u screen_dim);

function b32 OpenGLCompileProgram(GLuint *handle, const GLchar *vertex_code, const GLchar *fragment_code);
function b32 OpenGLCompileQuadProgram(OpenGL_Quad_Program *program);
function b32 OpenGLCompileCircleProgram(OpenGL_Context *gl);
function b32 OpenGLCompileMaskProgram(OpenGL_Context *gl);

function Renderer_Buffer *OpenGLBeginFrame(OpenGL_Context *gl, v2u window_dim, rect2 render_region);
function void OpenGLSubmitFrame(OpenGL_Context *gl);

function GLuint OpenGLGetTextureHandle(OpenGL_Context *gl, Texture_Handle handle);
function void OpenGLTransferTextures(OpenGL_Context *gl, Texture_Transfer_Queue *texture_queue);

function OpenGL_Framebuffer *OpenGLGetFramebuffer(OpenGL_Context *gl, Render_Target target);

//function void OpenGLResolveMasks(OpenGL_Context *gl, b32 reverse);

#endif  // RENDERER_OPENGL_H_
