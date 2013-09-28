#include "StdAfx.h"
#include "myUtility.h"
#include "libc.h"
#include "myCollision.h"
#include "myDxutApp.h"
#include "rapidxml.hpp"
#include <boost/bind.hpp>

using namespace my;

void DrawHelper::DrawLine(
	IDirect3DDevice9 * pd3dDevice,
	const Vector3 & v0,
	const Vector3 & v1,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	Vertex v[2];
	v[0].x = v0.x; v[0].y = v0.y; v[0].z = v0.z; v[0].color = Color;
	v[1].x = v1.x; v[1].y = v1.y; v[1].z = v1.z; v[1].color = Color;

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
}

void DrawHelper::DrawSphere(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	DrawSpereStage(pd3dDevice, radius, 0, 20, 0, Color, world);
}

void DrawHelper::DrawBox(
	IDirect3DDevice9 * pd3dDevice,
	const Vector3 & halfSize,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	Vertex v[8];
	v[0].x = -halfSize.x; v[0].y = -halfSize.y; v[0].z = -halfSize.z; v[0].color = Color;
	v[1].x =  halfSize.x; v[1].y = -halfSize.y; v[1].z = -halfSize.z; v[1].color = Color;
	v[2].x = -halfSize.x; v[2].y =  halfSize.y; v[2].z = -halfSize.z; v[2].color = Color;
	v[3].x =  halfSize.x; v[3].y =  halfSize.y; v[3].z = -halfSize.z; v[3].color = Color;
	v[4].x = -halfSize.x; v[4].y =  halfSize.y; v[4].z =  halfSize.z; v[4].color = Color;
	v[5].x =  halfSize.x; v[5].y =  halfSize.y; v[5].z =  halfSize.z; v[5].color = Color;
	v[6].x = -halfSize.x; v[6].y = -halfSize.y; v[6].z =  halfSize.z; v[6].color = Color;
	v[7].x =  halfSize.x; v[7].y = -halfSize.y; v[7].z =  halfSize.z; v[7].color = Color;

	unsigned short idx[12 * 2];
	int i = 0;
	idx[i++] = 0; idx[i++] = 1; idx[i++] = 1; idx[i++] = 3; idx[i++] = 3; idx[i++] = 2; idx[i++] = 2; idx[i++] = 0;
	idx[i++] = 0; idx[i++] = 6; idx[i++] = 1; idx[i++] = 7; idx[i++] = 3; idx[i++] = 5; idx[i++] = 2; idx[i++] = 4;
	idx[i++] = 6; idx[i++] = 7; idx[i++] = 7; idx[i++] = 5; idx[i++] = 5; idx[i++] = 4; idx[i++] = 4; idx[i++] = 6;

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, _countof(v), _countof(idx) / 2, idx, D3DFMT_INDEX16, v, sizeof(v[0]));
}

void DrawHelper::DrawTriangle(
	IDirect3DDevice9 * pd3dDevice,
	const Vector3 & v0,
	const Vector3 & v1,
	const Vector3 & v2,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	Vertex v[4];
	v[0].x = v0.x; v[0].y = v0.y; v[0].z = v0.z; v[0].color = Color;
	v[1].x = v1.x; v[1].y = v1.y; v[1].z = v1.z; v[1].color = Color;
	v[2].x = v2.x; v[2].y = v2.y; v[2].z = v2.z; v[2].color = Color;
	v[3] = v[0];

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, _countof(v) - 1, v, sizeof(v[0]));
}

void DrawHelper::DrawSpereStage(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	int VSTAGE_BEGIN,
	int VSTAGE_END,
	float offsetY,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	const int VSTAGE = 20;
	const int HSTAGE = 20;
	Vertex v[VSTAGE * HSTAGE * 4];
	for(int j = VSTAGE_BEGIN; j < VSTAGE_END; j++)
	{
		for(int i = 0; i < HSTAGE; i++)
		{
			float Theta[2] = {2 * D3DX_PI / HSTAGE * i, 2 * D3DX_PI / HSTAGE * (i + 1)};
			float Fi[2] = {D3DX_PI / VSTAGE * j, D3DX_PI / VSTAGE * (j + 1)};
			Vertex * pv = &v[(j * HSTAGE + i) * 4];
			pv[0].x = radius * sin(Fi[0]) * cos(Theta[0]);
			pv[0].y = radius * cos(Fi[0]) + offsetY;
			pv[0].z = radius * sin(Fi[0]) * sin(Theta[0]);
			pv[0].color = Color;

			pv[1].x = radius * sin(Fi[0]) * cos(Theta[1]);
			pv[1].y = radius * cos(Fi[0]) + offsetY;
			pv[1].z = radius * sin(Fi[0]) * sin(Theta[1]);
			pv[1].color = Color;

			pv[2] = pv[0];

			pv[3].x = radius * sin(Fi[1]) * cos(Theta[0]);
			pv[3].y = radius * cos(Fi[1]) + offsetY;
			pv[3].z = radius * sin(Fi[1]) * sin(Theta[0]);
			pv[3].color = Color;
		}
	}

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
}

void DrawHelper::DrawCylinderStage(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	float y0,
	float y1,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	struct Vertex
	{
		float x, y, z;
		D3DCOLOR color;
	};

	const int HSTAGE = 20;
	Vertex v[HSTAGE * 4];
	for(int i = 0; i < HSTAGE; i++)
	{
		float Theta[2] = {2 * D3DX_PI / HSTAGE * i, 2 * D3DX_PI / HSTAGE * (i + 1)};
		Vertex * pv = &v[i * 4];
		pv[0].x = radius * cos(Theta[0]);
		pv[0].y = y0;
		pv[0].z = radius * sin(Theta[0]);
		pv[0].color = Color;

		pv[1].x = radius * cos(Theta[1]);
		pv[1].y = y0;
		pv[1].z = radius * sin(Theta[1]);
		pv[1].color = Color;

		pv[2] = pv[0];

		pv[3].x = radius * cos(Theta[0]);
		pv[3].y = y1;
		pv[3].z = radius * sin(Theta[0]);
		pv[3].color = Color;
	}

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
}

void DrawHelper::DrawCapsule(
	IDirect3DDevice9 * pd3dDevice,
	float radius,
	float height,
	D3DCOLOR Color,
	const Matrix4 & world)
{
	float y0 = height * 0.5f;
	float y1 = -y0;
	DrawSpereStage(pd3dDevice, radius, 0, 10, y0, Color, world);
	DrawSpereStage(pd3dDevice, radius, 10, 20, y1, Color, world);
	DrawCylinderStage(pd3dDevice, radius, y0, y1, Color, world);
}

TimerPtr TimerMgr::AddTimer(float Interval, ControlEvent EventTimer)
{
	TimerPtr timer(new Timer(Interval, Interval));
	timer->m_EventTimer = EventTimer;
	InsertTimer(timer);
	return timer;
}

void TimerMgr::InsertTimer(TimerPtr timer)
{
	if(timer)
	{
		_ASSERT(timer->m_Removed);

		m_timerSet.insert(timer);

		timer->m_Removed = false;
	}
}

void TimerMgr::RemoveTimer(TimerPtr timer)
{
	if(timer)
	{
		_ASSERT(!timer->m_Removed);

		m_timerSet.erase(timer);

		timer->m_Removed = true;
	}
}

void TimerMgr::RemoveAllTimer(void)
{
	m_timerSet.clear();
}

void TimerMgr::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	TimerPtrSet::const_iterator timer_iter = m_timerSet.begin();
	for(; timer_iter != m_timerSet.end(); )
	{
		TimerPtr timer = (*timer_iter++);
		timer->m_RemainingTime = Min(m_MaxIterCount * timer->m_Interval, timer->m_RemainingTime + fElapsedTime);
		for(int i = 0; timer->m_RemainingTime >= timer->m_Interval && !timer->m_Removed; i++)
		{
			if(timer->m_EventTimer)
				timer->m_EventTimer(m_DefaultArgs);

			timer->m_RemainingTime -= timer->m_Interval;
		}
	}
}

BaseCamera::~BaseCamera(void)
{
}

void Camera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_View = Matrix4::Translation(-m_Position) * Matrix4::RotationQuaternion(m_Orientation.inverse());

	m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT Camera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

std::pair<Vector3, Vector3> Camera::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	Vector3 ptProj(Lerp(-1.0f, 1.0f, pt.x / dim.cx), Lerp(1.0f, -1.0f, pt.y / dim.cy), 1.0f);

	return std::make_pair(m_Position, (ptProj.transformCoord(m_InverseViewProj) - m_Position).normalize());
}

void ModelViewerCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_Orientation = Quaternion::RotationYawPitchRoll(m_Rotation.y, m_Rotation.x, 0);

	m_Position = Vector3(0,0,m_Distance).transform(m_Orientation) + m_LookAt;

	m_View = Matrix4::Translation(-m_LookAt)
		* Matrix4::RotationY(-m_Rotation.y)
		* Matrix4::RotationX(-m_Rotation.x)
		* Matrix4::RotationZ(-m_Rotation.z)
		* Matrix4::Translation(Vector3(0,0,-m_Distance));

	m_Proj = Matrix4::PerspectiveAovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT ModelViewerCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		m_bDrag = true;
		m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
		SetCapture(hWnd);
		*pbNoFurtherProcessing = true;
		return 0;

	case WM_LBUTTONUP:
		if(m_bDrag)
		{
			m_bDrag = false;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			m_Rotation.x -= D3DXToRadian(HIWORD(lParam) - m_DragPos.y);
			m_Rotation.y -= D3DXToRadian(LOWORD(lParam) - m_DragPos.x);
			m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEWHEEL:
		m_Distance -= (short)HIWORD(wParam) / WHEEL_DELTA;
		*pbNoFurtherProcessing = true;
		return 0;
	}
	return 0;
}

void FirstPersonCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_Orientation = Quaternion::RotationYawPitchRoll(m_Rotation.y, m_Rotation.x, 0);

	m_Position += (m_Velocity * 5.0f * fElapsedTime).transform(m_Orientation);

	m_View = Matrix4::Translation(-m_Position)
		* Matrix4::RotationY(-m_Rotation.y)
		* Matrix4::RotationX(-m_Rotation.x)
		* Matrix4::RotationZ(-m_Rotation.z);

	m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT FirstPersonCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		m_bDrag = true;
		m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
		SetCapture(hWnd);
		*pbNoFurtherProcessing = true;
		return 0;

	case WM_LBUTTONUP:
		if(m_bDrag)
		{
			m_bDrag = false;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			m_Rotation.x -= D3DXToRadian(HIWORD(lParam) - m_DragPos.y);
			m_Rotation.y -= D3DXToRadian(LOWORD(lParam) - m_DragPos.x);
			m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case 'W':
			m_Velocity.z = -1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'S':
			m_Velocity.z = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'A':
			m_Velocity.x = -1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'D':
			m_Velocity.x = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'E':
			m_Velocity.y = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'Q':
			m_Velocity.y = -1;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_KEYUP:
		switch(wParam)
		{
		case 'W':
			if(m_Velocity.z < 0)
				m_Velocity.z = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'S':
			if(m_Velocity.z > 0)
				m_Velocity.z = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'A':
			if(m_Velocity.x < 0)
				m_Velocity.x = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'D':
			if(m_Velocity.x > 0)
				m_Velocity.x = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'E':
			if(m_Velocity.y > 0)
				m_Velocity.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'Q':
			if(m_Velocity.y < 0)
				m_Velocity.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;
	}
	return 0;
}

void DialogMgr::SetDlgViewport(const Vector2 & vp)
{
	m_Camera.m_Position = Vector3(vp.x * 0.5f, vp.y * 0.5f, -vp.y * 0.5f * cot(m_Camera.m_Fov / 2));

	m_Camera.m_Rotation.x = D3DXToRadian(180);

	m_Camera.m_Aspect = vp.x / vp.y;

	m_Camera.OnFrameMove(0,0);

	DialogPtrSetMap::iterator dlg_layer_iter = m_dlgSetMap.begin();
	for(; dlg_layer_iter != m_dlgSetMap.end(); dlg_layer_iter++)
	{
		DialogPtrList::iterator dlg_iter = dlg_layer_iter->second.begin();
		for(; dlg_iter != dlg_layer_iter->second.end(); dlg_iter++)
		{
			if((*dlg_iter)->EventAlign)
				(*dlg_iter)->EventAlign(EventArgsPtr(new EventArgs()));
		}
	}
}

Vector2 DialogMgr::GetDlgViewport(void) const
{
	return Vector2(-m_Camera.m_View._41*2, m_Camera.m_View._42*2);
}

void DialogMgr::Draw(
	UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	ui_render->SetViewProj(m_Camera.m_ViewProj);

	DialogPtrSetMap::iterator dlg_layer_iter = m_dlgSetMap.begin();
	for(; dlg_layer_iter != m_dlgSetMap.end(); dlg_layer_iter++)
	{
		DialogPtrList::iterator dlg_iter = dlg_layer_iter->second.begin();
		for(; dlg_iter != dlg_layer_iter->second.end(); dlg_iter++)
		{
			ui_render->SetWorld((*dlg_iter)->m_World);

			(*dlg_iter)->Draw(ui_render, fElapsedTime);
		}
	}
}

bool DialogMgr::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	ControlPtr ControlFocus = Dialog::s_ControlFocus.lock();
	if(ControlFocus)
	{
		_ASSERT(!boost::dynamic_pointer_cast<Dialog>(ControlFocus));
		if(ControlFocus->MsgProc(hWnd, uMsg, wParam, lParam))
			return true;
	}

	switch(uMsg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if(ControlFocus)
		{
			if(ControlFocus->HandleKeyboard(uMsg, wParam, lParam))
				return true;
		}

		if(uMsg == WM_KEYDOWN)
		{
			DialogPtrSetMap::iterator dlg_layer_iter = m_dlgSetMap.begin();
			for(; dlg_layer_iter != m_dlgSetMap.end(); dlg_layer_iter++)
			{
				DialogPtrList::iterator dlg_iter = dlg_layer_iter->second.begin();
				for(; dlg_iter != dlg_layer_iter->second.end(); dlg_iter++)
				{
					if((*dlg_iter)->HandleKeyboard(uMsg, wParam, lParam))
						return true;
				}
			}
		}
		break;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
		{
			CRect ClientRect;
			GetClientRect(hWnd, &ClientRect);
			std::pair<Vector3, Vector3> ray = m_Camera.CalculateRay(
				Vector2((short)LOWORD(lParam) + 0.5f, (short)HIWORD(lParam) + 0.5f), ClientRect.Size());

			DialogPtrSetMap::reverse_iterator dlg_layer_iter = m_dlgSetMap.rbegin();
			for(; dlg_layer_iter != m_dlgSetMap.rend(); dlg_layer_iter++)
			{
				DialogPtrList::reverse_iterator dlg_iter = dlg_layer_iter->second.rbegin();
				for(; dlg_iter != dlg_layer_iter->second.rend(); dlg_iter++)
				{
					// ! 只处理看得见的 Dialog
					if((*dlg_iter)->GetEnabled() && (*dlg_iter)->GetVisible())
					{
						Vector3 dialogNormal = Vector3(0, 0, 1).transformNormal((*dlg_iter)->m_World);
						float dialogDistance = ((Vector3 &)(*dlg_iter)->m_World[3]).dot(dialogNormal);
						IntersectionTests::TestResult result = IntersectionTests::rayAndHalfSpace(ray.first, ray.second, dialogNormal, dialogDistance);

						if(result.first)
						{
							Vector3 ptInt(ray.first + ray.second * result.second);
							Vector3 pt = ptInt.transformCoord((*dlg_iter)->m_World.inverse());
							Vector2 ptLocal = pt.xy - (*dlg_iter)->m_Location;

							// ! 只处理自己的 ControlFocus
							if(ControlFocus && (*dlg_iter)->ContainsControl(ControlFocus))
							{
								if(ControlFocus->HandleMouse(uMsg, ptLocal, wParam, lParam))
									return true;
							}

							// ! 只处理自己的 m_ControlMouseOver
							ControlPtr ControlPtd = (*dlg_iter)->GetControlAtPoint(ptLocal);
							ControlPtr m_ControlMouseOver = (*dlg_iter)->m_ControlMouseOver.lock();
							if(ControlPtd != m_ControlMouseOver)
							{
								if(m_ControlMouseOver)
									m_ControlMouseOver->OnMouseLeave();

								if(ControlPtd && ControlPtd->GetEnabled())
								{
									(*dlg_iter)->m_ControlMouseOver = ControlPtd;
									ControlPtd->OnMouseEnter();
								}
								else
									(*dlg_iter)->m_ControlMouseOver.reset();
							}

							if(ControlPtd && ControlPtd->GetEnabled())
							{
								if(ControlPtd->HandleMouse(uMsg, ptLocal, wParam, lParam))
								{
									Dialog::RequestFocus(ControlPtd);
									return true;
								}
							}

							if(uMsg == WM_LBUTTONDOWN
								&& (*dlg_iter)->ContainsControl(ControlFocus) && !(*dlg_iter)->ContainsPoint(pt.xy))
							{
								// ! 用以解决对话框控件丢失焦点
								ControlFocus->OnFocusOut();
								Dialog::s_ControlFocus.reset();
							}

							if((*dlg_iter)->HandleMouse(uMsg, pt.xy, wParam, lParam))
							{
								// ! 强制让自己具有 FocusControl
								(*dlg_iter)->ForceFocusControl();
								return true;
							}
						}
					}
				}
			}
		}
		break;
	}

	return false;
}

void DialogMgr::InsertDlg(DialogPtr dlg)
{
	m_dlgSetMap[0].push_back(dlg);

	if(dlg->EventAlign)
		dlg->EventAlign(EventArgsPtr(new EventArgs()));
}

void DialogMgr::RemoveDlg(DialogPtr dlg)
{
	DialogPtrList::iterator dlg_iter = std::find(m_dlgSetMap[0].begin(), m_dlgSetMap[0].end(), dlg);
	if(dlg_iter != m_dlgSetMap[0].end())
	{
		m_dlgSetMap[0].erase(dlg_iter);
	}
}

void DialogMgr::RemoveAllDlg()
{
	m_dlgSetMap[0].clear();
}

void EmitterMgr::Update(
	double fTime,
	float fElapsedTime)
{
	EmitterPtrSet::iterator emitter_iter = m_EmitterSet.begin();
	for(; emitter_iter != m_EmitterSet.end(); emitter_iter++)
	{
		(*emitter_iter)->Update(fTime, fElapsedTime);
	}
}

void EmitterMgr::Draw(
	EmitterInstance * pInstance,
	Camera * pCamera,
	double fTime,
	float fElapsedTime)
{
	pInstance->SetViewProj(pCamera->m_ViewProj);

	EmitterPtrSet::iterator emitter_iter = m_EmitterSet.begin();
	for(; emitter_iter != m_EmitterSet.end(); emitter_iter++)
	{
		pInstance->SetWorld(Matrix4::identity);

		switch((*emitter_iter)->m_Direction)
		{
		case Emitter::DirectionTypeCamera:
			pInstance->SetDirection(
				Vector3(0,0,1).transform(pCamera->m_Orientation),
				Vector3(0,1,0).transform(pCamera->m_Orientation),
				Vector3(1,0,0).transform(pCamera->m_Orientation));
			break;

		case Emitter::DirectionTypeVertical:
			{
				Vector3 Up(0,1,0);
				Vector3 Right = Vector3(0,0,-1).transform(pCamera->m_Orientation).cross(Up);
				Vector3 Dir = Right.cross(Up);
				pInstance->SetDirection(Dir, Up, Right);
			}
			break;

		case Emitter::DirectionTypeHorizontal:
			pInstance->SetDirection(Vector3(0,1,0), Vector3(0,0,1), Vector3(-1,0,0));
			break;
		}

		(*emitter_iter)->Draw(pInstance, fTime, fElapsedTime);
	}
}

void EmitterMgr::InsertEmitter(EmitterPtr emitter)
{
	_ASSERT(m_EmitterSet.end() == m_EmitterSet.find(emitter));

	m_EmitterSet.insert(emitter);
}

void EmitterMgr::RemoveEmitter(EmitterPtr emitter)
{
	m_EmitterSet.erase(emitter);
}

void EmitterMgr::RemoveAllEmitter(void)
{
	m_EmitterSet.clear();
}

EffectParameterBase::~EffectParameterBase(void)
{
}

template <>
void EffectParameter<bool>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetBool(Name.c_str(), m_Value);
}

template <>
void EffectParameter<float>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetFloat(Name.c_str(), m_Value);
}

template <>
void EffectParameter<int>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetInt(Name.c_str(), m_Value);
}

template <>
void EffectParameter<Vector4>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetVector(Name.c_str(), m_Value);
}

template <>
void EffectParameter<Matrix4>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetMatrix(Name.c_str(), m_Value);
}

template <>
void EffectParameter<std::string>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetString(Name.c_str(), m_Value.c_str());
}

template <>
void EffectParameter<BaseTexturePtr>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetTexture(Name.c_str(), m_Value);
}

void EffectParameterMap::SetBool(const std::string & Name, bool Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<bool>(Value));
}

void EffectParameterMap::SetFloat(const std::string & Name, float Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<float>(Value));
}

void EffectParameterMap::SetInt(const std::string & Name, int Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<int>(Value));
}

void EffectParameterMap::SetVector(const std::string & Name, const Vector4 & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<Vector4>(Value));
}

void EffectParameterMap::SetMatrix(const std::string & Name, const Matrix4 & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<Matrix4>(Value));
}

void EffectParameterMap::SetString(const std::string & Name, const std::string & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<std::string>(Value));
}

void EffectParameterMap::SetTexture(const std::string & Name, BaseTexturePtr Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<BaseTexturePtr>(Value));
}

void Material::ApplyParameterBlock(UINT i)
{
	const value_type & effect_pair = operator [] (i);
	if(effect_pair.first)
	{
		EffectParameterMap::const_iterator param_iter = effect_pair.second.begin();
		for(; param_iter != effect_pair.second.end(); param_iter++)
		{
			param_iter->second->SetParameter(effect_pair.first.get(), param_iter->first);
		}
	}
}

UINT Material::Begin(UINT i, DWORD Flags)
{
	const value_type & effect_pair = operator [] (i);
	if(effect_pair.first)
	{
		return effect_pair.first->Begin(Flags);
	}
	return 0;
}

void Material::BeginPass(UINT i, UINT Pass)
{
	const value_type & effect_pair = operator [] (i);
	if(effect_pair.first)
	{
		effect_pair.first->BeginPass(Pass);
	}
}

void Material::EndPass(UINT i)
{
	const value_type & effect_pair = operator [] (i);
	if(effect_pair.first)
	{
		effect_pair.first->EndPass();
	}
}

void Material::End(UINT i)
{
	const value_type & effect_pair = operator [] (i);
	if(effect_pair.first)
	{
		effect_pair.first->End();
	}
}

void Material::DrawMeshSubset(UINT i, Mesh * pMesh, DWORD AttribId)
{
	ApplyParameterBlock(i);

	UINT Pass = Begin(i, 0);
	for(UINT p = 0; p < Pass; p++)
	{
		BeginPass(i, p);
		pMesh->DrawSubset(AttribId);
		EndPass(i);
	}
	End(i);
}

void MaterialMgr::InsertMaterial(const std::string & key, MaterialPtr material)
{
	_ASSERT(m_MaterialMap.end() == m_MaterialMap.find(key));

	m_MaterialMap[key] = material;
}

void MaterialMgr::RemoveMaterial(const std::string & key)
{
	_ASSERT(m_MaterialMap.end() != m_MaterialMap.find(key));

	m_MaterialMap.erase(key);
}

void MaterialMgr::RemoveAllMaterial(void)
{
	m_MaterialMap.clear();
}

class ResourceMgr::MaterialIORequest : public IORequest
{
protected:
	std::string m_path;

	ResourceMgr * m_arc;

	CachePtr m_cache;

	rapidxml::xml_document<char> m_doc;

public:
	MaterialIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckArchivePath(m_path))
		{
			m_cache = m_arc->OpenArchiveStream(m_path)->GetWholeCache();
			m_cache->push_back(0);
			try
			{
				m_doc.parse<0>((char *)&(*m_cache)[0]);
			}
			catch (rapidxml::parse_error & e)
			{
				THROW_CUSEXCEPTION(ms2ts(e.what()));
			}
		}
	}

	static void MaterialSetEffect(ResourceMgr::ResourceCallbackBoundlePtr boundle, size_t effect_i, DeviceRelatedObjectBasePtr effect_res)
	{
		(*boost::dynamic_pointer_cast<Material>(boundle->m_res))[effect_i].first = boost::dynamic_pointer_cast<Effect>(effect_res);
	}

	static void MaterialSetEffectTextureParameter(ResourceMgr::ResourceCallbackBoundlePtr boundle, size_t effect_i, std::string value, DeviceRelatedObjectBasePtr texture_res)
	{
		(*boost::dynamic_pointer_cast<Material>(boundle->m_res))[effect_i].second.SetTexture(value, boost::dynamic_pointer_cast<BaseTexture>(texture_res));
	}

	virtual void OnLoadEffect(ResourceCallbackBoundlePtr boundle, size_t i, const char * path, const EffectMacroPairList & macros)
	{
		m_arc->LoadEffectAsync(path, macros, boost::bind(&MaterialIORequest::MaterialSetEffect, boundle, i, _1));
	}

	virtual void OnLoadTexture(ResourceCallbackBoundlePtr boundle, size_t i, const char * path, const char * name)
	{
		m_arc->LoadTextureAsync(path, boost::bind(&MaterialIORequest::MaterialSetEffectTextureParameter, boundle, i, std::string(name), _1));
	}

	virtual void OnPostBuildResource(ResourceCallbackBoundlePtr boundle)
	{
		boundle->m_callbacks = m_callbacks;
		m_callbacks.clear();
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(m_doc.first_node())
		{
			MaterialPtr ret(new Material());
			ResourceCallbackBoundlePtr boundle(new ResourceCallbackBoundle(ret));

			rapidxml::xml_node<char> * node_root = &m_doc;
			DEFINE_XML_NODE_SIMPLE(material, root);
			DEFINE_XML_NODE_SIMPLE(shader, material);
			for(size_t i = 0; node_shader; node_shader = node_shader->next_sibling(), i++)
			{
				rapidxml::xml_attribute<char> * attr_shader_path;
				DEFINE_XML_ATTRIBUTE(attr_shader_path, node_shader, path);

				ret->push_back(Material::value_type());
				Material::value_type & effect_pair = ret->back();

				DEFINE_XML_NODE_SIMPLE(parameter, shader);
				for(; node_parameter; node_parameter = node_parameter->next_sibling())
				{
					DEFINE_XML_ATTRIBUTE_SIMPLE(name, parameter);
					rapidxml::xml_node<char> * node_value = node_parameter->first_node();

					if(0 == strcmp(node_value->name(), "Vector4"))
					{
						DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(x, value);
						DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(y, value);
						DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(z, value);
						DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(w, value);
						effect_pair.second.SetVector(attr_name->value(), Vector4(x, y, z, w));
					}
					else if(0 == strcmp(node_value->name(), "Texture"))
					{
						DEFINE_XML_ATTRIBUTE_SIMPLE(path, value);
						OnLoadTexture(boundle, i, attr_path->value(), attr_name->value());
					}
				}
				OnLoadEffect(boundle, i, attr_shader_path->value(), EffectMacroPairList());
			}
			m_res = ret;
			OnPostBuildResource(boundle);
		}
	}
};

void ResourceMgr::LoadMaterialAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new MaterialIORequest(callback, path, this)));
}

MaterialPtr ResourceMgr::LoadMaterial(const std::string & path)
{
	class SyncMaterialIORequest : public MaterialIORequest
	{
	public:
		SyncMaterialIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
			: MaterialIORequest(callback, path, arc)
		{
		}

		virtual void OnLoadEffect(ResourceCallbackBoundlePtr boundle, size_t i, const char * path, const EffectMacroPairList & macros)
		{
			(*boost::dynamic_pointer_cast<Material>(boundle->m_res))[i].first = m_arc->LoadEffect(path, macros);
		}

		virtual void OnLoadTexture(ResourceCallbackBoundlePtr boundle, size_t i, const char * path, const char * name)
		{
			(*boost::dynamic_pointer_cast<Material>(boundle->m_res))[i].second.SetTexture(name, m_arc->LoadTexture(path));
		}

		virtual void OnPostBuildResource(ResourceCallbackBoundlePtr boundle)
		{
		}
	};

	IORequestPtr request = LoadResourceAsync(path, IORequestPtr(new SyncMaterialIORequest(ResourceCallback(), path, this)));

	CheckRequest(path, request, INFINITE);

	return boost::dynamic_pointer_cast<Material>(request->m_res);
}
