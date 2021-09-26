function b32 IsValid(Renderer_Context *renderer) {
    b32 result = (renderer != 0) && (renderer->flags & RendererContext_Initialised);
    return result;
}

#define RenderCommand(batch, T) (T *) __RenderCommand(batch, sizeof(T), RenderCommand_##T)
function void *__RenderCommand(Draw_Batch *batch, uptr size, Render_Command_Type type) {
    void *result = 0;

    Renderer_Buffer *buffer = batch->buffer;
    uptr total_size = size + sizeof(u32);
    if ((buffer->buffer_used + total_size) <= buffer->buffer_size) {
        u32 *header = cast(u32 *) (buffer->buffer_base + buffer->buffer_used);
        *header = cast(u32) type;

        result = cast(void *) (header + 1);

        buffer->buffer_used += total_size;
    }

    return result;
}

function void SetCameraTransform(Draw_Batch *batch, u32 flags, v3 x, v3 y, v3 z, v3 p, f32 near_plane, f32 far_plane, f32 fov) {
    b32 is_ortho = (flags & DrawTransform_Orthographic);

    Draw_Transform *tx = &batch->game_tx;
    tx->x = x;
    tx->y = y;
    tx->z = z;
    tx->p = p;

    v2 window_dim = V2(batch->buffer->setup.window_dim);
    f32 aspect = (window_dim.w / window_dim.h);

    m4x4_inv transform;
    if (is_ortho) {
        transform = OrthographicProjection(aspect, near_plane, far_plane);
    }
    else {
        transform = PerspectiveProjection(fov, aspect, near_plane, far_plane);
    }

    m4x4_inv camera = CameraTransform(x, y, z, p);

    transform.forward = (transform.forward * camera.forward);
    transform.inverse = (camera.inverse * transform.forward);

    tx->transform = transform;

    batch->quad_batch = 0;

    Render_Command_Camera_Transform *camera_transform = RenderCommand(batch, Render_Command_Camera_Transform);
    if (camera_transform) {
        camera_transform->transform = transform.forward;
    }
}

function void DrawClear(Draw_Batch *batch, v4 colour, f32 depth) {
    Render_Command_Clear *clear = RenderCommand(batch, Render_Command_Clear);
    if (clear) {
        clear->colour = colour;
        clear->depth  = depth;
    }
}

function Render_Command_Quad_Batch *DrawQuadBatch(Draw_Batch *batch, u32 quad_count) {
    Render_Command_Quad_Batch *result = batch->quad_batch;

    Renderer_Buffer *buffer = batch->buffer;

    u32 required_vertices = (4 * quad_count);
    u32 required_indices  = (4 * quad_count);

    // We can't fit the number of quads in to the immeidate buffer, so return null and nothing will
    // be rendered
    //
    if (((buffer->num_immediate_vertices + required_vertices) > buffer->max_immediate_vertices) ||
        ((buffer->num_immediate_indices  + required_indices)  > buffer->max_immediate_indices))
    {
        result = batch->quad_batch = 0;
        return result;
    }

    if (!result || ((result->index_count + required_indices) > U16_MAX)) {
        result = RenderCommand(batch, Render_Command_Quad_Batch);

        if (result) {
            result->vertex_offset = buffer->num_immediate_vertices;
            result->vertex_count  = 0;

            result->index_offset  = buffer->num_immediate_indices;
            result->index_count   = 0;

            batch->quad_batch = result;
        }
    }

    return result;
}

function void DrawQuad(Draw_Batch *batch, vert3 vt0, vert3 vt1, vert3 vt2, vert3 vt3) {
    Render_Command_Quad_Batch *quad_batch = DrawQuadBatch(batch, 1);
    if (quad_batch) {
        Renderer_Buffer *buffer = batch->buffer;

        vert3 *vertices = &buffer->immediate_vertices[buffer->num_immediate_vertices];
        u16   *indices  = &buffer->immediate_indices[buffer->num_immediate_indices];

        vertices[0] = vt0;
        vertices[1] = vt1;
        vertices[2] = vt2;
        vertices[3] = vt3;

        // @Todo: Assert that vertex_count < U16_MAX here
        //
        u16 offset = cast(u16) quad_batch->vertex_count;

        indices[0] = offset + 0;
        indices[1] = offset + 1;
        indices[2] = offset + 3;

        indices[3] = offset + 1;
        indices[4] = offset + 3;
        indices[5] = offset + 2;

        buffer->num_immediate_vertices += 4;
        buffer->num_immediate_indices  += 6;

        quad_batch->vertex_count += 4;
        quad_batch->index_count  += 6;
    }
}
