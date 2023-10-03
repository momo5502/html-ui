#include "win_include.hpp"

#include <momo/html_value.hpp>

namespace momo
{
	html_value::html_value(const html_value& obj)
	{
		this->operator=(obj);
	}

	html_value::html_value(html_value&& obj) noexcept
	{
		this->operator=(std::move(obj));
	}

	html_value::html_value(const CComVariant& val)
	{
		this->operator=(val);
	}

	html_value::html_value(CComVariant&& val) noexcept
	{
		this->operator=(std::move(val));
	}

	html_value& html_value::operator=(const html_value& obj)
	{
		this->operator=(obj.value_);
		return *this;
	}

	html_value& html_value::operator=(html_value&& obj) noexcept
	{
		static_assert(sizeof(CComVariant) == sizeof(html_value));

		this->operator=(std::move(obj.value_));
		return *this;
	}

	html_value& html_value::operator=(const CComVariant& val)
	{
		if (&this->value_ != &val)
		{
			this->value_ = val;
		}

		return *this;
	}

	html_value& html_value::operator=(CComVariant&& val) noexcept
	{
		static_assert(sizeof(CComVariant) == sizeof(VARIANT));
		static_assert(sizeof(CComVariant) == sizeof(this->value_));
		static_assert(sizeof(CComVariant) == sizeof(val));

		if (&this->value_ != &val)
		{
			VariantClear(&this->value_);

			VARIANT* this_variant = &this->value_;
			const VARIANT* val_variant = &val;

			memcpy(this_variant, val_variant, sizeof(val));

			VariantInit(&val);
		}

		return *this;
	}

	void html_value::move_to(VARIANT& var, const bool is_initialized)
	{
		if (is_initialized)
		{
			VariantClear(&var);
		}

		VARIANT* this_variant = &this->value_;
		memcpy(&var, this_variant, sizeof(var));

		VariantInit(this_variant);
	}

	void html_value::move_to(VARIANT* var, const bool is_initialized)
	{
		if (var)
		{
			this->move_to(*var, is_initialized);
		}
	}

	html_value::operator bool() const
	{
		return !this->is(VT_EMPTY);
	}

	bool html_value::is(const VARENUM type) const
	{
		return (this->value_.vt & VT_TYPEMASK) == type;
	}

	/***************************************************************
	 * Integer
	 **************************************************************/

	template <>
	bool html_value::is<int8_t>() const
	{
		return this->is(VT_I1);
	}

	template <>
	bool html_value::is<int16_t>() const
	{
		return this->is(VT_I2);
	}

	template <>
	bool html_value::is<int32_t>() const
	{
		return this->is(VT_I4);
	}

	template <>
	bool html_value::is<int64_t>() const
	{
		return this->is(VT_I8);
	}

	template <>
	bool html_value::is<uint8_t>() const
	{
		return this->is(VT_UI1);
	}

	template <>
	bool html_value::is<uint16_t>() const
	{
		return this->is(VT_UI2);
	}

	template <>
	bool html_value::is<uint32_t>() const
	{
		return this->is(VT_UI4);
	}

	template <>
	bool html_value::is<uint64_t>() const
	{
		return this->is(VT_UI8);
	}

	template <>
	bool html_value::is<bool>() const
	{
		return this->is(VT_BOOL);
	}

	template <>
	int8_t html_value::get() const
	{
		return this->value_.cVal;
	}

	template <>
	int16_t html_value::get() const
	{
		return this->value_.iVal;
	}

	template <>
	int32_t html_value::get() const
	{
		return this->value_.intVal;
	}

	template <>
	int64_t html_value::get() const
	{
		return this->value_.llVal;
	}

	template <>
	uint8_t html_value::get() const
	{
		return this->value_.bVal;
	}

	template <>
	uint16_t html_value::get() const
	{
		return this->value_.uiVal;
	}

	template <>
	uint32_t html_value::get() const
	{
		return this->value_.uintVal;
	}

	template <>
	uint64_t html_value::get() const
	{
		return this->value_.ullVal;
	}

	template <>
	bool html_value::get() const
	{
		return this->value_.boolVal;
	}

	/***************************************************************
	 * Float
	 **************************************************************/

	template <>
	bool html_value::is<float>() const
	{
		return this->is(VT_R4);
	}

	template <>
	bool html_value::is<double>() const
	{
		return this->is(VT_R8);
	}

	template <>
	float html_value::get() const
	{
		return this->value_.fltVal;
	}

	template <>
	double html_value::get() const
	{
		return this->value_.dblVal;
	}

	/***************************************************************
	 * String
	 **************************************************************/

	template <>
	bool html_value::is<const wchar_t*>() const
	{
		return this->is(VT_BSTR);
	}

	template <>
	bool html_value::is<std::wstring>() const
	{
		return this->is<const wchar_t*>();
	}

	template <>
	bool html_value::is<std::string>() const
	{
		return this->is<std::wstring>();
	}

	template <>
	const wchar_t* html_value::get() const
	{
		return this->value_.bstrVal;
	}

	template <>
	std::wstring html_value::get() const
	{
		return this->get<const wchar_t*>();
	}

	template <>
	std::string html_value::get() const
	{
		const auto wide_string = this->get<std::wstring>();

#pragma warning(push)
#pragma warning(disable : 4244)
		return {wide_string.begin(), wide_string.end()};
#pragma warning(pop)
	}

	uint64_t html_value::as_integer() const
	{
		if (this->is<uint64_t>())
		{
			return get<uint64_t>();
		}

		if (this->is<uint32_t>())
		{
			return get<uint32_t>();
		}

		if (this->is<uint16_t>())
		{
			return get<uint16_t>();
		}

		if (this->is<uint8_t>())
		{
			return get<uint8_t>();
		}

		if (this->is<int64_t>())
		{
			return static_cast<uint64_t>(get<int64_t>());
		}

		if (this->is<int32_t>())
		{
			return static_cast<uint64_t>(static_cast<int64_t>(get<int32_t>()));
		}

		if (this->is<int16_t>())
		{
			return static_cast<uint64_t>(static_cast<int64_t>(get<int16_t>()));
		}

		if (this->is<int8_t>())
		{
			return static_cast<uint64_t>(static_cast<int64_t>(get<int8_t>()));
		}

		throw std::runtime_error("Invalid type");
	}
}
