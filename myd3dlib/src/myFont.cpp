
#include "stdafx.h"
#include "myFont.h"
#include "myException.h"
#include FT_SIZES_H
#include "libc.h"
#include <atltypes.h>

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

namespace my
{
	void Sprite::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		OnResetDevice();
	}

	void Sprite::OnD3D9LostDevice(void)
	{
		OnLostDevice();
	}

	SpritePtr Sprite::CreateSprite(LPDIRECT3DDEVICE9 pDevice)
	{
		LPD3DXSPRITE pSprite = NULL;
		HRESULT hres = D3DXCreateSprite(pDevice, &pSprite);
		if(FAILED(hres))
		{
			THROW_D3DEXCEPTION(hres);
		}

		return SpritePtr(new Sprite(pSprite));
	}

	RectAssignmentNode::RectAssignmentNode(const RECT & rect)
		: m_used(false)
		, m_rect(rect)
	{
	}

	bool RectAssignmentNode::AssignTopRect(const SIZE & size, RECT & outRect)
	{
		_ASSERT(size.cx <= m_rect.right - m_rect.left);
		_ASSERT(size.cy < m_rect.bottom - m_rect.top);

		RECT rectUp, rectDown;
		int y = m_rect.top + size.cy;
		SetRect(&rectUp, m_rect.left, m_rect.top, m_rect.right, y);
		SetRect(&rectDown, m_rect.left, y, m_rect.right, m_rect.bottom);

		_ASSERT(NULL == m_lchild);
		_ASSERT(NULL == m_rchild);

		m_lchild = RectAssignmentNodePtr(new RectAssignmentNode(rectUp));
		m_rchild = RectAssignmentNodePtr(new RectAssignmentNode(rectDown));
		bool ret = m_lchild->AssignRect(size, outRect);
		_ASSERT(ret);

		return true;
	}

	bool RectAssignmentNode::AssignLeftRect(const SIZE & size, RECT & outRect)
	{
		_ASSERT(size.cx < m_rect.right - m_rect.left);
		_ASSERT(size.cy <= m_rect.bottom - m_rect.top);

		RECT rectLeft, rectRight;
		int x = m_rect.left + size.cx;
		SetRect(&rectLeft, m_rect.left, m_rect.top, x, m_rect.bottom);
		SetRect(&rectRight, x, m_rect.top, m_rect.right, m_rect.bottom);

		_ASSERT(NULL == m_lchild);
		_ASSERT(NULL == m_rchild);

		m_lchild = RectAssignmentNodePtr(new RectAssignmentNode(rectLeft));
		m_rchild = RectAssignmentNodePtr(new RectAssignmentNode(rectRight));
		bool ret = m_lchild->AssignRect(size, outRect);
		_ASSERT(ret);

		return true;
	}

	bool RectAssignmentNode::AssignRect(const SIZE & size, RECT & outRect)
	{
		//_ASSERT(size.cx > 0 && size.cy > 0);

		if(m_used)
		{
			return false;
		}

		if(NULL != m_lchild)
		{
			if(!m_lchild->AssignRect(size, outRect))
			{
				_ASSERT(m_rchild);
				return m_rchild->AssignRect(size, outRect);
			}
			return true;
		}

		int width = m_rect.right - m_rect.left;
		int height = m_rect.bottom - m_rect.top;
		if(width == size.cx)
		{
			if(height == size.cy)
			{
				m_used = true;
				outRect = m_rect;
				return true;
			}
			else if(height > size.cy)
			{
				return AssignTopRect(size, outRect);
			}
		}
		else if(width > size.cx)
		{
			if(height == size.cy)
			{
				return AssignLeftRect(size, outRect);
			}
			else if(height > size.cy)
			{
				if(width > height)
				{
					return AssignLeftRect(size, outRect);
				}
				else
				{
					return AssignTopRect(size, outRect);
				}
			}
		}

		return false;
	}

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

	Font::Font(FT_Face face, int height, LPDIRECT3DDEVICE9 pDevice)
		: m_face(face)
		, m_Device(pDevice)
	{
		_ASSERT(m_face);

		FT_Error err = FT_Set_Pixel_Sizes(m_face, height, height);
		if(err)
		{
			THROW_CUSEXCEPTION(_T("FT_Set_Pixel_Sizes failed"));
		}

		m_lineHeight = m_face->size->metrics.height >> 6;

		m_maxAdvance = m_face->size->metrics.max_advance >> 6;

		m_texture = Texture::CreateTexture(pDevice, m_maxAdvance, m_lineHeight, 0, 0, D3DFMT_A8, D3DPOOL_MANAGED);

		m_textureRectRoot = RectAssignmentNodePtr(new RectAssignmentNode(CRect(0, 0, m_maxAdvance, m_lineHeight)));
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
		m_Device.Release();
	}

	FontPtr Font::CreateFontFromFile(
		LPDIRECT3DDEVICE9 pDevice,
		const char * filepathname,
		int height,
		FT_Long face_index /*= 0*/)
	{
		FT_Face face;
		FT_Error err = FT_New_Face(FontMgr::getSingleton().m_library, filepathname, face_index, &face);
		if(err)
		{
			THROW_CUSEXCEPTION(_T("FT_New_Face failed"));
		}

		FontPtr font(new Font(face, height, pDevice));
		return font;
	}

	FontPtr Font::CreateFontFromFileInMemory(
		LPDIRECT3DDEVICE9 pDevice,
		const void * file_base,
		long file_size,
		int height,
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

		FontPtr font(new Font(face, height, pDevice));
		font->m_cache = cache;
		return font;
	}

	void Font::AssignTextureRect(const SIZE & size, RECT & outRect)
	{
		D3DSURFACE_DESC desc = m_texture->GetLevelDesc(0);

		if(!m_textureRectRoot->AssignRect(size, outRect))
		{
			m_texture = TexturePtr();
			m_texture = Texture::CreateTexture(m_Device, desc.Width * 2, desc.Height * 2, 0, 0, D3DFMT_A8, D3DPOOL_MANAGED);
			m_textureRectRoot = RectAssignmentNodePtr(new RectAssignmentNode(CRect(0, 0, desc.Width * 2, desc.Height * 2)));

			if(!m_textureRectRoot->AssignRect(size, outRect))
			{
				THROW_CUSEXCEPTION(_T("m_textureRectRoot->AssignRect failed"));
			}

			m_characterMap.clear();
		}
	}

	void Font::LoadCharacter(int character)
	{
		_ASSERT(m_characterMap.end() == m_characterMap.find(character));

		FT_UInt glyph_index = FT_Get_Char_Index(m_face, character);

		FT_Error err = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER);
		if(err)
		{
			THROW_CUSEXCEPTION(wstringToTString(str_printf(L"FT_Load_Glyph \"%c\" failed", character)));
		}

		if(FT_PIXEL_MODE_GRAY != m_face->glyph->bitmap.pixel_mode)
		{
			THROW_CUSEXCEPTION(_T("FT_PIXEL_MODE_GRAY != ft_face->glyph->bitmap.pixel_mode"));
		}

		CharacterInfo info;
		info.first = character;
		CSize textureSize(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows);
		AssignTextureRect(textureSize, info.second.textureRect);
		info.second.horiAdvance = m_face->glyph->metrics.horiAdvance >> 6;
		info.second.horiBearingX = m_face->glyph->metrics.horiBearingX >> 6;
		info.second.horiBearingY = m_face->glyph->metrics.horiBearingY >> 6;

		D3DLOCKED_RECT lr = m_texture->LockRect(0, &info.second.textureRect, 0);
		for(int y = 0; y < textureSize.cy; y++)
		{
			void * src = m_face->glyph->bitmap.buffer + y * m_face->glyph->bitmap.pitch;
			void * dst = (unsigned char *)lr.pBits + y * lr.Pitch;
			memcpy(dst, src, textureSize.cx * sizeof(unsigned char));
		}
		m_texture->UnlockRect(0);

		m_characterMap.insert(info);
	}

	CharacterMap::const_iterator Font::GetCharacterInfoIter(int character)
	{
		CharacterMap::const_iterator char_info_iter = m_characterMap.find(character);
		if(m_characterMap.end() == char_info_iter)
		{
			LoadCharacter(character);
			char_info_iter = m_characterMap.find(character);
		}

		_ASSERT(m_characterMap.end() != char_info_iter);
		return char_info_iter;
	}

	void Font::DrawString(
		SpritePtr sprite,
		const std::basic_string<wchar_t> & str,
		const Rectangle & rect,
		Align align /*= alignLeftTop*/,
		D3DCOLOR Color /*= D3DCOLOR_ARGB(255, 255, 255, 255)*/)
	{
		Vector3 pen(rect.l, rect.t + m_lineHeight, 0);
		for(size_t i = 0; i < str.length(); i++)
		{
			wchar_t c = str[i];
			switch(c)
			{
			case L'\r':
				break;

			case L'\n':
				pen.x = rect.l;
				pen.y += m_lineHeight;
				break;

			default:
				{
					CharacterMap::const_iterator c_iter = GetCharacterInfoIter(c);
					sprite->Draw(
						m_texture, c_iter->second.textureRect, Vector3(0, 0, 0), Vector3(pen.x + c_iter->second.horiBearingX, pen.y - c_iter->second.horiBearingY, 0), Color);
					pen.x += c_iter->second.horiAdvance;
				}
				break;
			}
		}
	}
}