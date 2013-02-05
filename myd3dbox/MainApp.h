#pragma once

#include "resource.h"

class CMainApp
	: public CWinAppEx
	, public my::Clock
{
public:
	CMainApp(void);

	virtual BOOL InitInstance(void);

	DECLARE_MESSAGE_MAP()

public:
	CComPtr<IDirect3D9> m_d3d9;

	DWORD m_dwFrames;

	double m_fLastTime;

	float m_fFPS;

	virtual BOOL OnIdle(LONG lCount);
};

extern CMainApp theApp;
