
#include "stdafx.h"
#include "myTexture.h"
#include "myException.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

namespace my
{
	TexturePtr Texture::CreateTexture(
		LPDIRECT3DDEVICE9 pDevice,
		UINT Width,
		UINT Height,
		UINT MipLevels /*= 0*/,
		DWORD Usage /*= 0*/,
		D3DFORMAT Format /*= D3DFMT_UNKNOWN*/,
		D3DPOOL Pool /*= D3DPOOL_MANAGED*/)
	{
		LPDIRECT3DTEXTURE9 pTexture = NULL;
		HRESULT hres = pDevice->CreateTexture(
			Width, Height, MipLevels, Usage, Format, Pool, &pTexture, NULL);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}

		return TexturePtr(new Texture(pTexture));
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
		LPCVOID pSrcData,
		UINT SrcDataSize,
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
			pDevice, pSrcData, SrcDataSize, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &pTexture);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}

		return TexturePtr(new Texture(pTexture));
	}
}
