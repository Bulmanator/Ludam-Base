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

// Sets the camera transform to use. Constructed by the basis vectors provided as the coordinate system axes
// in x, y, z placed at position p in the world.
//
function void SetCameraTransform(Draw_Batch *batch, u32 flags = 0, v3 x = V3(1, 0, 0), v3 y = V3(0, 1, 0),
        v3 z = V3(0, 0, 1), v3 p = V3(0, 0, 0), f32 near = 0.1f, f32 far = 1000.0f, f32 fov = Radians(50));

// Clears the screen and the depth buffer to the values supplied
//
function void DrawClear(Draw_Batch *batch, v4 colour = V4(0, 0, 0, 1), f32 depth = 1.0f);

// Base DrawQuad call. Draws a quad with the specified vertex data
//
function void DrawQuad(Draw_Batch *batch, vert3 vt0, vert3 vt1, vert3 vt2, vert3 vt3);

// More general DrawQuad calls
//
function void DrawQuad(Draw_Batch *batch, v3 centre, v2 dim, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));
function void DrawQuad(Draw_Batch *batch, v2 centre, v2 dim, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));

// Quad outlines
//
function void DrawQuadOutline(Draw_Batch *batch, v3 centre, v2 dim, f32 angle = 0, v4 colour = V4(1, 1, 1, 1), f32 thickness = 0.05f);
function void DrawQuadOutline(Draw_Batch *batch, v2 centre, v2 dim, f32 angle = 0, v4 colour = V4(1, 1, 1, 1), f32 thickness = 0.05f);

// Line segments
//
function void DrawLine(Draw_Batch *batch, v2 start, v2 end, v4 start_colour = V4(1, 1, 1, 1), v4 end_colour = V4(1, 1, 1, 1), f32 thickness = 0.05f);

// Circles
//
function void DrawCircle(Draw_Batch *batch, v3 centre, f32 radius, f32 angle = 0, v4 colour = V4(1, 1, 1, 1), u32 segments = 100);
function void DrawCircle(Draw_Batch *batch, v2 centre, f32 radius, f32 angle = 0, v4 colour = V4(1, 1, 1, 1), u32 segments = 100);

// Animation functions
//
function void Initialise(Sprite_Animation *animation, Image_Handle image, u32 rows, u32 cols, f32 time_per_frame = 0.05f);

function void UpdateAnimation(Sprite_Animation *animation, f32 dt);

function void DrawAnimation(Draw_Batch *batch, Sprite_Animation *animation, v3 centre, v2 scale, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));
function void DrawAnimation(Draw_Batch *batch, Sprite_Animation *animation, v2 centre, v2 scale, f32 angle = 0, v4 colour = V4(1, 1, 1, 1));

#endif  // BASE_DRAW_H_
