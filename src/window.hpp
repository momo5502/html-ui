#pragma once

#include <string>
#include <optional>
#include <functional>

#include "win_include.hpp"

namespace momo
{
	class window
	{
	public:
		window(const std::string& title, int width, int height,
		       std::function<std::optional<LRESULT>(window*, UINT, WPARAM, LPARAM)> callback,
		       long flags = (WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX)));

		window(const std::wstring& title, int width, int height,
			std::function<std::optional<LRESULT>(window*, UINT, WPARAM, LPARAM)> callback,
			long flags = (WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX)));

		virtual ~window();

		void close();

		operator HWND() const;

		static void run();

		LRESULT processor(UINT message, WPARAM w_param, LPARAM l_param);

	private:
		uint32_t last_dpi_ = 96;

		WNDCLASSEXW wc_{};
		HWND handle_ = nullptr;
		std::wstring classname_;
		std::function<std::optional<LRESULT>(window*, UINT, WPARAM, LPARAM)> callback_;

		static LRESULT CALLBACK static_processor(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
	};
}
