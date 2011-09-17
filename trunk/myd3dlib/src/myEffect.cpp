
#include "stdafx.h"
#include "myEffect.h"
#include <atlbase.h>
#include "libc.h"

namespace my
{
	void Effect::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		OnResetDevice();
	}

	void Effect::OnD3D9LostDevice(void)
	{
		OnLostDevice();
	}

	EffectPtr Effect::CreateEffect(
		LPDIRECT3DDEVICE9 pDevice,
		my::ArchiveCachePtr cache,
		CONST D3DXMACRO * pDefines /*= NULL*/,
		LPD3DXINCLUDE pInclude /*= NULL*/,
		DWORD Flags /*= 0*/,
		LPD3DXEFFECTPOOL pPool /*= NULL*/)
	{
		LPD3DXEFFECT pEffect = NULL;
		CComPtr<ID3DXBuffer> CompilationErrors;
		HRESULT hres = D3DXCreateEffect(
			pDevice, &(*cache)[0], cache->size(), pDefines, pInclude, Flags, pPool, &pEffect, &CompilationErrors);
		if(FAILED(hres))
		{
			std::basic_string<char> info((char *)CompilationErrors->GetBufferPointer(), CompilationErrors->GetBufferSize());
			THROW_CUSEXCEPTION(mstringToTString(info));
		}

		return EffectPtr(new Effect(pEffect));
	}

	EffectPtr Effect::CreateEffectFromFile(
		LPDIRECT3DDEVICE9 pDevice,
		LPCTSTR pSrcFile,
		CONST D3DXMACRO * pDefines /*= NULL*/,
		LPD3DXINCLUDE pInclude /*= NULL*/,
		DWORD Flags /*= 0*/,
		LPD3DXEFFECTPOOL pPool /*= NULL*/)
	{
		LPD3DXEFFECT pEffect = NULL;
		CComPtr<ID3DXBuffer> CompilationErrors;
		HRESULT hres = D3DXCreateEffectFromFile(
			pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, &pEffect, &CompilationErrors);
		if(FAILED(hres))
		{
			std::basic_string<char> info((char *)CompilationErrors->GetBufferPointer(), CompilationErrors->GetBufferSize());
			THROW_CUSEXCEPTION(mstringToTString(info));
		}

		return EffectPtr(new Effect(pEffect));
	}
}
