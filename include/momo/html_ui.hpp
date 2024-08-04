#pragma once
#include <string>
#include <memory>
#include <functional>

#include <vector>
#include <functional>
#include <type_traits>

#include "html_value.hpp"

namespace momo
{
	namespace detail
	{
		using handler_type = std::function<html_value(const std::vector<html_value>&)>;

		template <typename T>
		static T resolve_html_value(const std::vector<html_value>& args, size_t& index)
		{
			const auto current_index = index++;
			return args.at(current_index).as<T>();
		}

		template <typename F, typename Return, typename... Args>
		handler_type make_handler(F&& f)
		{
			return [fun = std::forward<F>(f)](const std::vector<html_value>& args) -> html_value
			{
				if (args.size() != sizeof...(Args))
				{
					throw std::runtime_error("Bad argument count");
				}

				size_t index = 0;
				std::tuple func_args
				{
					resolve_html_value<std::remove_cv_t<std::remove_reference_t<Args>>>(args, index)...
				};

				(void)index;

				if constexpr (std::is_same_v<Return, void>)
				{
					std::apply(fun, std::move(func_args));
					return {};
				}
				else
				{
					auto ret = std::apply(fun, std::move(func_args));
					return html_value(std::move(ret));
				}
			};
		}

		template <typename T>
		struct callback_creator;

		template <typename Return, typename Class, typename... Args>
		struct callback_creator<Return(Class::*)(Args...) const>
		{
			template <typename F>
			static handler_type create(F&& func)
			{
				return make_handler<F, Return, Args...>(std::forward<F>(func));
			}
		};

		template <typename Return, typename... Args>
		struct callback_creator<Return(*)(Args...)>
		{
			template <typename F>
			static handler_type create(F&& func)
			{
				return make_handler<F, Return, Args...>(std::forward<F>(func));
			}
		};
	}

	class html_window;

	class html_ui
	{
	public:
		html_ui(const std::string& title, size_t width, size_t height);
		html_ui(const std::wstring& title, size_t width, size_t height);
		~html_ui();

		html_ui(const html_ui&) = delete;
		html_ui& operator=(const html_ui&) = delete;

		html_ui(html_ui&&) noexcept;
		html_ui& operator=(html_ui&&) noexcept;

		void resize(size_t width, size_t height);
		bool load_url(const std::string& url);
		bool load_html(const std::string& html);
		
		void close() const;

		html_value evaluate(const std::string& javascript) const;

		void register_raw_handler(const std::string& name, detail::handler_type handler);

		template <typename F>
		void register_handler(const std::string& name, F&& func)
		{
			this->register_raw_handler(
				name, detail::callback_creator<decltype(&F::operator())>::create(std::forward<F>(func)));
		}

		template <typename Return, typename... Args>
		void register_handler(const std::string& name, Return(*func)(Args...))
		{
			using FunctionType = Return(*)(Args...);
			this->register_raw_handler(name, detail::callback_creator<FunctionType>::create(func));
		}

		static void show_windows();

	private:
		std::unique_ptr<html_window> window_;
	};
}
