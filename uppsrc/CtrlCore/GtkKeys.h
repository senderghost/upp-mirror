
K_BACK       = 8,
K_BACKSPACE  = 8,

K_TAB        = 9,

K_RETURN     = 13,
K_ENTER      = 13,
K_ESCAPE     = 27,

K_SPACE      = 32,

K_DELETE     = GDKEY(Delete)|K_DELTA,

K_SHIFT_KEY  = GDKEY(Shift_L)|K_DELTA,
K_CTRL_KEY   = GDKEY(Control_L)|K_DELTA,
K_ALT_KEY    = GDKEY(Alt_L)|K_DELTA,
K_CAPSLOCK   = GDKEY(Caps_Lock)|K_DELTA,
K_PRIOR      = GDKEY(Page_Up)|K_DELTA,
K_PAGEUP     = GDKEY(Page_Up)|K_DELTA,
K_NEXT       = GDKEY(Page_Down)|K_DELTA,
K_PAGEDOWN   = GDKEY(Page_Down)|K_DELTA,
K_END        = GDKEY(End)|K_DELTA,
K_HOME       = GDKEY(Home)|K_DELTA,
K_LEFT       = GDKEY(Left)|K_DELTA,
K_UP         = GDKEY(Up)|K_DELTA,
K_RIGHT      = GDKEY(Right)|K_DELTA,
K_DOWN       = GDKEY(Down)|K_DELTA,
K_INSERT     = GDKEY(Insert)|K_DELTA,

K_NUMPAD0    = GDKEY(KP_0)|K_DELTA,
K_NUMPAD1    = GDKEY(KP_1)|K_DELTA,
K_NUMPAD2    = GDKEY(KP_2)|K_DELTA,
K_NUMPAD3    = GDKEY(KP_3)|K_DELTA,
K_NUMPAD4    = GDKEY(KP_4)|K_DELTA,
K_NUMPAD5    = GDKEY(KP_5)|K_DELTA,
K_NUMPAD6    = GDKEY(KP_6)|K_DELTA,
K_NUMPAD7    = GDKEY(KP_7)|K_DELTA,
K_NUMPAD8    = GDKEY(KP_8)|K_DELTA,
K_NUMPAD9    = GDKEY(KP_9)|K_DELTA,
K_MULTIPLY   = GDKEY(KP_Multiply)|K_DELTA,
K_ADD        = GDKEY(KP_Add)|K_DELTA,
K_SEPARATOR  = GDKEY(KP_Separator)|K_DELTA,
K_SUBTRACT   = GDKEY(KP_Subtract)|K_DELTA,
K_DECIMAL    = GDKEY(KP_Decimal)|K_DELTA,
K_DIVIDE     = GDKEY(KP_Divide)|K_DELTA,

K_SCROLL     = GDKEY(Scroll_Lock)|K_DELTA,

K_PLUS       = GDKEY(plus)|K_DELTA,
K_MINUS      = GDKEY(minus)|K_DELTA,
K_COMMA      = GDKEY(comma)|K_DELTA,
K_PERIOD     = GDKEY(period)|K_DELTA,
K_SEMICOLON  = GDKEY(semicolon)|K_DELTA,	
K_SLASH      = GDKEY(slash)|K_DELTA,
K_GRAVE      = GDKEY(grave)|K_DELTA,
K_LBRACKET   = GDKEY(bracketleft)|K_DELTA,
K_BACKSLASH  = GDKEY(backslash)|K_DELTA,
K_RBRACKET   = GDKEY(bracketright)|K_DELTA,
K_QUOTEDBL   = GDKEY(quotedbl)|K_DELTA,

K_F1 = GDKEY(F1)|K_DELTA,
K_F2 = GDKEY(F2)|K_DELTA,
K_F3 = GDKEY(F3)|K_DELTA,
K_F4 = GDKEY(F4)|K_DELTA,
K_F5 = GDKEY(F5)|K_DELTA,
K_F6 = GDKEY(F6)|K_DELTA,
K_F7 = GDKEY(F7)|K_DELTA,
K_F8 = GDKEY(F8)|K_DELTA,
K_F9 = GDKEY(F9)|K_DELTA,
K_F10 = GDKEY(F10)|K_DELTA,
K_F11 = GDKEY(F11)|K_DELTA,
K_F12 = GDKEY(F12)|K_DELTA,

K_A = GDKEY(A)|K_DELTA,
K_B = GDKEY(B)|K_DELTA,
K_C = GDKEY(C)|K_DELTA,
K_D = GDKEY(D)|K_DELTA,
K_E = GDKEY(E)|K_DELTA,
K_F = GDKEY(F)|K_DELTA,
K_G = GDKEY(G)|K_DELTA,
K_H = GDKEY(H)|K_DELTA,
K_I = GDKEY(I)|K_DELTA,
K_J = GDKEY(J)|K_DELTA,
K_K = GDKEY(K)|K_DELTA,
K_L = GDKEY(L)|K_DELTA,
K_M = GDKEY(M)|K_DELTA,
K_N = GDKEY(N)|K_DELTA,
K_O = GDKEY(O)|K_DELTA,
K_P = GDKEY(P)|K_DELTA,
K_Q = GDKEY(Q)|K_DELTA,
K_R = GDKEY(R)|K_DELTA,
K_S = GDKEY(S)|K_DELTA,
K_T = GDKEY(T)|K_DELTA,
K_U = GDKEY(U)|K_DELTA,
K_V = GDKEY(V)|K_DELTA,
K_W = GDKEY(W)|K_DELTA,
K_X = GDKEY(X)|K_DELTA,
K_Y = GDKEY(Y)|K_DELTA,
K_Z = GDKEY(Z)|K_DELTA,
K_0 = GDKEY(0)|K_DELTA,
K_1 = GDKEY(1)|K_DELTA,
K_2 = GDKEY(2)|K_DELTA,
K_3 = GDKEY(3)|K_DELTA,
K_4 = GDKEY(4)|K_DELTA,
K_5 = GDKEY(5)|K_DELTA,
K_6 = GDKEY(6)|K_DELTA,
K_7 = GDKEY(7)|K_DELTA,
K_8 = GDKEY(8)|K_DELTA,
K_9 = GDKEY(9)|K_DELTA,

K_CTRL_LBRACKET  = K_CTRL|GDKEY(bracketleft)|K_DELTA,
K_CTRL_RBRACKET  = K_CTRL|GDKEY(bracketright)|K_DELTA,
K_CTRL_MINUS     = K_CTRL|GDKEY(minus)|K_DELTA,
K_CTRL_GRAVE     = K_CTRL|GDKEY(grave)|K_DELTA,
K_CTRL_SLASH     = K_CTRL|GDKEY(slash)|K_DELTA,
K_CTRL_BACKSLASH = K_CTRL|GDKEY(backslash)|K_DELTA,
K_CTRL_COMMA     = K_CTRL|GDKEY(comma)|K_DELTA,
K_CTRL_PERIOD    = K_CTRL|GDKEY(period)|K_DELTA,
K_CTRL_SEMICOLON = K_CTRL|GDKEY(semicolon)|K_DELTA,
K_CTRL_EQUAL     = K_CTRL|GDKEY(equal)|K_DELTA,
K_CTRL_APOSTROPHE= K_CTRL|GDKEY(apostrophe)|K_DELTA,

K_BREAK      = GDKEY(Break)|K_DELTA,
