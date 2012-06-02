
#pragma once

#include <myD3dLib.h>

class MessagePanel
	: public my::Control
{
public:
	struct Line
	{
		std::wstring m_Text;

		D3DCOLOR m_Color;
	};

	Line m_lines[256];

	int m_lbegin;

	int m_lend;

	boost::weak_ptr<my::ScrollBar> m_scrollbar;

public:
	MessagePanel(void);

	~MessagePanel(void);

	virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Vector2 & Offset);

	int MoveLineIndex(int index, int step);

	int LineIndexDistance(int start, int end);

	void _update_scrollbar(void);

	void _push_enter(void);

	void _push_sline(LPCWSTR pString, D3DCOLOR Color);

	void AddLine(LPCWSTR pString, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));

	void puts(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));
};

typedef boost::shared_ptr<MessagePanel> MessagePanelPtr;

class Console
	: public my::Dialog
{
public:
	my::Vector4 m_Border;

	my::ImeEditBoxPtr m_edit;

	my::ScrollBarPtr m_scrollbar;

	MessagePanelPtr m_panel;

public:
	Console(void);

	~Console(void);

	void AddLine(LPCWSTR pString, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));

	void puts(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));
};

typedef boost::shared_ptr<Console> ConsolePtr;
