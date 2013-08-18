#pragma once

#include "myResource.h"
#include "mySkeleton.h"
#include "myMesh.h"
#include "myEffect.h"
#include "myEmitter.h"
#include "myUI.h"
#include <map>

namespace my
{
	class DrawHelper
	{
	public:
		static void DrawLine(
			IDirect3DDevice9 * pd3dDevice,
			const Vector3 & v0,
			const Vector3 & v1,
			D3DCOLOR Color,
			const Matrix4 & world = Matrix4::identity);

		static void DrawSphere(
			IDirect3DDevice9 * pd3dDevice,
			float radius,
			D3DCOLOR Color,
			const Matrix4 & world = Matrix4::identity);

		static void DrawBox(
			IDirect3DDevice9 * pd3dDevice,
			const Vector3 & halfSize,
			D3DCOLOR Color,
			const Matrix4 & world = Matrix4::identity);

		static void DrawTriangle(
			IDirect3DDevice9 * pd3dDevice,
			const Vector3 & v0,
			const Vector3 & v1,
			const Vector3 & v2,
			D3DCOLOR Color,
			const Matrix4 & world = Matrix4::identity);

		static void DrawSpereStage(
			IDirect3DDevice9 * pd3dDevice,
			float radius,
			int VSTAGE_BEGIN,
			int VSTAGE_END,
			float offsetY,
			D3DCOLOR Color,
			const Matrix4 & world = Matrix4::identity);

		static void DrawCylinderStage(
			IDirect3DDevice9 * pd3dDevice,
			float radius,
			float y0,
			float y1,
			D3DCOLOR Color,
			const Matrix4 & world = Matrix4::identity);

		static void DrawCapsule(
			IDirect3DDevice9 * pd3dDevice,
			float radius,
			float height,
			D3DCOLOR Color,
			const Matrix4 & world = Matrix4::identity);
	};

	class Timer
	{
	public:
		const float m_Interval;

		float m_RemainingTime;

		ControlEvent m_EventTimer;

		bool m_Removed;

	public:
		Timer(float Interval, float RemainingTime = 0)
			: m_Interval(Interval)
			, m_RemainingTime(RemainingTime)
			, m_Removed(true)
		{
		}
	};

	typedef boost::shared_ptr<Timer> TimerPtr;

	class TimerMgr
	{
	protected:
		typedef std::set<TimerPtr> TimerPtrSet;

		TimerPtrSet m_timerSet;

		EventArgsPtr m_DefaultArgs;

		const int m_MaxIterCount;

	public:
		TimerMgr(void)
			: m_DefaultArgs(new EventArgs())
			, m_MaxIterCount(4)
		{
		}

		TimerPtr AddTimer(float Interval, ControlEvent EventTimer);

		void InsertTimer(TimerPtr timer);

		void RemoveTimer(TimerPtr timer);

		void RemoveAllTimer(void);

		void OnFrameMove(
			double fTime,
			float fElapsedTime);
	};

	class BaseCamera
	{
	public:
		float m_Fov;

		float m_Aspect;

		float m_Nz;

		float m_Fz;

		Matrix4 m_View;

		Matrix4 m_Proj;

	public:
		BaseCamera(float Fov, float Aspect, float Nz, float Fz)
			: m_Fov(Fov)
			, m_Aspect(Aspect)
			, m_Nz(Nz)
			, m_Fz(Fz)
			, m_View(Matrix4::identity)
			, m_Proj(Matrix4::identity)
		{
		}

		virtual ~BaseCamera(void)
		{
		}

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime) = 0;

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing) = 0;
	};

	typedef boost::shared_ptr<BaseCamera> BaseCameraPtr;

	class Camera
		: public BaseCamera
	{
	public:
		Vector3 m_Position;

		Quaternion m_Orientation;

		Matrix4 m_ViewProj;

		Matrix4 m_InverseViewProj;

		ControlEvent EventAlign;

	public:
		Camera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: BaseCamera(Fov, Aspect, Nz, Fz)
			, m_Position(Vector3::zero)
			, m_Orientation(Quaternion::identity)
		{
		}

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

		std::pair<Vector3, Vector3> CalculateRay(const Vector2 & pt, const CSize & dim);
	};

	class ModelViewerCamera
		: public Camera
	{
	public:
		Vector3 m_LookAt;

		Vector3 m_Rotation;

		float m_Distance;

		bool m_bDrag;

		CPoint m_DragPos;

	public:
		ModelViewerCamera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: Camera(Fov, Aspect, Nz, Fz)
			, m_LookAt(Vector3::zero)
			, m_Rotation(Vector3::zero)
			, m_Distance(0)
			, m_bDrag(false)
		{
		}

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);
	};

	class FirstPersonCamera
		: public Camera
	{
	public:
		Vector3 m_Velocity;

		Vector3 m_Rotation;

		bool m_bDrag;

		CPoint m_DragPos;

	public:
		FirstPersonCamera(float Fov = D3DXToRadian(75.0f), float Aspect = 1.333333f, float Nz = 0.1f, float Fz = 3000.0f)
			: Camera(Fov, Aspect, Nz, Fz)
			, m_Velocity(0,0,0)
			, m_Rotation(0,0,0)
			, m_bDrag(false)
		{
		}

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);
	};

	class DialogMgr
	{
	public:
		typedef std::vector<DialogPtr> DialogPtrList;

		typedef std::map<int, DialogPtrList> DialogPtrSetMap;

		DialogPtrSetMap m_dlgSetMap;

		FirstPersonCamera m_Camera;

	public:
		DialogMgr(void)
		{
			SetDlgViewport(Vector2(800,600));
		}

		virtual ~DialogMgr(void)
		{
		}

		void SetDlgViewport(const Vector2 & vp);

		Vector2 GetDlgViewport(void) const;

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
			m_dlgSetMap[0].push_back(dlg);

			if(dlg->EventAlign)
				dlg->EventAlign(EventArgsPtr(new EventArgs()));
		}

		void RemoveDlg(DialogPtr dlg)
		{
			DialogPtrList::iterator dlg_iter = std::find(m_dlgSetMap[0].begin(), m_dlgSetMap[0].end(), dlg);
			if(dlg_iter != m_dlgSetMap[0].end())
			{
				m_dlgSetMap[0].erase(dlg_iter);
			}
		}

		void RemoveAllDlg()
		{
			m_dlgSetMap[0].clear();
		}
	};

	class EmitterMgr
	{
	public:
		typedef std::set<EmitterPtr> EmitterPtrSet;

		EmitterPtrSet m_EmitterSet;

	public:
		EmitterMgr(void)
		{
		}

		virtual ~EmitterMgr(void)
		{
		}

		void Update(
			double fTime,
			float fElapsedTime);

		void Draw(
			EmitterInstance * pInstance,
			Camera * pCamera,
			double fTime,
			float fElapsedTime);

		void InsertEmitter(EmitterPtr emitter)
		{
			_ASSERT(m_EmitterSet.end() == m_EmitterSet.find(emitter));

			m_EmitterSet.insert(emitter);
		}

		void RemoveEmitter(EmitterPtr emitter)
		{
			m_EmitterSet.erase(emitter);
		}

		void RemoveAllEmitter(void)
		{
			m_EmitterSet.clear();
		}
	};

	class EffectParameterBase
	{
	public:
		EffectParameterBase(void)
		{
		}

		virtual ~EffectParameterBase(void)
		{
		}

		virtual void SetParameter(Effect * pEffect, const std::string & Name) const = 0;
	};

	typedef boost::shared_ptr<EffectParameterBase> EffectParameterBasePtr;

	template <class T>
	class EffectParameter : public EffectParameterBase
	{
	public:
		T m_Value;

		EffectParameter(const T & Value)
			: m_Value(Value)
		{
		}

		virtual void SetParameter(Effect * pEffect, const std::string & Name) const;
	};

	class EffectParameterMap : public boost::unordered_map<std::string, EffectParameterBasePtr>
	{
	public:
		void SetBool(const std::string & Name, bool Value)
		{
			operator[](Name) = EffectParameterBasePtr(new EffectParameter<bool>(Value));
		}

		void SetFloat(const std::string & Name, float Value)
		{
			operator[](Name) = EffectParameterBasePtr(new EffectParameter<float>(Value));
		}

		void SetInt(const std::string & Name, int Value)
		{
			operator[](Name) = EffectParameterBasePtr(new EffectParameter<int>(Value));
		}

		void SetVector(const std::string & Name, const Vector4 & Value)
		{
			operator[](Name) = EffectParameterBasePtr(new EffectParameter<Vector4>(Value));
		}

		void SetMatrix(const std::string & Name, const Matrix4 & Value)
		{
			operator[](Name) = EffectParameterBasePtr(new EffectParameter<Matrix4>(Value));
		}

		void SetString(const std::string & Name, const std::string & Value)
		{
			operator[](Name) = EffectParameterBasePtr(new EffectParameter<std::string>(Value));
		}

		void SetTexture(const std::string & Name, BaseTexturePtr Value)
		{
			operator[](Name) = EffectParameterBasePtr(new EffectParameter<BaseTexturePtr>(Value));
		}
	};

	class Material : public std::vector<std::pair<EffectPtr, EffectParameterMap> >
	{
	public:
		Material(void)
		{
		}

		void ApplyParameterBlock(UINT i);

		UINT Begin(UINT i, DWORD Flags = 0);

		void BeginPass(UINT i, UINT Pass);

		void EndPass(UINT i);

		void End(UINT i);

		void DrawMeshSubset(UINT i, Mesh * pMesh, DWORD AttribId);
	};

	typedef boost::shared_ptr<Material> MaterialPtr;

	class MaterialMgr
	{
	public:
		typedef boost::unordered_map<std::string, MaterialPtr> MaterialPtrMap;

		MaterialPtrMap m_MaterialMap;

	public:
		MaterialMgr(void)
		{
		}

		virtual ~MaterialMgr(void)
		{
		}

		void InsertMaterial(const std::string & key, MaterialPtr material)
		{
			_ASSERT(m_MaterialMap.end() == m_MaterialMap.find(key));

			m_MaterialMap[key] = material;
		}

		void RemoveMaterial(const std::string & key)
		{
			_ASSERT(m_MaterialMap.end() != m_MaterialMap.find(key));

			m_MaterialMap.erase(key);
		}

		void RemoveAllMaterial(void)
		{
			m_MaterialMap.clear();
		}
	};

	class ResourceMgr : public DeviceRelatedResourceMgr
	{
	public:
		typedef boost::unordered_map<std::string, boost::weak_ptr<Material> > MaterialWeakPtrSet;

		MaterialWeakPtrSet m_materialSet;

	public:
		ResourceMgr(void)
		{
		}

		virtual ~ResourceMgr(void)
		{
		}

		MaterialPtr LoadMaterial(const std::string & path, bool reload = false);
	};
}
