function b32 IsPressed(Input_Button button) {
    b32 result = button.pressed;
    return result;
}

function b32 JustPressed(Input_Button button) {
    b32 result = button.pressed && (button.transitions != 0);
    return result;
}

function b32 WasPressed(Input_Button button) {
    b32 result = !button.pressed && (button.transitions != 0);
    return result;
}
