#pragma once

#include "Core/Base/Input.h"

#include <cstdint>
#include <string_view>

namespace Settings::KeyBinds {
    enum class KeyCode : uint32_t { INVALID = static_cast<uint32_t>(-1) };
    enum class ButtonCode : uint32_t { INVALID = static_cast<uint32_t>(-1) };

    class InputCode {
    public:
        InputCode() = default;
        constexpr InputCode(KeyCode code) { SetCode(code); }
        constexpr InputCode(ButtonCode code) { SetCode(code); }
        constexpr InputCode(uint32_t code) { SetCode(code); }

        inline constexpr bool HasValue() const { return HasKey() || HasButton(); }
        inline constexpr bool HasKey() const { return Core::Input::IsValidKeyCode(m_Code); };
        inline constexpr bool HasButton() const { return Core::Input::IsValidButtonCode(m_Code); };

        inline constexpr KeyCode GetKeyCode() const { return HasKey() ? KeyCode(m_Code) : KeyCode::INVALID; };       // Returns KeyCode::INVALID, if InputCode does not contain KeyCode
        inline constexpr ButtonCode GetButtonCode() const { return HasButton() ? ButtonCode(m_Code) : ButtonCode::INVALID; }; // Returns ButtonCode::INVALID, if InputCode does not contain ButtonCode

        inline constexpr bool SetCode(uint32_t code) {
            return SetCode(KeyCode(code)) || SetCode(ButtonCode(code));
        };
        inline constexpr bool SetCode(KeyCode code) {
            uint32_t newCode = static_cast<uint32_t>(code);
            bool valid = Core::Input::IsValidKeyCode(newCode);
            m_Code = m_Code * !valid + newCode * valid;
            return valid;
        };
        inline constexpr bool SetCode(ButtonCode code) {
            uint32_t newCode = static_cast<uint32_t>(code);
            bool valid = Core::Input::IsValidButtonCode(newCode);
            m_Code = m_Code * !valid + newCode * valid;
            return valid;
        };

        inline std::wstring_view ToString() const { return ToString(*this); };
        inline static std::wstring_view ToString(KeyCode code) { return Core::Input::KeyCodeToString(static_cast<uint32_t>(code)); }
        inline static std::wstring_view ToString(ButtonCode code) { return Core::Input::ButtonCodeToString(static_cast<uint32_t>(code)); }
        inline static std::wstring_view ToString(InputCode code) {
            if (!code.HasValue()) { return L""; }
            return code.HasKey() ? ToString((KeyCode)code.m_Code) : ToString((ButtonCode)code.m_Code);
        }

        inline constexpr bool operator==(KeyCode other) const { return HasValue() && m_Code == static_cast<uint32_t>(other); }
        inline constexpr bool operator==(ButtonCode other) const { return HasValue() && m_Code == static_cast<uint32_t>(other); }
        inline constexpr bool operator==(InputCode other) const { return m_Code == other.m_Code; }
    private:
        uint32_t m_Code = -1;
    };
}