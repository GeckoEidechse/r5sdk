//=============================================================================//
//
// Purpose: Callback functions for ConVar's.
//
//=============================================================================//

#include "core/stdafx.h"
#include "windows/id3dx.h"
#include "tier0/fasttimer.h"
#include "tier1/cvar.h"
#include "tier1/IConVar.h"
#ifdef DEDICATED
#include "engine/server/sv_rcon.h"
#endif // DEDICATED
#ifndef DEDICATED
#include "engine/client/cl_rcon.h"
#endif // !DEDICATED
#include "engine/client/client.h"
#include "engine/net.h"
#include "engine/host_cmd.h"
#include "engine/host_state.h"
#ifndef CLIENT_DLL
#include "engine/server/server.h"
#endif // !CLIENT_DLL
#ifndef DEDICATED
#include "client/cdll_engine_int.h"
#endif // !DEDICATED
#include "rtech/rtech_game.h"
#include "rtech/rtech_utils.h"
#include "filesystem/basefilesystem.h"
#include "filesystem/filesystem.h"
#include "vpklib/packedstore.h"
#include "squirrel/sqscript.h"
#include "ebisusdk/EbisuSDK.h"
#ifndef DEDICATED
#include "gameui/IBrowser.h"
#include "gameui/IConsole.h"
#endif // !DEDICATED
#ifndef CLIENT_DLL
#include "networksystem/bansystem.h"
#endif // !CLIENT_DLL
#include "public/edict.h"
#include "public/worldsize.h"
#include "mathlib/crc32.h"
#include "mathlib/mathlib.h"
#include "vstdlib/completion.h"
#include "vstdlib/callback.h"
#ifndef DEDICATED
#include "materialsystem/cmaterialglue.h"
#include "public/idebugoverlay.h"
#endif // !DEDICATED
#ifndef CLIENT_DLL
#include "game/server/detour_impl.h"
#endif // !CLIENT_DLL
#ifndef DEDICATED
#include "game/client/viewrender.h"
#endif // !DEDICATED


/*
=====================
MP_GameMode_Changed_f
=====================
*/
void MP_GameMode_Changed_f(IConVar* pConVar, const char* pOldString, float flOldValue)
{
	SetupGamemode(mp_gamemode->GetString());
}

/*
=====================
MP_HostName_Changed_f
=====================
*/
void MP_HostName_Changed_f(IConVar* pConVar, const char* pOldString, float flOldValue)
{
#ifndef DEDICATED
	g_pBrowser->SetHostName(pylon_matchmaking_hostname->GetString());
#endif // !DEDICATED
}

#ifndef DEDICATED
/*
=====================
ToggleConsole_f
=====================
*/
void ToggleConsole_f(const CCommand& args)
{
	g_pConsole->m_bActivate ^= true;
	ResetInput(); // Disable input to game when console is drawn.
}

/*
=====================
ToggleBrowser_f
=====================
*/
void ToggleBrowser_f(const CCommand& args)
{
	g_pBrowser->m_bActivate ^= true;
	ResetInput(); // Disable input to game when browser is drawn.
}
#endif // !DEDICATED
#ifndef CLIENT_DLL
/*
=====================
Host_Kick_f
=====================
*/
void Host_Kick_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	g_pBanSystem->KickPlayerByName(args.Arg(1));
}

/*
=====================
Host_KickID_f
=====================
*/
void Host_KickID_f(const CCommand& args)
{
	if (args.ArgC() < 2) // Do we at least have 2 arguments?
	{
		return;
	}

	g_pBanSystem->KickPlayerById(args.Arg(1));
}

/*
=====================
Host_Ban_f
=====================
*/
void Host_Ban_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	g_pBanSystem->BanPlayerByName(args.Arg(1));
}

/*
=====================
Host_BanID_f
=====================
*/
void Host_BanID_f(const CCommand& args)
{
	if (args.ArgC() < 2)
		return;

	g_pBanSystem->BanPlayerById(args.Arg(1));
}

/*
=====================
Host_Unban_f
=====================
*/
void Host_Unban_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	g_pBanSystem->UnbanPlayer(args.Arg(1));
}

/*
=====================
Host_ReloadBanList_f
=====================
*/
void Host_ReloadBanList_f(const CCommand& args)
{
	g_pBanSystem->Load(); // Reload banned list.
}

/*
=====================
Host_ReloadPlaylists_f
=====================
*/
void Host_ReloadPlaylists_f(const CCommand& args)
{
	_DownloadPlaylists_f();
	KeyValues::InitPlaylists(); // Re-Init playlist.
}

/*
=====================
Host_Changelevel_f

  Goes to a new map, 
  taking all clients along
=====================
*/
void Host_Changelevel_f(const CCommand& args)
{
	if (args.ArgC() >= 2
		&& IsOriginInitialized()
		&& g_pServer->IsActive())
	{
		v_SetLaunchOptions(args);
		v_HostState_ChangeLevelMP(args[1], args[2]);
	}
}

/*
=====================
Detour_HotSwap_f

  Hot swaps the NavMesh
  while the game is running
=====================
*/
void Detour_HotSwap_f(const CCommand& args)
{
	if (!g_pServer->IsActive())
		return; // Only execute if server is initialized and active.

	DevMsg(eDLL_T::SERVER, "Executing NavMesh hot swap for level '%s'\n",
		g_ServerGlobalVariables->m_pszMapName);

	CFastTimer timer;

	timer.Start();
	Detour_HotSwap();

	timer.End();
	DevMsg(eDLL_T::SERVER, "Hot swap took '%lf' seconds\n", timer.GetDuration().GetSeconds());
}
#endif // !CLIENT_DLL
/*
=====================
Pak_ListPaks_f
=====================
*/
void Pak_ListPaks_f(const CCommand& args)
{
	DevMsg(eDLL_T::RTECH, "| id   | name                                               | status                               | asset count |\n");
	DevMsg(eDLL_T::RTECH, "|------|----------------------------------------------------|--------------------------------------|-------------|\n");

	uint32_t nTotalLoaded = 0;

	for (int16_t i = 0; i < *s_pLoadedPakCount; ++i)
	{
		const RPakLoadedInfo_t& info = g_pLoadedPakInfo[i];

		if (info.m_nStatus == RPakStatus_t::PAK_STATUS_FREED)
			continue;

		string rpakStatus = "RPAK_CREATED_A_NEW_STATUS_SOMEHOW";

		auto it = g_PakStatusToString.find(info.m_nStatus);
		if (it != g_PakStatusToString.end())
			rpakStatus = it->second;

		// todo: make status into a string from an array/vector
		DevMsg(eDLL_T::RTECH, "| %04i | %-50s | %-36s | %11i |\n", info.m_nHandle, info.m_pszFileName, rpakStatus.c_str(), info.m_nAssetCount);
		nTotalLoaded++;
	}
	DevMsg(eDLL_T::RTECH, "|------|----------------------------------------------------|--------------------------------------|-------------|\n");
	DevMsg(eDLL_T::RTECH, "| %18i loaded paks.                                                                                |\n", nTotalLoaded);
	DevMsg(eDLL_T::RTECH, "|------|----------------------------------------------------|--------------------------------------|-------------|\n");
}

/*
=====================
Pak_ListTypes_f
=====================
*/
void Pak_ListTypes_f(const CCommand& args)
{
	DevMsg(eDLL_T::RTECH, "| ext  | description               | version | header size | native size |\n");
	DevMsg(eDLL_T::RTECH, "|------|---------------------------|---------|-------------|-------------|\n");

	uint32_t nRegistered = 0;

	for (int8_t i = 0; i < 64; ++i)
	{
		RPakAssetBinding_t* type = &g_pUnknownPakStruct->m_nAssetBindings[i];

		if (!type->m_szDescription)
			continue;

		DevMsg(eDLL_T::RTECH, "| %-4s | %-25s | %7i | %11i | %11i |\n", FourCCToString(type->m_nExtension).c_str(), type->m_szDescription, type->m_iVersion, type->m_iSubHeaderSize, type->m_iNativeClassSize);
		nRegistered++;
	}
	DevMsg(eDLL_T::RTECH, "|------|---------------------------|---------|-------------|-------------|\n");
	DevMsg(eDLL_T::RTECH, "| %18i registered types.                                   |\n", nRegistered);
	DevMsg(eDLL_T::RTECH, "|------|---------------------------|---------|-------------|-------------|\n");
}

/*
=====================
Pak_RequestUnload_f
=====================
*/
void Pak_RequestUnload_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	try
	{
		if (args.HasOnlyDigits(1))
		{
			const RPakHandle_t pakHandle = std::stoi(args.Arg(1));
			const RPakLoadedInfo_t* pakInfo = g_pRTech->GetPakLoadedInfo(pakHandle);
			if (!pakInfo)
			{
				throw std::exception("Found no pak entry for specified handle.");
			}

			const string pakName = pakInfo->m_pszFileName;
			!pakName.empty() ? DevMsg(eDLL_T::RTECH, "Requested pak unload for file '%s'\n", pakName.c_str()) : DevMsg(eDLL_T::RTECH, "Requested pak unload for handle '%d'\n", pakHandle);
			g_pakLoadApi->UnloadPak(pakHandle);
		}
		else
		{
			const RPakLoadedInfo_t* pakInfo = g_pRTech->GetPakLoadedInfo(args.Arg(1));
			if (!pakInfo)
			{
				throw std::exception("Found no pak entry for specified name.");
			}

			DevMsg(eDLL_T::RTECH, "Requested pak unload for file '%s'\n", args.Arg(1));
			g_pakLoadApi->UnloadPak(pakInfo->m_nHandle);
		}
	}
	catch (const std::exception& e)
	{
		Error(eDLL_T::RTECH, NO_ERROR, "%s - %s\n", __FUNCTION__, e.what());
		return;
	}
}

/*
=====================
Pak_RequestLoad_f
=====================
*/
void Pak_RequestLoad_f(const CCommand& args)
{
	g_pakLoadApi->LoadAsync(args.Arg(1));
}


/*
=====================
Pak_Swap_f
=====================
*/
void Pak_Swap_f(const CCommand& args)
{
	try
	{
		string pakName;
		RPakHandle_t pakHandle = 0;
		RPakLoadedInfo_t* pakInfo = nullptr;

		if (args.HasOnlyDigits(1))
		{
			pakHandle = std::stoi(args.Arg(1));
			pakInfo = g_pRTech->GetPakLoadedInfo(pakHandle);
			if (!pakInfo)
			{
				throw std::exception("Found no pak entry for specified handle.");
			}

			pakName = pakInfo->m_pszFileName;
		}
		else
		{
			pakName = args.Arg(1);
			pakInfo = g_pRTech->GetPakLoadedInfo(args.Arg(1));
			if (!pakInfo)
			{
				throw std::exception("Found no pak entry for specified name.");
			}

			pakHandle = pakInfo->m_nHandle;
		}

		!pakName.empty() ? DevMsg(eDLL_T::RTECH, "Requested pak swap for file '%s'\n", pakName.c_str()) : DevMsg(eDLL_T::RTECH, "Requested pak swap for handle '%d'\n", pakHandle);

		g_pakLoadApi->UnloadPak(pakHandle);

		while (pakInfo->m_nStatus != RPakStatus_t::PAK_STATUS_FREED) // Wait till this slot gets free'd.
			std::this_thread::sleep_for(std::chrono::seconds(1));

		g_pakLoadApi->LoadAsync(pakName.c_str());
	}
	catch (const std::exception& e)
	{
		Error(eDLL_T::RTECH, NO_ERROR, "%s - %s\n", __FUNCTION__, e.what());
		return;
	}
}

/*
=====================
RTech_StringToGUID_f
=====================
*/
void RTech_StringToGUID_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	unsigned long long guid = g_pRTech->StringToGuid(args.Arg(1));

	DevMsg(eDLL_T::RTECH, "______________________________________________________________\n");
	DevMsg(eDLL_T::RTECH, "] RTECH_HASH ]------------------------------------------------\n");
	DevMsg(eDLL_T::RTECH, "] GUID: '0x%llX'\n", guid);
}

/*
=====================
RTech_Decompress_f

  Decompresses input RPak file and
  dumps results to 'paks\Win32\*.rpak'
=====================
*/
void RTech_Decompress_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	static const string svModDir = "paks\\Win32\\";
	static const string svBaseDir = "paks\\Win64\\";

	const string svPakNameOut = svModDir + args.Arg(1);
	const string svPakNameIn = svBaseDir + args.Arg(1);

	DevMsg(eDLL_T::RTECH, "______________________________________________________________\n");
	DevMsg(eDLL_T::RTECH, "-+ RTech decompress ------------------------------------------\n");

	if (!FileSystem()->FileExists(svPakNameIn.c_str(), "GAME"))
	{
		Error(eDLL_T::RTECH, NO_ERROR, "%s - pak file '%s' does not exist!\n", __FUNCTION__, svPakNameIn.c_str());
		return;
	}

	DevMsg(eDLL_T::RTECH, " |-+ Processing: '%s'\n", svPakNameIn.c_str());
	FileHandle_t hPakFile = FileSystem()->Open(svPakNameIn.c_str(), "rb", "GAME");

	if (!hPakFile)
	{
		Error(eDLL_T::RTECH, NO_ERROR, "%s - Unable to open '%s' (insufficient rights?)\n", __FUNCTION__, svPakNameIn.c_str());
		return;
	}

	uint32_t nPakLen = FileSystem()->Size(hPakFile);
	uint8_t* pPakBuf = MemAllocSingleton()->Alloc<uint8_t>(nPakLen);

	FileSystem()->Read(pPakBuf, nPakLen, hPakFile);
	FileSystem()->Close(hPakFile);

	RPakHeader_t* pHeader = reinterpret_cast<RPakHeader_t*>(pPakBuf);

	uint32_t nLen = FileSystem()->Size(hPakFile);
	uint16_t flags = (pHeader->m_nFlags[0] << 8) | pHeader->m_nFlags[1];

	SYSTEMTIME systemTime;
	FileTimeToSystemTime(&pHeader->m_nFileTime, &systemTime);

	DevMsg(eDLL_T::RTECH, " | |-+ Header ------------------------------------------------\n");
	DevMsg(eDLL_T::RTECH, " |   |-- Magic    : '0x%08X'\n", pHeader->m_nMagic);
	DevMsg(eDLL_T::RTECH, " |   |-- Version  : '%hu'\n", pHeader->m_nVersion);
	DevMsg(eDLL_T::RTECH, " |   |-- Flags    : '0x%04hX'\n", flags);
	DevMsg(eDLL_T::RTECH, " |   |-- Time     : '%hu-%hu-%hu/%hu %hu:%hu:%hu.%hu'\n",systemTime.wYear,systemTime.wMonth,systemTime.wDay, systemTime.wDayOfWeek, systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
	DevMsg(eDLL_T::RTECH, " |   |-- Hash     : '0x%08llX'\n", pHeader->m_nHash);
	DevMsg(eDLL_T::RTECH, " |   |-- Entries  : '%u'\n", pHeader->m_nAssetEntryCount);
	DevMsg(eDLL_T::RTECH, " |   |-+ Compression -----------------------------------------\n");
	DevMsg(eDLL_T::RTECH, " |     |-- Size comp: '%llu'\n", pHeader->m_nSizeDisk);
	DevMsg(eDLL_T::RTECH, " |     |-- Size decp: '%llu'\n", pHeader->m_nSizeMemory);

	if (pHeader->m_nMagic != RPAKHEADER)
	{
		Error(eDLL_T::RTECH, NO_ERROR, "%s - pak file '%s' has invalid magic!\n", __FUNCTION__, svPakNameIn.c_str());
		MemAllocSingleton()->Free(pPakBuf);

		return;
	}
	if ((pHeader->m_nFlags[1] & 1) != 1)
	{
		Error(eDLL_T::RTECH, NO_ERROR, "%s - pak file '%s' already decompressed!\n", __FUNCTION__, svPakNameIn.c_str());
		MemAllocSingleton()->Free(pPakBuf);

		return;
	}
	if (pHeader->m_nSizeDisk != nPakLen)
	{
		Error(eDLL_T::RTECH, NO_ERROR, "%s - pak file '%s' decompressed size '%llu' doesn't match expected size '%llu'!\n", __FUNCTION__, svPakNameIn.c_str(), nPakLen, pHeader->m_nSizeMemory);
		MemAllocSingleton()->Free(pPakBuf);

		return;
	}

	RPakDecompState_t decompState;
	uint64_t nDecompSize = g_pRTech->DecompressPakFileInit(&decompState, pPakBuf, nPakLen, NULL, sizeof(RPakHeader_t));

	if (nDecompSize == pHeader->m_nSizeDisk)
	{
		Error(eDLL_T::RTECH, NO_ERROR, "%s - calculated size: '%llu' expected: '%llu'!\n", __FUNCTION__, nDecompSize, pHeader->m_nSizeMemory);
		MemAllocSingleton()->Free(pPakBuf);

		return;
	}
	else
	{
		DevMsg(eDLL_T::RTECH, " |     |-- Size calc: '%llu'\n", nDecompSize);
	}

	DevMsg(eDLL_T::RTECH, " |     |-- Ratio    : '%.02f'\n", (pHeader->m_nSizeDisk * 100.f) / pHeader->m_nSizeMemory);
	uint8_t* pDecompBuf = MemAllocSingleton()->Alloc<uint8_t>(pHeader->m_nSizeMemory);

	decompState.m_nOutMask = UINT64_MAX;
	decompState.m_nOut = uint64_t(pDecompBuf);

	uint8_t nDecompResult = g_pRTech->DecompressPakFile(&decompState, nPakLen, pHeader->m_nSizeMemory);
	if (nDecompResult != 1)
	{
		Error(eDLL_T::RTECH, NO_ERROR, "%s - decompression failed for '%s' return value: '%hu'!\n", __FUNCTION__, svPakNameIn.c_str(), nDecompResult);
		MemAllocSingleton()->Free(pPakBuf);
		MemAllocSingleton()->Free(pDecompBuf);

		return;
	}

	pHeader->m_nFlags[1] = 0x0; // Set compressed flag to false for the decompressed pak file.
	pHeader->m_nSizeDisk = pHeader->m_nSizeMemory; // Equal compressed size with decompressed.

	FileSystem()->CreateDirHierarchy(svModDir.c_str(), "GAME");
	FileHandle_t hDecompFile = FileSystem()->Open(svPakNameOut.c_str(), "wb", "GAME");

	if (!hDecompFile)
	{
		Error(eDLL_T::RTECH, NO_ERROR, "%s - Unable to write to '%s' (read-only?)\n", __FUNCTION__, svPakNameOut.c_str());

		MemAllocSingleton()->Free(pPakBuf);
		MemAllocSingleton()->Free(pDecompBuf);

		return;
	}

	if (pHeader->m_nPatchIndex > 0) // Check if its an patch rpak.
	{
		// Loop through all the structs and patch their compress size.
		for (uint32_t i = 1, nPatchOffset = (sizeof(RPakHeader_t) + sizeof(uint64_t));
			i <= pHeader->m_nPatchIndex; i++, nPatchOffset += sizeof(RPakPatchCompressedHeader_t))
		{
			RPakPatchCompressedHeader_t* pPatchHeader = reinterpret_cast<RPakPatchCompressedHeader_t*>(pDecompBuf + nPatchOffset);
			DevMsg(eDLL_T::RTECH, " |     |-+ Patch #%02u -----------------------------------------\n", i);
			DevMsg(eDLL_T::RTECH, " |     %s |-- Size comp: '%llu'\n", i < pHeader->m_nPatchIndex ? "|" : " ", pPatchHeader->m_nSizeDisk);
			DevMsg(eDLL_T::RTECH, " |     %s |-- Size decp: '%llu'\n", i < pHeader->m_nPatchIndex ? "|" : " ", pPatchHeader->m_nSizeMemory);

			pPatchHeader->m_nSizeDisk = pPatchHeader->m_nSizeMemory; // Fix size for decompress.
		}
	}

	memcpy_s(pDecompBuf, sizeof(RPakHeader_t), pPakBuf, sizeof(RPakHeader_t));// Overwrite first 0x80 bytes which are NULL with the header data.
	FileSystem()->Write(pDecompBuf, decompState.m_nDecompSize, hDecompFile);

	DevMsg(eDLL_T::RTECH, " |-- Checksum : '0x%08X'\n", crc32::update(NULL, pDecompBuf, decompState.m_nDecompSize));
	DevMsg(eDLL_T::RTECH, "-+ Decompressed pak file to: '%s'\n", svPakNameOut.c_str());
	DevMsg(eDLL_T::RTECH, "--------------------------------------------------------------\n");

	MemAllocSingleton()->Free(pPakBuf);
	MemAllocSingleton()->Free(pDecompBuf);

	FileSystem()->Close(hDecompFile);
}

/*
=====================
VPK_Pack_f

  Packs VPK files into
  'PLATFORM' VPK directory.
=====================
*/
void VPK_Pack_f(const CCommand& args)
{
	if (args.ArgC() < 4)
	{
		return;
	}

	VPKPair_t pair(args.Arg(1), args.Arg(2), args.Arg(3), NULL);
	CFastTimer timer;

	DevMsg(eDLL_T::FS, "*** Starting VPK build command for: '%s'\n", pair.m_svDirectoryName.c_str());
	timer.Start();

	g_pPackedStore->InitLzCompParams();
	g_pPackedStore->PackWorkspace(pair, fs_packedstore_workspace->GetString(), "vpk/", (args.ArgC() > 4));

	timer.End();
	DevMsg(eDLL_T::FS, "*** Time elapsed: '%lf' seconds\n", timer.GetDuration().GetSeconds());
	DevMsg(eDLL_T::FS, "\n");
}

/*
=====================
VPK_Unpack_f

  Unpacks VPK files into
  workspace directory.
=====================
*/
void VPK_Unpack_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	const char* pArg = args.Arg(1);
	VPKDir_t vpk(pArg, (args.ArgC() > 2));
	CFastTimer timer;

	DevMsg(eDLL_T::FS, "*** Starting VPK extraction command for: '%s'\n", pArg);
	timer.Start();

	g_pPackedStore->InitLzDecompParams();
	g_pPackedStore->UnpackWorkspace(vpk, fs_packedstore_workspace->GetString());

	timer.End();
	DevMsg(eDLL_T::FS, "*** Time elapsed: '%lf' seconds\n", timer.GetDuration().GetSeconds());
	DevMsg(eDLL_T::FS, "\n");
}

/*
=====================
VPK_Mount_f

  Mounts input VPK file for
  internal FileSystem usage
=====================
*/
void VPK_Mount_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	FileSystem()->MountVPKFile(args.Arg(1));
}

/*
=====================
VPK_Unmount_f

  Unmounts input VPK file
  and clears its cache
=====================
*/
void VPK_Unmount_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	FileSystem()->UnmountVPKFile(args.Arg(1));
}

/*
=====================
NET_SetKey_f

  Sets the input netchannel encryption key
=====================
*/
void NET_SetKey_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		return;
	}

	NET_SetKey(args.Arg(1));
}

/*
=====================
NET_GenerateKey_f

  Sets a random netchannel encryption key
=====================
*/
void NET_GenerateKey_f(const CCommand& args)
{
	NET_GenerateKey();
}

/*
=====================
NET_UseRandomKeyChanged_f

  Use random AES encryption
  key for game packets
=====================
*/
void NET_UseRandomKeyChanged_f(IConVar* pConVar, const char* pOldString, float flOldValue)
{
	if (ConVar* pConVarRef = g_pCVar->FindVar(pConVar->GetName()))
	{
		if (strcmp(pOldString, pConVarRef->GetString()) == NULL)
			return; // Same value.

		if (pConVarRef->GetBool())
			NET_GenerateKey();
		else
			NET_SetKey(DEFAULT_NET_ENCRYPTION_KEY);
	}
}
/*
=====================
CON_Help_f

  Shows the colors and
  description of each
  context.
=====================
*/
void CON_Help_f(const CCommand& args)
{
	DevMsg(eDLL_T::COMMON, "Contexts:\n");
	SQVM_PrintFunc(reinterpret_cast<HSQUIRRELVM>(SQCONTEXT::SERVER), (SQChar*)(" = Server DLL (Script)\n"));
	SQVM_PrintFunc(reinterpret_cast<HSQUIRRELVM>(SQCONTEXT::CLIENT), (SQChar*)(" = Client DLL (Script)\n"));
	SQVM_PrintFunc(reinterpret_cast<HSQUIRRELVM>(SQCONTEXT::UI), (SQChar*)(" = UI DLL (Script)\n"));

	DevMsg(eDLL_T::SERVER, " = Server DLL (Code)\n");
	DevMsg(eDLL_T::CLIENT, " = Client DLL (Code)\n");
	DevMsg(eDLL_T::UI, " = UI DLL (Code)\n");
	DevMsg(eDLL_T::ENGINE, " = Engine DLL (Code)\n");
	DevMsg(eDLL_T::FS, " = FileSystem (Code)\n");
	DevMsg(eDLL_T::RTECH, " = PakLoad API (Code)\n");
	DevMsg(eDLL_T::MS, " = MaterialSystem (Code)\n");
	DevMsg(eDLL_T::NETCON, " = Net Console (Code)\n");
}

#ifndef DEDICATED
/*
=====================
CON_LogHistory_f

  Shows the game console 
  submission history.
=====================
*/
void CON_LogHistory_f(const CCommand& args)
{
	const vector<string> vHistory = g_pConsole->GetHistory();
	for (size_t i = 0, nh = vHistory.size(); i < nh; i++)
	{
		DevMsg(eDLL_T::COMMON, "%3d: %s\n", i, vHistory[i].c_str());
	}
}

/*
=====================
CON_RemoveLine_f

  Removes a range of lines
  from the console.
=====================
*/
void CON_RemoveLine_f(const CCommand& args)
{
	if (args.ArgC() < 3)
	{
		DevMsg(eDLL_T::CLIENT, "Usage 'con_removeline': start(int) end(int)\n");
		return;
	}

	int start = atoi(args[1]);
	int end = atoi(args[2]);

	g_pConsole->RemoveLog(start, end);
}

/*
=====================
CON_ClearLines_f

  Clears all lines from
  the developer console.
=====================
*/
void CON_ClearLines_f(const CCommand& args)
{
	g_pConsole->ClearLog();
}

/*
=====================
CON_ClearHistory_f

  Clears all submissions from the
  developer console history.
=====================
*/
void CON_ClearHistory_f(const CCommand& args)
{
	g_pConsole->ClearHistory();
}

/*
=====================
RCON_CmdQuery_f

  Issues an RCON command to the
  RCON server.
=====================
*/
void RCON_CmdQuery_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		if (RCONClient()->IsInitialized()
			&& !RCONClient()->IsConnected()
			&& strlen(rcon_address->GetString()) > 0)
		{
			RCONClient()->Connect();
		}
	}
	else
	{
		if (!RCONClient()->IsInitialized())
		{
			Warning(eDLL_T::CLIENT, "Failed to issue command to RCON server: %s\n", "uninitialized");
			return;
		}
		else if (RCONClient()->IsConnected())
		{
			if (strcmp(args.Arg(1), "PASS") == 0) // Auth with RCON server using rcon_password ConVar value.
			{
				string svCmdQuery;
				if (args.ArgC() > 2)
				{
					svCmdQuery = RCONClient()->Serialize(args.Arg(2), "", cl_rcon::request_t::SERVERDATA_REQUEST_AUTH);
				}
				else // Use 'rcon_password' ConVar as password.
				{
					svCmdQuery = RCONClient()->Serialize(rcon_password->GetString(), "", cl_rcon::request_t::SERVERDATA_REQUEST_AUTH);
				}
				RCONClient()->Send(svCmdQuery);
				return;
			}
			else if (strcmp(args.Arg(1), "disconnect") == 0) // Disconnect from RCON server.
			{
				RCONClient()->Disconnect();
				return;
			}

			string svCmdQuery = RCONClient()->Serialize(args.ArgS(), "", cl_rcon::request_t::SERVERDATA_REQUEST_EXECCOMMAND);
			RCONClient()->Send(svCmdQuery);
			return;
		}
		else
		{
			Warning(eDLL_T::CLIENT, "Failed to issue command to RCON server: %s\n", "unconnected");
			return;
		}
	}
}

/*
=====================
RCON_Disconnect_f

  Disconnect from RCON server
=====================
*/
void RCON_Disconnect_f(const CCommand& args)
{
	if (RCONClient()->IsConnected())
	{
		RCONClient()->Disconnect();
		DevMsg(eDLL_T::CLIENT, "User closed RCON connection\n");
	}
}
#endif // !DEDICATED

/*
=====================
RCON_PasswordChanged_f

  Change password on RCON server
  and RCON client
=====================
*/
void RCON_PasswordChanged_f(IConVar* pConVar, const char* pOldString, float flOldValue)
{
	if (ConVar* pConVarRef = g_pCVar->FindVar(pConVar->GetName()))
	{
		if (strcmp(pOldString, pConVarRef->GetString()) == NULL)
			return; // Same password.

#ifndef DEDICATED
		if (RCONClient()->IsInitialized())
			RCONClient()->SetPassword(pConVarRef->GetString());
		else
			RCONClient()->Init(); // Initialize first.
#elif DEDICATED
		if (RCONServer()->IsInitialized())
			RCONServer()->SetPassword(pConVarRef->GetString());
		else
			RCONServer()->Init(); // Initialize first.
#endif // DEDICATED
	}
}

/*
=====================
SQVM_ServerScript_f

  Executes input on the
  VM in SERVER context.
=====================
*/
void SQVM_ServerScript_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
	{
		Script_Execute(args.ArgS(), SQCONTEXT::SERVER);
	}
}

#ifndef DEDICATED
/*
=====================
SQVM_ClientScript_f

  Executes input on the
  VM in CLIENT context.
=====================
*/
void SQVM_ClientScript_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
	{
		Script_Execute(args.ArgS(), SQCONTEXT::CLIENT);
	}
}

/*
=====================
SQVM_UIScript_f

  Executes input on the
  VM in UI context.
=====================
*/
void SQVM_UIScript_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
	{
		Script_Execute(args.ArgS(), SQCONTEXT::UI);
	}
}

/*
=====================
Mat_CrossHair_f

  Print the material under the crosshair.
=====================
*/
void Mat_CrossHair_f(const CCommand& args)
{
	CMaterialGlue* material = GetMaterialAtCrossHair();
	if (material)
	{
		DevMsg(eDLL_T::MS, "______________________________________________________________\n");
		DevMsg(eDLL_T::MS, "-+ Material --------------------------------------------------\n");
		DevMsg(eDLL_T::MS, " |-- ADDR: '%llX'\n", material);
		DevMsg(eDLL_T::MS, " |-- GUID: '%llX'\n", material->m_GUID);
		DevMsg(eDLL_T::MS, " |-- Streaming texture count: '%d'\n", material->m_nStreamableTextureCount);
		DevMsg(eDLL_T::MS, " |-- Material width: '%d'\n", material->m_iWidth);
		DevMsg(eDLL_T::MS, " |-- Material height: '%d'\n", material->m_iHeight);
		DevMsg(eDLL_T::MS, " |-- Flags: '%llX'\n", material->m_iFlags);

		std::function<void(CMaterialGlue*, const char*)> fnPrintChild = [](CMaterialGlue* material, const char* print)
		{
			DevMsg(eDLL_T::MS, " |-+\n");
			DevMsg(eDLL_T::MS, " | |-+ Child material ----------------------------------------\n");
			DevMsg(eDLL_T::MS, print, material);
			DevMsg(eDLL_T::MS, " |     |-- GUID: '%llX'\n", material->m_GUID);
			DevMsg(eDLL_T::MS, " |     |-- Material name: '%s'\n", material->m_pszName);
		};

		DevMsg(eDLL_T::MS, " |-- Material name: '%s'\n", material->m_pszName);
		DevMsg(eDLL_T::MS, " |-- Material surface name 1: '%s'\n", material->m_pszSurfaceProp);
		DevMsg(eDLL_T::MS, " |-- Material surface name 2: '%s'\n", material->m_pszSurfaceProp2);
		DevMsg(eDLL_T::MS, " |-- DX buffer: '%llX'\n", material->m_pDXBuffer);
		DevMsg(eDLL_T::MS, " |-- DX buffer VFTable: '%llX'\n", material->m_pID3D11BufferVTable);

		material->m_pDepthShadow 
			? fnPrintChild(material->m_pDepthShadow, " |   |-+ DepthShadow: '%llX'\n") 
			: DevMsg(eDLL_T::MS, " |   |-+ DepthShadow: 'NULL'\n");
		material->m_pDepthPrepass 
			? fnPrintChild(material->m_pDepthPrepass, " |   |-+ DepthPrepass: '%llX'\n") 
			: DevMsg(eDLL_T::MS, " |   |-+ DepthPrepass: 'NULL'\n");
		material->m_pDepthVSM 
			? fnPrintChild(material->m_pDepthVSM, " |   |-+ DepthVSM: '%llX'\n") 
			: DevMsg(eDLL_T::MS, " |   |-+ DepthVSM: 'NULL'\n");
		material->m_pDepthShadow 
			? fnPrintChild(material->m_pDepthShadow, " |   |-+ DepthShadowTight: '%llX'\n") 
			: DevMsg(eDLL_T::MS, " |   |-+ DepthShadowTight: 'NULL'\n");
		material->m_pColPass 
			? fnPrintChild(material->m_pColPass, " |   |-+ ColPass: '%llX'\n") 
			: DevMsg(eDLL_T::MS, " |   |-+ ColPass: 'NULL'\n");

		DevMsg(eDLL_T::MS, "-+ Texture GUID map ------------------------------------------\n");
		DevMsg(eDLL_T::MS, " |-- Texture handles: '%llX'\n", material->m_pTextureHandles);
		DevMsg(eDLL_T::MS, " |-- Streaming texture handles: '%llX'\n", material->m_pStreamableTextureHandles);

		DevMsg(eDLL_T::MS, "--------------------------------------------------------------\n");
	}
	else
	{
		DevMsg(eDLL_T::MS, __FUNCTION__": No material found >:(\n");
	}
}

/*
=====================
Line_f

  Draws a line at 
  start<x1 y1 z1> end<x2 y2 z2>.
=====================
*/
void Line_f(const CCommand& args)
{
	if (args.ArgC() != 7)
	{
		DevMsg(eDLL_T::CLIENT, "Usage 'line': start(vector) end(vector)\n");
		return;
	}

	Vector3D start, end;
	for (int i = 0; i < 3; ++i)
	{
		start[i] = atof(args[i + 1]);
		end[i] = atof(args[i + 4]);
	}

	g_pDebugOverlay->AddLineOverlay(start, end, 255, 255, 0, r_debug_overlay_zbuffer->GetBool(), 100);
}

/*
=====================
Sphere_f

  Draws a sphere at origin(x1 y1 z1) 
  radius(float) theta(int) phi(int).
=====================
*/
void Sphere_f(const CCommand& args)
{
	if (args.ArgC() != 7)
	{
		DevMsg(eDLL_T::CLIENT, "Usage 'sphere': origin(vector) radius(float) theta(int) phi(int)\n");
		return;
	}

	Vector3D start;
	for (int i = 0; i < 3; ++i)
	{
		start[i] = atof(args[i + 1]);
	}

	float radius = atof(args[4]);
	int theta = atoi(args[5]);
	int phi = atoi(args[6]);

	g_pDebugOverlay->AddSphereOverlay(start, radius, theta, phi, 20, 210, 255, 0, 100);
}

/*
=====================
Capsule_f

  Draws a capsule at start<x1 y1 z1> 
  end<x2 y2 z2> radius <x3 y3 z3>.
=====================
*/
void Capsule_f(const CCommand& args)
{
	if (args.ArgC() != 10)
	{
		DevMsg(eDLL_T::CLIENT, "Usage 'capsule': start(vector) end(vector) radius(vector)\n");
		return;
	}

	Vector3D start, end, radius;
	for (int i = 0; i < 3; ++i)
	{
		start[i] = atof(args[i + 1]);
		end[i] = atof(args[i + 4]);
		radius[i] = atof(args[i + 7]);
	}
	g_pDebugOverlay->AddCapsuleOverlay(start, end, radius, { 0,0,0 }, { 0,0,0 }, 141, 233, 135, 0, 100);
}
#endif // !DEDICATED
#if !defined (GAMEDLL_S0) && !defined (GAMEDLL_S1)
/*
=====================
BHit_f

  Bullet trajectory tracing
  from shooter to target entity.
=====================
*/
void BHit_f(const CCommand& args)
{
#ifndef DEDICATED // Stubbed to suppress server warnings as this is a GAMEDLL command!
	if (args.ArgC() != 9)
		return;

	if (!bhit_enable->GetBool() && !sv_visualizetraces->GetBool())
		return;

	Vector3D vecAbsStart;
	Vector3D vecAbsEnd;

	for (int i = 0; i < 3; ++i)
		vecAbsStart[i] = atof(args[i + 4]);

	QAngle vecBulletAngles;
	for (int i = 0; i < 2; ++i)
		vecBulletAngles[i] = atof(args[i + 7]);

	vecBulletAngles.z = 180.f; // Flipped axis.
	AngleVectors(vecBulletAngles, &vecAbsEnd);

	static char szBuf[2048]; // Render physics trace.
	snprintf(szBuf, sizeof(szBuf), "drawline %g %g %g %g %g %g",
		vecAbsStart.x, vecAbsStart.y, vecAbsStart.z,
		vecAbsStart.x + vecAbsEnd.x * MAX_COORD_RANGE,
		vecAbsStart.y + vecAbsEnd.y * MAX_COORD_RANGE,
		vecAbsStart.z + vecAbsEnd.z * MAX_COORD_RANGE);
	Cbuf_AddText(Cbuf_GetCurrentPlayer(), szBuf, cmd_source_t::kCommandSrcCode);

	if (bhit_abs_origin->GetBool())
	{
		const int iEnt = atoi(args[2]);
		if (const IClientEntity* pEntity = g_pClientEntityList->GetClientEntity(iEnt))
		{
			g_pDebugOverlay->AddSphereOverlay( // Render a debug sphere at the client's predicted entity origin.
				pEntity->GetAbsOrigin(), 10.f, 8, 6, 20, 60, 255, 0, sv_visualizetraces_duration->GetFloat());
		}
	}
#endif // !DEDICATED
}
#endif // !GAMEDLL_S0 && !GAMEDLL_S1
/*
=====================
CVHelp_f

  Show help text for a
  particular convar/concommand
=====================
*/
void CVHelp_f(const CCommand& args)
{
	cv->CvarHelp(args);
}

/*
=====================
CVList_f

  List all ConCommandBases
=====================
*/
void CVList_f(const CCommand& args)
{
	cv->CvarList(args);
}

/*
=====================
CVDiff_f

  List all ConVar's 
  who's values deviate 
  from default value
=====================
*/
void CVDiff_f(const CCommand& args)
{
	cv->CvarDifferences(args);
}

/*
=====================
CVFlag_f

  List all ConVar's
  with specified flags
=====================
*/
void CVFlag_f(const CCommand& args)
{
	cv->CvarFindFlags_f(args);
}
