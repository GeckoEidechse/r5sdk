#pragma once

//-------------------------------------------------------------------------
// CGAME
//-------------------------------------------------------------------------
inline CMemory p_CVideoMode_Common__CreateGameWindow;
inline auto CVideoMode_Common__CreateGameWindow = p_CVideoMode_Common__CreateGameWindow.RCast<bool (*)(int* pnRect)>();

inline CMemory p_CVideoMode_Common__CreateWindowClass;
inline auto CVideoMode_Common__CreateWindowClass = p_CVideoMode_Common__CreateWindowClass.RCast<HWND(*)(vrect_t* pnRect)>();

void HCVideoMode_Common_Attach();
void HCVideoMode_Common_Detach();

///////////////////////////////////////////////////////////////////////////////
class HVideoMode_Common : public IDetour
{
	virtual void GetAdr(void) const
	{
		spdlog::debug("| FUN: CVideoMode_Common::CreateGameWindow  : {:#18x} |\n", p_CVideoMode_Common__CreateGameWindow.GetPtr());
		spdlog::debug("| FUN: CVideoMode_Common::CreateWindowClass : {:#18x} |\n", p_CVideoMode_Common__CreateWindowClass.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
	}
	virtual void GetFun(void) const
	{
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		p_CVideoMode_Common__CreateGameWindow = g_GameDll.FindPatternSIMD("40 56 57 48 83 EC 38 48 8B F9 E8 ?? ?? ?? ??");
		p_CVideoMode_Common__CreateWindowClass = g_GameDll.FindPatternSIMD("40 55 53 57 41 56 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 4C 8B F1");
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
		p_CVideoMode_Common__CreateGameWindow = g_GameDll.FindPatternSIMD("40 56 57 48 83 EC 28 48 8B F9 E8 ?? ?? ?? ?? 48 8B F0");
		p_CVideoMode_Common__CreateWindowClass = g_GameDll.FindPatternSIMD("40 55 53 57 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 8B F9 FF 15 ?? ?? ?? ??");
#endif
		CVideoMode_Common__CreateGameWindow = p_CVideoMode_Common__CreateGameWindow.RCast<bool (*)(int*)>();      /*40 56 57 48 83 EC 28 48 8B F9 E8 ?? ?? ?? ?? 48 8B F0*/
		CVideoMode_Common__CreateWindowClass = p_CVideoMode_Common__CreateWindowClass.RCast<HWND(*)(vrect_t*)>(); /*40 55 53 57 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 8B F9 FF 15 ?? ?? ?? ??*/
	}
	virtual void GetVar(void) const { }
	virtual void GetCon(void) const { }
	virtual void Attach(void) const { }
	virtual void Detach(void) const { }
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(HVideoMode_Common);
