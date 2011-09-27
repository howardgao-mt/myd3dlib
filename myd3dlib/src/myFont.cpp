
#include "stdafx.h"
#include "myFont.h"
#include "myException.h"

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

	FontPtr Font::CreateFontFromFile(
		const char * filepathname,
		FT_Long face_index /*= 0*/)
	{
		FT_Face face;
		FT_Error err = FT_New_Face(FontMgr::getSingleton().m_library, filepathname, face_index, &face);
		if(err)
		{
			THROW_CUSEXCEPTION(_T("FT_New_Face failed"));
		}

		return FontPtr(new Font(face));
	}

	FontPtr Font::CreateFontFromFileInMemory(
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

		FontPtr font(new Font(face));
		font->m_cache = cache;
		return font;
	}
}
