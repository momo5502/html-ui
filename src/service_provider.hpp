#pragma once

#include "win_include.hpp"

namespace momo
{
	struct service_provider : public IServiceProvider
	{
		HRESULT STDMETHODCALLTYPE QueryService(REFGUID guidService, REFIID riid, void** ppvObject) override
		{
			if (IsEqualGUID(riid, IID_IInternetSecurityManager))
			{
				return QueryInterface(riid, ppvObject);
			}

			*ppvObject = nullptr;
			return E_NOINTERFACE;
		}
	};
}
