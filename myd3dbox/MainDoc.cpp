#include "StdAfx.h"
#include "MainDoc.h"
#include "MainFrm.h"
#include "MainView.h"
#include "resource.h"

CMainDoc::SingleInstance * my::SingleInstance<CMainDoc>::s_ptr(NULL);

IMPLEMENT_DYNCREATE(CMainDoc, CDocument)

BEGIN_MESSAGE_MAP(CMainDoc, CDocument)
	ON_COMMAND(ID_EDIT_UNDO, &CMainDoc::OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CMainDoc::OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CMainDoc::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CMainDoc::OnUpdateEditRedo)
	ON_COMMAND(ID_CREATE_STATICMESH, &CMainDoc::OnCreateStaticmesh)
END_MESSAGE_MAP()

CMainDoc::CMainDoc(void)
{
}

BOOL CMainDoc::OnNewDocument()
{
	// TODO: Add your specialized code here and/or call the base class
	const my::Vector3 boxHalfExtents(10.0f, 10.0f, 10.0f);
	m_groundShape.reset(new btBoxShape(btVector3(boxHalfExtents.x, boxHalfExtents.y, boxHalfExtents.z)));

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(0, -boxHalfExtents.y, 0));
	m_groundMotionState.reset(new btDefaultMotionState(transform));

	btVector3 localInertia(0, 0, 0);
	m_groundBody.reset(new btRigidBody(
		btRigidBody::btRigidBodyConstructionInfo(0.0f, m_groundMotionState.get(), m_groundShape.get(), localInertia)));
	m_groundBody->setRestitution(1.0f);

	CMainView::getSingleton().m_dynamicsWorld->addRigidBody(m_groundBody.get());

	return CDocument::OnNewDocument();
}

BOOL CMainDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here

	return TRUE;
}

BOOL CMainDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDocument::OnSaveDocument(lpszPathName);
}

void CMainDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::OnCloseDocument();
}

void CMainDoc::OnEditUndo()
{
	operator[](m_nStep--)->Undo();

	UpdateAllViews(NULL);
}

void CMainDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nStep >= 0);
}

void CMainDoc::OnEditRedo()
{
	operator[](++m_nStep)->Do();

	UpdateAllViews(NULL);
}

void CMainDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nStep < (int)CDocHistoryMgr::size() - 1);
}

void CMainDoc::OnCreateStaticmesh()
{
	CFileDialog dlg(TRUE);
	if(IDOK == dlg.DoModal())
	{
		try
		{
			my::OgreMeshPtr mesh = CMainFrame::getSingleton().LoadMesh(ws2ms(dlg.GetPathName()));

			static unsigned int i = 0;
			CString strItem;
			strItem.Format(_T("mesh_%03d"), i++);
			AddTreeStaticMeshNode(strItem, mesh);

			UpdateAllViews(NULL);
		}
		catch (const my::Exception & e)
		{
			AfxMessageBox(str_printf(_T("Cannot open: %s\n%s"),
				dlg.GetFileName(), ms2ws(e.GetFullDescription().c_str()).c_str()).c_str());
		}
	}
}
