#include "stdafx.h"
#include "myFont.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "myUI.h"
#include "myResource.h"
#include "libc.h"

using namespace my;

void Sprite::Create(ID3DXSprite * ptr)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;
}

void Sprite::CreateSprite(LPDIRECT3DDEVICE9 pDevice)
{
	LPD3DXSPRITE pSprite = NULL;
	HRESULT hres = D3DXCreateSprite(pDevice, &pSprite);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pSprite);
}

void Sprite::OnResetDevice(void)
{
	V(m_ptr->OnResetDevice());
}

void Sprite::OnLostDevice(void)
{
	V(m_ptr->OnLostDevice());
}

void Sprite::Begin(DWORD Flags)
{
	V(m_ptr->Begin(Flags));
}

void Sprite::Draw(Texture2DPtr texture, const CRect & SrcRect, const Vector3 & Center, const Vector3 & Position, D3DCOLOR Color)
{
	V(m_ptr->Draw(static_cast<IDirect3DTexture9 *>(texture->m_ptr), &SrcRect, (D3DXVECTOR3 *)&Center, (D3DXVECTOR3 *)&Position, Color));
}

void Sprite::End(void)
{
	V(m_ptr->End());
}

CComPtr<IDirect3DDevice9> Sprite::GetDevice(void)
{
	CComPtr<IDirect3DDevice9> ret;
	V(m_ptr->GetDevice(&ret));
	return ret;
}

Matrix4 Sprite::GetTransform(void)
{
	Matrix4 ret;
	V(m_ptr->GetTransform((D3DXMATRIX *)&ret));
	return ret;
}

void Sprite::SetTransform(const Matrix4 & Transform)
{
	V(m_ptr->SetTransform((D3DXMATRIX *)&Transform));
}

void Sprite::SetWorldViewLH(const Matrix4 & World, const Matrix4 & View)
{
	V(m_ptr->SetWorldViewLH((D3DXMATRIX *)&World, (D3DXMATRIX *)&View));
}

void Sprite::SetWorldViewRH(const Matrix4 & World, const Matrix4 & View)
{
	V(m_ptr->SetWorldViewRH((D3DXMATRIX *)&World, (D3DXMATRIX *)&View));
}

bool RectAssignmentNode::AssignTopRect(const CSize & size, CRect & outRect)
{
	_ASSERT(size.cx <= m_rect.Width());
	_ASSERT(size.cy < m_rect.Height());

	int y = m_rect.top + size.cy;
	CRect rectUp(m_rect.left, m_rect.top, m_rect.right, y);
	CRect rectDown(m_rect.left, y, m_rect.right, m_rect.bottom);

	_ASSERT(NULL == m_lchild);
	_ASSERT(NULL == m_rchild);

	m_lchild.reset(new RectAssignmentNode(rectUp));
	m_rchild.reset(new RectAssignmentNode(rectDown));
	bool ret = m_lchild->AssignRect(size, outRect);
	_ASSERT(ret);

	return true;
}

bool RectAssignmentNode::AssignLeftRect(const CSize & size, CRect & outRect)
{
	_ASSERT(size.cx < m_rect.Width());
	_ASSERT(size.cy <= m_rect.Height());

	int x = m_rect.left + size.cx;
	CRect rectLeft(m_rect.left, m_rect.top, x, m_rect.bottom);
	CRect rectRight(x, m_rect.top, m_rect.right, m_rect.bottom);

	_ASSERT(NULL == m_lchild);
	_ASSERT(NULL == m_rchild);

	m_lchild.reset(new RectAssignmentNode(rectLeft));
	m_rchild.reset(new RectAssignmentNode(rectRight));
	bool ret = m_lchild->AssignRect(size, outRect);
	_ASSERT(ret);

	return true;
}

bool RectAssignmentNode::AssignRect(const CSize & size, CRect & outRect)
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

	int width = m_rect.Width();
	int height = m_rect.Height();
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

Font::FontLibrary::FontLibrary(void)
{
	FT_Error err = FT_Init_FreeType(&m_Library);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_Init_FreeType failed");
	}
}

Font::FontLibrary::~FontLibrary(void)
{
	FT_Done_FreeType(m_Library);
}

Font::Font(int font_pixel_gap)
	: m_face(NULL)
	, FONT_PIXEL_GAP(font_pixel_gap)
	, m_Scale(0,0)
{
}

Font::~Font(void)
{
	if(m_face)
	{
		FT_Done_Face(m_face);
		m_face = NULL;
	}
}

void Font::SetScale(const Vector2 & Scale)
{
	FT_Size_RequestRec  req;
	req.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
	req.width = (FT_Long)(m_Height * Scale.x * 64);
	req.height = (FT_Long)(m_Height * Scale.y * 64);
	req.horiResolution = 0;
	req.vertResolution = 0;
	FT_Error err = FT_Request_Size(m_face, &req);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_Request_Size failed");
	}

	m_Scale = Scale;

	m_textureRectRoot.reset(new RectAssignmentNode(CRect(0, 0, m_textureDesc.Width, m_textureDesc.Height)));

	m_characterMap.clear();
}

void Font::Create(FT_Face face, int height, LPDIRECT3DDEVICE9 pDevice)
{
	_ASSERT(!m_face && !m_Device);

	m_face = face;

	m_Device = pDevice;

	m_Height = height;

	CreateFontTexture(512, 512);

	SetScale(Vector2(1,1));

	m_LineHeight = m_face->size->metrics.height / 64;
}

void Font::CreateFontFromFile(
	LPDIRECT3DDEVICE9 pDevice,
	LPCSTR pFilename,
	int height,
	long face_index)
{
	FT_Face face;
	FT_Error err = FT_New_Face(FontLibrary::getSingleton().m_Library, pFilename, face_index, &face);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_New_Face failed");
	}

	Create(face, height, pDevice);
}

void Font::CreateFontFromFileInMemory(
	LPDIRECT3DDEVICE9 pDevice,
	const void * file_base,
	long file_size,
	int height,
	long face_index)
{
	CachePtr cache(new Cache(file_size));
	memcpy(&(*cache)[0], file_base, cache->size());

	CreateFontFromFileInCache(pDevice, cache, height, face_index);
}

void Font::CreateFontFromFileInCache(
	LPDIRECT3DDEVICE9 pDevice,
	CachePtr cache_ptr,
	int height,
	long face_index)
{
	FT_Face face;
	FT_Error err = FT_New_Memory_Face(FontLibrary::getSingleton().m_Library, &(*cache_ptr)[0], cache_ptr->size(), face_index, &face);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_New_Memory_Face failed");
	}

	Create(face, height, pDevice);

	m_cache = cache_ptr;
}

void Font::OnResetDevice(void)
{
	m_Texture->OnResetDevice();
}

void Font::OnLostDevice(void)
{
	m_Texture->OnLostDevice();
}

void Font::OnDestroyDevice(void)
{
	m_Texture->OnDestroyDevice();

	m_Device.Release();

	if(m_face)
	{
		FT_Done_Face(m_face);
		m_face = NULL;
	}
}

void Font::CreateFontTexture(UINT Width, UINT Height)
{
	m_Texture.reset(new Texture2D());

	m_Texture->CreateTexture(m_Device, Width, Height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED);

	m_textureDesc = m_Texture->GetLevelDesc();
}

void Font::AssignTextureRect(const CSize & size, CRect & outRect)
{
	if(!m_textureRectRoot->AssignRect(size, outRect))
	{
		_ASSERT(m_textureDesc.Width > 0 && m_textureDesc.Height > 0);

		CreateFontTexture(m_textureDesc.Width * 2, m_textureDesc.Height * 2);

		m_textureRectRoot.reset(new RectAssignmentNode(CRect(0, 0, m_textureDesc.Width, m_textureDesc.Height)));

		if(!m_textureRectRoot->AssignRect(size, outRect))
		{
			THROW_CUSEXCEPTION("m_textureRectRoot->AssignRect failed");
		}

		m_characterMap.clear();
	}
}

void Font::InsertCharacter(
	int character,
	float width,
	float height,
	float horiBearingX,
	float horiBearingY,
	float horiAdvance,
	const unsigned char * bmpBuffer,
	int bmpWidth,
	int bmpHeight,
	int bmpPitch)
{
	_ASSERT(m_characterMap.end() == m_characterMap.find(character));

	CharacterInfo info;
	info.width = width;
	info.height = height;
	info.horiBearingX = horiBearingX;
	info.horiBearingY = horiBearingY;
	info.horiAdvance = horiAdvance;

	// Add pixel gap around each font cell to avoid uv boundaries issue when Antialiasing
	AssignTextureRect(CSize(bmpWidth + FONT_PIXEL_GAP * 2, bmpHeight + FONT_PIXEL_GAP * 2), info.textureRect);

	info.textureRect.InflateRect(-FONT_PIXEL_GAP, -FONT_PIXEL_GAP);

	if(!info.textureRect.IsRectEmpty())
	{
		D3DLOCKED_RECT lr = m_Texture->LockRect(info.textureRect);
		for(int y = 0; y < bmpHeight; y++)
		{
			for(int x = 0; x < bmpWidth; x++)
			{
				*((DWORD *)((unsigned char *)lr.pBits + y * lr.Pitch) + x) = D3DCOLOR_ARGB(*(bmpBuffer + y * bmpPitch + x),255,255,255);
			}
		}
		m_Texture->UnlockRect();
	}

	m_characterMap.insert(std::make_pair(character, info));
}

void Font::LoadCharacter(int character)
{
	_ASSERT(m_characterMap.end() == m_characterMap.find(character));

	FT_UInt glyph_index = FT_Get_Char_Index(m_face, character);

	FT_Error err = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER);
	if(err)
	{
		THROW_CUSEXCEPTION(str_printf("FT_Load_Glyph \"%c\" failed", character));
	}

	if(FT_PIXEL_MODE_GRAY != m_face->glyph->bitmap.pixel_mode)
	{
		THROW_CUSEXCEPTION("FT_PIXEL_MODE_GRAY != ft_face->glyph->bitmap.pixel_mode");
	}

	InsertCharacter(
		character,
		m_face->glyph->metrics.width / 64.0f / m_Scale.x,
		m_face->glyph->metrics.height / 64.0f / m_Scale.y,
		m_face->glyph->metrics.horiBearingX / 64.0f / m_Scale.x,
		m_face->glyph->metrics.horiBearingY / 64.0f / m_Scale.y,
		m_face->glyph->metrics.horiAdvance / 64.0f / m_Scale.x,
		m_face->glyph->bitmap.buffer,
		m_face->glyph->bitmap.width,
		m_face->glyph->bitmap.rows,
		m_face->glyph->bitmap.pitch);
}

const Font::CharacterInfo & Font::GetCharacterInfo(int character)
{
	CharacterMap::const_iterator char_info_iter = m_characterMap.find(character);
	if(m_characterMap.end() == char_info_iter)
	{
		LoadCharacter(character);
		char_info_iter = m_characterMap.find(character);
	}

	_ASSERT(m_characterMap.end() != char_info_iter);
	return (*char_info_iter).second;
}

Vector2 Font::CalculateStringExtent(LPCWSTR pString)
{
	Vector2 extent(0, (float)m_LineHeight);
	wchar_t c;
	while((c = *pString++))
	{
		const CharacterInfo & info = GetCharacterInfo(c);
		extent.x += info.horiAdvance;
	}

	return extent;
}

Vector2 Font::CalculateAlignedPen(LPCWSTR pString, const my::Rectangle & rect, Align align)
{
	Vector2 pen;
	if(align & AlignLeft)
	{
		pen.x = rect.l;
	}
	else if(align & AlignCenter)
	{
		Vector2 extent = CalculateStringExtent(pString);
		pen.x = rect.l + (rect.r - rect.l - extent.x) * 0.5f;
	}
	else
	{
		Vector2 extent = CalculateStringExtent(pString);
		pen.x = rect.r - extent.x;
	}

	if(align & AlignTop)
	{
		pen.y = rect.t;
	}
	else if(align & AlignMiddle)
	{
		pen.y = rect.t + (rect.b - rect.t - m_LineHeight) * 0.5f;
	}
	else
	{
		pen.y = rect.b - m_LineHeight;
	}
	pen.y += m_LineHeight;

	return pen;
}

void Font::PushStringVertices(
	UIRender * ui_render,
	LPCWSTR pString,
	const my::Rectangle & rect,
	D3DCOLOR Color,
	Align align)
{
	Vector2 pen = CalculateAlignedPen(pString, rect, align);

	wchar_t c;
	while(c = *pString++)
	{
		const CharacterInfo & info = GetCharacterInfo(c);

		Rectangle uv_rect(
			(float)info.textureRect.left / m_textureDesc.Width,
			(float)info.textureRect.top / m_textureDesc.Height,
			(float)info.textureRect.right / m_textureDesc.Width,
			(float)info.textureRect.bottom / m_textureDesc.Height);

		// ! frequently calling UIRender::PushVertex may obviously lose performance
		ui_render->PushRectangle(
			Rectangle::LeftTop(pen.x + info.horiBearingX, pen.y - info.horiBearingY, info.width, info.height), uv_rect, Color);

		pen.x += info.horiAdvance;
	}
}

void Font::DrawString(
	UIRender * ui_render,
	LPCWSTR pString,
	const my::Rectangle & rect,
	D3DCOLOR Color,
	Align align)
{
	ui_render->ClearVertexList();
	PushStringVertices(ui_render, pString, rect, Color, align);
	ui_render->SetTexture(m_Texture);
	ui_render->DrawVertexList();
}

void Font::DrawString(
	LPD3DXSPRITE pSprite,
	LPCWSTR pString,
	const my::Rectangle & rect,
	D3DCOLOR Color,
	Align align)
{
	Vector2 pen = CalculateAlignedPen(pString, rect, align);

	wchar_t c;
	while(c = *pString++)
	{
		const CharacterInfo & info = GetCharacterInfo(c);

		V(m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_ALPHAREPLICATE));

		V(m_Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
		V(m_Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
		V(m_Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

		V(pSprite->Draw(
			(IDirect3DTexture9 *)m_Texture->m_ptr,
			&info.textureRect,
			(D3DXVECTOR3 *)&Vector3(0, 0, 0),
			(D3DXVECTOR3 *)&Vector3(pen.x + info.horiBearingX, pen.y - info.horiBearingY, 0),
			Color));

		pen.x += info.horiAdvance;
	}
}

float Font::CPtoX(LPCWSTR pString, int nCP)
{
	float x = 0;
	int i = 0;
	wchar_t c;
	for(; (c = *pString++) && i < nCP; i++)
	{
		const CharacterInfo & info = GetCharacterInfo(c);
		x += info.horiAdvance;
	}
	return x;
}

int Font::XtoCP(LPCWSTR pString, float _x)
{
	float x = 0;
	int i = 0;
	wchar_t c;
	for(; (c = *pString++); i++)
	{
		const CharacterInfo & info = GetCharacterInfo(c);
		if(x + info.horiAdvance > _x)
		{
			break;
		}
		x += info.horiAdvance;
	}
	return i;
}
