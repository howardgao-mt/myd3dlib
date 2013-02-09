#include "StdAfx.h"
#include "MainView.h"
#include "MainFrm.h"
#include "MainApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace my;

void EffectUIRender::Begin(void)
{
	CRect rectClient;
	CMainView::getSingleton().GetClientRect(&rectClient);
	if(m_UIEffect->m_ptr)
		m_UIEffect->SetVector("g_ScreenDim", Vector4((float)rectClient.Width(), (float)rectClient.Height(), 0, 0));

	if(m_UIEffect->m_ptr)
		m_Passes = m_UIEffect->Begin();
}

void EffectUIRender::End(void)
{
	if(m_UIEffect->m_ptr)
		m_UIEffect->End();
}

void EffectUIRender::SetTexture(IDirect3DBaseTexture9 * pTexture)
{
	if(m_UIEffect->m_ptr)
		m_UIEffect->SetTexture("g_MeshTexture", pTexture ? pTexture : CMainView::getSingleton().m_WhiteTex->m_ptr);
}

void EffectUIRender::SetTransform(const Matrix4 & World, const Matrix4 & View, const Matrix4 & Proj)
{
	if(m_UIEffect->m_ptr)
		m_UIEffect->SetMatrix("g_mWorldViewProjection", World * View * Proj);
}

void EffectUIRender::DrawVertexList(void)
{
	if(m_UIEffect->m_ptr)
	{
		if(vertex_count > 0)
		{
			for(UINT p = 0; p < m_Passes; p++)
			{
				m_UIEffect->BeginPass(p);
				V(m_Device->SetFVF(D3DFVF_CUSTOMVERTEX));
				V(m_Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertex_count / 3, vertex_list, sizeof(vertex_list[0])));
				m_UIEffect->EndPass();
			}
		}
	}
}

CMainView::SingleInstance * SingleInstance<CMainView>::s_ptr(NULL);

IMPLEMENT_DYNCREATE(CMainView, CView)

CMainView::CMainView(void)
	: m_Camera()
	, m_bAltDown(FALSE)
	, m_bEatAltUp(FALSE)
	, m_DragCameraMode(DragCameraNone)
{
}

BEGIN_MESSAGE_MAP(CMainView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

int CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Font = CMainFrame::getSingleton().LoadFont("wqy-microhei.ttc", 13);

	m_WhiteTex = CMainFrame::getSingleton().LoadTexture("white.bmp");

	m_UIRender.reset(new EffectUIRender(
		CMainFrame::getSingleton().m_d3dDevice, CMainFrame::getSingleton().LoadEffect("UIEffect.fx")));

	m_Camera.m_Rotation = Vector3(D3DXToRadian(-45),D3DXToRadian(45),0);
	m_Camera.m_LookAt = Vector3(0,0,0);
	m_Camera.m_Distance = 20;

	return 0;
}

void CMainView::OnDestroy()
{
	CView::OnDestroy();

	m_Font.reset();

	m_d3dSwapChain.Release();
}

void CMainView::OnPaint()
{
	CPaintDC dc(this);

	if(m_d3dSwapChain)
	{
		OnFrameRender(CMainFrame::getSingleton().m_d3dDevice, theApp.m_fAbsoluteTime, 0);
	}
}

void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if(cx > 0 && cy > 0 && m_d3dSwapChain)
	{
		OnDeviceLost();

		HRESULT hr;
		if(FAILED(hr = OnDeviceReset()))
		{
			TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		}
	}
}

BOOL CMainView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

HRESULT CMainView::OnDeviceReset(void)
{
	D3DPRESENT_PARAMETERS d3dpp = {0};
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.hDeviceWindow = m_hWnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	HRESULT hr = CMainFrame::getSingleton().m_d3dDevice->CreateAdditionalSwapChain(&d3dpp, &m_d3dSwapChain);
	if(FAILED(hr))
	{
		TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}

	Surface BackBuffer;
	V(m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer.m_ptr));
	D3DSURFACE_DESC desc = BackBuffer.GetDesc();
	m_DepthStencil.CreateDepthStencilSurface(
		CMainFrame::getSingleton().m_d3dDevice, desc.Width, desc.Height, D3DFMT_D24X8, d3dpp.MultiSampleType, d3dpp.MultiSampleQuality);

	m_Camera.m_Aspect = (float)desc.Width / desc.Height;

	return S_OK;
}

void CMainView::OnDeviceLost(void)
{
	m_DepthStencil.OnDestroyDevice();
	m_d3dSwapChain.Release();
}

void CMainView::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	ASSERT(m_d3dSwapChain);

	HRESULT hr;
	Surface BackBuffer;
	V(m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer.m_ptr));
	V(pd3dDevice->SetRenderTarget(0, BackBuffer.m_ptr));
	V(pd3dDevice->SetDepthStencilSurface(m_DepthStencil.m_ptr));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,45,50,170), 1.0f, 0));

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		m_Camera.OnFrameMove(0,0);
		V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Matrix4::identity));
		V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera.m_View));
		V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera.m_Proj));

		DrawLine(pd3dDevice, Vector3(-10,0,0), Vector3(10,0,0), D3DCOLOR_ARGB(255,0,0,0));
		DrawLine(pd3dDevice, Vector3(0,0,-10), Vector3(0,0,10), D3DCOLOR_ARGB(255,0,0,0));
		for(int i = 1; i <= 10; i++)
		{
			DrawLine(pd3dDevice, Vector3(-10,0, (float)i), Vector3(10,0, (float)i), D3DCOLOR_ARGB(255,127,127,127));
			DrawLine(pd3dDevice, Vector3(-10,0,-(float)i), Vector3(10,0,-(float)i), D3DCOLOR_ARGB(255,127,127,127));
			DrawLine(pd3dDevice, Vector3( (float)i,0,-10), Vector3( (float)i,0,10), D3DCOLOR_ARGB(255,127,127,127));
			DrawLine(pd3dDevice, Vector3(-(float)i,0,-10), Vector3(-(float)i,0,10), D3DCOLOR_ARGB(255,127,127,127));
		}

		D3DSURFACE_DESC desc = BackBuffer.GetDesc();
		m_UIRender->SetTransform(Matrix4::Identity(),
			UIRender::PerspectiveView(D3DXToRadian(75.0f), (float)desc.Width, (float)desc.Height),
			UIRender::PerspectiveProj(D3DXToRadian(75.0f), (float)desc.Width, (float)desc.Height));
		m_UIRender->Begin();

		CString strText;
		strText.Format(_T("%d x %d"), desc.Width, desc.Height);
		m_Font->DrawString(m_UIRender.get(), strText, my::Rectangle(10,10,200,200), D3DCOLOR_ARGB(255,255,255,0));

		m_UIRender->End();
		V(pd3dDevice->EndScene());
	}

	if(FAILED(hr = m_d3dSwapChain->Present(NULL, NULL, NULL, NULL, 0)))
	{
		if(D3DERR_DEVICELOST == hr || D3DERR_DRIVERINTERNALERROR == hr)
		{
			CMainFrame::getSingleton().ResetD3DDevice();
		}
	}
}

BOOL CMainView::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
	case WM_SYSKEYDOWN:
		if(pMsg->wParam == VK_MENU)
		{
			m_bAltDown = TRUE;
		}
		break;

	case WM_SYSKEYUP:
		if(pMsg->wParam == VK_MENU && m_bAltDown)
		{
			m_bAltDown = FALSE;

			if(m_bEatAltUp)
			{
				m_bEatAltUp = FALSE;
				return TRUE;
			}
		}
		break;

	case WM_LBUTTONDOWN:
		if(m_bAltDown && DragCameraNone == m_DragCameraMode)
		{
			m_bEatAltUp = TRUE;
			m_DragCameraMode = DragCameraRotate;
			m_Camera.m_DragPos = CPoint(pMsg->lParam);
			SetCapture();
			return TRUE;
		}
		break;

	case WM_LBUTTONUP:
		if(DragCameraRotate == m_DragCameraMode)
		{
			m_DragCameraMode = DragCameraNone;
			ReleaseCapture();
			return TRUE;
		}
		break;

	case WM_MBUTTONDOWN:
		if(m_bAltDown && DragCameraNone == m_DragCameraMode)
		{
			m_bEatAltUp = TRUE;
			m_DragCameraMode = DragCameraTrack;
			m_Camera.m_DragPos = CPoint(pMsg->lParam);
			SetCapture();
			return TRUE;
		}
		break;

	case WM_MBUTTONUP:
		if(DragCameraTrack == m_DragCameraMode)
		{
			m_DragCameraMode = DragCameraNone;
			ReleaseCapture();
			return TRUE;
		}
		break;

	case WM_RBUTTONDOWN:
		if(m_bAltDown && DragCameraNone == m_DragCameraMode)
		{
			m_bEatAltUp = TRUE;
			m_DragCameraMode = DragCameraZoom;
			m_Camera.m_DragPos = CPoint(pMsg->lParam);
			SetCapture();
			return TRUE;
		}
		break;

	case WM_RBUTTONUP:
		if(DragCameraZoom == m_DragCameraMode)
		{
			m_DragCameraMode = DragCameraNone;
			ReleaseCapture();
			return TRUE;
		}
		break;
	}

	return CView::PreTranslateMessage(pMsg);
}

void CMainView::OnMouseMove(UINT nFlags, CPoint point)
{
	switch(m_DragCameraMode)
	{
	case DragCameraRotate:
		m_Camera.m_Rotation.x -= D3DXToRadian((point.y - m_Camera.m_DragPos.y) * 0.5f);
		m_Camera.m_Rotation.y -= D3DXToRadian((point.x - m_Camera.m_DragPos.x) * 0.5f);
		m_Camera.m_DragPos = point;
		break;

	case DragCameraTrack:
		{
			Vector3 mov((m_Camera.m_DragPos.x - point.x) * 0.02f, (point.y - m_Camera.m_DragPos.y) * 0.02f, 0);
			m_Camera.m_LookAt += mov.transform(m_Camera.m_Orientation);
			m_Camera.m_DragPos = point;
		}
		break;

	case DragCameraZoom:
		m_Camera.m_Distance -= (point.x - m_Camera.m_DragPos.x) * 0.02f;
		m_Camera.m_DragPos = point;
		break;
	}
}
