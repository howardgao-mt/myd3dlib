
#pragma once

#include "myDxutApp.h"
#include "myResource.h"

namespace my
{
	class Texture : public DeviceRelatedComObjectBase<IDirect3DTexture9>
	{
	public:
		Texture(
			UINT Width,
			UINT Height,
			UINT Levels = 0,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED);

		Texture(
			const my::ArchiveCachePtr & cache,
			UINT Width = D3DX_DEFAULT,
			UINT Height = D3DX_DEFAULT,
			UINT MipLevels = D3DX_DEFAULT,
			DWORD Usage = 0,
			D3DFORMAT Format = D3DFMT_UNKNOWN,
			D3DPOOL Pool = D3DPOOL_MANAGED,
			DWORD Filter = D3DPOOL_MANAGED,
			DWORD MipFilter = D3DPOOL_MANAGED,
			D3DCOLOR ColorKey = 0,
			D3DXIMAGE_INFO * pSrcInfo = NULL,
			PALETTEENTRY * pPalette = NULL);

		virtual HRESULT OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnD3D9LostDevice(void);

		virtual void OnD3D9DestroyDevice(void);
	};

	typedef boost::shared_ptr<Texture> TexturePtr;
}
