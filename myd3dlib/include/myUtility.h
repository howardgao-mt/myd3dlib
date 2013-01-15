#pragma once

#include "myResource.h"
#include "mySkeleton.h"
#include "myMesh.h"
#include "myEffect.h"
#include "myDxutApp.h"

namespace my
{
	class LoaderMgr
		: public ResourceMgr
		, public ID3DXInclude
	{
	protected:
		std::map<LPCVOID, CachePtr> m_cacheSet;

		CComPtr<ID3DXEffectPool> m_EffectPool;

		typedef std::map<std::string, boost::weak_ptr<DeviceRelatedObjectBase> > DeviceRelatedResourceSet;

		DeviceRelatedResourceSet m_resourceSet;

		typedef std::map<std::string, boost::weak_ptr<OgreSkeletonAnimation> > OgreSkeletonAnimationSet;

		OgreSkeletonAnimationSet m_skeletonSet;

	public:
		LoaderMgr(void);

		virtual ~LoaderMgr(void);

		virtual __declspec(nothrow) HRESULT __stdcall Open(
			D3DXINCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID * ppData,
			UINT * pBytes);

		virtual __declspec(nothrow) HRESULT __stdcall Close(
			LPCVOID pData);

		virtual HRESULT OnResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnLostDevice(void);

		virtual void OnDestroyDevice(void);

		template <class ResourceType>
		boost::shared_ptr<ResourceType> GetDeviceRelatedResource(const std::string & key, bool reload)
		{
			DeviceRelatedResourceSet::const_iterator res_iter = m_resourceSet.find(key);
			if(m_resourceSet.end() != res_iter)
			{
				boost::shared_ptr<DeviceRelatedObjectBase> res = res_iter->second.lock();
				if(res)
				{
					if(reload)
						res->OnDestroyDevice();

					return boost::dynamic_pointer_cast<ResourceType>(res);
				}
			}

			boost::shared_ptr<ResourceType> res(new ResourceType());
			m_resourceSet[key] = res;
			return res;
		}

		TexturePtr LoadTexture(const std::string & path, bool reload = false);

		CubeTexturePtr LoadCubeTexture(const std::string & path, bool reload = false);

		OgreMeshPtr LoadMesh(const std::string & path, bool reload = false);

		OgreSkeletonAnimationPtr LoadSkeleton(const std::string & path, bool reload = false);

		EffectPtr LoadEffect(const std::string & path, bool reload = false);

		FontPtr LoadFont(const std::string & path, int height, bool reload = false);
	};

	class Timer
	{
	public:
		const float m_Interval;

		float m_RemainingTime;

		ControlEvent m_EventTimer;

	public:
		Timer(float Interval, float RemainingTime = 0)
			: m_Interval(Interval)
			, m_RemainingTime(Interval)
		{
		}
	};

	typedef boost::shared_ptr<Timer> TimerPtr;

	class TimerMgr
	{
	protected:
		typedef std::map<TimerPtr, bool> TimerPtrSet;

		TimerPtrSet m_timerSet;

		const float m_MinRemainingTime;

		EventArgsPtr m_DefaultArgs;

	public:
		TimerMgr(void)
			: m_MinRemainingTime(-1/10.0f)
			, m_DefaultArgs(new EventArgs())
		{
		}

		TimerPtr AddTimer(float Interval, ControlEvent EventTimer)
		{
			TimerPtr timer(new Timer(Interval, Interval));
			timer->m_EventTimer = EventTimer;
			InsertTimer(timer);
			return timer;
		}

		void InsertTimer(TimerPtr timer)
		{
			m_timerSet.insert(std::make_pair(timer, true));
		}

		void RemoveTimer(TimerPtr timer)
		{
			TimerPtrSet::iterator timer_iter = m_timerSet.find(timer);
			if(timer_iter != m_timerSet.end())
			{
				timer_iter->second = false;
			}
		}

		void RemoveAllTimer(void)
		{
			m_timerSet.clear();
		}

		void OnFrameMove(
			double fTime,
			float fElapsedTime);
	};

	class DialogMgr
	{
	public:
		typedef std::vector<DialogPtr> DialogPtrSet;

		DialogPtrSet m_dlgSet;

	public:
		DialogMgr(void)
		{
		}

		static void UpdateDlgViewProj(DialogPtr dlg, const Vector2 & vp);

		void Draw(
			UIRender * ui_render,
			double fTime,
			float fElapsedTime);

		bool MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam);

		void InsertDlg(DialogPtr dlg)
		{
			const D3DSURFACE_DESC & desc = DxutApp::getSingleton().GetD3D9BackBufferSurfaceDesc();

			UpdateDlgViewProj(dlg, Vector2((float)desc.Width, (float)desc.Height));

			m_dlgSet.push_back(dlg);
		}

		void RemoveDlg(DialogPtr dlg)
		{
			DialogPtrSet::iterator dlg_iter = std::find(m_dlgSet.begin(), m_dlgSet.end(), dlg);
			if(dlg_iter != m_dlgSet.end())
			{
				m_dlgSet.erase(dlg_iter);
			}
		}

		void RemoveAllDlg(void)
		{
			m_dlgSet.clear();
		}
	};
}
