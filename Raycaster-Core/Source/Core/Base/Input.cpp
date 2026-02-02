#include "Input.h"
#include "Application.h"
#include "Core/Debug/Log.h"
#include "Platform.h"

#include <GLFW/glfw3.h>

#include <unordered_map>
#include <cwctype>

namespace Core {
    bool Input::IsKeyPressed(uint32_t key) {
        auto state = glfwGetKey(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }
    
    bool Input::IsButtonPressed(uint32_t button) {
        auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), button);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }
    
    bool Input::IsButtonReleased(uint32_t button) {
        auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), button);
        return state == GLFW_RELEASE;
    }

    float Input::GetMouseX() {
        double x, y;
        glfwGetCursorPos(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), &x, &y);
        return (float)x;
    }

    float Input::GetMouseY() {
        double x, y;
        glfwGetCursorPos(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), &x, &y);
        return (float)y;
    }

    std::pair<float, float> Input::GetMousePos() {
        double x, y;
        glfwGetCursorPos(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), &x, &y);
        return { (float)x, (float)y };
    }

    void Input::SetCursorMode(CursorMode mode) {
        void* window = Application::GetWindow().GetWindowPointer();

        uint32_t glfwMode = 0;
        switch (mode) {
            using enum CursorMode;
        case Normal:
            glfwMode = GLFW_CURSOR_NORMAL;
            break;
        case Hidden:
            glfwMode = GLFW_CURSOR_HIDDEN;
            break;
        case Captured:
            glfwMode = GLFW_CURSOR_CAPTURED;
            break;
        case Disabled:
            glfwMode = GLFW_CURSOR_DISABLED;
            break;
        }

        if (glfwMode == 0) {
            RC_WARN("Tried to set cursor mode to an invalid Input::CursorMode");
            return;
        }

        glfwSetInputMode(static_cast<GLFWwindow*>(window), GLFW_CURSOR, glfwMode);
    }

    std::wstring_view Input::KeyCodeToString(uint32_t keyCode) {
        static std::unordered_map<uint32_t, std::wstring> cache = [] {
            std::unordered_map<uint32_t, std::wstring> map;
            std::string locale(std::setlocale(LC_ALL, nullptr)); // Save current locale
            std::setlocale(LC_ALL, "");

            // GLFW only has names for (most) keys between apostrophe and numpad equal
            for (uint32_t i = RC_KEY_APOSTROPHE; i <= RC_KEY_KP_EQUAL; i++) {
                int32_t scanCode = glfwGetKeyScancode(i);
                if (scanCode == -1) {
                    continue;
                }

                const char* utf8 = glfwGetKeyName(i, scanCode);
                if (utf8 != nullptr) {
                    auto& string = map.emplace(i, UTF8ToWide(utf8)).first->second;
                    std::transform(string.begin(), string.end(), string.begin(), std::towupper);
                }
            }

            std::setlocale(LC_ALL, locale.c_str()); // Restore locale
            return map;
        }();
        
        if (auto iter = cache.find(keyCode); iter != cache.end()) {
            return iter->second;
        }

        // Default values, if GLFW doesn't have a name for the key
        switch(keyCode) {
            case RC_KEY_SPACE:                  return L"SPACE";
            case RC_KEY_APOSTROPHE:             return L"'";
            case RC_KEY_COMMA:                  return L",";
            case RC_KEY_MINUS:                  return L"-";
            case RC_KEY_PERIOD:                 return L".";
            case RC_KEY_SLASH:                  return L"/";
            case RC_KEY_0:                      return L"0";
            case RC_KEY_1:                      return L"1";
            case RC_KEY_2:                      return L"2";
            case RC_KEY_3:                      return L"3";
            case RC_KEY_4:                      return L"4";
            case RC_KEY_5:                      return L"5";
            case RC_KEY_6:                      return L"6";
            case RC_KEY_7:                      return L"7";
            case RC_KEY_8:                      return L"8";
            case RC_KEY_9:                      return L"9";
            case RC_KEY_SEMICOLON:              return L";";
            case RC_KEY_EQUAL:                  return L"=";
            case RC_KEY_A:                      return L"A";
            case RC_KEY_B:                      return L"B";
            case RC_KEY_C:                      return L"C";
            case RC_KEY_D:                      return L"D";
            case RC_KEY_E:                      return L"E";
            case RC_KEY_F:                      return L"F";
            case RC_KEY_G:                      return L"G";
            case RC_KEY_H:                      return L"H";
            case RC_KEY_I:                      return L"I";
            case RC_KEY_J:                      return L"J";
            case RC_KEY_K:                      return L"K";
            case RC_KEY_L:                      return L"L";
            case RC_KEY_M:                      return L"M";
            case RC_KEY_N:                      return L"N";
            case RC_KEY_O:                      return L"O";
            case RC_KEY_P:                      return L"P";
            case RC_KEY_Q:                      return L"Q";
            case RC_KEY_R:                      return L"R";
            case RC_KEY_S:                      return L"S";
            case RC_KEY_T:                      return L"T";
            case RC_KEY_U:                      return L"U";
            case RC_KEY_V:                      return L"V";
            case RC_KEY_W:                      return L"W";
            case RC_KEY_X:                      return L"X";
            case RC_KEY_Y:                      return L"Y";
            case RC_KEY_Z:                      return L"Z";
            case RC_KEY_LEFT_BRACKET:           return L"[";
            case RC_KEY_BACKSLASH:              return L"\\";
            case RC_KEY_RIGHT_BRACKET:          return L"]";
            case RC_KEY_GRAVE_ACCENT:           return L"`";
            case RC_KEY_WORLD_1:                return L"INTERNATIONAL 1";
            case RC_KEY_WORLD_2:                return L"INTERNATIONAL 2";
            case RC_KEY_ESCAPE:                 return L"ESCAPE";
            case RC_KEY_ENTER:                  return L"ENTER";
            case RC_KEY_TAB:                    return L"TAB";
            case RC_KEY_BACKSPACE:              return L"BACKSPACE";
            case RC_KEY_INSERT:                 return L"INSERT";
            case RC_KEY_DELETE:                 return L"DELETE";
            case RC_KEY_RIGHT:                  return L"RIGHT";
            case RC_KEY_LEFT:                   return L"LEFT";
            case RC_KEY_DOWN:                   return L"DOWN";
            case RC_KEY_UP:                     return L"UP";
            case RC_KEY_PAGE_UP:                return L"PAGE UP";
            case RC_KEY_PAGE_DOWN:              return L"PAGE DOWN";
            case RC_KEY_HOME:                   return L"HOME";
            case RC_KEY_END:                    return L"END";
            case RC_KEY_CAPS_LOCK:              return L"CAPS LOCK";
            case RC_KEY_SCROLL_LOCK:            return L"SCROLL LOCK";
            case RC_KEY_NUM_LOCK:               return L"NUM LOCK";
            case RC_KEY_PRINT_SCREEN:           return L"PRINT SCREEN";
            case RC_KEY_PAUSE:                  return L"PAUSE";
            case RC_KEY_F1:                     return L"F1";
            case RC_KEY_F2:                     return L"F2";
            case RC_KEY_F3:                     return L"F3";
            case RC_KEY_F4:                     return L"F4";
            case RC_KEY_F5:                     return L"F5";
            case RC_KEY_F6:                     return L"F6";
            case RC_KEY_F7:                     return L"F7";
            case RC_KEY_F8:                     return L"F8";
            case RC_KEY_F9:                     return L"F9";
            case RC_KEY_F10:                    return L"F10";
            case RC_KEY_F11:                    return L"F11";
            case RC_KEY_F12:                    return L"F12";
            case RC_KEY_F13:                    return L"F13";
            case RC_KEY_F14:                    return L"F14";
            case RC_KEY_F15:                    return L"F15";
            case RC_KEY_F16:                    return L"F16";
            case RC_KEY_F17:                    return L"F17";
            case RC_KEY_F18:                    return L"F18";
            case RC_KEY_F19:                    return L"F19";
            case RC_KEY_F20:                    return L"F20";
            case RC_KEY_F21:                    return L"F21";
            case RC_KEY_F22:                    return L"F22";
            case RC_KEY_F23:                    return L"F23";
            case RC_KEY_F24:                    return L"F24";
            case RC_KEY_F25:                    return L"F25";
            case RC_KEY_KP_0:                   return L"NUM 0";
            case RC_KEY_KP_1:                   return L"NUM 1";
            case RC_KEY_KP_2:                   return L"NUM 2";
            case RC_KEY_KP_3:                   return L"NUM 3";
            case RC_KEY_KP_4:                   return L"NUM 4";
            case RC_KEY_KP_5:                   return L"NUM 5";
            case RC_KEY_KP_6:                   return L"NUM 6";
            case RC_KEY_KP_7:                   return L"NUM 7";
            case RC_KEY_KP_8:                   return L"NUM 8";
            case RC_KEY_KP_9:                   return L"NUM 9";
            case RC_KEY_KP_DECIMAL:             return L"NUM DECIMAL";
            case RC_KEY_KP_DIVIDE:              return L"NUM DIVIDE";
            case RC_KEY_KP_MULTIPLY:            return L"NUM MULTIPLY";
            case RC_KEY_KP_SUBTRACT:            return L"NUM SUBTRACT";
            case RC_KEY_KP_ADD:                 return L"NUM ADD";
            case RC_KEY_KP_ENTER:               return L"NUM ENTER";
            case RC_KEY_KP_EQUAL:               return L"NUM EQUAL";
            case RC_KEY_LEFT_SHIFT:             return L"SHIFT";
            case RC_KEY_LEFT_CONTROL:           return L"CONTROL";
            case RC_KEY_LEFT_ALT:               return L"ALT";
            case RC_KEY_LEFT_SUPER:             return L"SUPER";
            case RC_KEY_RIGHT_SHIFT:            return L"RIGHT SHIFT";
            case RC_KEY_RIGHT_CONTROL:          return L"RIGHT CONTROL";
            case RC_KEY_RIGHT_ALT:              return L"RIGHT ALT";
            case RC_KEY_RIGHT_SUPER:            return L"RIGHT SUPER";
            case RC_KEY_MENU:                   return L"MENU";                       
        }

        RC_WARN("Tried to convert key code, {}, to string, but it is not a valid key code.", keyCode);
        return L"INVALID KEY";
    }
}
