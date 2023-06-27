#include "input.h"

#include "src/util/log.h"

using namespace prt3;

std::array<bool, KEY_CODE_LAST + 1> current_key_states{};
int s_cursor_x = 0;
int s_cursor_y = 0;

Input::Input() {}

static void key_callback(
    GLFWwindow * /*window*/,
    int key,
    int /*scancode*/,
    int action,
    int /*mods*/
) {
    if (action == GLFW_PRESS) {
        current_key_states[key] = true;

    } else if (action == GLFW_RELEASE) {
        current_key_states[key] = false;
    }
}

static void cursor_position_callback(
    GLFWwindow * /*window*/,
    double x,
    double y
) {
    s_cursor_x = static_cast<int>(x);
    s_cursor_y = static_cast<int>(y);

}

static void mouse_button_callback(
    GLFWwindow* /*window*/,
    int button,
    int action,
    int /*mods*/
) {
    prt3::KeyCode key_code;
    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            key_code = KEY_CODE_MOUSE_LEFT;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            key_code = KEY_CODE_MOUSE_RIGHT;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            key_code = KEY_CODE_MOUSE_MIDDLE;
            break;
        default:
            return;
    }

    if (action == GLFW_PRESS) {
        current_key_states[key_code] = true;

    } else if (action == GLFW_RELEASE) {
        current_key_states[key_code] = false;
    }
}

void Input::init(GLFWwindow * window) {
    m_window = window;
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
}

bool Input::get_key(KeyCode key_code) const {
    return m_current_key_states[key_code];
}
bool Input::get_key_down(KeyCode key_code) const {
    return m_current_key_states[key_code] && !m_previous_key_states[key_code];
}
bool Input::get_key_up(KeyCode key_code) const {
    return !m_current_key_states[key_code] && m_previous_key_states[key_code];
}

void Input::get_cursor_position(int & x, int & y) const {
    x = m_cursor_x;
    y = m_cursor_y;
}

void Input::get_cursor_delta(int & dx, int & dy) const {
    dx = m_cursor_dx;
    dy = m_cursor_dy;
}

void Input::update() {
    m_previous_key_states = m_current_key_states;
    m_current_key_states = current_key_states;

    int prev_x = m_cursor_x;
    int prev_y = m_cursor_y;

    m_cursor_x = s_cursor_x;
    m_cursor_y = s_cursor_y;

    m_cursor_dx = m_cursor_x - prev_x;
    m_cursor_dy = m_cursor_y - prev_y;
}
