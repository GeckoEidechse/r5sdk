#pragma once
#pragma message("Pre-compiling headers.\n")

#define WIN32_LEAN_AND_MEAN // Prevent winsock2 redefinition.
#include <windows.h>
#include <WinSock2.h>
#include <comdef.h>
#include <gdiplus.h>
#include <timeapi.h>
#include <shellapi.h>
#include <Psapi.h>
#include <setjmp.h>
#include <tchar.h>
#include <stdio.h>
#include <shlobj.h>
#include <objbase.h>
#include <intrin.h>
#include <emmintrin.h>
#include <cmath>
#include <regex>
#include <thread>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <filesystem>
#include <unordered_set>

#if !defined(DEDICATED) && !defined(SDKLAUNCHER) && !defined (NETCONSOLE) && !defined(PLUGINSDK)
#include <d3d11.h>
#endif // !DEDICATED && !SDKLAUNCHER && !NETCONSOLE && !PLUGINSDK

#include "thirdparty/nlohmann/json.hpp"

#include "thirdparty/detours/include/detours.h"
#include "thirdparty/detours/include/idetour.h"

#if defined(SDKLAUNCHER)
#include "thirdparty/cppnet/cppkore/Kore.h"
#include "thirdparty/cppnet/cppkore/UIXTheme.h"
#include "thirdparty/cppnet/cppkore/UIXLabel.h"
#include "thirdparty/cppnet/cppkore/UIXListView.h"
#include "thirdparty/cppnet/cppkore/UIXCheckBox.h"
#include "thirdparty/cppnet/cppkore/UIXComboBox.h"
#include "thirdparty/cppnet/cppkore/UIXTextBox.h"
#include "thirdparty/cppnet/cppkore/UIXGroupBox.h"
#include "thirdparty/cppnet/cppkore/UIXButton.h"
#include "thirdparty/cppnet/cppkore/UIXRadioButton.h"
#include "thirdparty/cppnet/cppkore/KoreTheme.h"

#include "launcher/launcherdefs.h"
#endif // SDKLAUNCHER

#if !defined(DEDICATED) && !defined(SDKLAUNCHER) && !defined (NETCONSOLE) && !defined(PLUGINSDK)
#include "thirdparty/imgui/include/imgui.h"
#include "thirdparty/imgui/include/imgui_stdlib.h"
#include "thirdparty/imgui/include/imgui_logger.h"
#include "thirdparty/imgui/include/imgui_editor.h"
#include "thirdparty/imgui/include/imgui_utility.h"
#include "thirdparty/imgui/include/imgui_internal.h"
#include "thirdparty/imgui/include/imgui_impl_dx11.h"
#include "thirdparty/imgui/include/imgui_impl_win32.h"
#endif // !DEDICATED && !SDKLAUNCHER && !NETCONSOLE && !PLUGINSDK

#if !defined(SDKLAUNCHER) && !defined (NETCONSOLE) && !defined(PLUGINSDK)
#include "thirdparty/lzham/include/lzham_types.h"
#include "thirdparty/lzham/include/lzham.h"
#endif // !SDKLAUNCHER && !NETCONSOLE && !PLUGINSDK

#include "thirdparty/spdlog/include/spdlog.h"
#include "thirdparty/spdlog/include/async.h"
#include "thirdparty/spdlog/include/sinks/ostream_sink.h"
#include "thirdparty/spdlog/include/sinks/basic_file_sink.h"
#include "thirdparty/spdlog/include/sinks/stdout_sinks.h"
#include "thirdparty/spdlog/include/sinks/stdout_color_sinks.h"
#include "thirdparty/spdlog/include/sinks/ansicolor_sink.h"
#include "thirdparty/spdlog/include/sinks/rotating_file_sink.h"

#include "common/pseudodefs.h"
#include "common/x86defs.h"
#include "common/sdkdefs.h"

#include "public/utility/utility.h"
#include "public/utility/memaddr.h"
#include "public/utility/module.h"
#include "public/utility/sigcache.h"
#include "public/utility/httplib.h"
#include "public/utility/vdf_parser.h"

#include "core/assert.h"
#include "core/termutil.h"
#include "tier0/basetypes.h"
#include "tier0/platform.h"
#include "tier0/commonmacros.h"
#if !defined(SDKLAUNCHER) && !defined (NETCONSOLE) && !defined(PLUGINSDK)
#include "tier0/dbg.h"
#endif // !SDKLAUNCHER && !NETCONSOLE && !PLUGINSDK

#if !defined(SDKLAUNCHER) && !defined (NETCONSOLE) && !defined(PLUGINSDK)
#if !defined (DEDICATED)
inline CModule g_GameDll = CModule("r5apex.exe");
inline CModule g_RadVideoToolsDll   = CModule("bink2w64.dll");
inline CModule g_RadAudioDecoderDll = CModule("binkawin64.dll");
inline CModule g_RadAudioSystemDll  = CModule("mileswin64.dll");
#if !defined (CLIENT_DLL)
inline CModule g_SDKDll = CModule("gamesdk.dll");
#else // This dll is loaded from 'bin/x64_retail//'
inline CModule g_SDKDll = CModule("client.dll");
#endif // !CLIENT_DLL
#else // No DirectX and Miles imports.
inline CModule g_GameDll = CModule("r5apex_ds.exe");
inline CModule g_SDKDll = CModule("dedicated.dll");
#endif // !DEDICATED
inline CSigCache g_SigCache;

#define VAR_NAME(varName)  #varName

#define MEMBER_AT_OFFSET(varType, varName, offset)             \
	varType& varName()                                         \
	{                                                          \
		static int _##varName = offset;                        \
		return *(varType*)((std::uintptr_t)this + _##varName); \
	}

template <typename ReturnType, typename ...Args>
ReturnType CallVFunc(int index, void* thisPtr, Args... args)
{
	return (*reinterpret_cast<ReturnType(__fastcall***)(void*, Args...)>(thisPtr))[index](thisPtr, args...);
}
#endif // !SDKLAUNCHER && !NETCONSOLE && !PLUGINSDK