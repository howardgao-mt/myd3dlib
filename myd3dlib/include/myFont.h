
#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include "mySingleton.h"
#include "myResource.h"
#include "myTexture.h"
#include <map>
#include "myMath.h"

namespace my
{
	class FontMgr
		: public Singleton<FontMgr>
	{
	public:
		FT_Library m_library;

	public:
		FontMgr(void);

		~FontMgr(void);
	};

	class Font;

	typedef boost::shared_ptr<Font> FontPtr;

	struct CharacterMetrics
	{
		RECT rect;

		unsigned long horiBearingX;

		unsigned long horiBearingY;

		unsigned long horiAdvance;
	};

	typedef std::pair<wchar_t, CharacterMetrics> CharacterInfo;

	typedef std::map<wchar_t, CharacterMetrics, std::less<wchar_t>, std::allocator<CharacterInfo> > CharacterMap;

	class Font : public DeviceRelatedObjectBase
	{
	public:
		enum Align
		{
			AlignLeftTop,
			AlignLeftMiddle,
			AlignLeftBottom,
			AlignCenterTop,
			AlignCenterMiddle,
			AlignCenterBottom,
			AlignRightTop,
			AlignRightMiddle,
			AlignRightBottom,
		};

	protected:
		CComPtr<IDirect3DDevice9> m_d3dDevice;

		FT_Face m_face;

		CachePtr m_cache;

		FT_Size_Metrics m_metrics;

		CharacterMap m_characterMap;

		TexturePtr m_texture;

		Font(LPDIRECT3DDEVICE9 pDevice, FT_Face face);

		virtual void OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnD3D9LostDevice(void);

		virtual void OnD3D9DestroyDevice(void);

		CharacterMap::const_iterator GetCharacterInfoIter(wchar_t character);

	public:
		virtual ~Font(void);

		static FontPtr CreateFontFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			const char * filepathname,
			FT_Long face_index = 0);

		static FontPtr CreateFontFromFileInMemory(
			LPDIRECT3DDEVICE9 pDevice,
			const void * file_base,
			long file_size,
			long face_index = 0);

		void DrawString(
			const std::basic_string<wchar_t> & str,
			const Rectangle & rect,
			Align align,
			const Vector4 & color);
	};
}
