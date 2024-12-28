#pragma once

#include <string>
#include <string_view>

namespace Core::Clipboard {
	std::u8string GetClipboard();
	std::wstring GetClipboardWide();

	template <typename T>
	std::basic_string<T> GetClipboard();
	
	void SetClipboard(std::u8string_view);
	void SetClipboard(std::string_view);
	void SetClipboardWide(std::wstring_view);

	template <typename T>
	void SetClipboard(std::basic_string<T>);
}