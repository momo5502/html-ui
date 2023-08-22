#pragma once

#include "win_include.hpp"

#include <vector>
#include <functional>

#include "ole_in_place_frame.hpp"
#include "ole_in_place_site.hpp"
#include "doc_host_ui_handler.hpp"
#include "ole_client_site.hpp"
#include "service_provider.hpp"
#include "internet_security_manager.hpp"
#include "dispatch.hpp"

#include <momo/html_value.hpp>

namespace momo
{
	class html_frame
		: doc_host_ui_handler
		  , service_provider
		  , internet_security_manager
		  , ole_client_site
		  , ole_in_place_frame
		  , ole_in_place_site
		  , dispatch
	{
	public:
		html_frame();
		html_frame(const html_frame&) = delete;
		html_frame& operator=(const html_frame&) = delete;
		html_frame(html_frame&&) = delete;
		html_frame& operator=(html_frame&&) = delete;

		~html_frame() = default;

		void initialize(HWND window);

		void resize(DWORD width, DWORD height) const;
		bool load_url(const std::string& url) const;
		bool load_html(const std::string& html) const;

		html_value evaluate(const std::string& javascript) const;

		HWND get_window() const;

		CComPtr<IOleObject> get_browser_object() const;
		CComPtr<IWebBrowser2> get_web_browser() const;
		CComPtr<IDispatch> get_dispatch() const;
		CComPtr<IHTMLDocument2> get_document() const;

		int get_callback_id(const std::string& name) const;
		html_value invoke_callback(int id, const std::vector<html_value>& params) const;

		using callback_function = std::function<html_value(const std::vector<html_value>&)>;
		void register_callback(const std::string& name, callback_function callback);

		HRESULT STDMETHODCALLTYPE QueryInterface(
			REFIID riid,
			void** ppvObject) override;

	private:
		HWND window_ = nullptr;
		CComPtr<IOleObject> browser_object_;

		std::vector<std::pair<std::string, callback_function>> callbacks_;

		void create_browser();
		void initialize_browser();

		HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO* pInfo) override;
		HRESULT STDMETHODCALLTYPE GetWindow(HWND* lphwnd) override;

		ULONG STDMETHODCALLTYPE AddRef() override
		{
			return 1;
		}

		ULONG STDMETHODCALLTYPE Release() override
		{
			return 1;
		}

		HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid,
		                                        DISPID* rgDispId)
		override;
		HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
		                                 DISPPARAMS* pDispParams,
		                                 VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override;


		HRESULT STDMETHODCALLTYPE GetExternal(IDispatch** ppDispatch) override;
	};
}
