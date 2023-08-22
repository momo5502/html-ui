#pragma once

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4458)
#pragma warning(disable : 6297)
#pragma warning(disable : 6385)
#pragma warning(disable : 6386)
#pragma warning(disable : 26451)
#pragma warning(disable : 26444)
#pragma warning(disable : 26451)
#pragma warning(disable : 26495)
#pragma warning(disable : 28020)

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <MsHTML.h>
#include <MsHtmHst.h>
#include <ExDisp.h>
#include <atlbase.h>
#include <atlsafe.h>
#include <shellscalingapi.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#pragma warning(pop)
