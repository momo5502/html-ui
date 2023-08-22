#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include <atlbase.h>
#include <atlsafe.h>

#include <string>
#include <stdexcept>

namespace momo
{
	class html_value final
	{
	public:
		html_value() = default;
		html_value(const html_value& obj);
		html_value(html_value&& obj) noexcept;

		html_value(const CComVariant& val);
		html_value(CComVariant&& val) noexcept;

		html_value& operator=(const html_value& obj);
		html_value& operator=(html_value&& obj) noexcept;

		html_value& operator=(const CComVariant& val);
		html_value& operator=(CComVariant&& val) noexcept;

		html_value(const char* str)
		{
			this->value_ = str;
		}

		html_value(const std::string& str)
			: html_value(str.data())
		{
		}

		html_value(const wchar_t* str)
		{
			this->value_ = str;
		}

		html_value(const std::wstring& str)
			: html_value(str.data())
		{
		}

		CComVariant& get()
		{
			return this->value_;
		}

		const CComVariant& get() const
		{
			return this->value_;
		}

		void move_to(VARIANT* var, bool is_initialized = true);
		void move_to(VARIANT& var, bool is_initialized = true);

		operator bool() const;

		bool is(VARENUM type) const;

		template <typename T>
		bool is() const;

		template <typename T>
		T get() const;

		uint64_t as_integer() const;

		template <typename T>
		T as() const
		{
			if (!this->is<T>())
			{
				throw std::runtime_error("Invalid type");
			}

			return get<T>();
		}

		template <>
		uint64_t as() const
		{
			return this->as_integer();
		}

		template <>
		uint32_t as() const
		{
			return static_cast<uint32_t>(this->as<uint64_t>());
		}

		template <>
		uint16_t as() const
		{
			return static_cast<uint16_t>(this->as<uint32_t>());
		}

		template <>
		uint8_t as() const
		{
			return static_cast<uint8_t>(this->as<uint16_t>());
		}

		template <>
		int64_t as() const
		{
			return static_cast<int64_t>(this->as_integer());
		}

		template <>
		int32_t as() const
		{
			return static_cast<int32_t>(this->as<int64_t>());
		}

		template <>
		int16_t as() const
		{
			return static_cast<int16_t>(this->as<int32_t>());
		}

		template <>
		int8_t as() const
		{
			return static_cast<int8_t>(this->as<int16_t>());
		}

		template <>
		bool as() const
		{
			return this->as<uint64_t>() != 0;
		}

	private:

		CComVariant value_{};
	};
}
