#pragma once

#ifdef DEDICATED
inline const char* g_szGameDll = "r5apex_ds.exe";
void Dedicated_Init();
#else
inline const char* g_szGameDll = "r5apex.exe";
#endif // DEDICATED

void RuntimePtc_Init();
#ifdef GAMEDLL_S3
/* -------------- OTHER ------------------------------------------------------------------------------------------------------------------------------------------------- */
inline CMemory dst007;
inline CMemory dst008;
/* -------------- ------- ----------------------------------------------------------------------------------------------------------------------------------------------- */
#endif // GAMEDLL_S3


//-------------------------------------------------------------------------
// CSHADERSYSTEM
//-------------------------------------------------------------------------
inline CMemory CShaderSystem__Init;

//-------------------------------------------------------------------------
// CVGUI
//-------------------------------------------------------------------------
inline CMemory CVGui__RunFrame;

//-------------------------------------------------------------------------
// CENGINEVGUI
//-------------------------------------------------------------------------
inline CMemory CEngineVGui__Shutdown;
inline CMemory CEngineVGui__ActivateGameUI;

//-------------------------------------------------------------------------
// CENGINEVGUI
//-------------------------------------------------------------------------
inline CMemory CInputSystem__RunFrameIME;

//-------------------------------------------------------------------------
// RUNTIME: SYS_INITGAME
//-------------------------------------------------------------------------
inline CMemory Sys_InitGame;

//-------------------------------------------------------------------------
// RUNTIME: HOST_INIT
//-------------------------------------------------------------------------
inline CMemory gHost_Init_1; // server Host_Init()?
inline CMemory gHost_Init_2; // client Host_Init()?

//-------------------------------------------------------------------------
// RUNTIME: HOST_SHUTDOWN
//-------------------------------------------------------------------------
inline CMemory Host_Shutdown;

//-------------------------------------------------------------------------
// RUNTIME: HOST_DISCONNECT
//-------------------------------------------------------------------------
inline CMemory Host_Disconnect;

//-------------------------------------------------------------------------
// RUNTIME: S2C_CHALLENGE
//-------------------------------------------------------------------------
#ifndef CLIENT_DLL
inline CMemory Server_S2C_CONNECT_1;
#endif // !CLIENT_DLL

//-------------------------------------------------------------------------
// RUNTIME: 
//-------------------------------------------------------------------------
inline CMemory MatchMaking_Frame;
inline CMemory GetEngineClientThread;
inline CMemory CWin32Surface_initStaticData;
#if !defined (GAMEDLL_S0) || !defined (GAMEDLL_S1)
inline CMemory KeyboardLayout_Init;
#endif

///////////////////////////////////////////////////////////////////////////////
class VOpcodes : public IDetour
{
	virtual void GetAdr(void) const
	{
		spdlog::debug("| FUN: CShaderSystem::Init                  : {:#18x} |\n", CShaderSystem__Init.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
		spdlog::debug("| FUN: CVGui::RunFrame                      : {:#18x} |\n", CVGui__RunFrame.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
		spdlog::debug("| FUN: CEngineVGui::Shutdown                : {:#18x} |\n", CEngineVGui__Shutdown.GetPtr());
		spdlog::debug("| FUN: CEngineVGui::ActivateGameUI          : {:#18x} |\n", CEngineVGui__ActivateGameUI.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
		spdlog::debug("| FUN: CInputSystem::RunFrameIME            : {:#18x} |\n", CInputSystem__RunFrameIME.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
		spdlog::debug("| FUN: Sys_InitGame                         : {:#18x} |\n", Sys_InitGame.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
		spdlog::debug("| FUN: Host_Init_1                          : {:#18x} |\n", gHost_Init_1.GetPtr());
		spdlog::debug("| FUN: Host_Init_2                          : {:#18x} |\n", gHost_Init_2.GetPtr());
		spdlog::debug("| FUN: Host_Disconnect                      : {:#18x} |\n", Host_Disconnect.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
#ifndef CLIENT_DLL
		spdlog::debug("| FUN: Server_S2C_CONNECT_1                 : {:#18x} |\n", Server_S2C_CONNECT_1.GetPtr());
#endif // !CLIENT_DLL
		spdlog::debug("+----------------------------------------------------------------+\n");
		spdlog::debug("| FUN: GetEngineClientThread                : {:#18x} |\n", GetEngineClientThread.GetPtr());
		spdlog::debug("| FUN: MatchMaking_Frame                    : {:#18x} |\n", MatchMaking_Frame.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
#if !defined (GAMEDLL_S0) || !defined (GAMEDLL_S1)
		spdlog::debug("| FUN: CWin32Surface::initStaticData        : {:#18x} |\n", CWin32Surface_initStaticData.GetPtr());
#endif
		spdlog::debug("| FUN: KeyboardLayout_Init                  : {:#18x} |\n", KeyboardLayout_Init.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
	}
	virtual void GetFun(void) const { }
	virtual void GetVar(void) const
	{
#ifdef GAMEDLL_S3
		/* -------------- OTHER ------------------------------------------------------------------------------------------------------------------------------------------------- */
		dst007 = /*0x14028F3B0*/ g_GameDll.FindPatternSIMD("48 8B C4 44 89 40 18 48 89 50 10 55 53 56 57 41");
		dst008 = /*0x140E3E110*/ g_GameDll.FindPatternSIMD("48 83 EC 78 48 8B 84 24 ?? ?? ?? ?? 4D 8B D8 ??");
		/* -------------- ------- ----------------------------------------------------------------------------------------------------------------------------------------------- */
#endif // GAMEDLL_S3


		//-------------------------------------------------------------------------
		CShaderSystem__Init = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC 20 C6 41 10 ??");
		// 0x1403DF870 // 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 C6 41 10 00 //

		//-------------------------------------------------------------------------
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		CVGui__RunFrame = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 55 56 57 48 83 EC 20 0F B6 69 5C");
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
		CVGui__RunFrame = g_GameDll.FindPatternSIMD("40 57 48 83 EC 20 48 89 5C 24 ?? 48 8B F9 48 89 6C 24 ?? 0F B6 69 5C");
#endif

		//-------------------------------------------------------------------------
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		CEngineVGui__Shutdown = g_GameDll.FindPatternSIMD("48 89 4C 24 ?? 57 41 54 48 83 EC 38");
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
		CEngineVGui__Shutdown = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC 20 80 3D ?? ?? ?? ?? ?? 48 8B D9");
#endif // 0x140282C90 // 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 80 3D ? ? ? ? ? 48 8B D9 //
		CEngineVGui__ActivateGameUI = g_GameDll.FindPatternSIMD("40 53 48 83 EC 20 F6 81 ?? ?? ?? ?? ?? 48 8B D9 74 08");
		// 

		//-------------------------------------------------------------------------
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		CInputSystem__RunFrameIME = g_GameDll.FindPatternSIMD("48 8B C4 57 41 55");
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
		CInputSystem__RunFrameIME = g_GameDll.FindPatternSIMD("40 57 41 54 41 55 48 83 EC 70");
#endif

		//-------------------------------------------------------------------------
		Sys_InitGame = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 57 48 81 EC ?? ?? ?? ?? 80 3D ?? ?? ?? ?? ?? 41 8B D8");
		// 0x1402958D0 // 48 89 5C 24 ? 57 48 81 EC ? ? ? ? 80 3D ? ? ? ? ? 41 8B D8 //

		//-------------------------------------------------------------------------
		gHost_Init_1 = g_GameDll.FindPatternSIMD("48 8B C4 41 56 48 81 EC ?? ?? ?? ?? 45 33 F6"); // server Host_Init()?
		// 0x140237B00 // 48 8B C4 41 56 48 81 EC ? ? ? ? 45 33 F6 //

		gHost_Init_2 = g_GameDll.FindPatternSIMD("88 4C 24 08 53 55 56 57 48 83 EC 68"); // client Host_Init()?
		// 0x140236640 // 88 4C 24 08 53 55 56 57 48 83 EC 68 //

		//-------------------------------------------------------------------------
		Host_Shutdown = g_GameDll.FindPatternSIMD("48 8B C4 48 83 EC ?? 80 3D ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? 8B 15 ?? ?? ?? ??");
		// 0x140239620 // 48 8B C4 48 83 EC ?? 80 3D ? ? ? ? ? 0F 85 ? ? ? ? 8B 15 ? ? ? ? //

		//-------------------------------------------------------------------------
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		Host_Disconnect = g_GameDll.FindPatternSIMD("48 83 EC 38 48 89 7C 24 ?? 0F B6 F9");
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
		Host_Disconnect = g_GameDll.FindPatternSIMD("40 53 48 83 EC 30 0F B6 D9");
#endif // 0x14023CCA0 // 40 53 48 83 EC 30 0F B6 D9 //

		//-------------------------------------------------------------------------
#ifndef CLIENT_DLL
		Server_S2C_CONNECT_1 = g_GameDll.FindPatternSIMD("48 3B 05 ?? ?? ?? ?? 74 0C");
#endif // !CLIENT_DLL

		//-------------------------------------------------------------------------
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		GetEngineClientThread = g_GameDll.FindPatternSIMD("40 53 48 83 EC 20 65 48 8B 04 25 ?? ?? ?? ?? 48 8B D9 B9 ?? ?? ?? ?? 48 8B 10 8B 04 11 39 05 ?? ?? ?? ?? 7F 15");
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
		GetEngineClientThread = g_GameDll.FindPatternSIMD("40 53 48 83 EC 20 65 48 8B 04 25 ?? ?? ?? ?? 48 8B D9 B9 ?? ?? ?? ?? 48 8B 10 8B 04 11 39 05 ?? ?? ?? ?? 7F 21");
#endif
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		MatchMaking_Frame = g_GameDll.FindPatternSIMD("40 55 56 41 54 41 55 48 8D AC 24 ?? ?? ?? ??");
#elif defined (GAMEDLL_S2)
		MatchMaking_Frame = g_GameDll.FindPatternSIMD("48 89 74 24 ?? 55 41 54 41 57 48 8D AC 24 ?? ?? ?? ??");
#elif defined (GAMEDLL_S3)
		MatchMaking_Frame = g_GameDll.FindPatternSIMD("48 8B C4 55 48 8D A8 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 89 78 18");
#endif


		CWin32Surface_initStaticData = g_GameDll.FindPatternSIMD("48 83 EC 28 E8 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? 48 83 C4 28 E9 ?? ?? ?? ?? CC CC CC CC CC CC CC 33 C9");
		// 48 83 EC 28 E8 ? ? ? ? 48 8D 0D ? ? ? ? 48 83 C4 28 E9 ? ? ? ? CC CC CC CC CC CC CC 33 C9 
#if !defined (GAMEDLL_S0) || !defined (GAMEDLL_S1)
		KeyboardLayout_Init = g_GameDll.FindPatternSIMD("48 83 EC 28 33 C9 FF 15 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ??");
#endif //48 83 EC 28 33 C9 FF 15 ? ? ? ? 48 8D 0D ? ? ? ?
	}
	virtual void GetCon(void) const { }
	virtual void Attach(void) const { }
	virtual void Detach(void) const { }
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(VOpcodes);
