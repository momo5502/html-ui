#pragma once

#include "win_include.hpp"

namespace momo
{
	struct dispatch : public IDispatch
	{
		HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctinfo) override
		{
			return S_FALSE;
		}

		HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override
		{
			return S_FALSE;
		}
	};
}
