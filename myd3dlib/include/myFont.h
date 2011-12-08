
#pragma once

namespace my
{
	class Sprite;

	typedef boost::shared_ptr<Sprite> SpritePtr;

	class Sprite : public DeviceRelatedObject<ID3DXSprite>
	{
	protected:
		Sprite(ID3DXSprite * ptr)
			: DeviceRelatedObject(ptr)
		{
		}

		virtual void OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnD3D9LostDevice(void);

	public:
		static SpritePtr CreateSprite(LPDIRECT3DDEVICE9 pDevice);

		void Begin(DWORD Flags)
		{
			V(m_ptr->Begin(Flags));
		}

		void Draw(TexturePtr texture, const RECT & SrcRect, const Vector3 & Center, const Vector3 & Position, D3DCOLOR Color)
		{
			V(m_ptr->Draw(static_cast<IDirect3DTexture9 *>(texture->m_ptr), &SrcRect, (D3DXVECTOR3 *)&Center, (D3DXVECTOR3 *)&Position, Color));
		}

		void End(void)
		{
			V(m_ptr->End());
		}

		CComPtr<IDirect3DDevice9> GetDevice(void)
		{
			CComPtr<IDirect3DDevice9> ret;
			V(m_ptr->GetDevice(&ret));
			return ret;
		}

		Matrix4 GetTransform(void)
		{
			Matrix4 ret;
			V(m_ptr->GetTransform((D3DXMATRIX *)&ret));
			return ret;
		}

		void OnLostDevice(void)
		{
			V(m_ptr->OnLostDevice());
		}

		void OnResetDevice(void)
		{
			V(m_ptr->OnResetDevice());
		}

		void SetTransform(const Matrix4 & Transform)
		{
			V(m_ptr->SetTransform((D3DXMATRIX *)&Transform));
		}

		void SetWorldViewLH(const Matrix4 & World, const Matrix4 & View)
		{
			V(m_ptr->SetWorldViewLH((D3DXMATRIX *)&World, (D3DXMATRIX *)&View));
		}

		void SetWorldViewRH(const Matrix4 & World, const Matrix4 & View)
		{
			V(m_ptr->SetWorldViewRH((D3DXMATRIX *)&World, (D3DXMATRIX *)&View));
		}
	};

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
		RectAssignmentNode(const RECT & rect)
			: m_used(false)
			, m_rect(rect)
		{
		}

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

		struct CharacterInfo
		{
			RECT textureRect;

			int horiAdvance;

			int horiBearingX;

			int horiBearingY;
		};

		typedef std::map<int, CharacterInfo> CharacterMap;

	protected:
		FT_Face m_face;

		CComPtr<IDirect3DDevice9> m_Device;

		CachePtr m_cache;

		int m_lineHeight;

		int m_maxAdvance;

		CharacterMap m_characterMap;

		TexturePtr m_texture;

		RectAssignmentNodePtr m_textureRectRoot;

		Font(FT_Face face, int height, LPDIRECT3DDEVICE9 pDevice);

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

		static TexturePtr CreateFontTexture(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height);

		void AssignTextureRect(const SIZE & size, RECT & outRect);

		void InsertCharacter(
			int character,
			int horiAdvance,
			int horiBearingX,
			int horiBearingY,
			const unsigned char * bmpBuffer,
			int bmpWidth,
			int bmpHeight,
			int bmpPitch);

		void LoadCharacter(int character);

		const CharacterInfo & GetCharacterInfo(int character);

		void DrawString(
			SpritePtr sprite,
			const std::basic_string<wchar_t> & str,
			const Rectangle & rect,
			Align align = alignLeftTop,
			D3DCOLOR Color = D3DCOLOR_ARGB(255, 255, 255, 255));
	};
}
