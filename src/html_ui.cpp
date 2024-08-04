#include <momo/html_ui.hpp>

#include "html_window.hpp"

namespace momo
{
	html_ui::html_ui(const std::string& title, const size_t width, const size_t height)
		: window_(std::make_unique<html_window>(title, static_cast<int>(width), static_cast<int>(height)))
	{
	}

	html_ui::html_ui(const std::wstring& title, const size_t width, const size_t height)
		: window_(std::make_unique<html_window>(title, static_cast<int>(width), static_cast<int>(height)))
	{
	}

	html_ui::~html_ui() = default;
	html_ui::html_ui(html_ui&&) noexcept = default;
	html_ui& html_ui::operator=(html_ui&&) noexcept = default;

	void html_ui::resize(const size_t width, const size_t height)
	{
		this->window_->get_html_frame().resize(static_cast<DWORD>(width), static_cast<DWORD>(height));
	}

	bool html_ui::load_url(const std::string& url)
	{
		return this->window_->get_html_frame().load_url(url);
	}

	bool html_ui::load_html(const std::string& html)
	{
		return this->window_->get_html_frame().load_html(html);
	}

	void html_ui::show_windows()
	{
		window::run();
	}

	void html_ui::close() const
	{
		this->window_->get_window().close();
	}

	html_value html_ui::evaluate(const std::string& javascript) const
	{
		return this->window_->get_html_frame().evaluate(javascript);
	}

	void html_ui::register_raw_handler(const std::string& name, detail::handler_type handler)
	{
		this->window_->get_html_frame().register_callback(name, std::move(handler));
	}
}
