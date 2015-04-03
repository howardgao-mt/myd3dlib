#pragma once

#include "Console.h"
#include "../myd3dbox/Component/PhysXContext.h"
#include "../myd3dbox/Component/RenderPipeline.h"
#include "../myd3dbox/Component/RenderComponent.h"
#include "../myd3dbox/Component/ComponentResMgr.h"

class EffectUIRender
	: public my::UIRender
{
public:
	my::EffectPtr m_UIEffect;

	UINT m_Passes;

public:
	EffectUIRender(IDirect3DDevice9 * pd3dDevice, my::EffectPtr effect)
		: UIRender(pd3dDevice)
		, m_UIEffect(effect)
		, m_Passes(0)
	{
		_ASSERT(m_UIEffect);
	}

	virtual void Begin(void);

	virtual void End(void);

	virtual void SetWorld(const my::Matrix4 & World);

	virtual void SetViewProj(const my::Matrix4 & ViewProj);

	virtual void SetTexture(const my::BaseTexturePtr & Texture);

	virtual void DrawVertexList(void);
};

class Game
	: public my::DxutApp
	, public my::TimerMgr
	, public my::DialogMgr
	, public my::InputMgr
	, public ComponentResMgr
	, public RenderPipeline
	, public PhysXContext
	, public PhysXSceneContext
	, public my::ParallelTaskManager
	, public my::DrawHelper
{
public:
	my::LuaContextPtr m_lua;

	typedef std::map<int, std::wstring> ScrInfoType;

	ScrInfoType m_ScrInfos;

	std::wstring m_LastErrorStr;

	my::UIRenderPtr m_UIRender;

	typedef boost::tuple<RenderPipeline::MeshType, RenderPipeline::DrawStage, bool, const Material *> ShaderCacheKey;

	typedef boost::unordered_map<ShaderCacheKey, my::EffectPtr> ShaderCacheMap;

	ShaderCacheMap m_ShaderCache;

	my::EffectPtr m_SimpleSample;

	my::FontPtr m_Font;

	ConsolePtr m_Console;

	my::BaseTexturePtr m_WhiteTex;

	my::BaseTexturePtr m_TexChecker;

	//my::OctRootPtr m_OctScene;

	my::CameraPtr m_Camera;

public:
	Game(void);

	virtual ~Game(void);

	static Game & getSingleton(void)
	{
		return *getSingletonPtr();
	}

	static Game * getSingletonPtr(void)
	{
		return static_cast<Game *>(DxutApp::getSingletonPtr());
	}

	virtual bool IsDeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed);

	virtual bool ModifyDeviceSettings(
		DXUTD3D9DeviceSettings * pDeviceSettings);

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual void OnLostDevice(void);

	virtual void OnDestroyDevice(void);

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	virtual void OnUIRender(
		my::UIRender * ui_render,
		double fTime,
		float fElapsedTime);

	virtual void OnFrameTick(
		double fTime,
		float fElapsedTime);

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing);

	virtual void OnResourceFailed(const std::basic_string<TCHAR> & error_str);

	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));

	void puts(const std::wstring & str);

	bool ExecuteCode(const char * code) throw();

	void OnShaderLoaded(my::DeviceRelatedObjectBasePtr res, ShaderCacheKey key);

	virtual my::Effect * QueryShader(RenderPipeline::MeshType mesh_type, RenderPipeline::DrawStage draw_stage, bool bInstance, const Material * material);

	//void LoadTriangleMeshAsync(const std::string & path, const my::ResourceCallback & callback);

	//PhysXTriangleMeshPtr LoadTriangleMesh(const std::string & path);

	//void LoadClothFabricAsync(const std::string & path, const my::ResourceCallback & callback);

	//PhysXClothFabricPtr LoadClothFabric(const std::string & path);
};
