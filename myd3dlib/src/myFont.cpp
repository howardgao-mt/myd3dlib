
#include "stdafx.h"
#include "myFont.h"
#include "myException.h"
#include FT_SIZES_H

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

namespace my
{
	FontMgr::DrivedClassPtr FontMgr::s_ptr;

	FontMgr::FontMgr(void)
	{
		FT_Error err = FT_Init_FreeType(&m_library);
		if(err)
		{
			THROW_CUSEXCEPTION(_T("FT_Init_FreeType failed"));
		}
	}

	FontMgr::~FontMgr(void)
	{
		FT_Error err = FT_Done_FreeType(m_library);
	}

	Font::Font(LPDIRECT3DDEVICE9 pDevice, FT_Face face)
		: m_d3dDevice(pDevice)
		, m_face(face)
	{
		_ASSERT(m_face);

		m_metrics = m_face->size->metrics;
	}

	Font::~Font(void)
	{
		FT_Done_Face(m_face);
	}

	void Font::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
	}

	void Font::OnD3D9LostDevice(void)
	{
	}

	void Font::OnD3D9DestroyDevice(void)
	{
		m_d3dDevice.Release();
	}

	CharacterMap::const_iterator Font::GetCharacterInfoIter(wchar_t character)
	{
		CharacterMap::const_iterator char_info_iter = m_characterMap.find(character);
		if(m_characterMap.end() == char_info_iter)
		{
			assert(false);
		}
		return char_info_iter;
	}

	FontPtr Font::CreateFontFromFile(
		LPDIRECT3DDEVICE9 pDevice,
		const char * filepathname,
		FT_Long face_index /*= 0*/)
	{
		FT_Face face;
		FT_Error err = FT_New_Face(FontMgr::getSingleton().m_library, filepathname, face_index, &face);
		if(err)
		{
			THROW_CUSEXCEPTION(_T("FT_New_Face failed"));
		}

		FontPtr font(new Font(pDevice, face));
		return font;
	}

	FontPtr Font::CreateFontFromFileInMemory(
		LPDIRECT3DDEVICE9 pDevice,
		const void * file_base,
		long file_size,
		long face_index /*= 0*/)
	{
		CachePtr cache(new Cache(file_size));
		memcpy(&(*cache)[0], file_base, cache->size());

		FT_Face face;
		FT_Error err = FT_New_Memory_Face(FontMgr::getSingleton().m_library, &(*cache)[0], cache->size(), face_index, &face);
		if(err)
		{
			THROW_CUSEXCEPTION(_T("FT_New_Memory_Face failed"));
		}

		FontPtr font(new Font(pDevice, face));
		font->m_cache = cache;
		return font;
	}

	void Font::DrawString(
		const std::basic_string<wchar_t> & str,
		const Rectangle & rect,
		Align align,
		const Vector4 & color)
	{
		assert(false);
	}
}
