
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
	struct CharacterMetrics
	{
		RECT textureRect;

		int horiAdvance;

		int horiBearingX;

		int horiBearingY;
	};

	typedef std::pair<int, CharacterMetrics> CharacterInfo;

	typedef std::map<int, CharacterMetrics, std::less<int>, std::allocator<CharacterInfo> > CharacterMap;

	class RectAssignmentNode;

	typedef boost::shared_ptr<RectAssignmentNode> RectAssignmentNodePtr;

	class RectAssignmentNode
	{
	protected:
		bool m_used;

		RECT m_rect;

		RectAssignmentNodePtr m_lchild;

		RectAssignmentNodePtr m_rchild;

		bool AssignTopRect(const SIZE & size, RECT & outRect);

		bool AssignLeftRect(const SIZE & size, RECT & outRect);

	public:
		RectAssignmentNode(const RECT & rect);

		bool AssignRect(const SIZE & size, RECT & outRect);
	};

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

	class Font : public DeviceRelatedObjectBase
	{
	public:
		enum Align
		{
			alignLeftTop,
		};

	protected:
		CComPtr<IDirect3DDevice9> m_d3dDevice;

		FT_Face m_face;

		CachePtr m_cache;

		int m_lineHeight;

		int m_maxAdvance;

		CharacterMap m_characterMap;

		TexturePtr m_texture;

		RectAssignmentNodePtr m_textureRectRoot;

		Font(LPDIRECT3DDEVICE9 pDevice, FT_Face face, int height);

		virtual void OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnD3D9LostDevice(void);

		virtual void OnD3D9DestroyDevice(void);

	public:
		virtual ~Font(void);

		static FontPtr CreateFontFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			const char * filepathname,
			int height,
			FT_Long face_index = 0);

		static FontPtr CreateFontFromFileInMemory(
			LPDIRECT3DDEVICE9 pDevice,
			const void * file_base,
			long file_size,
			int height,
			long face_index = 0);

		void AssignTextureRect(const SIZE & size, RECT & outRect);

		void LoadCharacter(int character);

		CharacterMap::const_iterator GetCharacterInfoIter(int character);

		void DrawString(
			LPD3DXSPRITE pSprite,
			const std::basic_string<wchar_t> & str,
			const Rectangle & rect,
			const Vector4 & color,
			Align align = alignLeftTop);
	};
}
