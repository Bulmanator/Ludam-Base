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

function void Initialise(Draw_Batch *batch, Asset_Manager *assets, Renderer_Buffer *renderer_buffer) {
    batch->game_tx = {};

    batch->buffer  = renderer_buffer;
    batch->assets  = assets;

    batch->quad_batch = 0;
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
    transform.inverse = (camera.inverse * transform.inverse);

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

function v3 Unproject(Draw_Transform *tx, v3 clip) {
    v3 result = {};

    v4 probe = tx->transform.forward * V4(tx->p - (clip.z * tx->z), 1.0f);
    clip.xy *= probe.w;

    v4 world = tx->transform.inverse * V4(clip.x, clip.y, probe.z, probe.w);
    result = world.xyz;

    return result;
}

function v3 Unproject(Draw_Transform *tx, v2 clip) {
    v3 result = Unproject(tx, V3(clip, tx->p.z));
    return result;
}

function rect3 GetCameraFrustum(Draw_Transform *tx, f32 z) {
    rect3 result;
    result.min = Unproject(tx, V3(-1, -1, z));
    result.max = Unproject(tx, V3( 1,  1, z));

    return result;
}

function rect3 GetCameraFrustum(Draw_Transform *tx) {
    rect3 result;
    result.min = Unproject(tx, V3(-1, -1, tx->p.z));
    result.max = Unproject(tx, V3( 1,  1, tx->p.z));

    return result;
}

function Render_Command_Quad_Batch *DrawQuadBatch(Draw_Batch *batch, Texture_Handle texture, u32 quad_count) {
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

    if (!result || ((result->vertex_count + 3) > U16_MAX) || (result->texture.value != texture.value)) {
        result = RenderCommand(batch, Render_Command_Quad_Batch);

        if (result) {
            result->vertex_offset = buffer->num_immediate_vertices;
            result->vertex_count  = 0;

            result->index_offset  = buffer->num_immediate_indices;
            result->index_count   = 0;

            result->texture       = texture;

            batch->quad_batch = result;
        }
    }

    return result;
}

function void DrawQuad(Draw_Batch *batch, Image_Handle image, vert3 vt0, vert3 vt1, vert3 vt2, vert3 vt3) {
    Texture_Handle texture = GetImageData(batch->assets, image);

    Render_Command_Quad_Batch *quad_batch = DrawQuadBatch(batch, texture, 1);
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

function void DrawQuad(Draw_Batch *batch, Image_Handle image, v3 centre, v2 dim, f32 angle, v4 colour) {
    vert3 vt[4] = {};

    // @Todo: I think this needs to be converted to sRGB space before doing this, so maybe we have
    // to check if sRGB is enabled or not first and pack normally if not
    //
    u32 ucolour  = ABGRPack(colour);
    v2  rot      = Arm2(angle);
    v2  half_dim = 0.5f * dim;

    vt[0].p  = V3(Rotate(-half_dim, rot)) + centre;
    vt[0].uv = V2(0, 0);
    vt[0].c  = ucolour;

    vt[1].p  = V3(Rotate(V2(-half_dim.x, half_dim.y), rot)) + centre;
    vt[1].uv = V2(0, 1);
    vt[1].c  = ucolour;

    vt[2].p  = V3(Rotate(half_dim, rot)) + centre;
    vt[2].uv = V2(1, 1);
    vt[2].c  = ucolour;

    vt[3].p  = V3(Rotate(V2(half_dim.x, -half_dim.y), rot)) + centre;
    vt[3].uv = V2(1, 0);
    vt[3].c  = ucolour;

    DrawQuad(batch, image, vt[0], vt[1], vt[2], vt[3]);
}

function void DrawQuad(Draw_Batch *batch, Image_Handle image, v2 centre, v2 dim, f32 angle, v4 colour) {
    DrawQuad(batch, image, V3(centre), dim, angle, colour);
}

function void DrawQuad(Draw_Batch *batch, Image_Handle image, v3 centre, f32 scale, f32 angle, v4 colour) {
    Amt_Image *image_info = GetImageInfo(batch->assets, image);
    if (!image_info) { return; }

    v2 dim = V2(image_info->width, image_info->height);
    if (dim.w > dim.h) {
        dim.h = dim.h / dim.w;
        dim.w = 1;
    }
    else {
        dim.w = dim.w / dim.h;
        dim.h = 1;
    }

    dim *= scale;

    DrawQuad(batch, image, centre, dim, angle, colour);
}

function void DrawQuad(Draw_Batch *batch, Image_Handle image, v2 centre, f32 scale, f32 angle, v4 colour) {
    DrawQuad(batch, image, V3(centre), scale, angle, colour);
}

function void DrawQuadOutline(Draw_Batch *batch, v2 centre, v2 dim, f32 angle, v4 colour, f32 thickness) {
    v2 half_dim = 0.5f * dim;
    v2 rot      = Arm2(angle);

    v2 p[4];

    p[0] = Rotate(-half_dim, rot) + centre;
    p[1] = Rotate(V2(-half_dim.x, half_dim.y), rot) + centre;
    p[2] = Rotate(half_dim, rot) + centre;
    p[3] = Rotate(V2(half_dim.x, -half_dim.y), rot) + centre;

    for (u32 it = 0; it < 4; ++it) {
        v2 extra = thickness * Noz(p[it] - p[(it + 1) % 4]);
        DrawLine(batch, p[it], p[(it + 1) % 4] - extra, colour, colour, thickness);
    }
}

function void DrawLine(Draw_Batch *batch, v2 start, v2 end, v4 start_colour, v4 end_colour, f32 thickness) {
    v2 perp = Perp(Noz(end - start));

    u32 ustart_colour = ABGRPack(start_colour);
    u32 uend_colour   = ABGRPack(end_colour);

    vert3 vt[4];

    vt[0].p  = V3(start);
    vt[0].uv = V2(0, 0);
    vt[0].c  = ustart_colour;

    vt[1].p  = V3(start + (thickness * perp));
    vt[1].uv = V2(1, 0);
    vt[1].c  = ustart_colour;

    vt[2].p  = V3(end + (thickness * perp));
    vt[2].uv = V2(1, 1);
    vt[2].c  = uend_colour;

    vt[3].p  = V3(end);
    vt[3].uv = V2(0, 1);
    vt[3].c  = uend_colour;

    DrawQuad(batch, { 0 }, vt[0], vt[1], vt[2], vt[3]);
}

// Animation functions
//
function void Initialise(Sprite_Animation *animation, Image_Handle image, u32 rows, u32 cols, f32 time_per_frame) {
    animation->image = image;

    animation->time  = 0;
    animation->time_per_frame = time_per_frame;

    animation->rows = rows;
    animation->cols = cols;

    animation->current_frame = 0;
}

function void UpdateAnimation(Sprite_Animation *animation, f32 dt) {
    animation->time += dt;
    if (animation->time >= animation->time_per_frame) {
        animation->time = 0;
        animation->current_frame += 1;
    }
}

function void DrawAnimation(Draw_Batch *batch, Sprite_Animation *animation, v3 centre, v2 scale, f32 angle, v4 colour) {
    Amt_Image *image_info = GetImageInfo(batch->assets, animation->image);
    if (!image_info) { return; }

    v2 dim = V2(image_info->width / cast(f32) animation->cols, image_info->height / cast(f32) animation->rows);
    if (dim.w > dim.h) {
        dim.h = dim.h / dim.w;
        dim.w = 1;
    }
    else {
        dim.w = dim.w / dim.h;
        dim.h = 1;
    }

    dim *= scale;

    v2 half_dim = 0.5f * dim;
    v2 rot      = Arm2(angle);

    u32 ucolour = ABGRPack(colour);

    u32 total_frames = animation->rows * animation->cols;
    u32 frame = animation->current_frame % total_frames;

    u32 row = cast(u32) ((cast(f32) frame) / cast(f32) animation->cols);
    u32 col = animation->current_frame % animation->cols;

    v2 uv_dim = V2(1.0f / cast(f32) animation->cols, 1.0f / cast(f32) animation->rows);
    v2 uv_min = uv_dim * V2(col, row);

    vert3 vt[4];

    vt[0].p  = V3(Rotate(-half_dim, rot)) + centre;
    vt[0].uv = uv_min;
    vt[0].c  = ucolour;

    vt[1].p  = V3(Rotate(V2(-half_dim.x, half_dim.y), rot)) + centre;
    vt[1].uv = V2(uv_min.x, uv_min.y + uv_dim.y);
    vt[1].c  = ucolour;

    vt[2].p  = V3(Rotate(half_dim, rot)) + centre;
    vt[2].uv = uv_min + uv_dim;
    vt[2].c  = ucolour;

    vt[3].p  = V3(Rotate(V2(half_dim.x, -half_dim.y), rot)) + centre;
    vt[3].uv = V2(uv_min.x + uv_dim.x, uv_min.y);
    vt[3].c  = ucolour;

    DrawQuad(batch, animation->image, vt[0], vt[1], vt[2], vt[3]);
}

function void DrawAnimation(Draw_Batch *batch, Sprite_Animation *animation, v2 centre, v2 scale, f32 angle, v4 colour) {
    DrawAnimation(batch, animation, V3(centre), scale, angle, colour);
}


