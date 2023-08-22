#include "html_frame.hpp"

#include "ie_setup.hpp"

#include <stdexcept>

namespace momo
{
	html_frame::html_frame()
	{
		setup_internet_explorer();
	}

	HRESULT html_frame::GetHostInfo(DOCHOSTUIINFO* pInfo)
	{
		pInfo->cbSize = sizeof(DOCHOSTUIINFO);
		pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DPI_AWARE | DOCHOSTUIFLAG_SCROLL_NO;
		pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

		return S_OK;
	}

	HRESULT html_frame::GetWindow(HWND* lphwnd)
	{
		*lphwnd = this->window_;
		return S_OK;
	}

	HRESULT html_frame::QueryInterface(REFIID riid, void** ppvObject)
	{
		if (IsEqualGUID(riid, IID_IDispatch))
		{
			*ppvObject = static_cast<IDispatch*>(this);
			return S_OK;
		}

		if (IsEqualGUID(riid, IID_IDispatch))
		{
			const auto d = get_dispatch();
			if (!d)
			{
				return E_NOINTERFACE;
			}

			(*d).AddRef();
			*ppvObject = &*d;
			return S_OK;
		}

		if (IsEqualGUID(riid, IID_IServiceProvider))
		{
			*ppvObject = static_cast<IServiceProvider*>(this);
			return S_OK;
		}

		if (IsEqualGUID(riid, IID_IInternetSecurityManager))
		{
			*ppvObject = static_cast<IInternetSecurityManager*>(this);
			return S_OK;
		}

		if (IsEqualGUID(riid, IID_IUnknown))
		{
			*ppvObject = static_cast<IUnknown*>(static_cast<IOleClientSite*>(this));
			return S_OK;
		}

		if (IsEqualGUID(riid, IID_IOleClientSite))
		{
			*ppvObject = static_cast<IOleClientSite*>(this);
			return S_OK;
		}

		if (IsEqualGUID(riid, IID_IOleInPlaceSite))
		{
			*ppvObject = static_cast<IOleInPlaceSite*>(this);
			return S_OK;
		}

		if (IsEqualGUID(riid, IID_IOleInPlaceFrame))
		{
			*ppvObject = static_cast<IOleInPlaceFrame*>(this);
			return S_OK;
		}

		if (IsEqualGUID(riid, IID_IDocHostUIHandler))
		{
			*ppvObject = static_cast<IDocHostUIHandler*>(this);
			return S_OK;
		}

		if (IsEqualGUID(riid, IID_IOleInPlaceObject) && this->browser_object_)
		{
			return this->browser_object_->QueryInterface(riid, ppvObject);
		}

		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	HWND html_frame::get_window() const
	{
		return this->window_;
	}

	CComPtr<IOleObject> html_frame::get_browser_object() const
	{
		return this->browser_object_;
	}

	CComPtr<IWebBrowser2> html_frame::get_web_browser() const
	{
		CComPtr<IWebBrowser2> web_browser{};
		if (!this->browser_object_ || FAILED(this->browser_object_.QueryInterface(&web_browser)))
		{
			return {};
		}

		return web_browser;
	}

	CComPtr<IDispatch> html_frame::get_dispatch() const
	{
		const auto web_browser = this->get_web_browser();

		CComPtr<IDispatch> dispatch{};
		if (!web_browser || FAILED(web_browser->get_Document(&dispatch)))
		{
			return {};
		}

		return dispatch;
	}

	CComPtr<IHTMLDocument2> html_frame::get_document() const
	{
		const auto dispatch = this->get_dispatch();

		CComPtr<IHTMLDocument2> document{};
		if (!dispatch || FAILED(dispatch.QueryInterface(&document)))
		{
			return {};
		}

		return document;
	}

	void html_frame::initialize(const HWND window)
	{
		if (this->window_) return;
		this->window_ = window;

		this->create_browser();
		this->initialize_browser();
	}

	void html_frame::create_browser()
	{
		CComPtr<IClassFactory> class_factory{};
		if (FAILED(
			CoGetClassObject(CLSID_WebBrowser, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, nullptr, IID_IClassFactory,
				reinterpret_cast<void**>(&class_factory))))
		{
			throw std::runtime_error("Unable to get the class factory");
		}

		class_factory->CreateInstance(nullptr, IID_IOleObject, reinterpret_cast<void**>(&this->browser_object_));

		if (!this->browser_object_)
		{
			throw std::runtime_error("Unable to create browser object");
		}
	}

	void html_frame::initialize_browser()
	{
		this->browser_object_->SetClientSite(this);
		this->browser_object_->SetHostNames(L"Hostname", nullptr);

		RECT rect;
		GetClientRect(this->get_window(), &rect);
		OleSetContainedObject(this->browser_object_, TRUE);

		this->browser_object_->DoVerb(OLEIVERB_SHOW, nullptr, this, -1, this->get_window(), &rect);
		this->resize(rect.right, rect.bottom);
	}

	void html_frame::resize(const DWORD width, const DWORD height) const
	{
		const auto web_browser = this->get_web_browser();
		if (web_browser)
		{
			web_browser->put_Left(0);
			web_browser->put_Top(0);
			web_browser->put_Width(static_cast<long>(width));
			web_browser->put_Height(static_cast<long>(height));
		}
	}

	bool html_frame::load_url(const std::string& url) const
	{
		const auto web_browser = this->get_web_browser();
		if (!web_browser) return false;

		CComVariant my_url(url.data());
		return SUCCEEDED(web_browser->Navigate2(&my_url, nullptr, nullptr, nullptr, nullptr));
	}

	bool html_frame::load_html(const std::string& html) const
	{
		if (!this->load_url("about:blank")) return false;

		const auto document = this->get_document();
		if (!document) return false;

		CComSafeArrayBound bound{};
		bound.SetCount(1);
		bound.SetLowerBound(0);

		CComSafeArray<VARIANT> array(&bound, 1);
		array[0] = CComVariant(html.data());

		document->write(array);
		document->close();

		return true;
	}

	html_value html_frame::evaluate(const std::string& javascript) const
	{
		auto dispDoc = this->get_dispatch();

		CComPtr<IHTMLDocument2> htmlDoc;
		dispDoc->QueryInterface(&htmlDoc);

		CComPtr<IHTMLWindow2> htmlWindow;
		htmlDoc->get_parentWindow(&htmlWindow);

		CComDispatchDriver dispWindow;
		htmlWindow->QueryInterface(&dispWindow);

		CComPtr<IDispatchEx> dispexWindow;
		htmlWindow->QueryInterface(&dispexWindow);

		DISPID dispidEval = -1;
		dispexWindow->GetDispID(CComBSTR("eval"), fdexNameCaseSensitive, &dispidEval);

		CComVariant result{};
		CComVariant code(javascript.data());
		(void)dispWindow.Invoke1(dispidEval, &code, &result);

		return result;
	}

	int html_frame::get_callback_id(const std::string& name) const
	{
		for (auto i = 0u; i < this->callbacks_.size(); ++i)
		{
			if (this->callbacks_[i].first == name)
			{
				return static_cast<int>(i);
			}
		}

		return -1;
	}

	html_value html_frame::invoke_callback(const int id, const std::vector<html_value>& params) const
	{
		if (id >= 0 && static_cast<unsigned int>(id) < this->callbacks_.size())
		{
			try
			{
				return this->callbacks_[id].second(params);
			}
			catch (const std::exception& e)
			{
				OutputDebugStringA(e.what());
			}
		}

		return {};
	}

	void html_frame::register_callback(const std::string& name, callback_function callback)
	{
		this->callbacks_.emplace_back(name, std::move(callback));
	}

	HRESULT html_frame::GetIDsOfNames(const IID& /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID /*lcid*/,
	                                  DISPID* rgDispId)
	{
		for (unsigned int i = 0; i < cNames; ++i)
		{
			std::wstring wide_name(rgszNames[i]);
#pragma warning(push)
#pragma warning(disable : 4244)
			std::string name(wide_name.begin(), wide_name.end());
#pragma warning(pop)

			rgDispId[i] = this->get_callback_id(name);
		}

		return S_OK;
	}

	HRESULT html_frame::Invoke(const DISPID dispIdMember, const IID& /*riid*/, LCID /*lcid*/, WORD /*wFlags*/,
	                           DISPPARAMS* pDispParams,
	                           VARIANT* pVarResult, EXCEPINFO* /*pExcepInfo*/, UINT* /*puArgErr*/)
	{
		std::vector<html_value> params{};
		for (auto i = pDispParams->cArgs; i > 0; --i)
		{
			auto& param = pDispParams->rgvarg[i - 1];
			params.emplace_back(param);
		}

		auto res = this->invoke_callback(dispIdMember, params);
		res.move_to(pVarResult, false);

		return S_OK;
	}

	HRESULT html_frame::GetExternal(IDispatch** ppDispatch)
	{
		*ppDispatch = this;
		return *ppDispatch ? S_OK : S_FALSE;
	}
}
