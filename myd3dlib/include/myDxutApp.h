
#pragma once

#include "mySingleton.h"
#include <atlbase.h>
#include <DXUT.h>
#include <DXUTgui.h>
#include <DXUTSettingsDlg.h>
#include <set>

namespace my
{
	class DeviceRelatedObjectBase
	{
	public:
		DeviceRelatedObjectBase(void);

		virtual ~DeviceRelatedObjectBase(void);

		virtual HRESULT OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc) = 0;

		virtual void OnD3D9LostDevice(void) = 0;

		virtual void OnD3D9DestroyDevice(void) = 0;
	};

	class DeviceRelatedObjectBaseSet
		: public std::set<DeviceRelatedObjectBase *>
		, public Singleton<DeviceRelatedObjectBaseSet>
	{
	public:
		HRESULT OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		void OnD3D9LostDevice(void);

		void OnD3D9DestroyDevice(void);
	};

	template <class DrivedClass> 
	class DeviceRelatedObject
		: public DeviceRelatedObjectBase
	{
	public:
		DrivedClass * m_ptr;

	public:
		DeviceRelatedObject(DrivedClass * ptr)
			: m_ptr(ptr)
		{
			_ASSERT(NULL != m_ptr);
		}

		virtual ~DeviceRelatedObject(void)
		{
			SAFE_RELEASE(m_ptr);
		}

		virtual void OnD3D9DestroyDevice(void)
		{
			SAFE_RELEASE(m_ptr);
		}
	};

	class DxutAppBase
	{
	public:
		static bool CALLBACK IsD3D9DeviceAcceptable_s(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed,
			void * pUserContext);

		virtual bool IsD3D9DeviceAcceptable(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed) = 0;

		static bool CALLBACK ModifyDeviceSettings_s(
			DXUTDeviceSettings * pDeviceSettings,
			void * pUserContext);

		virtual bool ModifyDeviceSettings(
			DXUTDeviceSettings * pDeviceSettings) = 0;

		static HRESULT CALLBACK OnD3D9CreateDevice_s(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
			void * pUserContext);

		virtual HRESULT OnD3D9CreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc) = 0;

		static HRESULT CALLBACK OnD3D9ResetDevice_s(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
			void * pUserContext);

		virtual HRESULT OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc) = 0;

		static void CALLBACK OnD3D9LostDevice_s(
			void * pUserContext);

		virtual void OnD3D9LostDevice(void) = 0;

		static void CALLBACK OnD3D9DestroyDevice_s(
			void * pUserContext);

		virtual void OnD3D9DestroyDevice(void) = 0;

		static void CALLBACK OnFrameMove_s(
			double fTime,
			float fElapsedTime,
			void * pUserContext);

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime) = 0;

		static void CALLBACK OnD3D9FrameRender_s(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime,
			void * pUserContext);

		virtual void OnD3D9FrameRender(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime) = 0;

		static LRESULT CALLBACK MsgProc_s(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing,
			void * pUserContext);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing) = 0;

		static void CALLBACK OnKeyboard_s(
			UINT nChar,
			bool bKeyDown,
			bool bAltDown,
			void * pUserContext);

		virtual void OnKeyboard(
			UINT nChar,
			bool bKeyDown,
			bool bAltDown) = 0;

	public:
		DxutAppBase(void);

		virtual ~DxutAppBase(void);

		virtual int Run(
			bool bWindowed = true,
			int nSuggestedWidth = 800,
			int nSuggestedHeight = 600);
	};

	class DxutApp : public DxutAppBase, public SingleInstance<DxutApp>
	{
	protected:
		enum
		{
			IDC_TOGGLEFULLSCREEN,
			IDC_TOGGLEREF,
			IDC_CHANGEDEVICE
		};

		CDXUTDialogResourceManager m_dlgResourceMgr;

		CD3DSettingsDlg m_settingsDlg;

		CDXUTDialog m_hudDlg;

		CComPtr<ID3DXFont> m_txtFont;

		CComPtr<ID3DXSprite> m_txtSprite;

	public:
		virtual bool IsD3D9DeviceAcceptable(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed);

		virtual bool ModifyDeviceSettings(
			DXUTDeviceSettings * pDeviceSettings);

		virtual HRESULT OnD3D9CreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual HRESULT OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnD3D9LostDevice(void);

		virtual void OnD3D9DestroyDevice(void);

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime);

		virtual void OnD3D9FrameRender(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime);

		virtual void RenderFrame(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime);

		static void CALLBACK OnGUIEvent_s(
			UINT nEvent,
			int nControlID,
			CDXUTControl * pControl,
			void * pUserContext);

		virtual void OnGUIEvent(
			UINT nEvent,
			int nControlID,
			CDXUTControl * pControl);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

		virtual void OnKeyboard(
			UINT nChar,
			bool bKeyDown,
			bool bAltDown);

		virtual void OnInit(void);

		virtual int Run(
			bool bWindowed = true,
			int nSuggestedWidth = 800,
			int nSuggestedHeight = 600);
	};
};