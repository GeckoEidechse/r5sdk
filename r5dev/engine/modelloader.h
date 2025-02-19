#pragma once
#include "engine/gl_model_private.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class IModelLoader
{
public:
	enum REFERENCETYPE
	{
		// The name is allocated, but nothing else is in memory or being referenced
		FMODELLOADER_NOTLOADEDORREFERENCED = 0,
		// The model has been loaded into memory
		FMODELLOADER_LOADED = (1 << 0),

		// The model is being referenced by the server code
		FMODELLOADER_SERVER = (1 << 1),
		// The model is being referenced by the client code
		FMODELLOADER_CLIENT = (1 << 2),
		// The model is being referenced in the client .dll
		FMODELLOADER_CLIENTDLL = (1 << 3),
		// The model is being referenced by static props
		FMODELLOADER_STATICPROP = (1 << 4),
		// The model is a detail prop
		FMODELLOADER_DETAILPROP = (1 << 5),
		// The model is the simple version of the world geometry
		FMODELLOADER_SIMPLEWORLD = (1 << 6),
		// The model is dynamically loaded
		FMODELLOADER_DYNSERVER = (1 << 7),
		FMODELLOADER_DYNCLIENT = (1 << 8),
		FMODELLOADER_COMBINED = (1 << 9),
		FMODELLOADER_DYNAMIC = FMODELLOADER_DYNSERVER | FMODELLOADER_DYNCLIENT | FMODELLOADER_COMBINED,

		FMODELLOADER_REFERENCEMASK = (FMODELLOADER_SERVER | FMODELLOADER_CLIENT | FMODELLOADER_CLIENTDLL | FMODELLOADER_STATICPROP | FMODELLOADER_DETAILPROP | FMODELLOADER_DYNAMIC | FMODELLOADER_SIMPLEWORLD),

		// The model was touched by the preload method
		FMODELLOADER_TOUCHED_BY_PRELOAD = (1 << 15),
		// The model was loaded by the preload method, a postload fixup is required
		FMODELLOADER_LOADED_BY_PRELOAD = (1 << 16),
		// The model touched its materials as part of its load
		FMODELLOADER_TOUCHED_MATERIALS = (1 << 17),
	};
};

class CModelLoader
{
public:
	static void LoadModel(CModelLoader* loader, model_t* model);
	static uint64_t Map_LoadModelGuts(CModelLoader* loader, model_t* model);
};

inline CMemory p_CModelLoader__FindModel;
inline auto CModelLoader__FindModel = p_CModelLoader__FindModel.RCast<void* (*)(CModelLoader* loader, const char* pszModelName)>();

inline CMemory p_CModelLoader__LoadModel;
inline auto CModelLoader__LoadModel = p_CModelLoader__LoadModel.RCast<void(*)(CModelLoader* loader, model_t* model)>();

inline CMemory p_CModelLoader__UnloadModel;
inline auto CModelLoader__UnloadModel = p_CModelLoader__UnloadModel.RCast<uint64_t(*)(CModelLoader* loader, model_t* model)>();

inline CMemory p_CModelLoader__Studio_LoadModel;
inline auto CModelLoader__Studio_LoadModel = p_CModelLoader__Studio_LoadModel.RCast<void* (*)(CModelLoader* loader)>();

inline CMemory p_CModelLoader__Map_LoadModelGuts;
inline auto CModelLoader__Map_LoadModelGuts = p_CModelLoader__Map_LoadModelGuts.RCast<uint64_t(*)(CModelLoader* loader, model_t* model)>();

inline CMemory p_CModelLoader__Map_IsValid;
inline auto CModelLoader__Map_IsValid = p_CModelLoader__Map_IsValid.RCast<bool(*)(CModelLoader* loader, const char* pszMapName)>();

inline CMemory p_GetSpriteInfo;
inline auto GetSpriteInfo = p_GetSpriteInfo.RCast<void* (*)(const char* pName, bool bIsAVI, bool bIsBIK, int& nWidth, int& nHeight, int& nFrameCount, void* a7)>();

inline CMemory p_BuildSpriteLoadName;
inline auto BuildSpriteLoadName = p_BuildSpriteLoadName.RCast<void* (*)(const char* pName, char* pOut, int outLen, bool& bIsAVI, bool& bIsBIK)>();

inline CModelLoader* g_pModelLoader;

void CModelLoader_Attach();
void CModelLoader_Detach();

///////////////////////////////////////////////////////////////////////////////
class VModelLoader : public IDetour
{
	virtual void GetAdr(void) const
	{
		spdlog::debug("| FUN: CModelLoader::FindModel              : {:#18x} |\n", p_CModelLoader__FindModel.GetPtr());
		spdlog::debug("| FUN: CModelLoader::LoadModel              : {:#18x} |\n", p_CModelLoader__LoadModel.GetPtr());
		spdlog::debug("| FUN: CModelLoader::UnloadModel            : {:#18x} |\n", p_CModelLoader__UnloadModel.GetPtr());
		spdlog::debug("| FUN: CModelLoader::Map_LoadModelGuts      : {:#18x} |\n", p_CModelLoader__Map_LoadModelGuts.GetPtr());
		spdlog::debug("| FUN: CModelLoader::Map_IsValid            : {:#18x} |\n", p_CModelLoader__Map_IsValid.GetPtr());
		spdlog::debug("| FUN: CModelLoader::Studio_LoadModel       : {:#18x} |\n", p_CModelLoader__Studio_LoadModel.GetPtr());
		spdlog::debug("| FUN: GetSpriteInfo                        : {:#18x} |\n", p_GetSpriteInfo.GetPtr());
		spdlog::debug("| FUN: BuildSpriteLoadName                  : {:#18x} |\n", p_BuildSpriteLoadName.GetPtr());
		spdlog::debug("| VAR: g_pModelLoader                       : {:#18x} |\n", reinterpret_cast<uintptr_t>(g_pModelLoader));
		spdlog::debug("+----------------------------------------------------------------+\n");
	}
	virtual void GetFun(void) const
	{
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		p_CModelLoader__FindModel         = g_GameDll.FindPatternSIMD("40 55 41 55 41 56 48 8D AC 24 ?? ?? ?? ??");
		p_CModelLoader__LoadModel         = g_GameDll.FindPatternSIMD("40 53 57 41 56 48 81 EC ?? ?? ?? ?? 48 8B FA");
		p_CModelLoader__UnloadModel       = g_GameDll.FindPatternSIMD("48 8B C4 48 89 58 18 55 48 81 EC ?? ?? ?? ?? 48 8B DA");
		p_CModelLoader__Studio_LoadModel  = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 55 56 57 41 54 41 56 48 8D AC 24 ?? ?? ?? ??");
		p_CModelLoader__Map_LoadModelGuts = g_GameDll.FindPatternSIMD("48 89 54 24 ?? 48 89 4C 24 ?? 55 53 41 54 41 55 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? FF 05 ?? ?? ?? ??"); // BSP.
		p_CModelLoader__Map_IsValid       = g_GameDll.FindPatternSIMD("48 8B C4 53 48 81 EC ?? ?? ?? ?? 48 8B DA");
		p_GetSpriteInfo                   = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 41 54 41 55 41 56 41 57 48 83 EC 30 4C 8B AC 24 ?? ?? ?? ?? BE ?? ?? ?? ??");
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
		p_CModelLoader__FindModel         = g_GameDll.FindPatternSIMD("40 55 41 57 48 83 EC 48 80 3A 2A");
		p_CModelLoader__LoadModel         = g_GameDll.FindPatternSIMD("40 53 57 41 57 48 81 EC ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ??");
		p_CModelLoader__UnloadModel       = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 57 48 81 EC ?? ?? ?? ?? 48 8B F9 33 ED");
		p_CModelLoader__Studio_LoadModel  = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 55 56 57 41 54 41 57 48 81 EC ?? ?? ?? ??");
		p_CModelLoader__Map_LoadModelGuts = g_GameDll.FindPatternSIMD("48 89 54 24 ?? 48 89 4C 24 ?? 55 53 56 57 41 54 41 55 41 57"); // BSP.
		p_CModelLoader__Map_IsValid       = g_GameDll.FindPatternSIMD("40 53 48 81 EC ?? ?? ?? ?? 48 8B DA 48 85 D2 0F 84 ?? ?? ?? ?? 80 3A ?? 0F 84 ?? ?? ?? ?? 4C 8B CA");
		p_GetSpriteInfo                   = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 41 54 41 55 41 56 41 57 48 83 EC 30 4C 8B BC 24 ?? ?? ?? ??");
#endif
		p_BuildSpriteLoadName             = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 41 56 48 81 EC ?? ?? ?? ?? 4D 8B F1 48 8B F2");

		CModelLoader__FindModel         = p_CModelLoader__FindModel.RCast<void* (*)(CModelLoader*, const char*)>();
		CModelLoader__LoadModel         = p_CModelLoader__LoadModel.RCast<void(*)(CModelLoader*, model_t*)>();
		CModelLoader__UnloadModel       = p_CModelLoader__UnloadModel.RCast<uint64_t(*)(CModelLoader*, model_t*)>();
		CModelLoader__Studio_LoadModel  = p_CModelLoader__Studio_LoadModel.RCast<void* (*)(CModelLoader*)>();
		CModelLoader__Map_LoadModelGuts = p_CModelLoader__Map_LoadModelGuts.RCast<uint64_t(*)(CModelLoader*, model_t* mod)>();
		CModelLoader__Map_IsValid       = p_CModelLoader__Map_IsValid.RCast<bool(*)(CModelLoader*, const char*)>();
		GetSpriteInfo                   = p_GetSpriteInfo.RCast<void* (*)(const char*, bool, bool, int&, int&, int&, void*)>();
		BuildSpriteLoadName             = p_BuildSpriteLoadName.RCast<void* (*)(const char*, char*, int, bool&, bool&)>();
	}
	virtual void GetVar(void) const
	{
		g_pModelLoader = g_GameDll.FindPatternSIMD(
			"48 89 4C 24 ?? 53 55 56 41 54 41 55 41 56 41 57 48 81 EC ?? ?? ?? ??").FindPatternSelf("48 ?? 0D", CMemory::Direction::DOWN).ResolveRelativeAddressSelf(3, 7).RCast<CModelLoader*>();
	}
	virtual void GetCon(void) const { }
	virtual void Attach(void) const { }
	virtual void Detach(void) const { }
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(VModelLoader);
