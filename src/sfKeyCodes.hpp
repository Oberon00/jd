// No include guards
//
// #define E(name, value to something meaningful

#ifndef E
#   error You have to provide a definiton for the macro E(name, value)
#endif

#define E1(n) E(#n, n)
        E1(A) E1(B) E1(C) E1(D) E1(E) E1(F) E1(G) E1(H) E1(I) E1(J) E1(K)
        E1(L) E1(M) E1(N) E1(O) E1(P) E1(Q) E1(R) E1(S) E1(T) E1(U) E1(V)
        E1(W) E1(X) E1(Y) E1(Z)
        E1(F1) E1(F2) E1(F3) E1(F4) E1(F5) E1(F6) E1(F7) E1(F8) E1(F9) E1(F10)
        E1(F11) E1(F12) E1(F13) E1(F14) E1(F15)
#undef E1

        E("UNKNOWN", Unknown)
        E("NUM0", Num0)
        E("NUM1", Num1) E("NUM2", Num2) E("NUM3", Num3)
        E("NUM4", Num4) E("NUM5", Num5) E("NUM6", Num6)
        E("NUM7", Num7) E("NUM8", Num8) E("NUM9", Num9)
        E("ESCAPE", Escape)
        E("LCONTROL", LControl) E("LSHIFT", LShift)
        E("LALT", LAlt) E("LSYSTEM", LSystem)
        E("RCONTROL", RControl) E("RSHIFT", RShift)
        E("RALT", RAlt) E("RSYSTEM", RSystem)
        E("MENU", Menu)
        E("LBRACKET", LBracket)
        E("RBRACKET", RBracket)
        E("SEMICOLON", SemiColon) E("COMMA", Comma) E("PERIOD", Period)
        E("QUOTE", Quote)
        E("SLASH", Slash) E("BACKSLASH", BackSlash)
        E("TILDE", Tilde)
        E("EQUAL", Equal)
        E("DASH", Dash)
        E("SPACE", Space)
        E("RETURN", Return)
        E("BACKSPACE", BackSpace)
        E("TAB", Tab)
        E("PAGEUP", PageUp) E("PAGEDOWN", PageDown)
        E("END", End) E("HOME", Home)
        E("INSERT", Insert) E("DELETE", Delete)
        E("ADD", Add) E("SUBTRACT", Subtract)
        E("MULTIPLY", Multiply) E("DIVIDE", Divide)
        E("LEFT", Left) E("RIGHT", Right) E("UP", Up) E("DOWN", Down)
        E("NUMPAD0", Numpad0)
        E("NUMPAD1", Numpad1) E("NUMPAD2", Numpad2) E("NUMPAD3", Numpad3)
        E("NUMPAD4", Numpad4) E("NUMPAD5", Numpad5) E("NUMPAD6", Numpad6)
        E("NUMPAD7", Numpad7) E("NUMPAD8", Numpad8) E("NUMPAD9", Numpad9)
        E("PAUSE", Pause)

#undef E