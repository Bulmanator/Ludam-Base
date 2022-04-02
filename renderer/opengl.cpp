function b32 OpenGLInitialise(OpenGL_Context *gl, Renderer_Parameters *params) {
    b32 result = false;

    //glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);

    if (gl->info.srgb_supported) {
        glEnable(GL_FRAMEBUFFER_SRGB);
        gl->texture_format = GL_SRGB8_ALPHA8_EXT;
    }
    else {
        gl->texture_format = GL_RGBA8;
    }

    // Generate buffers for immediate mode renderer
    //
    glGenVertexArrays(1, &gl->immediate_vao);
    glGenBuffers(1, &gl->immediate_vbo);
    glGenBuffers(1, &gl->immediate_ebo);

    glBindVertexArray(gl->immediate_vao);
    glBindBuffer(GL_ARRAY_BUFFER, gl->immediate_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl->immediate_ebo);

    // Setup vertex attributes
    //
    glVertexAttribPointer(0, 3, GL_FLOAT,         GL_FALSE, sizeof(vert3), cast(GLvoid *) OffsetTo(vert3, p));
    glVertexAttribPointer(1, 2, GL_FLOAT,         GL_FALSE, sizeof(vert3), cast(GLvoid *) OffsetTo(vert3, uv));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vert3), cast(GLvoid *) OffsetTo(vert3, c));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    Renderer_Buffer *command_buffer = &gl->command_buffer;

    u32 max_immediate_vertices = 4 * params->max_immediate_quads;
    u32 max_immediate_indices  = 6 * params->max_immediate_quads;

    glBufferData(GL_ARRAY_BUFFER, max_immediate_vertices * sizeof(vert3), 0, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_immediate_indices * sizeof(u16), 0, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Setup the command buffer for the game to use
    //
    command_buffer->buffer_base            = AllocArray(&gl->arena, u8, params->command_buffer_size);
    command_buffer->buffer_size            = params->command_buffer_size;
    command_buffer->buffer_used            = 0;

    command_buffer->immediate_vertices     = 0;
    command_buffer->max_immediate_vertices = max_immediate_vertices;
    command_buffer->num_immediate_vertices = 0;

    command_buffer->immediate_indices      = 0;
    command_buffer->max_immediate_indices  = max_immediate_indices;
    command_buffer->num_immediate_indices  = 0;

    // Setup texture transfer queue
    //
    Texture_Transfer_Queue *texture_queue = &gl->renderer.texture_queue;

    texture_queue->transfer_count = 0;

    texture_queue->transfer_size = params->texture_queue_size;
    texture_queue->transfer_used = 0;
    texture_queue->transfer_base = AllocArray(&gl->arena, u8, texture_queue->transfer_size);

    // Setup and generate texture handles
    //
    gl->max_texture_handles = params->max_texture_handles;
    gl->texture_handles     = AllocArray(&gl->arena, GLuint, gl->max_texture_handles);

    glGenTextures(gl->max_texture_handles, gl->texture_handles);

    // Create the default texture to be an error checkerboard pattern that is black and magenta for easy
    // visual error checking
    //
    u32 error_checkerboard[] = {
        0xFF00FFFF,
        0xFF000000,
        0xFF00FFFF,
        0xFF000000
    };

    glGenTextures(1, &gl->err_texture_handle);

    glBindTexture(GL_TEXTURE_2D, gl->err_texture_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, gl->texture_format, 2, 2, 0, GL_BGRA, GL_UNSIGNED_BYTE, error_checkerboard);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    if (!OpenGLCompileSimpleProgram(gl)) {
        return result;
    }

    if (!OpenGLCompileCircleProgram(gl)) {
        return result;
    }

    if (!OpenGLCompileMaskProgram(gl)) {
        return result;
    }

    f32 fullscreen_quad[] = {
        -1, -1, 0, 0,
        -1,  1, 0, 1,
         1,  1, 1, 1,

        -1, -1, 0, 0,
         1,  1, 1, 1,
         1, -1, 1, 0
    };

    glGenVertexArrays(1, &gl->fullscreen_vao);
    glGenBuffers(1, &gl->fullscreen_quad);

    glBindVertexArray(gl->fullscreen_vao);
    glBindBuffer(GL_ARRAY_BUFFER, gl->fullscreen_quad);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, cast(GLvoid *) 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, cast(GLvoid *) 8);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreen_quad), fullscreen_quad, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenFramebuffers(1, &gl->mask_fb.handle);
    glGenTextures(1, &gl->mask_fb.texture);

    glBindTexture(GL_TEXTURE_2D, gl->mask_fb.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, params->window_dim.w, params->window_dim.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, gl->mask_fb.handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl->mask_fb.texture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        return result;
    }

    glGenFramebuffers(1, &gl->masked_fb.handle);
    glGenTextures(1, &gl->masked_fb.texture);

    glBindTexture(GL_TEXTURE_2D, gl->masked_fb.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, params->window_dim.w, params->window_dim.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, gl->masked_fb.handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl->masked_fb.texture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        return result;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    result = true;
    return result;
}

function b32 OpenGLCompileProgram(GLuint *handle, const GLchar *vertex_code, const GLchar *fragment_code) {
    b32 result = false;

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_code, 0);
    glCompileShader(vertex_shader);

    s32 success = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(vertex_shader, sizeof(log), 0, log);
        glDeleteShader(vertex_shader);

        return result;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_code, 0);
    glCompileShader(fragment_shader);

    success = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(fragment_shader, sizeof(log), 0, log);
        glDeleteShader(fragment_shader);
        glDeleteShader(vertex_shader);

        return result;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), 0, log);
        glDeleteProgram(program);
    }
    else {
        *handle = program;
        result = true;
    }

    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);

    return result;
}

function b32 OpenGLCompileSimpleProgram(OpenGL_Context *gl) {
    b32 result = false;

    const GLchar *vertex_code = R"VERT(
        #version 330 core

        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 uv;
        layout(location = 2) in vec4 colour;

        out vec2 frag_uv;
        out vec4 frag_colour;

        uniform mat4 transform;

        void main() {
            gl_Position = transform * vec4(position, 1.0);
            frag_uv     = uv;
            frag_colour = colour;
        }
    )VERT";

    const GLchar *fragment_code = R"FRAG(
        #version 330 core

        in vec2 frag_uv;
        in vec4 frag_colour;

        out vec4 final_colour;

        uniform sampler2D image;

        void main() {
            final_colour = texture(image, frag_uv) * frag_colour;
        }
    )FRAG";

    result = OpenGLCompileProgram(&gl->program, vertex_code, fragment_code);
    if (result) {
        gl->program_transform_loc = glGetUniformLocation(gl->program, "transform");
        result = (gl->program_transform_loc != -1);
    }

    return result;
}

function b32 OpenGLCompileCircleProgram(OpenGL_Context *gl) {
    b32 result = false;

    const GLchar *vertex_code = R"VERT(
        #version 330 core

        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 uv;
        layout(location = 2) in vec4 colour;

        out vec2 frag_uv;
        out vec4 frag_colour;

        uniform mat4 transform;

        void main() {
            gl_Position = transform * vec4(position, 1.0);
            frag_uv     = uv;
            frag_colour = colour;
        }
    )VERT";

    const GLchar *fragment_code = R"FRAG(
        #version 330 core

        in vec2 frag_uv;
        in vec4 frag_colour;

        out vec4 final_colour;

        uniform sampler2D image;
        uniform float fade = 0.5;

        void main() {
            vec2 uv = (2.0 * frag_uv) - 1.0;
            float dist = 1.0 - length(uv);

            float alpha = smoothstep(0.0, fade, dist);
            //alpha *= smoothstep(1.0 + fade, 1.0, dist);

            final_colour    = frag_colour * texture(image, frag_uv);
            final_colour.a *= alpha;
        }
    )FRAG";

    result = OpenGLCompileProgram(&gl->circle_program, vertex_code, fragment_code);
    if (result) {
        gl->circle_tx   = glGetUniformLocation(gl->circle_program, "transform");
        gl->circle_fade = glGetUniformLocation(gl->circle_program, "fade");
    }

    return result;
}

function b32 OpenGLCompileMaskProgram(OpenGL_Context *gl) {
    b32 result = false;

    const GLchar *vertex_code = R"VERT(
        #version 330 core

        in vec2 p;
        in vec2 uv;

        out vec2 frag_uv;

        void main() {
            gl_Position = vec4(p, 0.0, 1.0);

            frag_uv = uv;
        }
    )VERT";

    const GLchar *fragment_code = R"FRAG(
        #version 330 core

        in vec2 frag_uv;

        out vec4 final_colour;

        uniform sampler2D mask;
        uniform sampler2D masked;

        uniform int reverse = 1;

        void main() {
            final_colour = texture(masked, frag_uv);

            float maskv;
            if (reverse != 0) {
                maskv = 1.0 - texture(mask, frag_uv).a; //step(0.5, texture(mask, frag_uv).a);
                final_colour.a *= maskv;
            }
            else {
                maskv = step(0.5, texture(mask, frag_uv).r);
            }

            final_colour.a *= maskv;
        }
    )FRAG";

    result = OpenGLCompileProgram(&gl->mask_program, vertex_code, fragment_code);

    if (result) {
        gl->mask_texture_unit   = glGetUniformLocation(gl->mask_program, "mask");
        gl->masked_texture_unit = glGetUniformLocation(gl->mask_program, "masked");
        gl->mask_reverse        = glGetUniformLocation(gl->mask_program, "reverse");
    }

    return result;
}

function Renderer_Buffer *OpenGLBeginFrame(OpenGL_Context *gl, v2u window_dim, rect2 render_region) {
    Renderer_Buffer *result = &gl->command_buffer;
    result->buffer_used        = 0;

    // Setup immediate vertices
    //
    glBindBuffer(GL_ARRAY_BUFFER, gl->immediate_vbo);
    result->immediate_vertices     = cast(vert3 *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    result->num_immediate_vertices = 0;

    // Setup immediate indices
    //
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl->immediate_ebo);
    result->immediate_indices     = cast(u16 *) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    result->num_immediate_indices = 0;

    Renderer_Setup *setup = &result->setup;
    setup->window_dim    = window_dim;
    setup->render_region = render_region;
    setup->vsync_enabled = true; // @Hardcoded: Allow this to be modified

    v2s viewport_pos;
    viewport_pos.x = cast(u32) render_region.min.x;
    viewport_pos.y = cast(u32) render_region.min.y;

    v2u viewport_dim;
    viewport_dim.w = cast(u32) (render_region.max.x - render_region.min.x);
    viewport_dim.h = cast(u32) (render_region.max.y - render_region.min.y);

    glViewport(viewport_pos.x, viewport_pos.y, viewport_dim.w, viewport_dim.h);

    return result;
}

function void OpenGLSubmitFrame(OpenGL_Context *gl) {
    Renderer_Buffer *buffer = &gl->command_buffer;

    OpenGLTransferTextures(gl, &gl->renderer.texture_queue);

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    m4x4 *current_tx = 0;
    for (uptr offset = 0; offset < buffer->buffer_used;) {
        u32 type = *cast(u32 *) (buffer->buffer_base + offset);
        offset += sizeof(u32);

        switch (type) {
            case RenderCommand_Render_Command_Clear: {
                Render_Command_Clear *clear = cast(Render_Command_Clear *) (buffer->buffer_base + offset);

                glClearColor(clear->colour.r, clear->colour.g, clear->colour.b, clear->colour.a);
                glClearDepth(clear->depth);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                offset += sizeof(Render_Command_Clear);
            }
            break;
            case RenderCommand_Render_Command_Camera_Transform: {
                Render_Command_Camera_Transform *transform = cast(Render_Command_Camera_Transform *) (buffer->buffer_base + offset);

                current_tx = &transform->transform;
                offset += sizeof(Render_Command_Camera_Transform);
            }
            break;
            case RenderCommand_Render_Command_Quad_Batch: {
                Render_Command_Quad_Batch *quad_batch = cast(Render_Command_Quad_Batch *) (buffer->buffer_base + offset);

                GLuint program;
                GLint  tx_loc;

                if (quad_batch->is_circle) {
                    program = gl->circle_program;
                    tx_loc  = gl->circle_tx;

                    glUniform1f(gl->circle_fade, 0.1f);
                }
                else {
                    program = gl->program;
                    tx_loc  = gl->program_transform_loc;
                }

                glUseProgram(program);

                glActiveTexture(GL_TEXTURE0);

                glBindVertexArray(gl->immediate_vao);
                glBindBuffer(GL_ARRAY_BUFFER, gl->immediate_vbo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl->immediate_ebo);

                GLuint texture_handle = OpenGLGetTextureHandle(gl, quad_batch->texture);
                glBindTexture(GL_TEXTURE_2D, texture_handle);

                glUniformMatrix4fv(tx_loc, 1, GL_TRUE, current_tx->e);
                glDrawElementsBaseVertex(GL_TRIANGLES, quad_batch->index_count, GL_UNSIGNED_SHORT, (void *) (quad_batch->index_offset * sizeof(u16)), quad_batch->vertex_offset);

                offset += sizeof(Render_Command_Quad_Batch);
            }
            break;
            case RenderCommand_Render_Command_Set_Target: {
                Render_Command_Set_Target *target = cast(Render_Command_Set_Target *) (buffer->buffer_base + offset);
                GLuint fb = OpenGLGetFramebuffer(gl, cast(Render_Target) target->target);

                glBindFramebuffer(GL_FRAMEBUFFER, fb);

                rect2 render_region = buffer->setup.render_region;

                v2s viewport_pos;
                viewport_pos.x = cast(s32) render_region.min.x;
                viewport_pos.y = cast(s32) render_region.min.y;

                v2u viewport_dim;
                viewport_dim.w = cast(u32) (render_region.max.x - render_region.min.x);
                viewport_dim.h = cast(u32) (render_region.max.y - render_region.min.y);

                glViewport(viewport_pos.x, viewport_pos.y, viewport_dim.w, viewport_dim.h);

                offset += sizeof(Render_Command_Set_Target);
            }
            break;
            case RenderCommand_Render_Command_Resolve_Masks: {
                Render_Command_Resolve_Masks *mask = cast(Render_Command_Resolve_Masks *) (buffer->buffer_base + offset);

                OpenGLResolveMasks(gl, mask->reverse);
                offset += sizeof(Render_Command_Resolve_Masks);
            }
            break;
        }
    }
}

function GLuint OpenGLGetTextureHandle(OpenGL_Context *gl, Texture_Handle handle) {
    GLuint result = 0;
    if (handle.index < gl->max_texture_handles) {
        result = gl->texture_handles[handle.index];
    }
    else {
        // Get error texture as we have an out of bounds texture
        //
        result = gl->err_texture_handle;
    }

    return result;
}

function void OpenGLTransferTextures(OpenGL_Context *gl, Texture_Transfer_Queue *texture_queue) {
    for (u32 it = 0; it < texture_queue->transfer_count; ++it) {
        Texture_Transfer_Info *info = &texture_queue->transfer_info[it];

        b32 filtered = (info->flags & TextureFlag_Filtered);
        b32 clamped  = (info->flags & TextureFlag_Clamped);

        Texture_Handle handle = info->handle;
        GLuint texture_handle = OpenGLGetTextureHandle(gl, handle);
        glBindTexture(GL_TEXTURE_2D, texture_handle);
        glTexImage2D(GL_TEXTURE_2D, 0, gl->texture_format, handle.width, handle.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, info->data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtered ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtered ? GL_LINEAR : GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamped ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamped ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    }

    texture_queue->transfer_count = 0;
    texture_queue->transfer_used  = 0;
}

function GLuint OpenGLGetFramebuffer(OpenGL_Context *gl, Render_Target target) {
    GLuint result;
    switch (target) {
        case RenderTarget_Mask: {
            result = gl->mask_fb.handle;
        }
        break;
        case RenderTarget_Masked: {
            result = gl->masked_fb.handle;
        }
        break;

        case RenderTarget_Default:
        default: {
            result = 0;
        }
        break;
    }

    return result;
}

function void OpenGLResolveMasks(OpenGL_Context *gl, b32 reverse) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthMask(GL_FALSE);

    glUseProgram(gl->mask_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl->mask_fb.texture);
    glUniform1i(gl->mask_texture_unit, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gl->masked_fb.texture);
    glUniform1i(gl->masked_texture_unit, 1);

    glUniform1i(gl->mask_reverse, reverse ? 0 : 1);

    glBindVertexArray(gl->fullscreen_vao);
    glBindBuffer(GL_ARRAY_BUFFER, gl->fullscreen_quad);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDepthMask(GL_TRUE);
}
