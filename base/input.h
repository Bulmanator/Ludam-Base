#if !defined(BASE_INPUT_H_)
#define BASE_INPUT_H_

enum Key_Code {
#define Key(key, ...) Key_##key
    Key(Unknown) = 0,

    // Alphabet
    //
    Key(A),
    Key(B),
    Key(C),
    Key(D),
    Key(E),
    Key(F),
    Key(G),
    Key(H),
    Key(I),
    Key(J),
    Key(K),
    Key(L),
    Key(M),
    Key(N),
    Key(O),
    Key(P),
    Key(Q),
    Key(R),
    Key(S),
    Key(T),
    Key(U),
    Key(V),
    Key(W),
    Key(X),
    Key(Y),
    Key(Z),

    // Numbers
    //
    Key(0),
    Key(1),
    Key(2),
    Key(3),
    Key(4),
    Key(5),
    Key(6),
    Key(7),
    Key(8),
    Key(9),

    // Punctuation
    //
    Key(Space),
    Key(Grave),
    Key(Minus),
    Key(Equals),
    Key(LeftBracket),
    Key(RightBracket),
    Key(Semicolon),
    Key(Quote),
    Key(Hash),
    Key(Backslash),
    Key(Comma),
    Key(Peroid),
    Key(Slash),

    // F keys
    //
    // @Todo: There are more than 12 F keys, should we support them?
    //
    Key(F1),
    Key(F2),
    Key(F3),
    Key(F4),
    Key(F5),
    Key(F6),
    Key(F7),
    Key(F8),
    Key(F9),
    Key(F10),
    Key(F11),
    Key(F12),

    // Arrow keys
    //
    Key(Up),
    Key(Down),
    Key(Left),
    Key(Right),

     // Other keys
     //
     Key(Shift),
     Key(Control),
     Key(Alt),
     Key(Super),
     Key(Tab),
     Key(Enter),
     Key(Backspace),
     Key(Escape),

     Key(Count)
#undef Key
};

enum Mouse_Button {
    Mouse_Unknown = -1,

    Mouse_Left,
    Mouse_Middle,
    Mouse_Right,
    Mouse_X1,
    Mouse_X2,

    Mouse_Count
};

struct Input_Button {
    b32 pressed;
    u32 transitions;
};

struct Input {
    Input_Button keys[Key_Count];

    u64 ticks;      // Nondescript number of ticks with no defined base
    f64 time;       // Total elapsed time
    f64 delta_time; // Frame delta time

    b32 requested_quit;

    v2 mouse_clip;
    v3 mouse_delta; // Z is the mouse wheel
    Input_Button mouse_buttons[Mouse_Count];
};

function b32 IsPressed(Input_Button button);
function b32 JustPressed(Input_Button button);
function b32 WasPressed(Input_Button button);

#endif  // BASE_INPUT_H_
