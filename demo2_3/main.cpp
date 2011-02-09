﻿
#include <atlbase.h>
#include <atlstr.h>
#include <vector>
#include <DXUT.h>
#include <DXUTgui.h>
#include <SDKmisc.h>
#include <DXUTSettingsDlg.h>
#include <DXUTCamera.h>
#include "rapidxml.hpp"

// ------------------------------------------------------------------------------------------
// Global variables
// ------------------------------------------------------------------------------------------

CDXUTDialogResourceManager		g_DialogResourceMgr;
CD3DSettingsDlg					g_SettingsDlg;
CDXUTDialog						g_HUD;
CComPtr<ID3DXFont>				g_Font9;
CComPtr<ID3DXSprite>			g_Sprite9;
CComPtr<ID3DXEffect>			g_Effect9;
CModelViewerCamera				g_Camera;

// 自定义渲染资源
CComPtr<ID3DXMesh>				g_Mesh;
std::vector<D3DMATERIAL9>		g_MeshMaterials;
typedef CComPtr<IDirect3DTexture9> IDirect3DTexture9Ptr;
std::vector<IDirect3DTexture9Ptr> g_MeshTextures;

// ------------------------------------------------------------------------------------------
// UI control IDs
// ------------------------------------------------------------------------------------------

#define IDC_TOGGLEFULLSCREEN	1
#define IDC_TOGGLEREF			2
#define IDC_CHANGEDEVICE		3

// ------------------------------------------------------------------------------------------
// IsD3D9DeviceAcceptable
// ------------------------------------------------------------------------------------------

bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9 * pCaps,
									 D3DFORMAT AdapterFormat,
									 D3DFORMAT BackBufferFormat,
									 bool bWindowed,
									 void * pUserContext)
{
	// 跳过不支持alpha blending的后缓存
	IDirect3D9 * pD3D = DXUTGetD3D9Object();
	if(FAILED((pD3D->CheckDeviceFormat(
		pCaps->AdapterOrdinal,
		pCaps->DeviceType,
		AdapterFormat,
		D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE,
		BackBufferFormat))))
	{
		return false;
	}

	// 至少要支持ps2.0，这还是要看实际使用情况
	if(pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
	{
		return false;
	}
	return true;
}

// ------------------------------------------------------------------------------------------
// ModifyDeviceSettings
// ------------------------------------------------------------------------------------------

bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings * pDeviceSettings,
								   void * pUserContext)
{
	// 如果创建一个ref设备（即软件模拟），则应该给出一个警告
	if(DXUT_D3D9_DEVICE == pDeviceSettings->ver
		&& D3DDEVTYPE_REF == pDeviceSettings->d3d9.DeviceType)
	{
		DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
	}

	return true;
}

// ------------------------------------------------------------------------------------------
// LoadMeshFromOgreMesh
// ------------------------------------------------------------------------------------------

template <typename T>
T getMinFromArray(int nStart, int nCount, T * list, T maximize)
{
	T tmp = maximize;
	for(int i = nStart; i < nStart + nCount; i++)
	{
		if(list[i] < tmp)
		{
			tmp = list[i];
		}
	}
	return tmp;
}

template <typename T>
T getMaxFromArray(int nStart, int nCount, T * list, T minimize)
{
	T tmp = minimize;
	for(int i = nStart; i < nStart + nCount; i++)
	{
		if(list[i] > tmp)
		{
			tmp = list[i];
		}
	}
	return tmp;
}

HRESULT LoadMeshFromOgreMesh(LPCWSTR pFilename,
							 LPDIRECT3DDEVICE9 pd3dDevice,
							 DWORD * pNumSubMeshes,
							 LPD3DXMESH * ppMesh)
{
	using namespace rapidxml;

	// 打开指定的文件
	FILE * fp;
	errno_t err = _wfopen_s(&fp, pFilename, L"r");
	if(0 != err)
	{
		return D3DERR_INVALIDCALL;
	}

	// 获取文件长度
	if(0 != fseek(fp, 0, SEEK_END))
	{
		return D3DERR_INVALIDCALL;
	}
	long len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// 分配字符串
	std::string strXml;
	strXml.resize(len + 1);
	strXml.resize(fread_s(&strXml[0], strXml.size(), sizeof(char), len, fp));
	fclose(fp);

	// 分析xml文件
	xml_document<char> doc;
	doc.parse<0>(const_cast<char *>(strXml.c_str()));

	// 获取vertex总数，注意：这里略过解析错误
	xml_node<char> * node_mesh = doc.first_node("mesh");
	xml_node<char> * node_sharedgeometry = node_mesh->first_node("sharedgeometry");
	xml_attribute<char> * attr_vertexcount = node_sharedgeometry->first_attribute("vertexcount");
	int vertexcount = atoi(attr_vertexcount->value());

	// 获取submeshes的face总数
	xml_node<char> * node_submeshes = node_mesh->first_node("submeshes");
	int facecount = 0;
	xml_node<char> * node_submesh = node_submeshes->first_node("submesh");
	for(; node_submesh != NULL; node_submesh = node_submesh->next_sibling())
	{
		xml_node<char> * node_faces = node_submesh->first_node("faces");
		xml_attribute<char> * attr_count = node_faces->first_attribute("count");
		facecount += atoi(attr_count->value());
	}

	// 自定义顶点格式
	struct CUSTOMVERTEX
	{
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
		FLOAT tu;
		FLOAT tv;
	};
	const DWORD D3DFVF_CUSTOMVERTEX = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

	// 哈哈，数据收齐，可以创建D3DX Mesh
	CComPtr<ID3DXMesh> ret_mesh;
	HRESULT hres;
	if(FAILED(hres = D3DXCreateMeshFVF(
		facecount, vertexcount, D3DXMESH_SYSTEMMEM, D3DFVF_CUSTOMVERTEX, pd3dDevice, &ret_mesh)))
	{
		return hres;
	}

	// 获取顶点缓存
	CUSTOMVERTEX * pVertices;
	if(FAILED(hres = ret_mesh->LockVertexBuffer(0, (VOID **)&pVertices)))
	{
		return hres;
	}

	// 赋值顶点数据，由于是D3DXMESH_SYSTEMMEM，所以这个过程应该是内存到显存的传输（AGP或PCIE接口）
	xml_node<char> * node_vertexbuffer = node_sharedgeometry->first_node("vertexbuffer");
	xml_node<char> * node_vertex = node_vertexbuffer->first_node("vertex");
	for(int i = 0; i < vertexcount && node_vertex != NULL; i++, node_vertex = node_vertex->next_sibling())
	{
		xml_node<char> * node_position = node_vertex->first_node("position");
		xml_node<char> * node_normal = node_vertex->first_node("normal");
		xml_node<char> * node_texcoord = node_vertex->first_node("texcoord");

		// 注意：x轴是反的，因为原始模型是从Maya导出的，其内部使用的是opengl右手系
		pVertices[i].position.x = -atof(node_position->first_attribute("x")->value());
		pVertices[i].position.y = atof(node_position->first_attribute("y")->value());
		pVertices[i].position.z = atof(node_position->first_attribute("z")->value());

		pVertices[i].normal.x = -atof(node_normal->first_attribute("x")->value());
		pVertices[i].normal.y = atof(node_normal->first_attribute("y")->value());
		pVertices[i].normal.z = atof(node_normal->first_attribute("z")->value());

		pVertices[i].tu = atof(node_texcoord->first_attribute("u")->value());
		pVertices[i].tv = atof(node_texcoord->first_attribute("v")->value());
	}

	// 解锁顶点缓存
	ret_mesh->UnlockVertexBuffer();

	// 获取索引缓存
	WORD * pIndices;
	if(FAILED(hres = ret_mesh->LockIndexBuffer(0, (VOID **)&pIndices)))
	{
		return hres;
	}

	// 赋值索引数据
	std::vector<D3DXATTRIBUTERANGE> d3dxAttrList;
	int face_i = 0;
	node_submesh = node_submeshes->first_node("submesh");
	for(int i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), i++)
	{
		xml_node<char> * node_faces = node_submesh->first_node("faces");
		D3DXATTRIBUTERANGE attr;
		attr.AttribId = i;
		attr.FaceStart = face_i;
		attr.FaceCount = atoi(node_faces->first_attribute("count")->value());
		xml_node<char> * node_face = node_faces->first_node("face");
		for(; face_i < facecount && node_face != NULL; node_face = node_face->next_sibling())
		{
			// 同理，三角形也应当是右手系转为左手系
			pIndices[i++] = atoi(node_face->first_attribute("v1")->value());
			pIndices[i++] = atoi(node_face->first_attribute("v3")->value());
			pIndices[i++] = atoi(node_face->first_attribute("v2")->value());
		}
		attr.VertexStart = getMinFromArray<WORD>(face_i, attr.FaceCount * 3, pIndices, USHRT_MAX);
		attr.VertexCount = getMaxFromArray<WORD>(face_i, attr.FaceCount * 3, pIndices, 0) - attr.VertexStart;
		d3dxAttrList.push_back(attr);
	}

	// 解锁索引缓存
	ret_mesh->UnlockIndexBuffer();

	// 设置D3DX Mesh Attribute Table
	if(FAILED(hres = ret_mesh->SetAttributeTable(&d3dxAttrList[0], d3dxAttrList.size())))
	{
		return hres;
	}

	// 保存pNumSubMeshes及ppMesh
	_ASSERT(NULL != pNumSubMeshes);
	*pNumSubMeshes = d3dxAttrList.size();
	*ppMesh = ret_mesh.Detach();
	return D3D_OK;
}

// ------------------------------------------------------------------------------------------
// OnD3D9CreateDevice
// ------------------------------------------------------------------------------------------

HRESULT CALLBACK OnD3D9CreateDevice(IDirect3DDevice9 * pd3dDevice,
									const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
									void * pUserContext)
{
	// 在这里创建d3d9资源，但这些资源应该不受device reset限制的
	HRESULT hr;
	V_RETURN(g_DialogResourceMgr.OnD3D9CreateDevice(pd3dDevice));
	V_RETURN(g_SettingsDlg.OnD3D9CreateDevice(pd3dDevice));
	V_RETURN(D3DXCreateFont(
		pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &g_Font9));
	V_RETURN(D3DXCreateSprite(pd3dDevice, &g_Sprite9));

	// 读取D3DX Effect文件
	V_RETURN(D3DXCreateEffectFromFile(
		pd3dDevice, L"SimpleSample.fx", NULL, NULL, D3DXFX_NOT_CLONEABLE, NULL, &g_Effect9, NULL));
	V_RETURN(g_Effect9->SetTechnique("RenderScene"));

	// 初始化相机
	D3DXVECTOR3 vecEye(0.0f, 0.0f, 50.0f);
	D3DXVECTOR3 vecAt(0.0f, 0.0f, -0.0f);
	g_Camera.SetViewParams(&vecEye, &vecAt);
	g_Camera.SetModelCenter(D3DXVECTOR3(0.0f, 15.0f, 0.0f));

	//// 读取D3DX Mesh
	//CComPtr<ID3DXBuffer> d3dxMaterialBuf;
	DWORD dwNumMaterials;
	//V_RETURN(D3DXLoadMeshFromX(
	//	L"Tiger.x", D3DXMESH_SYSTEMMEM, pd3dDevice, NULL, &d3dxMaterialBuf, NULL, &dwNumMaterials, &g_Mesh));

	// 从ogre mesh文件读取到D3DX Mesh
	V_RETURN(LoadMeshFromOgreMesh(L"jack_hres.mesh.xml", pd3dDevice, &dwNumMaterials, &g_Mesh));

	//D3DXMATERIAL * d3dxMaterials = (D3DXMATERIAL *)d3dxMaterialBuf->GetBufferPointer();
	g_MeshMaterials.resize(dwNumMaterials);
	g_MeshTextures.resize(dwNumMaterials);
	for(DWORD i = 0; i < dwNumMaterials; i++)
	{
		//// 拷贝材质信息，及设置环境光反射属性
		//g_MeshMaterials[i] = d3dxMaterials[i].MatD3D;
		//g_MeshMaterials[i].Ambient = g_MeshMaterials[i].Diffuse;
		g_MeshMaterials[i].Ambient = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f);
		g_MeshMaterials[i].Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

		// 创建贴图
		//if(d3dxMaterials[i].pTextureFilename != NULL
		//	&& lstrlenA(d3dxMaterials[i].pTextureFilename) > 0)
		//{
		//	V_RETURN(D3DXCreateTextureFromFileA(pd3dDevice, d3dxMaterials[i].pTextureFilename, &g_MeshTextures[i]));
		//}
		V_RETURN(D3DXCreateTextureFromFileA(pd3dDevice, "jack_texture.jpg", &g_MeshTextures[i]));
	}
	return S_OK;
}

// ------------------------------------------------------------------------------------------
// OnD3D9ResetDevice
// ------------------------------------------------------------------------------------------

HRESULT CALLBACK OnD3D9ResetDevice(IDirect3DDevice9 * pd3dDevice,
								   const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
								   void * pUserContext)
{
	// 在这里创建d3d9资源，但这些资源将受到device reset限制
	HRESULT hr;
	V_RETURN(g_DialogResourceMgr.OnD3D9ResetDevice());
	V_RETURN(g_SettingsDlg.OnD3D9ResetDevice());
	V_RETURN(g_Font9->OnResetDevice());
	V_RETURN(g_Sprite9->OnResetDevice());
	V_RETURN(g_Effect9->OnResetDevice());

	// 重新设置相机的投影
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f);
	g_Camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	// 更新HUD坐标
	g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	g_HUD.SetSize(170, 170);
	return S_OK;
}

// ------------------------------------------------------------------------------------------
// OnD3D9LostDevice
// ------------------------------------------------------------------------------------------

void CALLBACK OnD3D9LostDevice(void * pUserContext)
{
	// 在这里处理在reset中创建的资源
	g_DialogResourceMgr.OnD3D9LostDevice();
	g_SettingsDlg.OnD3D9LostDevice();
	g_Font9->OnLostDevice();
	g_Sprite9->OnLostDevice();
	g_Effect9->OnLostDevice();
}

// ------------------------------------------------------------------------------------------
// wWinMain
// ------------------------------------------------------------------------------------------

void CALLBACK OnD3D9DestroyDevice(void * pUserContext)
{
	// 在这里销毁在create中创建的资源
	g_DialogResourceMgr.OnD3D9DestroyDevice();
	g_SettingsDlg.OnD3D9DestroyDevice();
	g_Font9.Release();
	g_Sprite9.Release();
	g_Effect9.Release();
	g_Mesh.Release();
	g_MeshTextures.clear();
}

// ------------------------------------------------------------------------------------------
// OnFrameMove
// ------------------------------------------------------------------------------------------

void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void * pUserContext)
{
	// 在这里更新场景
	g_Camera.FrameMove(fElapsedTime);
}

// ------------------------------------------------------------------------------------------
// OnD3D9FrameRender
// ------------------------------------------------------------------------------------------

void CALLBACK OnD3D9FrameRender(IDirect3DDevice9 * pd3dDevice,
								double fTime,
								float fElapsedTime,
								void * pUserContext)
{
	// 在这里渲染场景
	HRESULT hr;

	// 清理back buffer
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 66, 75, 121), 1.0f, 0));

	// 如果是设置模式，则渲染设置对话框，然后跳过其他渲染
	if(g_SettingsDlg.IsActive())
	{
		g_SettingsDlg.OnRender(fElapsedTime);
		return;
	}

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		// 获得相机投影矩阵
		D3DXMATRIXA16 mWorld = *g_Camera.GetWorldMatrix();
		D3DXMATRIXA16 mProj = *g_Camera.GetProjMatrix();
		D3DXMATRIXA16 mView = *g_Camera.GetViewMatrix();
		D3DXMATRIXA16 mWorldViewProjection = mWorld * mView * mProj;

		// 更新D3DX Effect值
		V(g_Effect9->SetMatrix("g_mWorldViewProjection", &mWorldViewProjection));
		V(g_Effect9->SetMatrix("g_mWorld", &mWorld));
		V(g_Effect9->SetFloat("g_fTime", (float)fTime));

		// 渲染D3DX Mesh
		for(DWORD i = 0; i < g_MeshMaterials.size(); i++)
		{
			V(g_Effect9->SetVector("g_MaterialAmbientColor", (D3DXVECTOR4 *)&g_MeshMaterials[i].Ambient));
			V(g_Effect9->SetVector("g_MaterialDiffuseColor", (D3DXVECTOR4 *)&g_MeshMaterials[i].Diffuse));
			V(g_Effect9->SetTexture("g_MeshTexture", g_MeshTextures[i]));
			g_Effect9->SetFloatArray("g_LightDir", (float *)&D3DXVECTOR3(0.0f, 0.0f, -1.0f), 3);
			g_Effect9->SetVector("g_LightDiffuse", &D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));

			UINT cPasses;
			V(g_Effect9->Begin(&cPasses, 0));
			for(UINT p = 0; p < cPasses; ++p)
			{
				V(g_Effect9->BeginPass(p));
				V(g_Mesh->DrawSubset(i));
				V(g_Effect9->EndPass());
			}
			V(g_Effect9->End());
		}

		// 输出渲染设备信息
		CDXUTTextHelper txtHelper(g_Font9, g_Sprite9, 15);
		txtHelper.Begin();
		txtHelper.SetInsertionPos(5, 5);
		txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		txtHelper.DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
		txtHelper.DrawTextLine(DXUTGetDeviceStats());
		txtHelper.End();
		V(g_HUD.OnRender(fElapsedTime));
		V(pd3dDevice->EndScene());
	}
}

// ------------------------------------------------------------------------------------------
// OnGUIEvent
// ------------------------------------------------------------------------------------------

void CALLBACK OnGUIEvent(UINT nEvent,
						 int nControlID,
						 CDXUTControl * pControl,
						 void * pUserContext)
{
	// 在这里处理ui事件
	switch(nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		// 切换全屏窗口模式
		DXUTToggleFullScreen();
		break;

	case IDC_TOGGLEREF:
		// 软硬件渲染模式
		DXUTToggleREF();
		break;

	case IDC_CHANGEDEVICE:
		// 显示设置对话框
		g_SettingsDlg.SetActive(!g_SettingsDlg.IsActive());
		break;
	}
}

// ------------------------------------------------------------------------------------------
// MsgProc
// ------------------------------------------------------------------------------------------

LRESULT CALLBACK MsgProc(HWND hWnd,
						 UINT uMsg,
						 WPARAM wParam,
						 LPARAM lParam,
						 bool * pbNoFurtherProcessing,
						 void * pUserContext)
{
	// 在这里进行消息处理
	*pbNoFurtherProcessing = g_DialogResourceMgr.MsgProc(hWnd, uMsg, wParam, lParam);
	if(*pbNoFurtherProcessing)
	{
		return 0;
	}

	// 如果当前是设置模式，则只将消息发送到设置对话框
	if(g_SettingsDlg.IsActive())
	{
		g_SettingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	// HUD处理消息
	*pbNoFurtherProcessing = g_HUD.MsgProc(hWnd, uMsg, wParam, lParam);
	if(*pbNoFurtherProcessing)
	{
		return 0;
	}

	// 相机消息处理
	g_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);

	return 0;
}

// ------------------------------------------------------------------------------------------
// OnKeyboard
// ------------------------------------------------------------------------------------------

void CALLBACK OnKeyboard(UINT nChar,
						 bool bKeyDown,
						 bool bAltDown,
						 void * pUserContext)
{
	// 在这里进行键盘事件处理
	// 更具DXUT的源代码可以看出，如果要阻止Escape推出窗口，应当
	// 在MsgProc处理WM_KEYDOWN中的VK_ESCAPE，并给出bNoFurtherProcessing结果即可
}

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

	// 设置DXUT资源管理的回调函数
	DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackD3D9DeviceCreated(OnD3D9CreateDevice);
	DXUTSetCallbackD3D9DeviceReset(OnD3D9ResetDevice);
	DXUTSetCallbackD3D9DeviceLost(OnD3D9LostDevice);
	DXUTSetCallbackD3D9DeviceDestroyed(OnD3D9DestroyDevice);

	// 设置渲染的回调函数
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackD3D9FrameRender(OnD3D9FrameRender);

	// 设置消息回调函数
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(OnKeyboard);

	// 全局初始化工作
	g_SettingsDlg.Init(&g_DialogResourceMgr);
	g_HUD.Init(&g_DialogResourceMgr);
	g_HUD.SetCallback(OnGUIEvent);
	int nY = 10;
	g_HUD.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, nY, 125, 22);
	g_HUD.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", 35, nY += 24, 125, 22, VK_F3);
	g_HUD.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 35, nY += 24, 125, 22, VK_F2);

	// 设置相机操作按键
    g_Camera.SetButtonMasks(MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_RIGHT_BUTTON);

	// 启动DXUT
	DXUTInit(true, true, NULL);
	DXUTSetCursorSettings(true, true);
	WCHAR szPath[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);
	DXUTCreateWindow(szPath);
	DXUTCreateDevice(true, 800, 600);
	DXUTMainLoop();

	// 获取并返回DXUT退出值
	return DXUTGetExitCode();
}
