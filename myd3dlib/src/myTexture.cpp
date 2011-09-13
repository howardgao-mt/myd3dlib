
#include "stdafx.h"
#include "myTexture.h"
#include "myException.h"

namespace my
{
	Texture::Texture(
		LPDIRECT3DDEVICE9 pDevice,
		UINT Width,
		UINT Height,
		UINT MipLevels /*= 0*/,
		DWORD Usage /*= 0*/,
		D3DFORMAT Format /*= D3DFMT_UNKNOWN*/,
		D3DPOOL Pool /*= D3DPOOL_MANAGED*/)
	{
		FAILED_THROW_D3DEXCEPTION(pDevice->CreateTexture(Width, Height, MipLevels, Usage, Format, Pool, &m_ptr, NULL));
	}

	TexturePtr Texture::CreateAdjustedTexture(
		LPDIRECT3DDEVICE9 pDevice,
		UINT Width,
		UINT Height,
		UINT MipLevels /*= D3DX_DEFAULT*/,
		DWORD Usage /*= 0*/,
		D3DFORMAT Format /*= D3DFMT_UNKNOWN*/,
		D3DPOOL Pool /*= D3DPOOL_MANAGED*/)
	{
		LPDIRECT3DTEXTURE9 pTexture = NULL;
		HRESULT hres = D3DXCreateTexture(
			pDevice, Width, Height, MipLevels, Usage, Format, Pool, &pTexture);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}

		return TexturePtr(new Texture(pTexture));
	}

	TexturePtr Texture::CreateTextureFromFile(
		LPDIRECT3DDEVICE9 pDevice,
		LPCTSTR pSrcFile,
		UINT Width /*= D3DX_DEFAULT*/,
		UINT Height /*= D3DX_DEFAULT*/,
		UINT MipLevels /*= D3DX_DEFAULT*/,
		DWORD Usage /*= 0*/,
		D3DFORMAT Format /*= D3DFMT_UNKNOWN*/,
		D3DPOOL Pool /*= D3DPOOL_MANAGED*/,
		DWORD Filter /*= D3DX_DEFAULT*/,
		DWORD MipFilter /*= D3DX_DEFAULT*/,
		D3DCOLOR ColorKey /*= 0*/,
		D3DXIMAGE_INFO * pSrcInfo /*= NULL*/,
		PALETTEENTRY * pPalette /*= NULL*/)
	{
		LPDIRECT3DTEXTURE9 pTexture = NULL;
		HRESULT hres = D3DXCreateTextureFromFileEx(
			pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pTexture);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}

		return TexturePtr(new Texture(pTexture));
	}

	TexturePtr Texture::CreateTextureFromFileInMemory(
		LPDIRECT3DDEVICE9 pDevice,
		my::ArchiveCachePtr cache,
		UINT Width /*= D3DX_DEFAULT*/,
		UINT Height /*= D3DX_DEFAULT*/,
		UINT MipLevels /*= D3DX_DEFAULT*/,
		DWORD Usage /*= 0*/,
		D3DFORMAT Format /*= D3DFMT_UNKNOWN*/,
		D3DPOOL Pool /*= D3DPOOL_MANAGED*/,
		DWORD Filter /*= D3DX_DEFAULT*/,
		DWORD MipFilter /*= D3DX_DEFAULT*/,
		D3DCOLOR ColorKey /*= 0*/,
		D3DXIMAGE_INFO * pSrcInfo /*= NULL*/,
		PALETTEENTRY * pPalette /*= NULL*/)
	{
		LPDIRECT3DTEXTURE9 pTexture = NULL;
		HRESULT hres = D3DXCreateTextureFromFileInMemoryEx(
			pDevice, &(*cache)[0], cache->size(), Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pTexture);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}

		return TexturePtr(new Texture(pTexture));
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
