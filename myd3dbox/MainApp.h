#pragma once

#include "resource.h"

class CMainApp : public CWinAppEx
{
public:
	CMainApp(void);

	virtual BOOL InitInstance(void);

	DECLARE_MESSAGE_MAP()

public:
	CComPtr<IDirect3D9> m_d3d9;
};

extern CMainApp theApp;
