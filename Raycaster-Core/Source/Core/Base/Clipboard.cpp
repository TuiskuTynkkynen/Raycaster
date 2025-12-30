#include "Clipboard.h"

#include "Application.h"
#include "Window.h"


#include "Platform.h"
#ifdef PLATFORM_WINDOWS
	#include <Windows.h>
#elif defined(PLATFORM_LINUX)
	#ifndef __clang__
		#error "Only clang is supported on Linux"
	#endif 

	#include <codecvt>
	#include <locale>

	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <GLFW/glfw3.h>

namespace Core {
	std::u8string Clipboard::GetClipboard() {
		return (const char8_t*)glfwGetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()));
	}

	std::wstring Clipboard::GetClipboardWide() {
#ifdef PLATFORM_WINDOWS
		const char* clipboard = glfwGetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()));
		int32_t utf8Size = static_cast<int32_t>(std::strlen(clipboard));
		int32_t wideSize = MultiByteToWideChar(CP_UTF8, 0, clipboard, utf8Size, NULL, 0);
		std::wstring wide(wideSize, 0);
		MultiByteToWideChar(CP_UTF8, 0, clipboard, utf8Size, wide.data(), wideSize);
		return wide;
#else
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(glfwGetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer())));
#endif
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
#ifdef PLATFORM_WINDOWS
		int32_t wideSize = static_cast<int32_t>(str.size());
		int32_t utf8Size = WideCharToMultiByte(CP_UTF8, 0, str.data(), wideSize, NULL, 0, NULL, NULL);
		std::string utf8(utf8Size, 0);
		WideCharToMultiByte(CP_UTF8, 0, str.data(), wideSize, &utf8[0], utf8Size, NULL, NULL);
		
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), utf8.c_str());
#else
		glfwSetClipboardString(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str.data()).c_str());
#endif 
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

#ifdef PLATFORM_LINUX
	# pragma clang diagnostic pop
#endif