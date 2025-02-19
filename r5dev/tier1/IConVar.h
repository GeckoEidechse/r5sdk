#pragma once
#include "tier1/cmd.h"
#include "mathlib/color.h"
#include "public/iconvar.h"
#include "tier1/utlvector.h"

//-----------------------------------------------------------------------------
// Purpose: A console variable
//-----------------------------------------------------------------------------
class ConVar : public ConCommandBase
{
public:
	static ConVar* Create(const char* pszName, const char* pszDefaultValue, int nFlags, const char* pszHelpString,
		bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t pCallback, const char* pszUsageString);
	ConVar(void);
	~ConVar(void);

	static void Init(void);
	static void InitShipped(void);

	static void PurgeShipped(void);
	static void PurgeHostNames(void);

	void AddFlags(int nFlags);
	void RemoveFlags(int nFlags);

	const char* GetBaseName(void) const;
	const char* GetHelpText(void) const;
	const char* GetUsageText(void) const;

	bool GetBool(void) const;
	float GetFloat(void) const;
	double GetDouble(void) const;
	int GetInt(void) const;
	int64_t GetInt64(void) const;
	size_t GetSizeT(void) const;
	Color GetColor(void) const;
	const char* GetString(void) const;

	bool GetMin(float& flMinValue) const;
	bool GetMax(float& flMaxValue) const;
	float GetMinValue(void) const;
	float GetMaxValue(void) const;
	bool HasMin(void) const;
	bool HasMax(void) const;

	void SetValue(int nValue);
	void SetValue(float flValue);
	void SetValue(const char* pszValue);
	void SetValue(Color clValue);

	void InternalSetValue(const char* pszValue);
	void InternalSetIntValue(int nValue);
	void InternalSetFloatValue(float flValue);
	void InternalSetColorValue(Color value);

	void Revert(void);
	bool ClampValue(float& flValue);

	const char* GetDefault(void) const;
	void SetDefault(const char* pszDefault);
	bool SetColorFromString(const char* pszValue);

	void ChangeStringValue(const char* pszTempValue);
	void ChangeStringValueUnsafe(const char* pszNewValue);

	void InstallChangeCallback(FnChangeCallback_t callback, bool bInvoke);
	void RemoveChangeCallback(FnChangeCallback_t callback);

	bool IsRegistered(void) const;
	bool IsCommand(void) const;

	bool IsFlagSet(int nFlags) const { return IsFlagSetInternal(this, nFlags); };
	static bool IsFlagSetInternal(const ConVar* pConVar, int nFlags);

	struct CVValue_t
	{
		char*      m_pszString;
		size_t     m_iStringLength;
		float      m_fValue;
		int        m_nValue;
	};

	IConVar*       m_pIConVarVFTable; //0x0040
	ConVar*        m_pParent        ; //0x0048
	const char*    m_pszDefaultValue; //0x0050
	CVValue_t      m_Value          ; //0c0058
	bool           m_bHasMin        ; //0x0070
	float          m_fMinVal        ; //0x0074
	bool           m_bHasMax        ; //0x0078
	float          m_fMaxVal        ; //0x007C
	CUtlVector<FnChangeCallback_t> m_fnChangeCallbacks; //0x0080
}; //Size: 0x00A0
static_assert(sizeof(ConVar) == 0xA0);

/* ==== ICONVAR ========================================================================================================================================================= */
inline CMemory p_ConVar_IsFlagSet;
inline auto v_ConVar_IsFlagSet = p_ConVar_IsFlagSet.RCast<bool (*)(ConVar* pConVar, int nFlag)>();

inline CMemory p_ConVar_Register;
inline auto v_ConVar_Register = p_ConVar_Register.RCast<void* (*)(ConVar* thisptr, const char* szName, const char* szDefaultValue, int nFlags, const char* szHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t pCallback, const char* pszUsageString)>();

inline CMemory p_ConVar_PrintDescription;
inline auto v_ConVar_PrintDescription = p_ConVar_PrintDescription.RCast<void* (*)(ConCommandBase* pVar)>();

inline CMemory g_pConVarVFTable;
inline CMemory g_pIConVarVFTable;

///////////////////////////////////////////////////////////////////////////////
void IConVar_Attach();
void IConVar_Detach();

void ConVar_PrintDescription(ConCommandBase* pVar);

///////////////////////////////////////////////////////////////////////////////
class VConVar : public IDetour
{
	virtual void GetAdr(void) const
	{
		spdlog::debug("| FUN: ConVar::IsFlagSet                    : {:#18x} |\n", p_ConVar_IsFlagSet.GetPtr());
		spdlog::debug("| FUN: ConVar::Register                     : {:#18x} |\n", p_ConVar_Register.GetPtr());
		spdlog::debug("| FUN: ConVar_PrintDescription              : {:#18x} |\n", p_ConVar_PrintDescription.GetPtr());
		spdlog::debug("| CON: g_pConVarVFTable                     : {:#18x} |\n", g_pConVarVFTable.GetPtr());
		spdlog::debug("| CON: g_pIConVarVFTable                    : {:#18x} |\n", g_pIConVarVFTable.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
	}
	virtual void GetFun(void) const
	{
		p_ConVar_IsFlagSet = g_GameDll.FindPatternSIMD("48 8B 41 48 85 50 38");
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		p_ConVar_Register = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 41 56 48 83 EC 30 F3 0F 10 44 24 ??");
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
		p_ConVar_Register = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 48 83 EC 40 F3 0F 10 84 24 ?? ?? ?? ??");
#endif
		p_ConVar_PrintDescription = g_GameDll.FindPatternSIMD("B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 2B E0 48 8B 01 48 89 9C 24 ?? ?? ?? ??");

		v_ConVar_IsFlagSet = p_ConVar_IsFlagSet.RCast<bool (*)(ConVar*, int)>();                                                                                                  /*48 8B 41 48 85 50 38*/
		v_ConVar_Register = p_ConVar_Register.RCast<void* (*)(ConVar*, const char*, const char*, int, const char*, bool, float, bool, float, FnChangeCallback_t, const char*)>(); /*48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 40 F3 0F 10 84 24 ? ? ? ?*/
		v_ConVar_PrintDescription = p_ConVar_PrintDescription.RCast<void* (*)(ConCommandBase*)>();                                                                                /*B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8B 01 48 89 9C 24 ? ? ? ?*/
	}
	virtual void GetVar(void) const { }
	virtual void GetCon(void) const
	{
		g_pConVarVFTable = g_GameDll.GetVirtualMethodTable(".?AVConVar@@", 0);
		g_pIConVarVFTable = g_GameDll.GetVirtualMethodTable(".?AVConVar@@", 1);
	}
	virtual void Attach(void) const { }
	virtual void Detach(void) const { }
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(VConVar);
