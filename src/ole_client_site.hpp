#pragma once

#include "win_include.hpp"

namespace momo
{
	struct ole_client_site : public IOleClientSite
	{
		HRESULT STDMETHODCALLTYPE SaveObject() override
		{
			return E_NOTIMPL;
		}

		HRESULT STDMETHODCALLTYPE GetMoniker(DWORD, DWORD, IMoniker**) override
		{
			return E_NOTIMPL;
		}

		HRESULT STDMETHODCALLTYPE GetContainer(LPOLECONTAINER* ppContainer) override
		{
			*ppContainer = nullptr;
			return E_NOINTERFACE;
		}

		HRESULT STDMETHODCALLTYPE ShowObject() override
		{
			return NOERROR;
		}

		HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL) override
		{
			return E_NOTIMPL;
		}

		HRESULT STDMETHODCALLTYPE RequestNewObjectLayout() override
		{
			return E_NOTIMPL;
		}
	};
}
