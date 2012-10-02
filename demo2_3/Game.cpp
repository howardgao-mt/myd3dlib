﻿#include "stdafx.h"
#include "Game.h"
#include "GameState.h"
#include "LuaExtension.h"
#include <luabind/luabind.hpp>

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

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

LoaderMgr::LoaderMgr(void)
{
	RegisterFileDir("Media");
	RegisterZipArchive("Media.zip");
	RegisterFileDir("..\\demo2_3\\Media");
	RegisterZipArchive("..\\demo2_3\\Media.zip");
}

LoaderMgr::~LoaderMgr(void)
{
}

HRESULT LoaderMgr::Open(
	D3DXINCLUDE_TYPE IncludeType,
	LPCSTR pFileName,
	LPCVOID pParentData,
	LPCVOID * ppData,
	UINT * pBytes)
{
	CachePtr cache;
	std::string loc_path = std::string("shader/") + pFileName;
	switch(IncludeType)
	{
	case D3DXINC_SYSTEM:
	case D3DXINC_LOCAL:
		if(CheckArchivePath(loc_path))
		{
			cache = OpenArchiveStream(loc_path)->GetWholeCache();
			*ppData = &(*cache)[0];
			*pBytes = cache->size();
			_ASSERT(m_cacheSet.end() == m_cacheSet.find(*ppData));
			m_cacheSet[*ppData] = cache;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT LoaderMgr::Close(
	LPCVOID pData)
{
	_ASSERT(m_cacheSet.end() != m_cacheSet.find(pData));
	m_cacheSet.erase(m_cacheSet.find(pData));
	return S_OK;
}

HRESULT LoaderMgr::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	DeviceRelatedResourceSet::iterator res_iter = m_resourceSet.begin();
	for(; res_iter != m_resourceSet.end();)
	{
		boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
		if(res)
		{
			res->OnResetDevice();
			res_iter++;
		}
		else
		{
			m_resourceSet.erase(res_iter++);
		}
	}

	return S_OK;
}

void LoaderMgr::OnLostDevice(void)
{
	DeviceRelatedResourceSet::iterator res_iter = m_resourceSet.begin();
	for(; res_iter != m_resourceSet.end();)
	{
		boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
		if(res)
		{
			res->OnLostDevice();
			res_iter++;
		}
		else
		{
			m_resourceSet.erase(res_iter++);
		}
	}
}

void LoaderMgr::OnDestroyDevice(void)
{
	DeviceRelatedResourceSet::iterator res_iter = m_resourceSet.begin();
	for(; res_iter != m_resourceSet.end();)
	{
		boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
		if(res)
		{
			res->OnDestroyDevice();
			res_iter++;
		}
		else
		{
			m_resourceSet.erase(res_iter++);
		}
	}

	m_resourceSet.clear();
}

boost::shared_ptr<my::BaseTexture> LoaderMgr::LoadTexture(const std::string & path)
{
	TexturePtr ret(new Texture());
	std::string loc_path = std::string("texture/") + path;
	std::string full_path = GetFullPath(loc_path);
	if(!full_path.empty())
	{
		ret->CreateTextureFromFile(GetD3D9Device(), ms2ts(full_path.c_str()).c_str());
	}
	else
	{
		CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
		ret->CreateTextureFromFileInMemory(GetD3D9Device(), &(*cache)[0], cache->size());
	}

	m_resourceSet.insert(std::make_pair(path, ret));
	return ret;
}

boost::shared_ptr<my::BaseTexture> LoaderMgr::LoadCubeTexture(const std::string & path)
{
	CubeTexturePtr ret(new CubeTexture());
	std::string loc_path = std::string("texture/") + path;
	std::string full_path = GetFullPath(loc_path);
	if(!full_path.empty())
	{
		ret->CreateCubeTextureFromFile(GetD3D9Device(), ms2ts(full_path.c_str()).c_str());
	}
	else
	{
		CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
		ret->CreateCubeTextureFromFileInMemory(GetD3D9Device(), &(*cache)[0], cache->size());
	}

	m_resourceSet.insert(std::make_pair(path, ret));
	return ret;
}

OgreMeshPtr LoaderMgr::LoadMesh(const std::string & path)
{
	OgreMeshPtr ret(new OgreMesh());
	std::string loc_path = std::string("mesh/") + path;
	std::string full_path = GetFullPath(loc_path);
	if(!full_path.empty())
	{
		ret->CreateMeshFromOgreXml(GetD3D9Device(), full_path.c_str(), true);
	}
	else
	{
		CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
		ret->CreateMeshFromOgreXmlInMemory(GetD3D9Device(), (char *)&(*cache)[0], cache->size(), true);
	}

	m_resourceSet.insert(std::make_pair(path, ret));
	return ret;
}

OgreSkeletonAnimationPtr LoaderMgr::LoadSkeleton(const std::string & path)
{
	OgreSkeletonAnimationPtr ret(new OgreSkeletonAnimation());
	std::string loc_path = std::string("mesh/") + path;
	std::string full_path = GetFullPath(loc_path);
	if(!full_path.empty())
	{
		ret->CreateOgreSkeletonAnimationFromFile(ms2ts(full_path.c_str()).c_str());
	}
	else
	{
		CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
		ret->CreateOgreSkeletonAnimation((char *)&(*cache)[0], cache->size());
	}
	return ret;
}

EffectPtr LoaderMgr::LoadEffect(const std::string & path)
{
	EffectPtr ret(new Effect());
	std::string loc_path = std::string("shader/") + path;
	std::string full_path = GetFullPath(loc_path);
	if(!full_path.empty())
	{
		ret->CreateEffectFromFile(GetD3D9Device(), ms2ts(full_path.c_str()).c_str(), NULL, NULL, 0, m_EffectPool);
	}
	else
	{
		CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
		ret->CreateEffect(GetD3D9Device(), &(*cache)[0], cache->size(), NULL, this, 0, m_EffectPool);
	}

	m_resourceSet.insert(std::make_pair(path, ret));
	return ret;
}

FontPtr LoaderMgr::LoadFont(const std::string & path, int height)
{
	FontPtr ret(new Font());
	std::string loc_path = std::string("font/") + path;
	std::string full_path = GetFullPath(loc_path);
	if(!full_path.empty())
	{
		ret->CreateFontFromFile(GetD3D9Device(), full_path.c_str(), height);
	}
	else
	{
		CachePtr cache = OpenArchiveStream(loc_path)->GetWholeCache();
		ret->CreateFontFromFileInCache(GetD3D9Device(), cache, height);
	}

	m_resourceSet.insert(std::make_pair(str_printf("%s, %d", path.c_str(), height), ret));
	return ret;
}

void Timer::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_RemainingTime += fElapsedTime;
	unsigned int iter = 0;
	while(m_RemainingTime >= 0 && iter++ < m_MaxIter)
	{
		m_RemainingTime -= m_Interval;

		if(m_EventTimer)
			m_EventTimer();
	}
}

void TimerMgr::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	TimerPtrSet::const_iterator timer_iter = m_timerSet.begin();
	for(; timer_iter != m_timerSet.end(); timer_iter++)
	{
		(*timer_iter)->OnFrameMove(fElapsedTime, fElapsedTime);
	}
}

void DialogMgr::OnAlign(void)
{
	DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
	for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
	{
		UpdateDlgViewProj(*dlg_iter);
	}
}

void DialogMgr::UpdateDlgViewProj(DialogPtr dlg)
{
	const D3DSURFACE_DESC & desc = GetD3D9BackBufferSurfaceDesc();

	float aspect = desc.Width / (float)desc.Height;

	float height = (float)desc.Height;

	Vector2 vp(height * aspect, height);

	UIRender::BuildPerspectiveMatrices(D3DXToRadian(75.0f), vp.x, vp.y, dlg->m_View, dlg->m_Proj);

	if(dlg->EventAlign)
		dlg->EventAlign(EventArgsPtr(new AlignEventArgs(vp)));
}

void DialogMgr::Draw(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
	for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
	{
		(*dlg_iter)->Draw(pd3dDevice, fElapsedTime);
	}
}

bool DialogMgr::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	DialogPtrSet::reverse_iterator dlg_iter = m_dlgSet.rbegin();
	for(; dlg_iter != m_dlgSet.rend(); dlg_iter++)
	{
		if((*dlg_iter)->MsgProc(hWnd, uMsg, wParam, lParam))
			return true;
	}

	return false;
}

Game::Game(void)
{
	m_lua.reset(new LuaContext());

	Export2Lua(m_lua->_state);
}

Game::~Game(void)
{
	ClearAllDlg(); // ! m_dlgSet must be destroyed before distruct m_lua for some EventXxx Delegates

	ImeEditBox::Uninitialize();
}

bool Game::IsDeviceAcceptable(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed)
{
	if( FAILED( m_d3d9->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
		return false;

	if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
		return false;

	return true;
}

bool Game::ModifyDeviceSettings(
	DXUTD3D9DeviceSettings * pDeviceSettings)
{
	D3DCAPS9 caps;
	V( m_d3d9->GetDeviceCaps( pDeviceSettings->AdapterOrdinal,
		pDeviceSettings->DeviceType,
		&caps ) );

	if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
		caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
	{
		pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	if( caps.MaxVertexBlendMatrices < 2 )
		pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// ! Fix lua print(0xffffffff) issue, ref: http://www.lua.org/bugs.html#5.1-3
	pDeviceSettings->BehaviorFlags |= D3DCREATE_FPU_PRESERVE;

	static bool s_bFirstTime = true;
	if( s_bFirstTime )
	{
		s_bFirstTime = false;
		//if( pDeviceSettings->DeviceType == D3DDEVTYPE_REF )
		//	DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
	}

	return true;
}

HRESULT Game::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	if(FAILED(hr = D3DXCreateEffectPool(&m_EffectPool)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	ImeEditBox::Initialize(GetHWND());

	ImeEditBox::EnableImeSystem(false);

	ExecuteCode("dofile \"Font.lua\"");

	ExecuteCode("dofile \"Console.lua\"");

	if(!m_font || !m_console || !m_panel)
	{
		THROW_CUSEXCEPTION("m_font, m_console, m_panel must be created");
	}

	m_console->SetVisible(false);

	UpdateDlgViewProj(m_console);

	AddLine(L"Game::OnCreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	if(!m_input)
	{
		m_input.reset(new Input());
		m_input->CreateInput(GetModuleHandle(NULL));

		m_keyboard.reset(new Keyboard());
		m_keyboard->CreateKeyboard(m_input->m_ptr);
		m_keyboard->SetCooperativeLevel(GetHWND(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

		m_mouse.reset(new Mouse());
		m_mouse->CreateMouse(m_input->m_ptr);
		m_mouse->SetCooperativeLevel(GetHWND(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	}

	if(!m_sound)
	{
		m_sound.reset(new Sound());
		m_sound->CreateSound();
		m_sound->SetCooperativeLevel(GetHWND(), DSSCL_PRIORITY);
	}

	initiate();

	SafeCreateCurrentState();

	//THROW_CUSEXCEPTION("aaa");

	return S_OK;
}

HRESULT Game::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	AddLine(L"Game::OnResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	HRESULT hres;
	if(FAILED(hres = LoaderMgr::OnResetDevice(
		pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hres;
	}

	UpdateDlgViewProj(m_console);

	OnAlign();

	SafeResetCurrentState();

	return S_OK;
}

void Game::OnLostDevice(void)
{
	AddLine(L"Game::OnLostDevice", D3DCOLOR_ARGB(255,255,255,0));

	SafeLostCurrentState();

	LoaderMgr::OnLostDevice();
}

void Game::OnDestroyDevice(void)
{
	AddLine(L"Game::OnDestroyDevice", D3DCOLOR_ARGB(255,255,255,0));

	SafeDestroyCurrentState();

	terminate();

	m_EffectPool.Release();

	ExecuteCode("collectgarbage(\"collect\")");

	m_console.reset();

	ClearAllDlg();

	ImeEditBox::Uninitialize();

	ClearAllTimer();

	LoaderMgr::OnDestroyDevice();
}

void Game::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_keyboard->Capture();

	m_mouse->Capture();

	if(cs = CurrentState())
		cs->OnFrameMove(fTime, fElapsedTime);

	TimerMgr::OnFrameMove(fTime, fElapsedTime);
}

void Game::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	// 当状态切换时发生异常会导致新状态没有被创建，所以有必要判断之
	if(cs = CurrentState())
		cs->OnFrameRender(pd3dDevice, fTime, fElapsedTime);
	else
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,45,50,170), 1.0f, 0));

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		UIRender::Begin(pd3dDevice);

		DialogMgr::Draw(pd3dDevice, fTime, fElapsedTime);

		m_console->Draw(pd3dDevice, fElapsedTime);

		_ASSERT(m_font);

		Matrix4 View, Proj;
		D3DVIEWPORT9 vp;
		pd3dDevice->GetViewport(&vp);
		UIRender::BuildPerspectiveMatrices(
			D3DXToRadian(75.0f), (float)vp.Width, (float)vp.Height, View, Proj);
		V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Matrix4::identity));
		V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&View));
		V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&Proj));
		//m_font->DrawString(DXUTGetFrameStats(DXUTIsVsyncEnabled()),
		//	Rectangle::LeftTop(5,5,500,10), D3DCOLOR_ARGB(255,255,255,0), Font::AlignLeftTop);
		//m_font->DrawString(DXUTGetDeviceStats(),
		//	Rectangle::LeftTop(5,5 + (float)m_font->m_LineHeight,500,10), D3DCOLOR_ARGB(255,255,255,0), Font::AlignLeftTop);
		m_font->DrawString(m_strFPS, Rectangle::LeftTop(5,5,500,10), D3DCOLOR_ARGB(255,255,255,0));

		UIRender::End(pd3dDevice);

		V(pd3dDevice->EndScene());
	}
}

LRESULT Game::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	if(m_console && uMsg == WM_CHAR && (WCHAR)wParam == L'`')
	{
		m_console->SetVisible(!m_console->GetVisible());
		*pbNoFurtherProcessing = true;
		return 0;
	}

	if(m_console && (*pbNoFurtherProcessing = m_console->MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if((*pbNoFurtherProcessing = DialogMgr::MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if((cs = CurrentState()) &&
		(FAILED(hr = cs->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)) || *pbNoFurtherProcessing))
	{
		return hr;
	}

	return 0;
}

bool Game::ExecuteCode(const char * code)
{
	try
	{
		m_lua->executeCode(code);
		return true;
	}
	catch(const std::runtime_error & e)
	{
		if(!m_panel)
			THROW_CUSEXCEPTION(e.what());

		AddLine(L"");
		puts(ms2ws(e.what()));

		m_console->SetVisible(true);
	}
	return false;
}
