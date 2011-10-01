
#pragma once

#include "myDxutApp.h"
#include "myResource.h"
#include <atlbase.h>

namespace my
{
	class BaseTexture;

	typedef boost::shared_ptr<BaseTexture> BaseTexturePtr;

	class BaseTexture : public DeviceRelatedObject<IDirect3DBaseTexture9>
	{
	public:
		BaseTexture(IDirect3DBaseTexture9 * ptr)
			: DeviceRelatedObject(ptr)
		{
		}

	public:
		void GenerateMipSubLevels(void)
		{
			m_ptr->GenerateMipSubLevels();
		}

		D3DTEXTUREFILTERTYPE GetAutoGenFilterType(void)
		{
			return m_ptr->GetAutoGenFilterType();
		}

		DWORD GetLevelCount(void)
		{
			return m_ptr->GetLevelCount();
		}

		DWORD GetLOD(void)
		{
			return m_ptr->GetLOD();
		}

		void SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType)
		{
			V(m_ptr->SetAutoGenFilterType(FilterType));
		}

		DWORD SetLOD(DWORD LODNew)
		{
			return m_ptr->SetLOD(LODNew);
		}
	};

	class Texture;

	typedef boost::shared_ptr<Texture> TexturePtr;

	class Texture : public BaseTexture
	{
	protected:
		Texture(IDirect3DTexture9 * pd3dTexture)
			: BaseTexture(pd3dTexture)
		{
		}

	public:
		static TexturePtr CreateTexture(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Width,
			UINT Height,
			UINT MipLevels = 0,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		static TexturePtr CreateAdjustedTexture(
			LPDIRECT3DDEVICE9 pDevice,
			UINT Width,
			UINT Height,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		static TexturePtr CreateTextureFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCTSTR pSrcFile,
			UINT Width = D3DX_DEFAULT,
			UINT Height = D3DX_DEFAULT,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED,
			DWORD Filter = D3DX_DEFAULT,
			DWORD MipFilter = D3DX_DEFAULT,
			D3DCOLOR ColorKey = 0,
			D3DXIMAGE_INFO * pSrcInfo = NULL,
			PALETTEENTRY * pPalette = NULL);

		static TexturePtr CreateTextureFromFileInMemory(
			LPDIRECT3DDEVICE9 pDevice,
			LPCVOID pSrcData,
			UINT SrcDataSize,
			UINT Width = D3DX_DEFAULT,
			UINT Height = D3DX_DEFAULT,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED,
			DWORD Filter = D3DX_DEFAULT,
			DWORD MipFilter = D3DX_DEFAULT,
			D3DCOLOR ColorKey = 0,
			D3DXIMAGE_INFO * pSrcInfo = NULL,
			PALETTEENTRY * pPalette = NULL);

	public:
		void AddDirtyRect(CONST RECT * pDirtyRect = NULL)
		{
			V(static_cast<IDirect3DTexture9 *>(m_ptr)->AddDirtyRect(pDirtyRect));
		}

		D3DSURFACE_DESC GetLevelDesc(UINT Level = 0)
		{
			D3DSURFACE_DESC desc;
			V(static_cast<IDirect3DTexture9 *>(m_ptr)->GetLevelDesc(Level, &desc));
			return desc;
		}

		CComPtr<IDirect3DSurface9> GetSurfaceLevel(UINT Level)
		{
			CComPtr<IDirect3DSurface9> Surface;
			V(static_cast<IDirect3DTexture9 *>(m_ptr)->GetSurfaceLevel(Level, &Surface));
			return Surface;
		}

		D3DLOCKED_RECT LockRect(UINT Level, CONST RECT * pRect = NULL, DWORD Flags = 0)
		{
			D3DLOCKED_RECT LockedRect;
			V(static_cast<IDirect3DTexture9 *>(m_ptr)->LockRect(Level, &LockedRect, pRect, Flags));
			return LockedRect;
		}

		void UnlockRect(UINT Level)
		{
			V(static_cast<IDirect3DTexture9 *>(m_ptr)->UnlockRect(Level));
		}
	};
}
