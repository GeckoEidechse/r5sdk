#ifndef MDLCACHE_H
#define MDLCACHE_H
#include "tier0/threadtools.h"
#include "tier1/utldict.h"
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "public/studio.h"

struct RStaticProp_t
{
	studiohdr_t* m_pStudioHDR{};
	CStudioHWDataRef* m_pHardWareData{};
	const char* m_szPropName{};
	uint8_t m_pUnknown[0x62]{};
};

struct RMDLFallBack_t
{
	studiohdr_t* m_pErrorHDR;
	studiohdr_t* m_pEmptyHDR;
	MDLHandle_t m_hErrorMDL;
	MDLHandle_t m_hEmptyMDL;

	RMDLFallBack_t(void)
		: m_pErrorHDR(nullptr)
		, m_pEmptyHDR(nullptr)
		, m_hErrorMDL(NULL)
		, m_hEmptyMDL(NULL)
	{
	}

	// This must be cleared if 'common.rpak' is getting unloaded!
	void Clear(void)
	{
		m_pErrorHDR = nullptr;
		m_pEmptyHDR = nullptr;
		m_hErrorMDL = NULL;
		m_hEmptyMDL = NULL;
	}
};

// only models with type "mod_studio" have this data
struct studiodata_t
{
	DataCacheHandle_t m_MDLCache;
	void* m_pAnimData; // !TODO: reverse struct.
	unsigned short m_nRefCount;
	unsigned short m_nFlags;
	MDLHandle_t m_Handle;
#ifndef GAMEDLL_S3
	void* Unk1; // TODO: unverified!
	void* Unk2; // TODO: unverified!
#endif // !GAMEDLL_S3
	void* Unk3; // ptr to flags and model string.
	CStudioHWDataRef* m_pHardwareRef;
	void* m_pMaterialTable; // contains a large table of CMaterialGlue objects.
	int Unk5;
	char pad[72];
	CThreadFastMutex m_Mutex;
	int m_nGuidLock; // always -1, set to 1 and 0 in CMDLCache::FindUncachedMDL.
};

inline RMDLFallBack_t* g_pMDLFallback = new RMDLFallBack_t();
inline vector<MDLHandle_t> g_vBadMDLHandles;

class CMDLCache
{
public:
	static studiohdr_t* FindMDL(CMDLCache* cache, MDLHandle_t handle, void* a3);
	static void FindCachedMDL(CMDLCache* cache, studiodata_t* pStudioData, void* a3);
	static studiohdr_t* FindUncachedMDL(CMDLCache* cache, MDLHandle_t handle, studiodata_t* pStudioData, void* a4);
	static studiohdr_t* GetStudioHDR(CMDLCache* cache, MDLHandle_t handle);
	static studiohwdata_t* GetHardwareData(CMDLCache* cache, MDLHandle_t handle);
	static void* GetMaterialTable(CMDLCache* cache, MDLHandle_t handle);
	static studiohdr_t* GetErrorModel(void);
	static bool IsKnownBadModel(MDLHandle_t handle);

	CMDLCache* m_pVTable;
	void* m_pStrCmp;             // string compare func;
	void* m_pModelCacheSection;  // IDataCacheSection*
	int m_nModelCacheFrameLocks; //
	// TODO..
};

inline CMemory p_CMDLCache__FindMDL;
inline auto v_CMDLCache__FindMDL = p_CMDLCache__FindMDL.RCast<studiohdr_t* (*)(CMDLCache* pCache, void* a2, void* a3)>();
#if !defined (GAMEDLL_S0) && !defined (GAMEDLL_S1) && !defined (GAMEDLL_S2)
inline CMemory p_CMDLCache__FindCachedMDL;
inline auto v_CMDLCache__FindCachedMDL = p_CMDLCache__FindCachedMDL.RCast<void(*)(CMDLCache* pCache, void* a2, void* a3)>();

inline CMemory p_CMDLCache__FindUncachedMDL;
inline auto v_CMDLCache__FindUncachedMDL = p_CMDLCache__FindUncachedMDL.RCast<studiohdr_t* (*)(CMDLCache* pCache, MDLHandle_t handle, void* a3, void* a4)>();
#endif
inline CMemory p_CMDLCache__GetStudioHDR;
inline auto v_CMDLCache__GetStudioHDR = p_CMDLCache__GetStudioHDR.RCast<studiohdr_t* (*)(CMDLCache* pCache, MDLHandle_t handle)>();

inline CMemory p_CMDLCache__GetHardwareData;
inline auto v_CMDLCache__GetHardwareData = p_CMDLCache__GetHardwareData.RCast<studiohwdata_t* (*)(CMDLCache* pCache, MDLHandle_t handle)>();
#if !defined (GAMEDLL_S0) && !defined (GAMEDLL_S1) && !defined (GAMEDLL_S2)
inline CMemory p_CStudioHWDataRef__SetFlags; // Probably incorrect.
inline auto v_CStudioHWDataRef__SetFlags = p_CStudioHWDataRef__SetFlags.RCast<bool (*)(CStudioHWDataRef* ref, int64_t flags)>();
#endif
inline CUtlDict<studiodata_t*, MDLHandle_t>* m_MDLDict;
inline LPCRITICAL_SECTION* m_MDLMutex = nullptr;
inline PSRWLOCK* m_MDLLock = nullptr;
inline CMDLCache* g_MDLCache = nullptr;


void MDLCache_Attach();
void MDLCache_Detach();
///////////////////////////////////////////////////////////////////////////////
class VMDLCache : public IDetour
{
	virtual void GetAdr(void) const
	{
		spdlog::debug("| FUN: CMDLCache::FindMDL                   : {:#18x} |\n", p_CMDLCache__FindMDL.GetPtr());
#if !defined (GAMEDLL_S0) && !defined (GAMEDLL_S1) && !defined (GAMEDLL_S2)
		spdlog::debug("| FUN: CMDLCache::FindCachedMDL             : {:#18x} |\n", p_CMDLCache__FindCachedMDL.GetPtr());
		spdlog::debug("| FUN: CMDLCache::FindUncachedMDL           : {:#18x} |\n", p_CMDLCache__FindUncachedMDL.GetPtr());
#endif
		spdlog::debug("| FUN: CMDLCache::GetStudioHDR              : {:#18x} |\n", p_CMDLCache__GetStudioHDR.GetPtr());
		spdlog::debug("| FUN: CMDLCache::GetHardwareData           : {:#18x} |\n", p_CMDLCache__GetHardwareData.GetPtr());
#if !defined (GAMEDLL_S0) && !defined (GAMEDLL_S1) && !defined (GAMEDLL_S2)
		spdlog::debug("| FUN: CStudioHWDataRef::SetFlags           : {:#18x} |\n", p_CStudioHWDataRef__SetFlags.GetPtr());
#endif
		spdlog::debug("| VAR: m_MDLMutex                           : {:#18x} |\n", reinterpret_cast<uintptr_t>(m_MDLMutex));
		spdlog::debug("| VAR: m_MDLLock                            : {:#18x} |\n", reinterpret_cast<uintptr_t>(m_MDLLock));
		spdlog::debug("| VAR: m_MDLDict                            : {:#18x} |\n", reinterpret_cast<uintptr_t>(m_MDLDict));
		spdlog::debug("| VAR: g_MDLCache                           : {:#18x} |\n", reinterpret_cast<uintptr_t>(g_MDLCache));
		spdlog::debug("+----------------------------------------------------------------+\n");
	}
	virtual void GetFun(void) const
	{
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1) || defined (GAMEDLL_S2)
		p_CMDLCache__FindMDL = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 41 56 48 83 EC 20 4C 8B F1 0F B7 DA");
		v_CMDLCache__FindMDL = p_CMDLCache__FindMDL.RCast<studiohdr_t* (*)(CMDLCache*, void*, void*)>(); /*48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 4C 8B F1 0F B7 DA*/

		p_CMDLCache__GetStudioHDR = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC 20 48 8B F1 0F B7 FA 48 8D 0D ?? ?? ?? ??");
		v_CMDLCache__GetStudioHDR = p_CMDLCache__GetStudioHDR.RCast<studiohdr_t* (*)(CMDLCache*, MDLHandle_t)>(); /*48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F1 0F B7 FA 48 8D 0D ? ? ? ?*/

#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		p_CMDLCache__GetHardwareData = g_GameDll.FindPatternSIMD("40 56 48 83 EC 20 48 89 5C 24 ?? 48 8D 0D ?? ?? ?? ??");
		v_CMDLCache__GetHardwareData = p_CMDLCache__GetHardwareData.RCast<studiohwdata_t* (*)(CMDLCache*, MDLHandle_t)>(); /*40 56 48 83 EC 20 48 89 5C 24 ? 48 8D 0D ? ? ? ?*/
#else
		p_CMDLCache__GetHardwareData = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 57 48 83 EC 20 48 8D 0D ?? ?? ?? ?? 0F B7 DA FF 15 ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 8D 14 5B 48 8D 0D ?? ?? ?? ?? 48 8B 7C D0 ?? FF 15 ?? ?? ?? ?? 48 8B 1F");
		v_CMDLCache__GetHardwareData = p_CMDLCache__GetHardwareData.RCast<studiohwdata_t* (*)(CMDLCache*, MDLHandle_t)>(); /*48 89 5C 24 ? 57 48 83 EC 20 48 8D 0D ? ? ? ? 0F B7 DA FF 15 ? ? ? ? 48 8B 05 ? ? ? ? 48 8D 14 5B 48 8D 0D ? ? ? ? 48 8B 7C D0 ? FF 15 ? ? ? ? 48 8B 1F*/
#endif
#else
		p_CMDLCache__FindMDL = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 48 83 EC 20 48 8B F1 0F B7 EA");
		v_CMDLCache__FindMDL = p_CMDLCache__FindMDL.RCast<studiohdr_t* (*)(CMDLCache*, void*, void*)>(); /*48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F1 0F B7 EA*/

		p_CMDLCache__FindCachedMDL = g_GameDll.FindPatternSIMD("4D 85 C0 74 7A 48 89 6C 24 ??");
		v_CMDLCache__FindCachedMDL = p_CMDLCache__FindCachedMDL.RCast<void(*)(CMDLCache*, void*, void*)>(); /*4D 85 C0 74 7A 48 89 6C 24 ?*/

		p_CMDLCache__FindUncachedMDL = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 41 56 48 83 EC 20 48 8B E9 0F B7 FA");
		v_CMDLCache__FindUncachedMDL = p_CMDLCache__FindUncachedMDL.RCast<studiohdr_t* (*)(CMDLCache*, MDLHandle_t, void*, void*)>(); /*48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 48 8B E9 0F B7 FA*/

		p_CMDLCache__GetStudioHDR = g_GameDll.FindPatternSIMD("40 53 48 83 EC 20 48 8D 0D ?? ?? ?? ?? 0F B7 DA FF 15 ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 8D 14 5B 48 8D 0D ?? ?? ?? ?? 48 8B 5C D0 ?? FF 15 ?? ?? ?? ?? 48 8B 03 48 8B 48 08");
		v_CMDLCache__GetStudioHDR = p_CMDLCache__GetStudioHDR.RCast<studiohdr_t* (*)(CMDLCache*, MDLHandle_t)>(); /*40 53 48 83 EC 20 48 8D 0D ? ? ? ? 0F B7 DA FF 15 ? ? ? ? 48 8B 05 ? ? ? ? 48 8D 14 5B 48 8D 0D ? ? ? ? 48 8B 5C D0 ? FF 15 ? ? ? ? 48 8B 03 48 8B 48 08*/

		p_CMDLCache__GetHardwareData = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 57 48 83 EC 20 48 8D 0D ?? ?? ?? ?? 0F B7 DA FF 15 ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 8D 14 5B 48 8D 0D ?? ?? ?? ?? 48 8B 7C D0 ?? FF 15 ?? ?? ?? ?? 48 8B 1F");
		v_CMDLCache__GetHardwareData = p_CMDLCache__GetHardwareData.RCast<studiohwdata_t* (*)(CMDLCache*, MDLHandle_t)>(); /*48 89 5C 24 ? 57 48 83 EC 20 48 8D 0D ? ? ? ? 0F B7 DA FF 15 ? ? ? ? 48 8B 05 ? ? ? ? 48 8D 14 5B 48 8D 0D ? ? ? ? 48 8B 7C D0 ? FF 15 ? ? ? ? 48 8B 1F*/

		p_CStudioHWDataRef__SetFlags = g_GameDll.FindPatternSIMD("48 83 EC 08 4C 8D 14 12");
		v_CStudioHWDataRef__SetFlags = p_CStudioHWDataRef__SetFlags.RCast<bool (*)(CStudioHWDataRef*, int64_t)>(); /*48 83 EC 08 4C 8D 14 12*/
#endif
	}
	virtual void GetVar(void) const
	{
		m_MDLMutex = g_GameDll.FindPatternSIMD("48 83 EC 28 BA ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? FF 15 ?? ?? ?? ?? 0F B6 05 ?? ?? ?? ??")
			.FindPatternSelf("48 8D 0D").ResolveRelativeAddressSelf(0x3, 0x7).RCast<LPCRITICAL_SECTION*>();

		m_MDLLock = p_CMDLCache__GetHardwareData.Offset(0x35).FindPatternSelf("48 8D 0D").ResolveRelativeAddressSelf(0x3, 0x7).RCast<PSRWLOCK*>();

		m_MDLDict = p_CMDLCache__FindMDL.FindPattern("48 8B 05").ResolveRelativeAddressSelf(0x3, 0x7).RCast<CUtlDict<studiodata_t*, MDLHandle_t>*>();

		g_MDLCache = g_GameDll.FindPatternSIMD("48 8B 0D ?? ?? ?? ?? 44 0F B7 82 ?? ?? ?? ?? 48 8B 01 48 FF A0 30 01 ?? ??")
			.ResolveRelativeAddressSelf(0x3, 0x7).RCast<CMDLCache*>();
	}
	virtual void GetCon(void) const { }
	virtual void Attach(void) const { }
	virtual void Detach(void) const { }
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(VMDLCache);
#endif // MDLCACHE_H
