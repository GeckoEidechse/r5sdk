#pragma once
#include "squirrel/sqtype.h"
#include "squirrel/sqvm.h"

///////////////////////////////////////////////////////////////////////////////
SQRESULT sq_pushroottable(HSQUIRRELVM v);
SQChar* sq_getstring(HSQUIRRELVM v, SQInteger i);
SQInteger sq_getinteger(HSQUIRRELVM v, SQInteger i);
SQRESULT sq_pushroottable(HSQUIRRELVM v);
void sq_pushbool(HSQUIRRELVM v, SQBool b);
void sq_pushstring(HSQUIRRELVM v, const SQChar* string, SQInteger len);
void sq_pushinteger(HSQUIRRELVM v, SQInteger val);
void sq_newarray(HSQUIRRELVM v, SQInteger size);
void sq_newtable(HSQUIRRELVM v);
SQRESULT sq_newslot(HSQUIRRELVM v, SQInteger idx);
SQRESULT sq_arrayappend(HSQUIRRELVM v, SQInteger idx);
SQRESULT sq_pushstructure(HSQUIRRELVM v, const SQChar* name, const SQChar* member, const SQChar* codeclass1, const SQChar* codeclass2);
SQRESULT sq_compilebuffer(HSQUIRRELVM v, SQBufState* bufferState, const SQChar* buffer, SQInteger context);
SQRESULT sq_call(HSQUIRRELVM v, SQInteger params, SQBool retval, SQBool raiseerror);

void SQAPI_Attach();
void SQAPI_Detach();

/* ==== SQUIRREL ======================================================================================================================================================== */
inline CMemory p_sq_pushroottable;
inline auto v_sq_pushroottable = p_sq_pushroottable.RCast<SQRESULT(*)(HSQUIRRELVM v)>();

inline CMemory p_sq_pushbool;
inline auto v_sq_pushbool = p_sq_pushbool.RCast<void (*)(HSQUIRRELVM v, SQBool b)>();

inline CMemory p_sq_pushstring;
inline auto v_sq_pushstring = p_sq_pushstring.RCast<void (*)(HSQUIRRELVM v, const SQChar* string, SQInteger len)>();

inline CMemory p_sq_pushinteger;
inline auto v_sq_pushinteger = p_sq_pushinteger.RCast<void (*)(HSQUIRRELVM v, SQInteger val)>();

inline CMemory p_sq_newarray;
inline auto v_sq_newarray = p_sq_newarray.RCast<void (*)(HSQUIRRELVM v, SQInteger size)>();

inline CMemory p_sq_newtable;
inline auto v_sq_newtable = p_sq_newtable.RCast<void (*)(HSQUIRRELVM v)>();

inline CMemory p_sq_newslot;
inline auto v_sq_newslot = p_sq_newslot.RCast<SQRESULT(*)(HSQUIRRELVM v, SQInteger idx)>();

inline CMemory p_sq_arrayappend;
inline auto v_sq_arrayappend = p_sq_arrayappend.RCast<SQRESULT(*)(HSQUIRRELVM v, SQInteger idx)>();

inline CMemory p_sq_pushstructure;
inline auto v_sq_pushstructure = p_sq_pushstructure.RCast<SQRESULT(*)(HSQUIRRELVM v, const SQChar* name, const SQChar* member, const SQChar* codeclass1, const SQChar* codeclass2)>();

inline CMemory p_sq_compilebuffer;
inline auto v_sq_compilebuffer = p_sq_compilebuffer.RCast<SQRESULT(*)(HSQUIRRELVM v, SQBufState* bufferstate, const SQChar* buffer, SQInteger level)>();

inline CMemory p_sq_call;
inline auto v_sq_call = p_sq_call.RCast<SQRESULT(*)(HSQUIRRELVM v, SQInteger params, SQBool retval, SQBool raiseerror)>();

///////////////////////////////////////////////////////////////////////////////
class VSqapi : public IDetour
{
	virtual void GetAdr(void) const
	{
		spdlog::debug("| FUN: sq_pushroottable                     : {:#18x} |\n", p_sq_pushroottable.GetPtr());
		spdlog::debug("| FUN: sq_pushbool                          : {:#18x} |\n", p_sq_pushbool.GetPtr());
		spdlog::debug("| FUN: sq_pushstring                        : {:#18x} |\n", p_sq_pushstring.GetPtr());
		spdlog::debug("| FUN: sq_pushinteger                       : {:#18x} |\n", p_sq_pushinteger.GetPtr());
		spdlog::debug("| FUN: sq_newarray                          : {:#18x} |\n", p_sq_newarray.GetPtr());
		spdlog::debug("| FUN: sq_arrayappend                       : {:#18x} |\n", p_sq_arrayappend.GetPtr());
		spdlog::debug("| FUN: sq_newtable                          : {:#18x} |\n", p_sq_newtable.GetPtr());
		spdlog::debug("| FUN: sq_newslot                           : {:#18x} |\n", p_sq_newslot.GetPtr());
		spdlog::debug("| FUN: sq_pushstructure                     : {:#18x} |\n", p_sq_pushstructure.GetPtr());
		spdlog::debug("| FUN: sq_compilebuffer                     : {:#18x} |\n", p_sq_compilebuffer.GetPtr());
		spdlog::debug("| FUN: sq_call                              : {:#18x} |\n", p_sq_call.GetPtr());
		spdlog::debug("+----------------------------------------------------------------+\n");
	}
	virtual void GetFun(void) const
	{
		p_sq_pushroottable = g_GameDll.FindPatternSIMD("48 83 EC 28 8B 51 ?? 44 8B C2");
		p_sq_pushbool      = g_GameDll.FindPatternSIMD("48 83 EC 38 33 C0 48 C7 44 24 20 08 ?? ?? 01 48");
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1) || defined (GAMEDLL_S2)
		p_sq_pushstring = g_GameDll.FindPatternSIMD("40 56 48 83 EC 30 48 8B F1 48 85 D2 0F 84 8C ??");
#elif defined (GAMEDLL_S3)
		p_sq_pushstring = g_GameDll.FindPatternSIMD("40 56 48 83 EC 30 48 8B F1 48 85 D2 0F 84 8F ??");
#endif
		p_sq_pushinteger  = g_GameDll.FindPatternSIMD("48 83 EC 38 33 C0 48 C7 44 24 20 02 ?? ?? 05 48");
		p_sq_newarray     = g_GameDll.FindPatternSIMD("48 89 5C 24 08 57 48 83 EC 30 48 8B D9 48 C7 44 24 20 40");
		p_sq_newtable     = g_GameDll.FindPatternSIMD("48 89 5C 24 08 57 48 83 EC 30 48 8B D9 48 C7 44 24 20 20");
		p_sq_newslot      = g_GameDll.FindPatternSIMD("40 53 48 83 EC 30 44 8B 49 ?? 48 8B D9 41 8B C1");
		p_sq_arrayappend  = g_GameDll.FindPatternSIMD("40 53 48 83 EC 20 8B 41 ?? 48 8B D9 2B 41 ?? 83 F8 02 7D");
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1) || defined (GAMEDLL_S2)
		p_sq_pushstructure = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 4C 89 4C 24 ?? 55 41 54 41 55 41 56 41 57 48 8B EC");
#elif defined (GAMEDLL_S3)
		p_sq_pushstructure = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 60 48 8B 59 60");
#endif
		p_sq_compilebuffer = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 41 56 41 57 48 83 EC 50 41 8B E9 49 8B F8");
		p_sq_call          = g_GameDll.FindPatternSIMD("4C 8B DC 49 89 5B 08 49 89 6B 10 49 89 73 18 57 48 83 EC 50 8B F2");

		v_sq_pushroottable = p_sq_pushroottable.RCast<SQRESULT(*)(HSQUIRRELVM)>();                                                             /*48 83 EC 28 8B 51 ?? 44 8B C2*/
		v_sq_pushbool      = p_sq_pushbool.RCast<void (*)(HSQUIRRELVM, SQBool)>();                                                             /*48 83 EC 38 33 C0 48 C7 44 24 20 08 00 00 01 48*/
		v_sq_pushstring    = p_sq_pushstring.RCast<void (*)(HSQUIRRELVM, const SQChar*, SQInteger)>();                                         /*40 56 48 83 EC 30 48 8B F1 48 85 D2 0F 84 8F 00*/
		v_sq_pushinteger   = p_sq_pushinteger.RCast<void (*)(HSQUIRRELVM, SQInteger)>();                                                       /*48 83 EC 38 33 C0 48 C7 44 24 20 02 00 00 05 48*/
		v_sq_newarray      = p_sq_newarray.RCast<void (*)(HSQUIRRELVM, SQInteger)>();                                                          /*48 89 5C 24 08 57 48 83 EC 30 48 8B D9 48 C7 44 24 20 40*/
		v_sq_newtable      = p_sq_newtable.RCast<void (*)(HSQUIRRELVM)>();                                                                     /*48 89 5C 24 08 57 48 83 EC 30 48 8B D9 48 C7 44 24 20 20*/
		v_sq_newslot       = p_sq_newslot.RCast<SQRESULT(*)(HSQUIRRELVM, SQInteger)>();                                                        /*40 53 48 83 EC 20 8B 41 ?? 48 8B D9 2B 41 ?? 83 F8 02 7D*/
		v_sq_arrayappend   = p_sq_arrayappend.RCast<SQRESULT(*)(HSQUIRRELVM, SQInteger)>();                                                    /*40 53 48 83 EC 20 8B 41 ?? 48 8B D9 2B 41 ?? 83 F8 02 7D*/
		v_sq_pushstructure = p_sq_pushstructure.RCast<SQRESULT(*)(HSQUIRRELVM, const SQChar*, const SQChar*, const SQChar*, const SQChar*)>(); /*48 89 5C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 60 48 8B 59 60*/
		v_sq_compilebuffer = p_sq_compilebuffer.RCast<SQRESULT(*)(HSQUIRRELVM, SQBufState*, const SQChar*, SQInteger)>();                      /*48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 41 56 41 57 48 83 EC 50 41 8B E9 49 8B F8*/
		v_sq_call          = p_sq_call.RCast<SQRESULT(*)(HSQUIRRELVM, SQInteger, SQBool, SQBool)>();                                           /*4C 8B DC 49 89 5B 08 49 89 6B 10 49 89 73 18 57 48 83 EC 50 8B F2*/

	}
	virtual void GetVar(void) const { }
	virtual void GetCon(void) const { }
	virtual void Attach(void) const { }
	virtual void Detach(void) const { }
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(VSqapi);
