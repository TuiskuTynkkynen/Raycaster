#include "Clipboard.h"

#include "Application.h"
#include "Window.h"
#include "Platform.h"

#include <GLFW/glfw3.h>

namespace Core {
	std::u8string Clipboard::GetClipboard() {
		return (const char8_t*)glfwGetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()));
	}

	std::wstring Clipboard::GetClipboardWide() {
		const char* clipboard = glfwGetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()));
		return UTF8ToWide(clipboard);
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
		return GetClipboardWide();
	}

	void Clipboard::SetClipboard(std::u8string_view str) {
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), (const char*)str.data());
	}

	void Clipboard::SetClipboard(std::string_view str) {
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), str.data());
	}

	void Clipboard::SetClipboardWide(std::wstring_view str) {
		std::string utf8 = WideToUTF8(str);
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), utf8.c_str());
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
		SetClipboardWide(str);
	}
}