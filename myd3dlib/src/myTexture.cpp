
#include "stdafx.h"
#include "myTexture.h"
#include "myException.h"

namespace my
{
	Texture::Texture(
		UINT Width,
		UINT Height,
		UINT Levels /*= 0*/,
		DWORD Usage /*= 0*/,
		D3DFORMAT Format /*= D3DFMT_UNKNOWN*/,
		D3DPOOL Pool /*= D3DPOOL_MANAGED*/)
	{
		IDirect3DDevice9 * d3d9Device = DXUTGetD3D9Device();
		_ASSERT(d3d9Device);
		FAILED_THROW_D3DEXCEPTION(d3d9Device->CreateTexture(Width, Height, Levels, Usage, Format, Pool, &m_ptr, NULL));
	}

	Texture::Texture(
		const my::ArchiveCachePtr & cache,
		UINT Width /*= D3DX_DEFAULT*/,
		UINT Height /*= D3DX_DEFAULT*/,
		UINT MipLevels /*= D3DX_DEFAULT*/,
		DWORD Usage /*= 0*/,
		D3DFORMAT Format /*= D3DFMT_UNKNOWN*/,
		D3DPOOL Pool /*= D3DPOOL_MANAGED*/,
		DWORD Filter /*= D3DPOOL_MANAGED*/,
		DWORD MipFilter /*= D3DPOOL_MANAGED*/,
		D3DCOLOR ColorKey /*= 0*/,
		D3DXIMAGE_INFO * pSrcInfo /*= NULL*/,
		PALETTEENTRY * pPalette /*= NULL*/)
	{
		IDirect3DDevice9 * d3d9Device = DXUTGetD3D9Device();
		_ASSERT(d3d9Device);
		FAILED_THROW_D3DEXCEPTION(D3DXCreateTextureFromFileInMemoryEx(
			d3d9Device, &(*cache)[0], cache->size(), Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &m_ptr));
	}

	HRESULT Texture::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		return S_OK;
	}

	void Texture::OnD3D9LostDevice(void)
	{
	}

	void Texture::OnD3D9DestroyDevice(void)
	{
		SAFE_RELEASE(m_ptr);
	}
}
