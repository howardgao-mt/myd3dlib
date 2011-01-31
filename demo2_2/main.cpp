
#include <atlbase.h>
#include <atlstr.h>
#include <DXUT.h>
#include <DXUTgui.h>
#include <SDKmisc.h>
#include <DXUTSettingsDlg.h>

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

CDXUTDialogResourceManager		g_DialogResourceMgr;
CD3DSettingsDlg					g_SettingsDlg;
CDXUTDialog						g_HUD;
CComPtr<ID3DXFont>				g_Font9;
CComPtr<ID3DXSprite>			g_Sprite9;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------

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

	//// 至少要支持ps2.0，这还是要看实际使用情况
	//if(pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
	//{
	//	return false;
	//}
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
}

// ------------------------------------------------------------------------------------------
// OnFrameMove
// ------------------------------------------------------------------------------------------

void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void * pUserContext)
{
	// 在这里更新场景
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

	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0));

	if(g_SettingsDlg.IsActive())
	{
		g_SettingsDlg.OnRender(fElapsedTime);
		return;
	}

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
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
		DXUTToggleFullScreen();
		break;

	case IDC_TOGGLEREF:
		DXUTToggleREF();
		break;

	case IDC_CHANGEDEVICE:
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

	if(g_SettingsDlg.IsActive())
	{
		g_SettingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	*pbNoFurtherProcessing = g_HUD.MsgProc(hWnd, uMsg, wParam, lParam);
	if(*pbNoFurtherProcessing)
	{
		return 0;
	}

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

	// 启动DXUT
	DXUTInit(true, true, NULL);
	DXUTSetCursorSettings(true, true);
	DXUTCreateWindow(L"demo2_2");
	DXUTCreateDevice(true, 800, 600);
	DXUTMainLoop();

	return DXUTGetExitCode();
}
