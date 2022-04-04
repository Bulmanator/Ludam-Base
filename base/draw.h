#if !defined(BASE_DRAW_H_)
#define BASE_DRAW_H_

enum Draw_Transform_Flags {
    DrawTransform_Orthographic = (1 << 0),
};

struct Draw_Transform {
    v3 x;
    v3 y;
    v3 z;
    v3 p;

    m4x4_inv transform; // Combined projection & view
};

struct Draw_Batch {
    Draw_Transform game_tx;

    Asset_Manager *assets;

    Render_Target target;
    Render_Target mask;
    b32 reverse_mask;

    Renderer_Buffer *buffer;
    Render_Command_Quad_Batch *quad_batch;
};

struct Sprite_Animation {
    Image_Handle image;

    f32 time;
    f32 time_per_frame;

    u32 rows;
    u32 cols;

    u32 current_frame;
};

// Checks if the result of loading a renderer has been initialised correctly and is ready for use
//
function b32 IsValid(Renderer_Context *renderer);

// Initialises a draw batch for use with the rest of the draw system
//
function void Initialise(Draw_Batch *batch, Asset_Manager *assets, Renderer_Buffer *renderer_buffer);

// Sets the camera transform to use. Constructed by the basis vectors provided as the coordinate system axes
// in x, y, z placed at position p in the world.
//
function void SetCameraTransform(Draw_Batch *batch, u32 flags = 0, v3 x = V3(1, 0, 0), v3 y = V3(0, 1, 0),
        v3 z = V3(0, 0, 1), v3 p = V3(0, 0, 0), f32 near = 0.1f, f32 far = 1000.0f, f32 fov = Radians(50));

// Get the world space position to the corresponding clip space coordinate with the given z value. If
// the z value is omitted the it will use the camera z value
//
function v3 Unproject(Draw_Transform *tx, v3 clip);
function v3 Unproject(Draw_Transform *tx, v2 clip);

// Get the bounds of the camera frustum at the given z value. If the z value is omitted it will use the
// camera z value
//
function rect3 GetCameraFrustum(Draw_Transform *tx, f32 z);
function rect3 GetCameraFrustum(Draw_Transform *tx);

// Clears the screen and the depth buffer to the values supplied
//
function void DrawClear(Draw_Batch *batch, v4 colour = V4(0, 0, 0, 1), f32 depth = 1.0f);

// Sets the render target
//
function void SetRenderTarget(Draw_Batch *batch, Render_Target target = RenderTarget_Default);

// Sets the target to mask to
//
function void SetMaskTarget(Draw_Batch *batch, Render_Target mask = RenderTarget_Default, b32 reverse = false);

// Base DrawQuad call. Draws a quad with the specified vertex data
//
function void DrawQuad(Draw_Batch *batch, Image_Handle image, vert3 vt0, vert3 vt1, vert3 vt2, vert3 vt3, b32 is_circle = false);

// More general DrawQuad calls
//
function void DrawQuad(Draw_Batch *batch, Image_Handle image, v3 centre, v2 dim, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));
function void DrawQuad(Draw_Batch *batch, Image_Handle image, v2 centre, v2 dim, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));

function void DrawQuad(Draw_Batch *batch, Image_Handle image, v3 centre, f32 scale, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));
function void DrawQuad(Draw_Batch *batch, Image_Handle image, v2 centre, f32 scale, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));

function void DrawCircle(Draw_Batch *batch, Image_Handle image, v3 centre, f32 radius, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));
function void DrawCircle(Draw_Batch *batch, Image_Handle image, v2 centre, f32 radius, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));

// Quad outlines
//
function void DrawQuadOutline(Draw_Batch *batch, v2 centre, v2 dim, f32 angle = 0, v4 colour = V4(1, 1, 1, 1), f32 thickness = 0.05f);

// Line segments
//
function void DrawLine(Draw_Batch *batch, v2 start, v2 end, v4 start_colour = V4(1, 1, 1, 1), v4 end_colour = V4(1, 1, 1, 1), f32 thickness = 0.05f);

// Animation functions
//
function void Initialise(Sprite_Animation *animation, Image_Handle image, u32 rows, u32 cols, f32 time_per_frame = 0.05f);

function void UpdateAnimation(Sprite_Animation *animation, f32 dt);

function void DrawAnimation(Draw_Batch *batch, Sprite_Animation *animation, v3 centre, v2 scale, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));
function void DrawAnimation(Draw_Batch *batch, Sprite_Animation *animation, v2 centre, v2 scale, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));

#endif  // BASE_DRAW_H_
