
#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include "mySingleton.h"
#include "myResource.h"

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

	class Font
	{
	protected:
		FT_Face m_face;

		CachePtr m_cache;

		Font(FT_Face face)
			: m_face(face)
		{
			_ASSERT(m_face);
		}

	public:
		virtual ~Font(void)
		{
			FT_Done_Face(m_face);
		}

		static FontPtr CreateFontFromFile(
			const char * filepathname,
			FT_Long face_index = 0);

		static FontPtr CreateFontFromFileInMemory(
			const void * file_base,
			long file_size,
			long face_index = 0);
	};
}
