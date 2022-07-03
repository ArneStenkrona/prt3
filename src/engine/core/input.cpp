#include "input.h"

using namespace prt3;

Input::Input() {
    for (size_t i = 0; i < m_current_key_states.size(); i++) {
        m_current_key_states[i] = false;
        m_previous_key_states[i] = false;
    }
}

void Input::init(SDL_Window * window) {
    m_window = window;
}

bool Input::get_key(KeyCode key_code) {
    return m_current_key_states[key_code];
}
bool Input::get_key_down(KeyCode key_code) {
    return m_current_key_states[key_code] && !m_previous_key_states[key_code];
}
bool Input::get_key_up(KeyCode key_code) {
    return !m_current_key_states[key_code] && m_previous_key_states[key_code];
}

void Input::get_cursor_position(int & x, int & y) {
    x = m_cursor_x;
    y = m_cursor_y;
}

void Input::get_cursor_delta(int & dx, int & dy) {
    dx = m_cursor_dx;
    dy = m_cursor_dy;
}

void Input::update() {
    m_previous_key_states = m_current_key_states;
    /* mouse */
    /* poll events */
    SDL_Event e;
    uint32_t window_id = SDL_GetWindowID(m_window);
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_WINDOWEVENT && e.window.windowID == window_id) {
            switch (e.window.event) {
                case SDL_WINDOWEVENT_ENTER:
                    //Mouse entered window
                    m_mouse_over = true;
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    //Mouse left window
                    m_mouse_over = false;
                    break;
            }
        } else {
            switch (e.type) {
                case SDL_MOUSEBUTTONDOWN:
                    switch (e.button.button) {
                    case SDL_BUTTON_LEFT:
                        m_mouse_left = true;
                        break;
                    case SDL_BUTTON_RIGHT:
                        m_mouse_right = true;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        m_mouse_middle = true;
                        break;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    switch (e.button.button) {
                    case SDL_BUTTON_LEFT:
                        m_mouse_left = false;
                        break;
                    case SDL_BUTTON_RIGHT:
                        m_mouse_right = false;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        m_mouse_middle = false;
                        break;
                    }
                    break;
                case SDL_TEXTINPUT:
                    // m_input_buffer = e.text.text;
                    break;
            }
        }
    }
    /* cursor position */
    int prev_x = m_cursor_x;
    int prev_y = m_cursor_y;
    SDL_GetMouseState(&m_cursor_x, &m_cursor_y);
    m_cursor_dx = m_cursor_x - prev_x;
    m_cursor_dy = m_cursor_y - prev_y;

    /* keyboard */
    const Uint8 *sdl_keyboard_state = SDL_GetKeyboardState(NULL);
    m_current_key_states[KEY_CODE_UNKNOWN]            = sdl_keyboard_state[KEY_CODE_UNKNOWN];
    m_current_key_states[KEY_CODE_A]                  = sdl_keyboard_state[KEY_CODE_A];
    m_current_key_states[KEY_CODE_B]                  = sdl_keyboard_state[KEY_CODE_B];
    m_current_key_states[KEY_CODE_C]                  = sdl_keyboard_state[KEY_CODE_C];
    m_current_key_states[KEY_CODE_D]                  = sdl_keyboard_state[KEY_CODE_D];
    m_current_key_states[KEY_CODE_E]                  = sdl_keyboard_state[KEY_CODE_E];
    m_current_key_states[KEY_CODE_F]                  = sdl_keyboard_state[KEY_CODE_F];
    m_current_key_states[KEY_CODE_G]                  = sdl_keyboard_state[KEY_CODE_G];
    m_current_key_states[KEY_CODE_H]                  = sdl_keyboard_state[KEY_CODE_H];
    m_current_key_states[KEY_CODE_I]                  = sdl_keyboard_state[KEY_CODE_I];
    m_current_key_states[KEY_CODE_J]                  = sdl_keyboard_state[KEY_CODE_J];
    m_current_key_states[KEY_CODE_K]                  = sdl_keyboard_state[KEY_CODE_K];
    m_current_key_states[KEY_CODE_L]                  = sdl_keyboard_state[KEY_CODE_L];
    m_current_key_states[KEY_CODE_M]                  = sdl_keyboard_state[KEY_CODE_M];
    m_current_key_states[KEY_CODE_N]                  = sdl_keyboard_state[KEY_CODE_N];
    m_current_key_states[KEY_CODE_O]                  = sdl_keyboard_state[KEY_CODE_O];
    m_current_key_states[KEY_CODE_P]                  = sdl_keyboard_state[KEY_CODE_P];
    m_current_key_states[KEY_CODE_Q]                  = sdl_keyboard_state[KEY_CODE_Q];
    m_current_key_states[KEY_CODE_R]                  = sdl_keyboard_state[KEY_CODE_R];
    m_current_key_states[KEY_CODE_S]                  = sdl_keyboard_state[KEY_CODE_S];
    m_current_key_states[KEY_CODE_T]                  = sdl_keyboard_state[KEY_CODE_T];
    m_current_key_states[KEY_CODE_U]                  = sdl_keyboard_state[KEY_CODE_U];
    m_current_key_states[KEY_CODE_V]                  = sdl_keyboard_state[KEY_CODE_V];
    m_current_key_states[KEY_CODE_W]                  = sdl_keyboard_state[KEY_CODE_W];
    m_current_key_states[KEY_CODE_X]                  = sdl_keyboard_state[KEY_CODE_X];
    m_current_key_states[KEY_CODE_Y]                  = sdl_keyboard_state[KEY_CODE_Y];
    m_current_key_states[KEY_CODE_Z]                  = sdl_keyboard_state[KEY_CODE_Z];
    m_current_key_states[KEY_CODE_1]                  = sdl_keyboard_state[KEY_CODE_1];
    m_current_key_states[KEY_CODE_2]                  = sdl_keyboard_state[KEY_CODE_2];
    m_current_key_states[KEY_CODE_3]                  = sdl_keyboard_state[KEY_CODE_3];
    m_current_key_states[KEY_CODE_4]                  = sdl_keyboard_state[KEY_CODE_4];
    m_current_key_states[KEY_CODE_5]                  = sdl_keyboard_state[KEY_CODE_5];
    m_current_key_states[KEY_CODE_6]                  = sdl_keyboard_state[KEY_CODE_6];
    m_current_key_states[KEY_CODE_7]                  = sdl_keyboard_state[KEY_CODE_7];
    m_current_key_states[KEY_CODE_8]                  = sdl_keyboard_state[KEY_CODE_8];
    m_current_key_states[KEY_CODE_9]                  = sdl_keyboard_state[KEY_CODE_9];
    m_current_key_states[KEY_CODE_0]                  = sdl_keyboard_state[KEY_CODE_0];
    m_current_key_states[KEY_CODE_RETURN]             = sdl_keyboard_state[KEY_CODE_RETURN];
    m_current_key_states[KEY_CODE_ESCAPE]             = sdl_keyboard_state[KEY_CODE_ESCAPE];
    m_current_key_states[KEY_CODE_BACKSPACE]          = sdl_keyboard_state[KEY_CODE_BACKSPACE];
    m_current_key_states[KEY_CODE_TAB]                = sdl_keyboard_state[KEY_CODE_TAB];
    m_current_key_states[KEY_CODE_SPACE]              = sdl_keyboard_state[KEY_CODE_SPACE];
    m_current_key_states[KEY_CODE_MINUS]              = sdl_keyboard_state[KEY_CODE_MINUS];
    m_current_key_states[KEY_CODE_EQUALS]             = sdl_keyboard_state[KEY_CODE_EQUALS];
    m_current_key_states[KEY_CODE_LEFTBRACKET]        = sdl_keyboard_state[KEY_CODE_LEFTBRACKET];
    m_current_key_states[KEY_CODE_RIGHTBRACKET]       = sdl_keyboard_state[KEY_CODE_RIGHTBRACKET];
    m_current_key_states[KEY_CODE_BACKSLASH]          = sdl_keyboard_state[KEY_CODE_BACKSLASH];
    m_current_key_states[KEY_CODE_NONUSHASH]          = sdl_keyboard_state[KEY_CODE_NONUSHASH];
    m_current_key_states[KEY_CODE_SEMICOLON]          = sdl_keyboard_state[KEY_CODE_SEMICOLON];
    m_current_key_states[KEY_CODE_APOSTROPHE]         = sdl_keyboard_state[KEY_CODE_APOSTROPHE];
    m_current_key_states[KEY_CODE_GRAVE]              = sdl_keyboard_state[KEY_CODE_GRAVE];
    m_current_key_states[KEY_CODE_COMMA]              = sdl_keyboard_state[KEY_CODE_COMMA];
    m_current_key_states[KEY_CODE_PERIOD]             = sdl_keyboard_state[KEY_CODE_PERIOD];
    m_current_key_states[KEY_CODE_SLASH]              = sdl_keyboard_state[KEY_CODE_SLASH];
    m_current_key_states[KEY_CODE_CAPSLOCK]           = sdl_keyboard_state[KEY_CODE_CAPSLOCK];
    m_current_key_states[KEY_CODE_F1]                 = sdl_keyboard_state[KEY_CODE_F1];
    m_current_key_states[KEY_CODE_F2]                 = sdl_keyboard_state[KEY_CODE_F2];
    m_current_key_states[KEY_CODE_F3]                 = sdl_keyboard_state[KEY_CODE_F3];
    m_current_key_states[KEY_CODE_F4]                 = sdl_keyboard_state[KEY_CODE_F4];
    m_current_key_states[KEY_CODE_F5]                 = sdl_keyboard_state[KEY_CODE_F5];
    m_current_key_states[KEY_CODE_F6]                 = sdl_keyboard_state[KEY_CODE_F6];
    m_current_key_states[KEY_CODE_F7]                 = sdl_keyboard_state[KEY_CODE_F7];
    m_current_key_states[KEY_CODE_F8]                 = sdl_keyboard_state[KEY_CODE_F8];
    m_current_key_states[KEY_CODE_F9]                 = sdl_keyboard_state[KEY_CODE_F9];
    m_current_key_states[KEY_CODE_F10]                = sdl_keyboard_state[KEY_CODE_F10];
    m_current_key_states[KEY_CODE_F11]                = sdl_keyboard_state[KEY_CODE_F11];
    m_current_key_states[KEY_CODE_F12]                = sdl_keyboard_state[KEY_CODE_F12];
    m_current_key_states[KEY_CODE_PRINTSCREEN]        = sdl_keyboard_state[KEY_CODE_PRINTSCREEN];
    m_current_key_states[KEY_CODE_SCROLLLOCK]         = sdl_keyboard_state[KEY_CODE_SCROLLLOCK];
    m_current_key_states[KEY_CODE_PAUSE]              = sdl_keyboard_state[KEY_CODE_PAUSE];
    m_current_key_states[KEY_CODE_INSERT]             = sdl_keyboard_state[KEY_CODE_INSERT];
    m_current_key_states[KEY_CODE_HOME]               = sdl_keyboard_state[KEY_CODE_HOME];
    m_current_key_states[KEY_CODE_PAGEUP]             = sdl_keyboard_state[KEY_CODE_PAGEUP];
    m_current_key_states[KEY_CODE_DELETE]             = sdl_keyboard_state[KEY_CODE_DELETE];
    m_current_key_states[KEY_CODE_END]                = sdl_keyboard_state[KEY_CODE_END];
    m_current_key_states[KEY_CODE_PAGEDOWN]           = sdl_keyboard_state[KEY_CODE_PAGEDOWN];
    m_current_key_states[KEY_CODE_RIGHT]              = sdl_keyboard_state[KEY_CODE_RIGHT];
    m_current_key_states[KEY_CODE_LEFT]               = sdl_keyboard_state[KEY_CODE_LEFT];
    m_current_key_states[KEY_CODE_DOWN]               = sdl_keyboard_state[KEY_CODE_DOWN];
    m_current_key_states[KEY_CODE_UP]                 = sdl_keyboard_state[KEY_CODE_UP];
    m_current_key_states[KEY_CODE_NUMLOCKCLEAR]       = sdl_keyboard_state[KEY_CODE_NUMLOCKCLEAR];
    m_current_key_states[KEY_CODE_KP_DIVIDE]          = sdl_keyboard_state[KEY_CODE_KP_DIVIDE];
    m_current_key_states[KEY_CODE_KP_MULTIPLY]        = sdl_keyboard_state[KEY_CODE_KP_MULTIPLY];
    m_current_key_states[KEY_CODE_KP_MINUS]           = sdl_keyboard_state[KEY_CODE_KP_MINUS];
    m_current_key_states[KEY_CODE_KP_PLUS]            = sdl_keyboard_state[KEY_CODE_KP_PLUS];
    m_current_key_states[KEY_CODE_KP_ENTER]           = sdl_keyboard_state[KEY_CODE_KP_ENTER];
    m_current_key_states[KEY_CODE_KP_1]               = sdl_keyboard_state[KEY_CODE_KP_1];
    m_current_key_states[KEY_CODE_KP_2]               = sdl_keyboard_state[KEY_CODE_KP_2];
    m_current_key_states[KEY_CODE_KP_3]               = sdl_keyboard_state[KEY_CODE_KP_3];
    m_current_key_states[KEY_CODE_KP_4]               = sdl_keyboard_state[KEY_CODE_KP_4];
    m_current_key_states[KEY_CODE_KP_5]               = sdl_keyboard_state[KEY_CODE_KP_5];
    m_current_key_states[KEY_CODE_KP_6]               = sdl_keyboard_state[KEY_CODE_KP_6];
    m_current_key_states[KEY_CODE_KP_7]               = sdl_keyboard_state[KEY_CODE_KP_7];
    m_current_key_states[KEY_CODE_KP_8]               = sdl_keyboard_state[KEY_CODE_KP_8];
    m_current_key_states[KEY_CODE_KP_9]               = sdl_keyboard_state[KEY_CODE_KP_9];
    m_current_key_states[KEY_CODE_KP_0]               = sdl_keyboard_state[KEY_CODE_KP_0];
    m_current_key_states[KEY_CODE_KP_PERIOD]          = sdl_keyboard_state[KEY_CODE_KP_PERIOD];
    m_current_key_states[KEY_CODE_NONUSBACKSLASH]     = sdl_keyboard_state[KEY_CODE_NONUSBACKSLASH];
    m_current_key_states[KEY_CODE_APPLICATION]        = sdl_keyboard_state[KEY_CODE_APPLICATION];
    m_current_key_states[KEY_CODE_POWER]              = sdl_keyboard_state[KEY_CODE_POWER];
    m_current_key_states[KEY_CODE_KP_EQUALS]          = sdl_keyboard_state[KEY_CODE_KP_EQUALS];
    m_current_key_states[KEY_CODE_F13]                = sdl_keyboard_state[KEY_CODE_F13];
    m_current_key_states[KEY_CODE_F14]                = sdl_keyboard_state[KEY_CODE_F14];
    m_current_key_states[KEY_CODE_F15]                = sdl_keyboard_state[KEY_CODE_F15];
    m_current_key_states[KEY_CODE_F16]                = sdl_keyboard_state[KEY_CODE_F16];
    m_current_key_states[KEY_CODE_F17]                = sdl_keyboard_state[KEY_CODE_F17];
    m_current_key_states[KEY_CODE_F18]                = sdl_keyboard_state[KEY_CODE_F18];
    m_current_key_states[KEY_CODE_F19]                = sdl_keyboard_state[KEY_CODE_F19];
    m_current_key_states[KEY_CODE_F20]                = sdl_keyboard_state[KEY_CODE_F20];
    m_current_key_states[KEY_CODE_F21]                = sdl_keyboard_state[KEY_CODE_F21];
    m_current_key_states[KEY_CODE_F22]                = sdl_keyboard_state[KEY_CODE_F22];
    m_current_key_states[KEY_CODE_F23]                = sdl_keyboard_state[KEY_CODE_F23];
    m_current_key_states[KEY_CODE_F24]                = sdl_keyboard_state[KEY_CODE_F24];
    m_current_key_states[KEY_CODE_EXECUTE]            = sdl_keyboard_state[KEY_CODE_EXECUTE];
    m_current_key_states[KEY_CODE_HELP]               = sdl_keyboard_state[KEY_CODE_HELP];
    m_current_key_states[KEY_CODE_MENU]               = sdl_keyboard_state[KEY_CODE_MENU];
    m_current_key_states[KEY_CODE_SELECT]             = sdl_keyboard_state[KEY_CODE_SELECT];
    m_current_key_states[KEY_CODE_STOP]               = sdl_keyboard_state[KEY_CODE_STOP];
    m_current_key_states[KEY_CODE_AGAIN]              = sdl_keyboard_state[KEY_CODE_AGAIN];
    m_current_key_states[KEY_CODE_UNDO]               = sdl_keyboard_state[KEY_CODE_UNDO];
    m_current_key_states[KEY_CODE_CUT]                = sdl_keyboard_state[KEY_CODE_CUT];
    m_current_key_states[KEY_CODE_COPY]               = sdl_keyboard_state[KEY_CODE_COPY];
    m_current_key_states[KEY_CODE_PASTE]              = sdl_keyboard_state[KEY_CODE_PASTE];
    m_current_key_states[KEY_CODE_FIND]               = sdl_keyboard_state[KEY_CODE_FIND];
    m_current_key_states[KEY_CODE_MUTE]               = sdl_keyboard_state[KEY_CODE_MUTE];
    m_current_key_states[KEY_CODE_VOLUMEUP]           = sdl_keyboard_state[KEY_CODE_VOLUMEUP];
    m_current_key_states[KEY_CODE_VOLUMEDOWN]         = sdl_keyboard_state[KEY_CODE_VOLUMEDOWN];
    m_current_key_states[KEY_CODE_LOCKINGCAPSLOCK]    = sdl_keyboard_state[KEY_CODE_LOCKINGCAPSLOCK];
    m_current_key_states[KEY_CODE_LOCKINGNUMLOCK]     = sdl_keyboard_state[KEY_CODE_LOCKINGNUMLOCK];
    m_current_key_states[KEY_CODE_LOCKINGSCROLLLOCK]  = sdl_keyboard_state[KEY_CODE_LOCKINGSCROLLLOCK];
    m_current_key_states[KEY_CODE_KP_COMMA]           = sdl_keyboard_state[KEY_CODE_KP_COMMA];
    m_current_key_states[KEY_CODE_KP_EQUALSAS400]     = sdl_keyboard_state[KEY_CODE_KP_EQUALSAS400];
    m_current_key_states[KEY_CODE_INTERNATIONAL1]     = sdl_keyboard_state[KEY_CODE_INTERNATIONAL1];
    m_current_key_states[KEY_CODE_INTERNATIONAL2]     = sdl_keyboard_state[KEY_CODE_INTERNATIONAL2];
    m_current_key_states[KEY_CODE_INTERNATIONAL3]     = sdl_keyboard_state[KEY_CODE_INTERNATIONAL3];
    m_current_key_states[KEY_CODE_INTERNATIONAL4]     = sdl_keyboard_state[KEY_CODE_INTERNATIONAL4];
    m_current_key_states[KEY_CODE_INTERNATIONAL5]     = sdl_keyboard_state[KEY_CODE_INTERNATIONAL5];
    m_current_key_states[KEY_CODE_INTERNATIONAL6]     = sdl_keyboard_state[KEY_CODE_INTERNATIONAL6];
    m_current_key_states[KEY_CODE_INTERNATIONAL7]     = sdl_keyboard_state[KEY_CODE_INTERNATIONAL7];
    m_current_key_states[KEY_CODE_INTERNATIONAL8]     = sdl_keyboard_state[KEY_CODE_INTERNATIONAL8];
    m_current_key_states[KEY_CODE_INTERNATIONAL9]     = sdl_keyboard_state[KEY_CODE_INTERNATIONAL9];
    m_current_key_states[KEY_CODE_LANG1]              = sdl_keyboard_state[KEY_CODE_LANG1];
    m_current_key_states[KEY_CODE_LANG2]              = sdl_keyboard_state[KEY_CODE_LANG2];
    m_current_key_states[KEY_CODE_LANG3]              = sdl_keyboard_state[KEY_CODE_LANG3];
    m_current_key_states[KEY_CODE_LANG4]              = sdl_keyboard_state[KEY_CODE_LANG4];
    m_current_key_states[KEY_CODE_LANG5]              = sdl_keyboard_state[KEY_CODE_LANG5];
    m_current_key_states[KEY_CODE_LANG6]              = sdl_keyboard_state[KEY_CODE_LANG6];
    m_current_key_states[KEY_CODE_LANG7]              = sdl_keyboard_state[KEY_CODE_LANG7];
    m_current_key_states[KEY_CODE_LANG8]              = sdl_keyboard_state[KEY_CODE_LANG8];
    m_current_key_states[KEY_CODE_LANG9]              = sdl_keyboard_state[KEY_CODE_LANG9];
    m_current_key_states[KEY_CODE_ALTERASE]           = sdl_keyboard_state[KEY_CODE_ALTERASE];
    m_current_key_states[KEY_CODE_SYSREQ]             = sdl_keyboard_state[KEY_CODE_SYSREQ];
    m_current_key_states[KEY_CODE_CANCEL]             = sdl_keyboard_state[KEY_CODE_CANCEL];
    m_current_key_states[KEY_CODE_CLEAR]              = sdl_keyboard_state[KEY_CODE_CLEAR];
    m_current_key_states[KEY_CODE_PRIOR]              = sdl_keyboard_state[KEY_CODE_PRIOR];
    m_current_key_states[KEY_CODE_RETURN2]            = sdl_keyboard_state[KEY_CODE_RETURN2];
    m_current_key_states[KEY_CODE_SEPARATOR]          = sdl_keyboard_state[KEY_CODE_SEPARATOR];
    m_current_key_states[KEY_CODE_OUT]                = sdl_keyboard_state[KEY_CODE_OUT];
    m_current_key_states[KEY_CODE_OPER]               = sdl_keyboard_state[KEY_CODE_OPER];
    m_current_key_states[KEY_CODE_CLEARAGAIN]         = sdl_keyboard_state[KEY_CODE_CLEARAGAIN];
    m_current_key_states[KEY_CODE_CRSEL]              = sdl_keyboard_state[KEY_CODE_CRSEL];
    m_current_key_states[KEY_CODE_EXSEL]              = sdl_keyboard_state[KEY_CODE_EXSEL];
    m_current_key_states[KEY_CODE_KP_00]              = sdl_keyboard_state[KEY_CODE_KP_00];
    m_current_key_states[KEY_CODE_KP_000]             = sdl_keyboard_state[KEY_CODE_KP_000];
    m_current_key_states[KEY_CODE_THOUSANDSSEPARATOR] = sdl_keyboard_state[KEY_CODE_THOUSANDSSEPARATOR];
    m_current_key_states[KEY_CODE_DECIMALSEPARATOR]   = sdl_keyboard_state[KEY_CODE_DECIMALSEPARATOR];
    m_current_key_states[KEY_CODE_CURRENCYUNIT]       = sdl_keyboard_state[KEY_CODE_CURRENCYUNIT];
    m_current_key_states[KEY_CODE_CURRENCYSUBUNIT]    = sdl_keyboard_state[KEY_CODE_CURRENCYSUBUNIT];
    m_current_key_states[KEY_CODE_KP_LEFTPAREN]       = sdl_keyboard_state[KEY_CODE_KP_LEFTPAREN];
    m_current_key_states[KEY_CODE_KP_RIGHTPAREN]      = sdl_keyboard_state[KEY_CODE_KP_RIGHTPAREN];
    m_current_key_states[KEY_CODE_KP_LEFTBRACE]       = sdl_keyboard_state[KEY_CODE_KP_LEFTBRACE];
    m_current_key_states[KEY_CODE_KP_RIGHTBRACE]      = sdl_keyboard_state[KEY_CODE_KP_RIGHTBRACE];
    m_current_key_states[KEY_CODE_KP_TAB]             = sdl_keyboard_state[KEY_CODE_KP_TAB];
    m_current_key_states[KEY_CODE_KP_BACKSPACE]       = sdl_keyboard_state[KEY_CODE_KP_BACKSPACE];
    m_current_key_states[KEY_CODE_KP_A]               = sdl_keyboard_state[KEY_CODE_KP_A];
    m_current_key_states[KEY_CODE_KP_B]               = sdl_keyboard_state[KEY_CODE_KP_B];
    m_current_key_states[KEY_CODE_KP_C]               = sdl_keyboard_state[KEY_CODE_KP_C];
    m_current_key_states[KEY_CODE_KP_D]               = sdl_keyboard_state[KEY_CODE_KP_D];
    m_current_key_states[KEY_CODE_KP_E]               = sdl_keyboard_state[KEY_CODE_KP_E];
    m_current_key_states[KEY_CODE_KP_F]               = sdl_keyboard_state[KEY_CODE_KP_F];
    m_current_key_states[KEY_CODE_KP_XOR]             = sdl_keyboard_state[KEY_CODE_KP_XOR];
    m_current_key_states[KEY_CODE_KP_POWER]           = sdl_keyboard_state[KEY_CODE_KP_POWER];
    m_current_key_states[KEY_CODE_KP_PERCENT]         = sdl_keyboard_state[KEY_CODE_KP_PERCENT];
    m_current_key_states[KEY_CODE_KP_LESS]            = sdl_keyboard_state[KEY_CODE_KP_LESS];
    m_current_key_states[KEY_CODE_KP_GREATER]         = sdl_keyboard_state[KEY_CODE_KP_GREATER];
    m_current_key_states[KEY_CODE_KP_AMPERSAND]       = sdl_keyboard_state[KEY_CODE_KP_AMPERSAND];
    m_current_key_states[KEY_CODE_KP_DBLAMPERSAND]    = sdl_keyboard_state[KEY_CODE_KP_DBLAMPERSAND];
    m_current_key_states[KEY_CODE_KP_VERTICALBAR]     = sdl_keyboard_state[KEY_CODE_KP_VERTICALBAR];
    m_current_key_states[KEY_CODE_KP_DBLVERTICALBAR]  = sdl_keyboard_state[KEY_CODE_KP_DBLVERTICALBAR];
    m_current_key_states[KEY_CODE_KP_COLON]           = sdl_keyboard_state[KEY_CODE_KP_COLON];
    m_current_key_states[KEY_CODE_KP_HASH]            = sdl_keyboard_state[KEY_CODE_KP_HASH];
    m_current_key_states[KEY_CODE_KP_SPACE]           = sdl_keyboard_state[KEY_CODE_KP_SPACE];
    m_current_key_states[KEY_CODE_KP_AT]              = sdl_keyboard_state[KEY_CODE_KP_AT];
    m_current_key_states[KEY_CODE_KP_EXCLAM]          = sdl_keyboard_state[KEY_CODE_KP_EXCLAM];
    m_current_key_states[KEY_CODE_KP_MEMSTORE]        = sdl_keyboard_state[KEY_CODE_KP_MEMSTORE];
    m_current_key_states[KEY_CODE_KP_MEMRECALL]       = sdl_keyboard_state[KEY_CODE_KP_MEMRECALL];
    m_current_key_states[KEY_CODE_KP_MEMCLEAR]        = sdl_keyboard_state[KEY_CODE_KP_MEMCLEAR];
    m_current_key_states[KEY_CODE_KP_MEMADD]          = sdl_keyboard_state[KEY_CODE_KP_MEMADD];
    m_current_key_states[KEY_CODE_KP_MEMSUBTRACT]     = sdl_keyboard_state[KEY_CODE_KP_MEMSUBTRACT];
    m_current_key_states[KEY_CODE_KP_MEMMULTIPLY]     = sdl_keyboard_state[KEY_CODE_KP_MEMMULTIPLY];
    m_current_key_states[KEY_CODE_KP_MEMDIVIDE]       = sdl_keyboard_state[KEY_CODE_KP_MEMDIVIDE];
    m_current_key_states[KEY_CODE_KP_PLUSMINUS]       = sdl_keyboard_state[KEY_CODE_KP_PLUSMINUS];
    m_current_key_states[KEY_CODE_KP_CLEAR]           = sdl_keyboard_state[KEY_CODE_KP_CLEAR];
    m_current_key_states[KEY_CODE_KP_CLEARENTRY]      = sdl_keyboard_state[KEY_CODE_KP_CLEARENTRY];
    m_current_key_states[KEY_CODE_KP_BINARY]          = sdl_keyboard_state[KEY_CODE_KP_BINARY];
    m_current_key_states[KEY_CODE_KP_OCTAL]           = sdl_keyboard_state[KEY_CODE_KP_OCTAL];
    m_current_key_states[KEY_CODE_KP_DECIMAL]         = sdl_keyboard_state[KEY_CODE_KP_DECIMAL];
    m_current_key_states[KEY_CODE_KP_HEXADECIMAL]     = sdl_keyboard_state[KEY_CODE_KP_HEXADECIMAL];
    m_current_key_states[KEY_CODE_LCTRL]              = sdl_keyboard_state[KEY_CODE_LCTRL];
    m_current_key_states[KEY_CODE_LSHIFT]             = sdl_keyboard_state[KEY_CODE_LSHIFT];
    m_current_key_states[KEY_CODE_LALT]               = sdl_keyboard_state[KEY_CODE_LALT];
    m_current_key_states[KEY_CODE_LGUI]               = sdl_keyboard_state[KEY_CODE_LGUI];
    m_current_key_states[KEY_CODE_RCTRL]              = sdl_keyboard_state[KEY_CODE_RCTRL];
    m_current_key_states[KEY_CODE_RSHIFT]             = sdl_keyboard_state[KEY_CODE_RSHIFT];
    m_current_key_states[KEY_CODE_RALT]               = sdl_keyboard_state[KEY_CODE_RALT];
    m_current_key_states[KEY_CODE_RGUI]               = sdl_keyboard_state[KEY_CODE_RGUI];
    m_current_key_states[KEY_CODE_MODE]               = sdl_keyboard_state[KEY_CODE_MODE];
    m_current_key_states[KEY_CODE_AUDIONEXT]          = sdl_keyboard_state[KEY_CODE_AUDIONEXT];
    m_current_key_states[KEY_CODE_AUDIOPREV]          = sdl_keyboard_state[KEY_CODE_AUDIOPREV];
    m_current_key_states[KEY_CODE_AUDIOSTOP]          = sdl_keyboard_state[KEY_CODE_AUDIOSTOP];
    m_current_key_states[KEY_CODE_AUDIOPLAY]          = sdl_keyboard_state[KEY_CODE_AUDIOPLAY];
    m_current_key_states[KEY_CODE_AUDIOMUTE]          = sdl_keyboard_state[KEY_CODE_AUDIOMUTE];
    m_current_key_states[KEY_CODE_MEDIASELECT]        = sdl_keyboard_state[KEY_CODE_MEDIASELECT];
    m_current_key_states[KEY_CODE_WWW]                = sdl_keyboard_state[KEY_CODE_WWW];
    m_current_key_states[KEY_CODE_MAIL]               = sdl_keyboard_state[KEY_CODE_MAIL];
    m_current_key_states[KEY_CODE_CALCULATOR]         = sdl_keyboard_state[KEY_CODE_CALCULATOR];
    m_current_key_states[KEY_CODE_COMPUTER]           = sdl_keyboard_state[KEY_CODE_COMPUTER];
    m_current_key_states[KEY_CODE_AC_SEARCH]          = sdl_keyboard_state[KEY_CODE_AC_SEARCH];
    m_current_key_states[KEY_CODE_AC_HOME]            = sdl_keyboard_state[KEY_CODE_AC_HOME];
    m_current_key_states[KEY_CODE_AC_BACK]            = sdl_keyboard_state[KEY_CODE_AC_BACK];
    m_current_key_states[KEY_CODE_AC_FORWARD]         = sdl_keyboard_state[KEY_CODE_AC_FORWARD];
    m_current_key_states[KEY_CODE_AC_STOP]            = sdl_keyboard_state[KEY_CODE_AC_STOP];
    m_current_key_states[KEY_CODE_AC_REFRESH]         = sdl_keyboard_state[KEY_CODE_AC_REFRESH];
    m_current_key_states[KEY_CODE_AC_BOOKMARKS]       = sdl_keyboard_state[KEY_CODE_AC_BOOKMARKS];
    m_current_key_states[KEY_CODE_BRIGHTNESSDOWN]     = sdl_keyboard_state[KEY_CODE_BRIGHTNESSDOWN];
    m_current_key_states[KEY_CODE_BRIGHTNESSUP]       = sdl_keyboard_state[KEY_CODE_BRIGHTNESSUP];
    m_current_key_states[KEY_CODE_DISPLAYSWITCH]      = sdl_keyboard_state[KEY_CODE_DISPLAYSWITCH];
    m_current_key_states[KEY_CODE_KBDILLUMTOGGLE]     = sdl_keyboard_state[KEY_CODE_KBDILLUMTOGGLE];
    m_current_key_states[KEY_CODE_KBDILLUMDOWN]       = sdl_keyboard_state[KEY_CODE_KBDILLUMDOWN];
    m_current_key_states[KEY_CODE_KBDILLUMUP]         = sdl_keyboard_state[KEY_CODE_KBDILLUMUP];
    m_current_key_states[KEY_CODE_EJECT]              = sdl_keyboard_state[KEY_CODE_EJECT];
    m_current_key_states[KEY_CODE_SLEEP]              = sdl_keyboard_state[KEY_CODE_SLEEP];

    /* mouse buttons */
    m_current_key_states[KEY_CODE_MOUSE_LEFT] = m_mouse_left;
    m_current_key_states[KEY_CODE_MOUSE_RIGHT] = m_mouse_right;
    m_current_key_states[KEY_CODE_MOUSE_MIDDLE] = m_mouse_middle;
}
