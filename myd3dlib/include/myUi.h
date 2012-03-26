
#pragma once

#include <boost/weak_ptr.hpp>
#include "FastDelegate.h"

namespace my
{
	class UIRender
	{
	public:
		typedef Font::CUSTOMVERTEX CUSTOMVERTEX;

		static const DWORD D3DFVF_CUSTOMVERTEX = Font::D3DFVF_CUSTOMVERTEX;

		static void BuildOrthoMatrices(DWORD Width, DWORD Height, Matrix4 & outView, Matrix4 & outProj);

		static void BuildPerspectiveMatrices(float fovy, DWORD Width, DWORD Height, Matrix4 & outView, Matrix4 & outProj);

		// Rendering UI under Fixed Pipeline is not recommended
		static void Begin(IDirect3DDevice9 * pd3dDevice);

		static void End(IDirect3DDevice9 * pd3dDevice);

		static Rectangle CalculateUVRect(const SIZE & textureSize, const RECT & textureRect);

		static size_t BuildRectangleVertices(
			CUSTOMVERTEX * pBuffer,
			size_t bufferSize,
			const Rectangle & rect,
			DWORD color,
			const Rectangle & uvRect);

		// Example of Draw BuildRectangleVertices
		static void DrawRectangle(
			IDirect3DDevice9 * pd3dDevice,
			const Rectangle & rect,
			DWORD color,
			TexturePtr texture = TexturePtr(),
			const RECT & SrcRect = CRect(0,0,0,0));
	};

	class ControlSkin
	{
	public:
		TexturePtr m_Texture;

		RECT m_TextureRect;

		FontPtr m_Font;

		D3DCOLOR m_TextColor;

		Font::Align m_TextAlign;

	public:
		ControlSkin(void)
			: m_TextureRect(CRect(0,0,0,0))
			, m_TextColor(D3DCOLOR_ARGB(255,255,255,0))
			, m_TextAlign(my::Font::AlignLeftTop)
		{
		}

		virtual ~ControlSkin(void)
		{
		}
	};

	typedef boost::shared_ptr<ControlSkin> ControlSkinPtr;

	class Control
	{
	public:
		bool m_bEnabled;

		bool m_bVisible;

		bool m_bMouseOver;

		bool m_bHasFocus;

		bool m_bIsDefault;

		UINT m_nHotkey;

		Vector2 m_Location;

		Vector2 m_Size;

		D3DCOLOR m_Color;

		ControlSkinPtr m_Skin;

		HRESULT hr;

		boost::weak_ptr<Control> this_ptr;

	public:
		Control(void)
			: m_bEnabled(true)
			, m_bVisible(true)
			, m_bMouseOver(false)
			, m_bHasFocus(false)
			, m_bIsDefault(false)
			, m_nHotkey(0)
			, m_Location(100, 100)
			, m_Size(100, 100)
			, m_Color(D3DCOLOR_ARGB(255,255,255,255))
		{
		}

		virtual ~Control(void)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnFocusIn(void);

		virtual void OnFocusOut(void);

		virtual void OnMouseEnter(void);

		virtual void OnMouseLeave(void);

		virtual void OnHotkey(void);

		virtual bool ContainsPoint(const Vector2 & pt);

		virtual void SetEnabled(bool bEnabled);

		virtual bool GetEnabled(void);

		virtual void SetVisible(bool bVisible);

		virtual bool GetVisible(void);

		virtual void Refresh(void);

		void SetHotkey(UINT nHotkey);

		UINT GetHotkey(void);
	};

	typedef boost::shared_ptr<Control> ControlPtr;

	class Static : public Control
	{
	public:
		std::wstring m_Text;

	public:
		Static(void)
			: m_Text(L"")
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool ContainsPoint(const Vector2 & pt);
	};

	typedef boost::shared_ptr<Static> StaticPtr;

	class ButtonSkin : public ControlSkin
	{
	public:
		RECT m_DisabledTexRect;

		RECT m_PressedTexRect;

		RECT m_MouseOverTexRect;

		Vector2 m_PressedOffset;

	public:
		ButtonSkin(void)
			: m_DisabledTexRect(CRect(0,0,0,0))
			, m_PressedTexRect(CRect(0,0,0,0))
			, m_MouseOverTexRect(CRect(0,0,0,0))
			, m_PressedOffset(0,0)
		{
		}
	};

	typedef boost::shared_ptr<ButtonSkin> ButtonSkinPtr;

	typedef fastdelegate::FastDelegate1<ControlPtr> ControlEvent;

	class Button : public Static
	{
	public:
		bool m_bPressed;

		D3DXCOLOR m_BlendColor;

		ControlEvent EventClick;

	public:
		Button(void)
			: m_bPressed(false)
			, m_BlendColor(m_Color)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnHotkey(void);

		virtual bool ContainsPoint(const Vector2 & pt);

		virtual void Refresh(void);
	};

	typedef boost::shared_ptr<Button> ButtonPtr;

	class EditBoxSkin : public ControlSkin
	{
	public:
		RECT m_DisabledTexRect;

		RECT m_FocusedTexRect;

		D3DCOLOR m_SelTextColor;

		D3DCOLOR m_SelBkColor;

		D3DCOLOR m_CaretColor;

	public:
		EditBoxSkin(void)
			: m_DisabledTexRect(CRect(0,0,0,0))
			, m_FocusedTexRect(CRect(0,0,0,0))
			, m_SelTextColor(D3DCOLOR_ARGB(255,255,255,255))
			, m_SelBkColor(D3DCOLOR_ARGB(197,0,0,0))
			, m_CaretColor(D3DCOLOR_ARGB(255,255,255,255))
		{
		}
	};

	typedef boost::shared_ptr<EditBoxSkin> EditBoxSkinPtr;

	class EditBox : public Control
	{
	public:
		std::wstring m_Text;

		int m_nCaret;

		bool m_bCaretOn;
    
		double m_dfBlink;

		double m_dfLastBlink;

		int m_nFirstVisible;

		Vector4 m_Border;

		bool m_bMouseDrag;

		int m_nSelStart;

		bool m_bInsertMode;

		ControlEvent EventChange;

		ControlEvent EventEnter;

	public:
		EditBox(void)
			: m_nCaret(0)
			, m_bCaretOn(true)
			, m_dfBlink(GetCaretBlinkTime() * 0.001f)
			, m_dfLastBlink(0)
			, m_nFirstVisible(0)
			, m_Border(0,0,0,0)
			, m_bMouseDrag(false)
			, m_nSelStart(0)
			, m_bInsertMode(true)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnFocusIn(void);

		void PlaceCaret(int nCP);

		void ResetCaretBlink(void);

		void DeleteSelectionText(void);

		void CopyToClipboard(void);

		void PasteFromClipboard(void);

		int GetPriorItemPos(int nCP);

		int GetNextItemPos(int nCP);
	};

	typedef boost::shared_ptr<EditBox> EditBoxPtr;

	class ImeEditBox : public EditBox
	{
	public:
		static bool s_bHideCaret;

		static std::wstring s_CompString;

		D3DCOLOR m_CompWinColor;

		D3DCOLOR m_CandidateWinColor;

	public:
		ImeEditBox(void)
			: m_CompWinColor(D3DCOLOR_ARGB(197,0,0,0))
			, m_CandidateWinColor(D3DCOLOR_ARGB(197,0,0,0))
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void OnFocusIn(void);

		virtual void OnFocusOut(void);

		static void Initialize(HWND hWnd);

		static void Uninitialize(void);

		static bool StaticMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static void ResetCompositionString(void);

		static void EnableImeSystem(bool bEnable);

		void RenderIndicator(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		void RenderComposition(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		void RenderCandidateWindow(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);
	};

	typedef boost::shared_ptr<ImeEditBox> ImeEditBoxPtr;

	class ScrollBarSkin : public ControlSkin
	{
	public:
		RECT m_UpBtnNormalTexRect;

		RECT m_UpBtnDisabledTexRect;

		RECT m_DownBtnNormalTexRect;

		RECT m_DownBtnDisabledTexRect;

		RECT m_ThumbBtnNormalTexRect;

		RECT m_ThumbBtnDisabledTexRect;

	public:
		ScrollBarSkin(void)
			: m_UpBtnNormalTexRect(CRect(0,0,0,0))
			, m_UpBtnDisabledTexRect(CRect(0,0,0,0))
			, m_DownBtnNormalTexRect(CRect(0,0,0,0))
			, m_DownBtnDisabledTexRect(CRect(0,0,0,0))
			, m_ThumbBtnNormalTexRect(CRect(0,0,0,0))
			, m_ThumbBtnDisabledTexRect(CRect(0,0,0,0))
		{
		}
	};

	typedef boost::shared_ptr<ScrollBarSkin> ScrollBarSkinPtr;

	class ScrollBar : public Control
	{
	public:
		bool m_bDrag;

		float m_UpDownButtonHeight;

		int m_nPosition;

		int m_nPageSize;

		int m_nStart;

		int m_nEnd;

		enum ARROWSTATE
		{
			CLEAR,
			CLICKED_UP,
			CLICKED_DOWN,
			HELD_UP,
			HELD_DOWN,
		};

		ARROWSTATE m_Arrow;

		double m_dArrowTS;

		float m_fThumbOffsetY;

	public:
		ScrollBar(void)
			: m_bDrag(false)
			, m_UpDownButtonHeight(20)
			, m_nPosition(0)
			, m_nPageSize(1)
			, m_nStart(0)
			, m_nEnd(10)
			, m_Arrow(CLEAR)
			, m_dArrowTS(0)
			, m_fThumbOffsetY(0)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		void Scroll(int nDelta);
	};

	typedef boost::shared_ptr<ScrollBar> ScrollBarPtr;

	class Dialog : public Control
	{
	public:
		typedef std::set<ControlPtr> ControlPtrSet;

		ControlPtrSet m_Controls;

		ControlPtr m_ControlMouseOver;

		Matrix4 m_Transform;

		Matrix4 m_ViewMatrix;

		Matrix4 m_ProjMatrix;

		bool m_bMouseDrag;

		Vector2 m_MouseOffset;

	public:
		Dialog(void)
			: m_Transform(Matrix4::Identity())
			, m_bMouseDrag(false)
			, m_MouseOffset(0,0)
		{
			UIRender::BuildPerspectiveMatrices(D3DXToRadian(75.0f), 800, 600, m_ViewMatrix, m_ProjMatrix);
		}

		virtual ~Dialog(void)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void Refresh(void);

		ControlPtr GetControlAtPoint(const Vector2 & pt);

		void RequestFocus(ControlPtr control);

		bool ContainsControl(ControlPtr control);
	};

	typedef boost::shared_ptr<Dialog> DialogPtr;
}
