
#pragma once

#include "myDxutApp.h"
#include "myResource.h"
#include <atltypes.h>

namespace my
{
	template <class DrivedClass>
	class TextureBase : public DeviceRelatedObject<DrivedClass>
	{
	public:
		TextureBase(DrivedClass * ptr = NULL)
			: DeviceRelatedObject(ptr)
		{
		}

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

	class Texture : public TextureBase<IDirect3DTexture9>
	{
	protected:
		Texture(IDirect3DTexture9 * pd3dTexture)
			: TextureBase(pd3dTexture)
		{
		}

	public:
		Texture(
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
			my::ArchiveCachePtr cache,
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

		virtual HRESULT OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnD3D9LostDevice(void);

	public:
		void AddDirtyRect(CONST RECT * pDirtyRect = NULL)
		{
			HRESULT hr;
			V(m_ptr->AddDirtyRect(pDirtyRect));
		}

		D3DSURFACE_DESC GetLevelDesc(UINT Level)
		{
			D3DSURFACE_DESC desc;
			HRESULT hr;
			V(m_ptr->GetLevelDesc(Level, &desc));
			return desc;
		}

		//SurfacePtr GetSurfaceLevel(UINT Level)
		//{
		//	IDirect3DSurface9 * pSurface;
		//	HRESULT hr;
		//	V(m_ptr->GetSurfaceLevel(Level, &pSurface));
		//	return SurfacePtr(new Surface(pSurface));
		//}

		D3DLOCKED_RECT LockRect(UINT Level, CONST RECT * pRect = NULL, DWORD Flags = 0)
		{
			D3DLOCKED_RECT LockedRect;
			HRESULT hr;
			V(m_ptr->LockRect(Level, &LockedRect, pRect, Flags));
			return LockedRect;
		}

		void UnlockRect(UINT Level)
		{
			HRESULT hr;
			V(m_ptr->UnlockRect(Level));
		}
	};
}
