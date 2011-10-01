﻿
#include <mySingleton.h>
#include <myDxutApp.h>
#include <DXUTCamera.h>
#include <myException.h>
#include <myResource.h>
#include <myMesh.h>
#include <myTexture.h>
#include <myEffect.h>
#include <libc.h>
#include <myFont.h>
//#include <atltypes.h>

// ------------------------------------------------------------------------------------------
// MyDemo
// ------------------------------------------------------------------------------------------

class MyDemo : public my::DxutApp
{
protected:
	CModelViewerCamera m_camera;

	my::EffectPtr m_effect;

	CComPtr<ID3DXMesh> m_mesh;

	my::TexturePtr m_texture;

	static const unsigned int SHADOWMAP_SIZE = 1024;

	my::TexturePtr m_shadowMapRT;

	CComPtr<IDirect3DSurface9> m_shadowMapDS;

	my::FontPtr m_font;

	my::SpritePtr m_sprite;

	my::PixelShaderPtr m_ps;

	bool IsD3D9DeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed)
	{
		if(!DxutApp::IsD3D9DeviceAcceptable(
			pCaps, AdapterFormat, BackBufferFormat, bWindowed))
		{
			return false;
		}

		//// 判断是否支持32位浮点贴图
		//IDirect3D9 * pD3D = DXUTGetD3D9Object();
		//if(FAILED(pD3D->CheckDeviceFormat(
		//	pCaps->AdapterOrdinal, pCaps->DeviceType, AdapterFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_CUBETEXTURE, D3DFMT_R32F)))
		//{
		//	return false;
		//}

		return true;
	}

	void OnInit(void)
	{
		DxutApp::OnInit();

		// 初始化资源管理器收索路径
		my::ResourceMgr::getSingleton().RegisterFileDir(_T("."));
		my::ResourceMgr::getSingleton().RegisterFileDir(_T("..\\..\\Common\\medias"));
		my::ResourceMgr::getSingleton().RegisterZipArchive(_T("Data.zip"));
	}

	HRESULT OnD3D9CreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hres;
		if(FAILED(hres = DxutApp::OnD3D9CreateDevice(
			pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hres;
		}

		// 初始化相机
		D3DXVECTOR3 vecEye(0.0f, 0.0f, 50.0f);
		D3DXVECTOR3 vecAt(0.0f, 0.0f, -0.0f);
		m_camera.SetViewParams(&vecEye, &vecAt);
		m_camera.SetModelCenter(D3DXVECTOR3(0.0f, 15.0f, 0.0f));

		// 读取D3DX Effect文件
		my::CachePtr cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("SimpleSample.fx")));
		m_effect = my::Effect::CreateEffect(pd3dDevice, &(*cache)[0], cache->size());

		// 从资源管理器中读出模型文件
		DWORD dwNumSubMeshes;
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("jack_hres_all.mesh.xml")));
		my::LoadMeshFromOgreMesh(std::string((char *)&(*cache)[0], cache->size()), pd3dDevice, &dwNumSubMeshes, &m_mesh);

		// 创建贴图
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("jack_texture.jpg")));
		m_texture = my::Texture::CreateTextureFromFileInMemory(pd3dDevice, &(*cache)[0], cache->size());

		// 读取字体文件
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("wqy-microhei.ttc")));
		m_font = my::Font::CreateFontFromFileInMemory(pd3dDevice, &(*cache)[0], cache->size(), 32);

		// 创建精灵
		m_sprite = my::Sprite::CreateSprite(pd3dDevice);

		// 创建用以绘制字体的ps
		std::string psData(
			"sampler2D input : register(s0);"
			"float4 Color;"
			"float4 pixelShader(float2 uv : TEXCOORD) : COLOR"
			"{"
			"    return float4(Color.r, Color.g, Color.b, Color.a * tex2D(input, uv).a);"
			"}");
		m_ps = my::PixelShader::CreatePixelShader(pd3dDevice, &psData[0], psData.length() + 1, "pixelShader", "ps_2_0");

		return S_OK;
	}

	HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hres;
		if(FAILED(hres = DxutApp::OnD3D9ResetDevice(
			pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hres;
		}

		// 重新设置相机的投影
		float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
		m_camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f);
		m_camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

		// 创建用于shadow map的render target，使用D3DXCreateTexture可以为不支持设备创建兼容贴图
		m_shadowMapRT = my::Texture::CreateAdjustedTexture(
			pd3dDevice,
			SHADOWMAP_SIZE,
			SHADOWMAP_SIZE,
			1,
			D3DUSAGE_RENDERTARGET,
			D3DFMT_R32F,
			D3DPOOL_DEFAULT);

		// 创建用于shadow map的depth scentil
		DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();
		FAILED_THROW_D3DEXCEPTION(pd3dDevice->CreateDepthStencilSurface(
			SHADOWMAP_SIZE,
			SHADOWMAP_SIZE,
			d3dSettings.d3d9.pp.AutoDepthStencilFormat,
			D3DMULTISAMPLE_NONE,
			0,
			TRUE,
			&m_shadowMapDS,
			NULL));

		return S_OK;
	}

	void OnD3D9LostDevice(void)
	{
		DxutApp::OnD3D9LostDevice();

		// 在这里处理在reset中创建的资源
		m_shadowMapRT = my::TexturePtr();
		m_shadowMapDS.Release();
	}

	void OnD3D9DestroyDevice(void)
	{
		DxutApp::OnD3D9DestroyDevice();

		// 在这里销毁在create中创建的资源
		m_mesh.Release();
	}

	void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		DxutApp::OnFrameMove(fTime, fElapsedTime);

		// 在这里更新场景
		m_camera.FrameMove(fElapsedTime);
	}

	void RenderFrame(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		// 获得相机投影矩阵
		my::Matrix4 mWorld = *(my::Matrix4 *)m_camera.GetWorldMatrix();
		my::Matrix4 mProj = *(my::Matrix4 *)m_camera.GetProjMatrix();
		my::Matrix4 mView = *(my::Matrix4 *)m_camera.GetViewMatrix();
		my::Matrix4 mWorldViewProjection = mWorld * mView * mProj;

		// 计算光照的透视变换
		my::Matrix4 mViewLight(my::Matrix4::LookAtLH(
			my::Vector3(0.0f, 0.0f, 50.0f),
			my::Vector3(0.0f, 0.0f, 0.0f),
			my::Vector3(0.0f, 1.0f, 0.0f)));
		my::Matrix4 mProjLight(my::Matrix4::OrthoLH(50, 50, 25, 75));
		my::Matrix4 mWorldViewProjLight = mWorld * mViewLight * mProjLight;

		// 将shadow map作为render target，注意保存恢复原来的render target
		HRESULT hr;
		LPDIRECT3DSURFACE9 pOldRT = NULL;
		V(pd3dDevice->GetRenderTarget(0, &pOldRT));
		LPDIRECT3DSURFACE9 pShadowSurf;
		V(static_cast<IDirect3DTexture9 *>(m_shadowMapRT->m_ptr)->GetSurfaceLevel(0, &pShadowSurf));
		V(pd3dDevice->SetRenderTarget(0, pShadowSurf));
		SAFE_RELEASE(pShadowSurf);
		LPDIRECT3DSURFACE9 pOldDS = NULL;
		V(pd3dDevice->GetDepthStencilSurface(&pOldDS));
		V(pd3dDevice->SetDepthStencilSurface(m_shadowMapDS));
		V(pd3dDevice->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			// 更新d3dx effect变量
			m_effect->SetMatrix("g_mWorldViewProjectionLight", mWorldViewProjLight);
			m_effect->SetTechnique("RenderShadow");

			// 渲染模型的两个部分，注意，头发的部分不要背面剔除
			UINT cPasses = m_effect->Begin();
			for(UINT p = 0; p < cPasses; ++p)
			{
				m_effect->BeginPass(p);
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
				V(m_mesh->DrawSubset(1));
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
				V(m_mesh->DrawSubset(0));
				m_effect->EndPass();
			}
			m_effect->End();

			V(pd3dDevice->EndScene());
		}
		V(pd3dDevice->SetRenderTarget(0, pOldRT));
		V(pd3dDevice->SetDepthStencilSurface(pOldDS));
		SAFE_RELEASE(pOldRT);
		SAFE_RELEASE(pOldDS);

		// 清理缓存背景及depth stencil
		V(pd3dDevice->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 72), 1, 0));

		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			// 更新D3DX Effect值
			m_effect->SetMatrix("g_mWorldViewProjection", mWorldViewProjection);
			m_effect->SetMatrix("g_mWorld", mWorld);
			m_effect->SetFloat("g_fTime", (float)fTime);

			// 所有的mesh使用同一种材质，同一张贴图
			m_effect->SetVector("g_MaterialAmbientColor", my::Vector4(0.27f, 0.27f, 0.27f, 1.0f));
			m_effect->SetVector("g_MaterialDiffuseColor", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
			m_effect->SetTexture("g_MeshTexture", m_texture);
			m_effect->SetFloatArray("g_LightDir", (float *)&my::Vector3(0.0f, 0.0f, -1.0f), 3);
			m_effect->SetVector("g_LightDiffuse", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));

			// 设置阴影贴图，及光源变换
			m_effect->SetTexture("g_ShadowTexture", m_shadowMapRT);
			m_effect->SetMatrix("g_mWorldViewProjectionLight", mWorldViewProjLight);
			m_effect->SetTechnique("RenderScene");

			// 渲染模型的两个部分，注意，头发的部分不要背面剔除
			UINT cPasses = m_effect->Begin();
			for(UINT p = 0; p < cPasses; ++p)
			{
				m_effect->BeginPass(p);
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
				V(m_mesh->DrawSubset(1));
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
				V(m_mesh->DrawSubset(0));
				m_effect->EndPass();
			}
			m_effect->End();

			// 画一些字体吧
			m_sprite->Begin(D3DXSPRITE_ALPHABLEND);
			V(pd3dDevice->SetPixelShader(m_ps->m_ptr));
			m_ps->SetFloatArray(pd3dDevice, "Color", (FLOAT *)&my::Vector4(1, 1, 0, 1), 4);
			m_font->DrawString(m_sprite, L"tangyin &*^是×&2 =+◎●▲★好人efin\n打完俄方inwe囧寄蓁豟\n嗯，怎么说呢，我可是很勇敢的，我告诉你。\n你们要是再hold不住，哥我就不客气了的说！", my::Rectangle::LeftTop(50, 50, 0, 0));
			m_sprite->End();

			V(pd3dDevice->EndScene());
		}
	}

	LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		LRESULT hres;
		if(FAILED(hres = DxutApp::MsgProc(
			hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)) || *pbNoFurtherProcessing)
		{
			return hres;
		}

		// 相机消息处理
		return m_camera.HandleMessages(hWnd, uMsg, wParam, lParam);
	}
};

// ------------------------------------------------------------------------------------------
// wWinMain
// ------------------------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPWSTR lpCmdLine,
					int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	// 设置crtdbg监视内存泄漏
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return MyDemo().Run(true, 800, 600);
}
