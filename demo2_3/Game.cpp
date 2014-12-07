#include "stdafx.h"
#include "Game.h"

extern void Export2Lua(lua_State * L);

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

void EffectUIRender::Begin(void)
{
	if(m_UIEffect->m_ptr)
	{
		const D3DSURFACE_DESC & desc = DxutApp::getSingleton().GetD3D9BackBufferSurfaceDesc();
		m_UIEffect->SetVector("g_ScreenDim", Vector4((float)desc.Width, (float)desc.Height, 0, 0));
		m_Passes = m_UIEffect->Begin();
	}
}

void EffectUIRender::End(void)
{
	if(m_UIEffect->m_ptr)
	{
		m_UIEffect->End();
		m_Passes = 0;
	}
}

void EffectUIRender::SetWorld(const Matrix4 & World)
{
	if(m_UIEffect->m_ptr)
	{
		m_UIEffect->SetMatrix("g_World", World);
	}
}

void EffectUIRender::SetViewProj(const Matrix4 & ViewProj)
{
	if(m_UIEffect->m_ptr)
	{
		m_UIEffect->SetMatrix("g_ViewProj", ViewProj);
	}
}

void EffectUIRender::SetTexture(const BaseTexturePtr & Texture)
{
	if(m_UIEffect->m_ptr)
	{
		_ASSERT(Game::getSingleton().m_WhiteTex);
		m_UIEffect->SetTexture("g_MeshTexture", Texture ? Texture : Game::getSingleton().m_WhiteTex);
	}
}

void EffectUIRender::DrawVertexList(void)
{
	if(m_UIEffect->m_ptr && vertex_count > 0)
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

void EffectEmitterInstance::Begin(void)
{
	if (m_ParticleEffect->m_ptr)
	{
		m_Passes = m_ParticleEffect->Begin();
	}
}

void EffectEmitterInstance::End(void)
{
	if (m_ParticleEffect->m_ptr)
	{
		m_ParticleEffect->End();
		m_Passes = 0;
	}
}

void EffectEmitterInstance::SetWorld(const Matrix4 & World)
{
	if (m_ParticleEffect->m_ptr)
	{
		m_ParticleEffect->SetMatrix("g_World", World);
	}
}

void EffectEmitterInstance::SetViewProj(const Matrix4 & ViewProj)
{
	if (m_ParticleEffect->m_ptr)
	{
		m_ParticleEffect->SetMatrix("g_ViewProj", ViewProj);
	}
}

void EffectEmitterInstance::SetTexture(const BaseTexturePtr & Texture)
{
	if (m_ParticleEffect->m_ptr)
	{
		_ASSERT(Game::getSingleton().m_WhiteTex);
		m_ParticleEffect->SetTexture("g_MeshTexture", Texture ? Texture : Game::getSingleton().m_WhiteTex);
	}
}

void EffectEmitterInstance::SetDirection(const Vector3 & Dir, const Vector3 & Up, const Vector3 & Right)
{
	if (m_ParticleEffect->m_ptr)
	{
		m_ParticleEffect->SetVector("g_ParticleDir", Dir);
		m_ParticleEffect->SetVector("g_ParticleUp", Up);
		m_ParticleEffect->SetVector("g_ParticleRight", Right);
	}
}

void EffectEmitterInstance::SetAnimationColumnRow(unsigned char Column, unsigned char Row)
{
	if (m_ParticleEffect->m_ptr)
	{
		m_ParticleEffect->SetFloatArray("g_AnimationColumnRow", &(Vector2((float)Column, (float)Row).x), 2);
	}
}

void EffectEmitterInstance::DrawInstance(DWORD NumInstances)
{
	if(m_ParticleEffect->m_ptr && NumInstances > 0)
	{
		for(UINT p = 0; p < m_Passes; p++)
		{
			m_ParticleEffect->BeginPass(p);
			EmitterInstance::DrawInstance(NumInstances);
			m_ParticleEffect->EndPass();
		}
	}
}

Game::Game(void)
{
	RegisterFileDir("Media");
	RegisterZipDir("Media.zip");
	RegisterFileDir("..\\demo2_3\\Media");
	RegisterZipDir("..\\demo2_3\\Media.zip");

	m_lua.reset(new LuaContext());

	Export2Lua(m_lua->_state);
}

Game::~Game(void)
{
	// ! Must manually call destructors at specific order
	OnDestroyDevice();
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

	return true;
}

HRESULT Game::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	ImeEditBox::Initialize(m_wnd->m_hWnd);

	ImeEditBox::EnableImeSystem(false);

	InputMgr::Create(m_hinst, m_wnd->m_hWnd);

	ParallelTaskManager::StartParallelThread(3);

	if(FAILED(hr = ResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if(!PhysXContext::OnInit())
	{
		THROW_CUSEXCEPTION(_T("PhysXContext::OnInit failed"));
	}

	if(!PhysXSceneContext::OnInit(m_sdk.get(), m_CpuDispatcher.get()))
	{
		THROW_CUSEXCEPTION(_T("PhysXSceneContext::OnInit failed"));
	}

	m_UIRender.reset(new EffectUIRender(pd3dDevice, LoadEffect("shader/UIEffect.fx", "")));

	m_EmitterInst.reset(new EffectEmitterInstance(LoadEffect("shader/Particle.fx", "")));

	if(FAILED(hr = m_EmitterInst->OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if (!(m_SimpleSample = LoadEffect("shader/SimpleSample.fx", "")))
	{
		THROW_CUSEXCEPTION(m_LastErrorStr);
	}

	if (!(m_SimpleSampleSkel = LoadEffect("shader/SimpleSample.fx", "VS_SKINED_DQ 1")))
	{
		THROW_CUSEXCEPTION(m_LastErrorStr);
	}

	if (!(m_Font = LoadFont("font/wqy-microhei.ttc", 13)))
	{
		THROW_CUSEXCEPTION(m_LastErrorStr);
	}

	m_Console = ConsolePtr(new Console());

	m_Console->SetVisible(false);

	DialogMgr::InsertDlg(m_Console);

	if (!(m_WhiteTex = LoadTexture("texture/white.bmp")))
	{
		THROW_CUSEXCEPTION(m_LastErrorStr);
	}

	m_OctScene.reset(new OctRoot(Vector3(-1000,-1000,-1000), Vector3(1000,1000,1000), 1.1f));

	m_Camera.reset(new Camera(D3DXToRadian(75), 1.333333f, 0.1f, 3000.0f));

	AddLine(L"Game::OnCreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	return S_OK;
}

HRESULT Game::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	AddLine(L"Game::OnResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	if(FAILED(hr = ResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if(FAILED(hr = m_EmitterInst->OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	Vector2 vp(600 * (float)pBackBufferSurfaceDesc->Width / pBackBufferSurfaceDesc->Height, 600);

	DialogMgr::SetDlgViewport(vp, D3DXToRadian(75.0f));

	m_Font->SetScale(Vector2(pBackBufferSurfaceDesc->Width / vp.x, pBackBufferSurfaceDesc->Height / vp.y));

	if(m_Camera->EventAlign)
	{
		m_Camera->EventAlign(&EventArgs());
	}

	return S_OK;
}

void Game::OnLostDevice(void)
{
	AddLine(L"Game::OnLostDevice", D3DCOLOR_ARGB(255,255,255,0));

	m_EmitterInst->OnLostDevice();

	ResourceMgr::OnLostDevice();
}

void Game::OnDestroyDevice(void)
{
	AddLine(L"Game::OnDestroyDevice", D3DCOLOR_ARGB(255,255,255,0));

	ParallelTaskManager::StopParallelThread();

	ExecuteCode("collectgarbage(\"collect\")");

	m_Camera.reset();

	m_OctScene.reset();

	m_Console.reset();

	RemoveAllDlg();

	m_SimpleSample.reset();

	m_SimpleSampleSkel.reset();

	m_ShaderCache.clear();

	m_EmitterInst.reset();

	m_UIRender.reset();

	RemoveAllTimer();

	PhysXSceneContext::OnShutdown();

	PhysXContext::OnShutdown();

	ResourceMgr::OnDestroyDevice();

	InputMgr::Destroy();

	ImeEditBox::Uninitialize();
}

void Game::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	InputMgr::Update(fTime, fElapsedTime);

	ResourceMgr::CheckRequests();

	//m_Camera->OnFrameMove(fTime, fElapsedTime);

	TimerMgr::OnFrameMove(fTime, fElapsedTime);

	EmitterMgr::Update(fTime, fElapsedTime);
}

void Game::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
	pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);

	m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);

	struct QueryCallbackFunc
	{
		Game & device;
		QueryCallbackFunc(Game & _device)
			: device(_device)
		{
		}

		void operator() (AABBComponent * cmp, IntersectionTests::IntersectionType type)
		{
			MeshComponent * mesh_cmp = static_cast<MeshComponent *>(cmp);
			device.DrawMesh(mesh_cmp, 0);
		}
	};

	Frustum frustum(Frustum::ExtractMatrix(m_Camera->m_ViewProj));
	m_OctScene->QueryComponent(frustum, QueryCallbackFunc(*this));

	DrawHelper::EndLine(m_d3dDevice, Matrix4::identity);

	m_EmitterInst->Begin();

	EmitterMgr::Draw(m_EmitterInst.get(), m_Camera->m_ViewProj, m_Camera->m_View, fTime, fElapsedTime);

	m_EmitterInst->End();

	m_UIRender->Begin();

	m_UIRender->SetWorld(Matrix4::identity);

	m_UIRender->SetViewProj(DialogMgr::m_ViewProj);

	OnUIRender(m_UIRender.get(), fTime, fElapsedTime);

	m_UIRender->End();
}

void Game::OnUIRender(
	my::UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	DialogMgr::Draw(m_UIRender.get(), fTime, fElapsedTime);

	_ASSERT(m_Font);

	m_UIRender->SetWorld(Matrix4::identity);

	ScrInfoType::const_iterator info_iter = m_ScrInfos.begin();
	for (int y = 5; info_iter != m_ScrInfos.end(); info_iter++, y += m_Font->m_LineHeight)
	{
		m_Font->DrawString(m_UIRender.get(), info_iter->second.c_str(), Rectangle::LeftTop(5,(float)y,500,10), D3DCOLOR_ARGB(255,255,255,0));
	}
}

void Game::OnFrameTick(
	double fTime,
	float fElapsedTime)
{
	DrawHelper::BeginLine();

	PhysXSceneContext::PushRenderBuffer(this);

	OnFrameMove(fTime, fElapsedTime);

	PhysXSceneContext::OnTickPreRender(fElapsedTime);

	ParallelTaskManager::DoAllParallelTasks();

	// ! Ogre & Apex模型都是顺时针，右手系应该是逆时针
	V(m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));

	V(m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,45,50,170), 1.0f, 0));

	if(SUCCEEDED(hr = m_d3dDevice->BeginScene()))
	{
		OnFrameRender(m_d3dDevice, fTime, fElapsedTime);

		V(m_d3dDevice->EndScene());
	}

	Present(NULL,NULL,NULL,NULL);

	PhysXSceneContext::OnTickPostRender(fElapsedTime);
}

LRESULT Game::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	if(m_Console
		&& uMsg == WM_CHAR && (WCHAR)wParam == L'`')
	{
		m_Console->SetVisible(!m_Console->GetVisible());
		*pbNoFurtherProcessing = true;
		return 0;
	}

	if((*pbNoFurtherProcessing = DialogMgr::MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if((*pbNoFurtherProcessing = InputMgr::MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	LRESULT lr;
	if(m_Camera
		&& (lr = m_Camera->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing))
	{
		return lr;
	}
	return 0;
}

static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}

static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}

static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(Game::getSingleton().m_lua->_state, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}
//
//static void l_message (const char *pname, const char *msg) {
//  //if (pname) fprintf(stderr, "%s: ", pname);
//  //fprintf(stderr, "%s\n", msg);
//  //fflush(stderr);
//	Game::getSingleton().AddLine(L"");
//	Game::getSingleton().puts(ms2ws(msg));
//}
//
//static int report (lua_State *L, int status) {
//  if (status && !lua_isnil(L, -1)) {
//    const char *msg = lua_tostring(L, -1);
//    if (msg == NULL) msg = "(error object is not a string)";
//    l_message("aaa", msg);
//    lua_pop(L, 1);
//  }
//  return status;
//}

static int dostring (lua_State *L, const char *s, const char *name) {
  //int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  //return report(L, status);
  return luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
}

void Game::OnResourceFailed(const std::basic_string<TCHAR> & error_str)
{
	_ASSERT(m_Console && m_Console->m_Panel);

	AddLine(error_str, D3DCOLOR_ARGB(255,255,255,255));

	if(m_Console && !m_Console->GetVisible())
	{
		m_Console->SetVisible(true);
	}
}

void Game::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	switch(code)
	{
	case PxErrorCode::eDEBUG_INFO:
		AddLine(ms2ws(str_printf("%s (%d) : info: %s", file, line, message)));
		break;

	case PxErrorCode::eDEBUG_WARNING:
	case PxErrorCode::ePERF_WARNING:
		AddLine(ms2ws(str_printf("%s (%d) : warning: %s", file, line, message)), D3DCOLOR_ARGB(255,255,255,0));
		break;

	default:
		OutputDebugStringA(str_printf("%s (%d) : error: %s\n", file, line, message).c_str());
		AddLine(ms2ws(str_printf("%s, (%d) : error: %s", file, line, message)), D3DCOLOR_ARGB(255,255,0,0));
		break;
	}
}

void Game::AddLine(const std::wstring & str, D3DCOLOR Color)
{
	m_LastErrorStr = str;

	if (m_Console)
	{
		m_Console->m_Panel->AddLine(m_LastErrorStr, Color);
	}
}

void Game::puts(const std::wstring & str)
{
	if (m_Console)
	{
		m_Console->m_Panel->puts(str);
	}
}

bool Game::ExecuteCode(const char * code) throw()
{
	if(dostring(m_lua->_state, code, "Game::ExecuteCode") && !lua_isnil(m_lua->_state, -1))
	{
		std::wstring msg = ms2ws(lua_tostring(m_lua->_state, -1));
		if(msg.empty())
			msg = L"(error object is not a string)";
		lua_pop(m_lua->_state, 1);

		OnResourceFailed(msg);

		return false;
	}
	return true;
}

class TriangleMeshIORequest : public my::IORequest
{
protected:
	std::string m_path;

	Game * m_arc;

	my::CachePtr m_cache;

public:
	TriangleMeshIORequest(const my::ResourceCallback & callback, const std::string & path, Game * arc)
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
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		PxTriangleMesh * ptr = m_arc->CreateTriangleMesh(my::IStreamPtr(new my::MemoryIStream(&(*m_cache)[0], m_cache->size())));
		if (!ptr)
		{
			THROW_CUSEXCEPTION(str_printf(_T("CreateTriangleMesh failed")));
		}
		m_res.reset(new PhysXTriangleMesh(ptr));
	}
};

void Game::LoadTriangleMeshAsync(const std::string & path, const my::ResourceCallback & callback)
{
	LoadResourceAsync(path, my::IORequestPtr(new TriangleMeshIORequest(callback, path, this)), false);
}

PhysXTriangleMeshPtr Game::LoadTriangleMesh(const std::string & path)
{
	return LoadResource<PhysXTriangleMesh>(path, my::IORequestPtr(new TriangleMeshIORequest(my::ResourceCallback(), path, this)));
}

class ClothFabricIORequest : public my::IORequest
{
protected:
	std::string m_path;

	Game * m_arc;

	my::CachePtr m_cache;

public:
	ClothFabricIORequest(const my::ResourceCallback & callback, const std::string & path, Game * arc)
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
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		PxClothFabric * ptr = m_arc->CreateClothFabric(my::IStreamPtr(new my::MemoryIStream(&(*m_cache)[0], m_cache->size())));
		if (!ptr)
		{
			THROW_CUSEXCEPTION(str_printf(_T("CreateClothFabric failed")));
		}
		m_res.reset(new PhysXClothFabric(ptr));
	}
};

void Game::LoadClothFabricAsync(const std::string & path, const my::ResourceCallback & callback)
{
	LoadResourceAsync(path, my::IORequestPtr(new ClothFabricIORequest(callback, path, this)), false);
}

PhysXClothFabricPtr Game::LoadClothFabric(const std::string & path)
{
	return LoadResource<PhysXClothFabric>(path, my::IORequestPtr(new ClothFabricIORequest(my::ResourceCallback(), path, this)));
}

void Game::OnMeshLodMaterialLoaded(my::DeviceRelatedObjectBasePtr res, boost::weak_ptr<MeshLOD> weak_mesh_lod, unsigned int i, CounterPtr counter)
{
	MeshLODPtr mesh_lod = weak_mesh_lod.lock();
	if (mesh_lod)
	{
		mesh_lod->m_Materials[i] = boost::dynamic_pointer_cast<Material>(res);
		if (++counter->first == counter->second)
		{
			mesh_lod->m_IsReady = true;
		}
	}
}

void Game::OnMeshLodMeshLoaded(my::DeviceRelatedObjectBasePtr res, boost::weak_ptr<MeshLOD> weak_mesh_lod)
{
	MeshLODPtr mesh_lod = weak_mesh_lod.lock();
	if (mesh_lod)
	{
		mesh_lod->m_Mesh = boost::dynamic_pointer_cast<OgreMesh>(res);

		if (mesh_lod->m_Mesh)
		{
			mesh_lod->m_Materials.resize(mesh_lod->m_Mesh->m_MaterialNameList.size());
			CounterPtr counter(new Counter(0, (int)mesh_lod->m_Materials.size()));
			for(DWORD i = 0; i < mesh_lod->m_Mesh->m_MaterialNameList.size(); i++)
			{
				LoadMaterialAsync(str_printf("material/%s.xml", mesh_lod->m_Mesh->m_MaterialNameList[i].c_str()), boost::bind(&Game::OnMeshLodMaterialLoaded, this, _1, mesh_lod, i, counter));
			}
		}
	}
}

void Game::LoadMeshLodAsync(MeshLODPtr mesh_lod, const std::string & mesh_path)
{
	LoadMeshAsync(mesh_path, boost::bind(&Game::OnMeshLodMeshLoaded, this, _1, mesh_lod));
}

std::list<MeshComponentPtr> Game::LoadStaticMeshComponentListFromMeshSet(my::OgreMeshSetPtr mesh_set)
{
	std::list<MeshComponentPtr> ret;
	OgreMeshSet::iterator mesh_iter = mesh_set->begin();
	for (; mesh_iter != mesh_set->end(); mesh_iter++)
	{
		MeshComponentPtr mesh_cmp(new StaticMeshComponent());
		mesh_cmp->Min = (*mesh_iter)->m_aabb.Min;
		mesh_cmp->Max = (*mesh_iter)->m_aabb.Max;
		MeshLODPtr mesh_lod(new MeshLOD());
		mesh_lod->m_Mesh = *mesh_iter;

		if (mesh_lod->m_Mesh)
		{
			mesh_lod->m_Materials.resize(mesh_lod->m_Mesh->m_MaterialNameList.size());
			CounterPtr counter(new Counter(0, (int)mesh_lod->m_Materials.size()));
			for(DWORD i = 0; i < mesh_lod->m_Mesh->m_MaterialNameList.size(); i++)
			{
				LoadMaterialAsync(str_printf("material/%s.xml", mesh_lod->m_Mesh->m_MaterialNameList[i].c_str()), boost::bind(&Game::OnMeshLodMaterialLoaded, this, _1, mesh_lod, i, counter));
			}
		}

		mesh_cmp->m_Lod[0] = mesh_lod;
		ret.push_back(mesh_cmp);
	}
	return ret;
}

void Game::PushMeshSetToOctScene(my::OgreMeshSetPtr mesh_set)
{
	std::list<MeshComponentPtr> mesh_cmp_list = LoadStaticMeshComponentListFromMeshSet(mesh_set);
	std::list<MeshComponentPtr>::iterator mesh_cmp_iter = mesh_cmp_list.begin();
	for (; mesh_cmp_iter != mesh_cmp_list.end(); mesh_cmp_iter++)
	{
		m_OctScene->PushComponent(*mesh_cmp_iter);
	}
}

void Game::OnShaderLoaded(my::DeviceRelatedObjectBasePtr res, ShaderKeyType key)
{
	m_ShaderCache.insert(ShaderCacheMap::value_type(key, boost::dynamic_pointer_cast<my::Effect>(res)));
}

static size_t hash_value(const Game::ShaderKeyType & key)
{
	size_t seed = 0;
	boost::hash_combine(seed, key.get<0>());
	boost::hash_combine(seed, key.get<1>());
	boost::hash_combine(seed, key.get<2>());
	return seed;
}

my::Effect * Game::QueryShader(MeshComponent::MeshType mesh_type, DrawStage draw_stage, const my::Material * material)
{
	// ! make sure hash_value(ShaderKeyType ..) is valid
	ShaderKeyType key = boost::make_tuple(mesh_type, draw_stage, material);

	_ASSERT(material);

	ShaderCacheMap::iterator shader_iter = m_ShaderCache.find(key);
	if (shader_iter != m_ShaderCache.end())
	{
		return shader_iter->second.get();
	}

	switch (draw_stage)
	{
	case DrawStageCBuffer:
		{
			std::string macros(mesh_type == MeshComponent::MeshTypeAnimation ? "VS_SKINED_DQ 1" : "");
			std::string path("shader/SimpleSample.fx");
			std::string key_str = ResourceMgr::EffectIORequest::BuildKey(path, macros);
			ResourceCallback callback = boost::bind(&Game::OnShaderLoaded, this, _1, key);
			LoadResourceAsync(key_str, IORequestPtr(new ResourceMgr::EffectIORequest(callback, path, macros, this)), true);
		}
		break;
	}

	return mesh_type == MeshComponent::MeshTypeAnimation ? m_SimpleSampleSkel.get() : m_SimpleSample.get();
}

void Game::DrawMesh(MeshComponent * mesh_cmp, int lod)
{
	_ASSERT(mesh_cmp);

	mesh_cmp->OnPreRender(mesh_cmp->m_MeshType == MeshComponent::MeshTypeAnimation ? m_SimpleSampleSkel.get() : m_SimpleSample.get(), DrawStageCBuffer);

	MeshComponent::MeshLODPtrMap::iterator lod_iter = mesh_cmp->m_Lod.find(lod);
	if (lod_iter != mesh_cmp->m_Lod.end())
	{
		DrawMeshLOD(mesh_cmp->m_MeshType, lod_iter->second.get());
	}
}

void Game::DrawMeshLOD(MeshComponent::MeshType mesh_type, MeshLOD * mesh_lod)
{
	_ASSERT(mesh_lod);

	for (DWORD i = 0; i < mesh_lod->m_Materials.size(); i++)
	{
		_ASSERT(mesh_lod->m_Mesh);

		if (mesh_lod->m_Materials[i])
		{
			my::Effect * shader = QueryShader(mesh_type, DrawStageCBuffer, mesh_lod->m_Materials[i].get());
			_ASSERT(shader);

			mesh_lod->OnPreRender(shader, DrawStageCBuffer, i);

			UINT passes = shader->Begin();
			for (UINT p = 0; p < passes; p++)
			{
				shader->BeginPass(p);
				mesh_lod->m_Mesh->DrawSubset(i);
				shader->EndPass();
			}
			shader->End();
		}
	}
}
