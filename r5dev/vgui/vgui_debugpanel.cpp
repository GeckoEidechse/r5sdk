//===========================================================================//
//
// Purpose: Implements the debug panels.
//
// $NoKeywords: $
//===========================================================================//

#include <core/stdafx.h>
#include <tier1/cvar.h>
#include <windows/id3dx.h>
#include <vpc/keyvalues.h>
#include <mathlib/color.h>
#include <rtech/rtech_utils.h>
#include <rtech/rui/rui.h>
#include <vgui/vgui_debugpanel.h>
#include <vguimatsurface/MatSystemSurface.h>
#include <materialsystem/cmaterialsystem.h>
#ifndef CLIENT_DLL
#include <engine/server/server.h>
#endif // !CLIENT_DLL
#include <engine/sys_engine.h>
#include <engine/debugoverlay.h>
#include <engine/client/clientstate.h>
#include <materialsystem/cmaterialglue.h>

//-----------------------------------------------------------------------------
// Purpose: proceed a log update
//-----------------------------------------------------------------------------
void CTextOverlay::Update(void)
{
	if (!g_pMatSystemSurface)
	{
		return;
	}
	if (con_drawnotify->GetBool())
	{
		DrawNotify();
	}
	if (cl_showsimstats->GetBool())
	{
		DrawSimStats();
	}
	if (cl_showgpustats->GetBool())
	{
		DrawGPUStats();
	}
	if (cl_showhoststats->GetBool())
	{
		DrawHostStats();
	}
	if (cl_showmaterialinfo->GetBool())
	{
		DrawCrosshairMaterial();
	}
	if (stream_overlay->GetBool())
	{
		DrawStreamOverlay();
	}
}

//-----------------------------------------------------------------------------
// Purpose: add a log to the vector.
//-----------------------------------------------------------------------------
void CTextOverlay::AddLog(const EGlobalContext_t context, const string& svText)
{
	if (!con_drawnotify->GetBool() || svText.empty())
	{
		return;
	}

	std::lock_guard<std::mutex> l(m_Mutex);
	m_vNotifyText.push_back(CTextNotify{ context, con_notifytime->GetFloat() , svText });

	while (m_vNotifyText.size() > 0 &&
		(m_vNotifyText.size() > con_notifylines->GetInt()))
	{
		m_vNotifyText.erase(m_vNotifyText.begin());
	}
}

//-----------------------------------------------------------------------------
// Purpose: draw notify logs on screen.
//-----------------------------------------------------------------------------
void CTextOverlay::DrawNotify(void)
{
	int x = con_notify_invert_x->GetBool() ? g_nWindowWidth - con_notify_offset_x->GetInt() : con_notify_offset_x->GetInt();
	int y = con_notify_invert_y->GetBool() ? g_nWindowHeight - con_notify_offset_y->GetInt() : con_notify_offset_y->GetInt();

	std::lock_guard<std::mutex> l(m_Mutex);
	for (size_t i = 0, j = m_vNotifyText.size(); i < j; i++)
	{
		CTextNotify* pNotify = &m_vNotifyText[i];
		Color c = GetLogColorForType(m_vNotifyText[i].m_type);

		float flTimeleft = pNotify->m_flLifeRemaining;

		if (flTimeleft < 1.0f)
		{
			float f = clamp(flTimeleft, 0.0f, 1.0f) / 1.0f;
			c[3] = static_cast<int>(f * 255.0f);

			if (i == 0 && f < 0.2f)
			{
				y -= m_nFontHeight * (1.0f - f / 0.2f);
			}
		}
		else
		{
			c[3] = 255;
		}
		CMatSystemSurface_DrawColoredText(g_pMatSystemSurface, v_Rui_GetFontFace(), m_nFontHeight, x, y, c.r(), c.g(), c.b(), c.a(), m_vNotifyText[i].m_svMessage.c_str());

		if (IsX360())
		{
			// For some reason the fontTall value on 360 is about twice as high as it should be
			y += 12;
		}
		else
		{
			y += m_nFontHeight;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: draws formatted text over RUI
// Input  : x - 
//			y - 
//			pszFormat - 
//			... - 
//-----------------------------------------------------------------------------
void CTextOverlay::DrawFormat(const int x, const int y, const Color c, const char* pszFormat, ...) const
{
	static char szLogbuf[4096]{};
	{/////////////////////////////
		va_list args{};
		va_start(args, pszFormat);

		vsnprintf(szLogbuf, sizeof(szLogbuf), pszFormat, args);

		szLogbuf[sizeof(szLogbuf) - 1] = '\0';
		va_end(args);
	}/////////////////////////////

	CMatSystemSurface_DrawColoredText(g_pMatSystemSurface, v_Rui_GetFontFace(), m_nFontHeight, x, y, c.r(), c.g(), c.b(), c.a(), szLogbuf);
}

//-----------------------------------------------------------------------------
// Purpose: checks if the notify text is expired
// Input  : flFrameTime - 
//-----------------------------------------------------------------------------
void CTextOverlay::ShouldDraw(const float flFrameTime)
{
	if (con_drawnotify->GetBool())
	{
		std::lock_guard<std::mutex> l(m_Mutex);

		int i;
		int c = m_vNotifyText.size();
		for (i = c - 1; i >= 0; i--)
		{
			CTextNotify* pNotify = &m_vNotifyText[i];
			pNotify->m_flLifeRemaining -= flFrameTime;

			if (pNotify->m_flLifeRemaining <= 0.0f)
			{
				m_vNotifyText.erase(m_vNotifyText.begin() + i);
				continue;
			}
		}
	}
	else if (!m_vNotifyText.empty())
	{
		std::lock_guard<std::mutex> l(m_Mutex);
		m_vNotifyText.clear();
	}
}

//-----------------------------------------------------------------------------
// Purpose: draws live host stats on screen.
//-----------------------------------------------------------------------------
void CTextOverlay::DrawHostStats(void) const
{
	static const Color c = { 255, 255, 255, 255 };
	const int nWidth = cl_hoststats_invert_x->GetBool() ? g_nWindowWidth - cl_hoststats_offset_x->GetInt() : cl_hoststats_offset_x->GetInt();
	const int nHeight = cl_hoststats_invert_y->GetBool() ? g_nWindowHeight - cl_hoststats_offset_y->GetInt() : cl_hoststats_offset_y->GetInt();

	CMatSystemSurface_DrawColoredText(g_pMatSystemSurface, v_Rui_GetFontFace(), m_nFontHeight, nWidth, nHeight, c.r(), c.g(), c.b(), c.a(), m_pszCon_NPrintf_Buf);
}

//-----------------------------------------------------------------------------
// Purpose: draws live simulation stats on screen.
//-----------------------------------------------------------------------------
void CTextOverlay::DrawSimStats(void) const
{
	static const Color c = { 255, 255, 255, 255 };
	const int nWidth = cl_simstats_invert_x->GetBool() ? g_nWindowWidth - cl_simstats_offset_x->GetInt() : cl_simstats_offset_x->GetInt();
	const int nHeight = cl_simstats_invert_y->GetBool() ? g_nWindowHeight - cl_simstats_offset_y->GetInt() : cl_simstats_offset_y->GetInt();

	DrawFormat(nWidth, nHeight, c, "Server Frame: (%d) Client Frame: (%d) Render Frame: (%d)\n", 
		g_pClientState->GetServerTickCount(), g_pClientState->GetClientTickCount(), *g_nRenderTickCount);
}

//-----------------------------------------------------------------------------
// Purpose: draws live gpu stats on screen.
//-----------------------------------------------------------------------------
void CTextOverlay::DrawGPUStats(void) const
{
	static const Color c = { 255, 255, 255, 255 };
	const int nWidth  = cl_gpustats_invert_x->GetBool() ? g_nWindowWidth - cl_gpustats_offset_x->GetInt() : cl_gpustats_offset_x->GetInt();
	const int nHeight = cl_gpustats_invert_y->GetBool() ? g_nWindowHeight - cl_gpustats_offset_y->GetInt() : cl_gpustats_offset_y->GetInt();

	DrawFormat(nWidth, nHeight, c, "%8d/%8d/%8dkiB unusable/unfree/total GPU Streaming Texture memory\n",
		*g_nUnusableStreamingTextureMemory / 1024, *g_nUnfreeStreamingTextureMemory / 1024, *g_nTotalStreamingTextureMemory / 1024);
}

//-----------------------------------------------------------------------------
// Purpose: draws currently traced material info on screen.
//-----------------------------------------------------------------------------
void CTextOverlay::DrawCrosshairMaterial(void) const
{
	CMaterialGlue* pMaterialGlue = GetMaterialAtCrossHair();
	if (!pMaterialGlue)
		return;

	static Color c = { 255, 255, 255, 255 };
	DrawFormat(cl_materialinfo_offset_x->GetInt(), cl_materialinfo_offset_y->GetInt(), c, "name: %s\nguid: %llx\ndimensions: %d x %d\nsurface: %s/%s\nstc: %i\ntc: %i",
		pMaterialGlue->m_pszName,
		pMaterialGlue->m_GUID,
		pMaterialGlue->m_iWidth, pMaterialGlue->m_iHeight,
		pMaterialGlue->m_pszSurfaceProp, pMaterialGlue->m_pszSurfaceProp2,
		pMaterialGlue->m_nStreamableTextureCount,
		pMaterialGlue->m_pShaderGlue->m_nTextureInputCount);
}

//-----------------------------------------------------------------------------
// Purpose: draws the stream overlay on screen.
//-----------------------------------------------------------------------------
void CTextOverlay::DrawStreamOverlay(void) const
{
	static char szLogbuf[4096];
	static const Color c = { 255, 255, 255, 255 };
	
	GetStreamOverlay(stream_overlay_mode->GetString(), szLogbuf, sizeof(szLogbuf));
	CMatSystemSurface_DrawColoredText(g_pMatSystemSurface, v_Rui_GetFontFace(), m_nFontHeight, 20, 300, c.r(), c.g(), c.b(), c.a(), szLogbuf);
}

//-----------------------------------------------------------------------------
// Purpose: gets the log color for context.
// Input  : context - 
// Output : Color
//-----------------------------------------------------------------------------
Color CTextOverlay::GetLogColorForType(const EGlobalContext_t context) const
{
	switch (context)
	{
	case EGlobalContext_t::SCRIPT_SERVER:
		return { con_notify_script_server_clr->GetColor() };
	case EGlobalContext_t::SCRIPT_CLIENT:
		return { con_notify_script_client_clr->GetColor() };
	case EGlobalContext_t::SCRIPT_UI:
		return { con_notify_script_ui_clr->GetColor() };
	case EGlobalContext_t::NATIVE_SERVER:
		return { con_notify_native_server_clr->GetColor() };
	case EGlobalContext_t::NATIVE_CLIENT:
		return { con_notify_native_client_clr->GetColor() };
	case EGlobalContext_t::NATIVE_UI:
		return { con_notify_native_ui_clr->GetColor() };
	case EGlobalContext_t::NATIVE_ENGINE:
		return { con_notify_native_engine_clr->GetColor() };
	case EGlobalContext_t::NATIVE_FS:
		return { con_notify_native_fs_clr->GetColor() };
	case EGlobalContext_t::NATIVE_RTECH:
		return { con_notify_native_rtech_clr->GetColor() };
	case EGlobalContext_t::NATIVE_MS:
		return { con_notify_native_ms_clr->GetColor() };
	case EGlobalContext_t::NATIVE_AUDIO:
		return { con_notify_native_audio_clr->GetColor() };
	case EGlobalContext_t::NATIVE_VIDEO:
		return { con_notify_native_video_clr->GetColor() };
	case EGlobalContext_t::NETCON_S:
		return { con_notify_netcon_clr->GetColor() };
	case EGlobalContext_t::COMMON_C:
		return { con_notify_common_clr->GetColor() };
	case EGlobalContext_t::WARNING_C:
		return { con_notify_warning_clr->GetColor() };
	case EGlobalContext_t::ERROR_C:
		return { con_notify_error_clr->GetColor() };
	default:
		return { con_notify_native_engine_clr->GetColor() };
	}
}

///////////////////////////////////////////////////////////////////////////////
CTextOverlay* g_pOverlay = new CTextOverlay();
