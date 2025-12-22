#include "Clipboard.h"

#include "Application.h"
#include "Window.h"

#include <GLFW/glfw3.h>

#include <codecvt>
#include <locale>

namespace Core {
	std::u8string Clipboard::GetClipboard() {
		return (const char8_t*)glfwGetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()));
	}

	std::wstring Clipboard::GetClipboardWide() {
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(glfwGetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer())));
	}

	template <>
	std::basic_string<char8_t> Clipboard::GetClipboard() {
		return (const char8_t*)glfwGetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()));
	}

	template <>
	std::basic_string<char> Clipboard::GetClipboard() {
		return glfwGetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()));
	}

	template <>
	std::basic_string<wchar_t> Clipboard::GetClipboard() {
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(glfwGetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer())));
	}

	void Clipboard::SetClipboard(std::u8string_view str) {
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), (const char*)str.data());
	}

	void Clipboard::SetClipboard(std::string_view str) {
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), str.data());
	}

	void Clipboard::SetClipboardWide(std::wstring_view str) {
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str.data()).c_str());
	}

	template <>
	void Clipboard::SetClipboard(std::basic_string<char8_t> str) {
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), (const char*)str.c_str());
	}

	template <>
	void Clipboard::SetClipboard(std::basic_string<char> str) {
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), str.c_str());
	}

	template <>
	void Clipboard::SetClipboard(std::basic_string<wchar_t> str) {
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str).c_str());
	}
}
