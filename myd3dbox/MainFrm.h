#pragma once

#include "OutlinerView.h"
#include "PropertiesWnd.h"

class CMainFrame
	: public CFrameWndEx
{
public:
	CMainFrame(void);

	DECLARE_DYNCREATE(CMainFrame)

	UINT  m_nAppLook;

	CMFCMenuBar m_wndMenuBar;

	CMFCToolBar m_wndToolBar;

	CMFCStatusBar m_wndStatusBar;

	COutlinerView m_wndOutliner;

	CPropertiesWnd m_wndProperties;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnViewCustomize();

	afx_msg void OnApplicationLook(UINT id);

	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);

	afx_msg void OnDestroy();
};
