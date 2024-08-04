#include "window.hpp"

#include <ctime>
#include <dwmapi.h>
#pragma comment (lib, "dwmapi.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace momo
{
	namespace
	{
		thread_local uint32_t window_count = 0;

		uint32_t get_dpi_for_window(const HWND window)
		{
			auto* user32 = GetModuleHandleA("user32.dll");
			const auto get_dpi = user32
				                     ? reinterpret_cast<UINT(WINAPI*)(HWND)>(GetProcAddress(user32, "GetDpiForWindow"))
				                     : nullptr;

			if (!get_dpi)
			{
				return USER_DEFAULT_SCREEN_DPI;
			}

			return get_dpi(window);
		}

		std::wstring convert_utf8_to_wide(const std::string& str)
		{
			const auto count = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);

			std::wstring wstr(count, 0);
			MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), wstr.data(), count);

			return wstr;
		}
	}

	window::window(const std::string& title, const int width, const int height,
		std::function<std::optional<LRESULT>(window*, UINT, WPARAM, LPARAM)> callback,
		const long flags)
		: window(convert_utf8_to_wide(title), width, height,std::move(callback), flags)
	{

	}

	window::window(const std::wstring& title, const int width, const int height,
	               std::function<std::optional<LRESULT>(window*, UINT, WPARAM, LPARAM)> callback,
	               const long flags)
		: callback_(std::move(callback))
	{
		ZeroMemory(&this->wc_, sizeof(this->wc_));

		this->classname_ = L"window-base-" + std::to_wstring(time(nullptr));

		this->wc_.cbSize = sizeof(this->wc_);
		this->wc_.style = CS_HREDRAW | CS_VREDRAW;
		this->wc_.lpfnWndProc = &static_processor;
		this->wc_.hInstance = GetModuleHandleA(nullptr);
		this->wc_.hCursor = LoadCursorA(nullptr, IDC_ARROW);
		this->wc_.hIcon = LoadIconW(this->wc_.hInstance, MAKEINTRESOURCEW(102));
		this->wc_.hIconSm = this->wc_.hIcon;
		this->wc_.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
		this->wc_.lpszClassName = this->classname_.data();
		RegisterClassExW(&this->wc_);

		const auto x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		const auto y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

		++window_count;

		this->handle_ = CreateWindowExW(NULL, this->wc_.lpszClassName, title.data(), flags, x, y,
		                                width, height, nullptr, nullptr, this->wc_.hInstance, this);

		constexpr BOOL value = TRUE;
		DwmSetWindowAttribute(this->handle_,
		                      DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

		SendMessageA(this->handle_, WM_DPICHANGED, 0, 0);
		ShowWindow(this->handle_, SW_SHOW);
		SetForegroundWindow(this->handle_);
	}

	window::~window()
	{
		this->close();
		UnregisterClassW(this->wc_.lpszClassName, this->wc_.hInstance);
	}

	void window::close()
	{
		if (!this->handle_) return;

		DestroyWindow(this->handle_);
		this->handle_ = nullptr;
	}

	void window::run()
	{
		MSG msg{};
		while (GetMessageW(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	LRESULT window::processor(const UINT message, const WPARAM w_param, const LPARAM l_param)
	{
		if (message == WM_DPICHANGED)
		{
			const auto dpi = get_dpi_for_window(*this);
			if (dpi != this->last_dpi_)
			{
				RECT rect;
				GetWindowRect(*this, &rect);

				const auto scale = dpi * 1.0 / this->last_dpi_;
				this->last_dpi_ = dpi;

				const auto width = rect.right - rect.left;
				const auto height = rect.bottom - rect.top;

				MoveWindow(*this, rect.left, rect.top, static_cast<int>(width * scale),
				           static_cast<int>(height * scale),
				           TRUE);
			}
		}

		if (message == WM_DESTROY)
		{
			if (--window_count == 0)
			{
				PostQuitMessage(0);
			}

			return TRUE;
		}

		if (this->callback_)
		{
			const auto res = this->callback_(this, message, w_param, l_param);
			if (res)
			{
				return *res;
			}
		}

		return DefWindowProcW(*this, message, w_param, l_param);
	}

	LRESULT CALLBACK window::static_processor(const HWND hwnd, const UINT message, const WPARAM w_param,
	                                          const LPARAM l_param)
	{
		if (message == WM_CREATE)
		{
			auto* data = reinterpret_cast<LPCREATESTRUCT>(l_param);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data->lpCreateParams));

			static_cast<window*>(data->lpCreateParams)->handle_ = hwnd;
		}

		const auto self = reinterpret_cast<window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (self) return self->processor(message, w_param, l_param);

		return DefWindowProcW(hwnd, message, w_param, l_param);
	}

	window::operator HWND() const
	{
		return this->handle_;
	}
}
