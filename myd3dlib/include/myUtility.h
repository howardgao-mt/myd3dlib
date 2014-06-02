#pragma once

#include "myResource.h"
#include "mySkeleton.h"
#include "myMesh.h"
#include "myEffect.h"
#include "myEmitter.h"
#include "myUI.h"
#include "myInput.h"
#include <set>

namespace my
{
	class DrawHelper
	{
	protected:
		struct Vertex
		{
			float x, y, z;

			D3DCOLOR color;

			Vertex(const Vector3 & v, D3DCOLOR _color)
				: x(v.x), y(v.y), z(v.z), color(_color)
			{
			}
		};

		std::vector<Vertex> m_vertices;

	public:
		DrawHelper(void)
		{
		}

		void BeginLine(void);

		void EndLine(IDirect3DDevice9 * pd3dDevice, const Matrix4 & Transform = Matrix4::identity);

		void PushLine(const Vector3 & v0, const Vector3 & v1, D3DCOLOR Color);

		void PushWireAABB(const AABB & aabb, D3DCOLOR Color);

		void PushWireAABB(const AABB & aabb, D3DCOLOR Color, const Matrix4 & Transform);

		void PushGrid(float length = 12, float linesEvery = 5, unsigned subLines = 5, D3DCOLOR GridColor = D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR AxisColor = D3DCOLOR_ARGB(255,0,0,0));
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
		Vector3 m_Position;

		Quaternion m_Orientation;

		float m_Nz;

		float m_Fz;

		Matrix4 m_View;

		Matrix4 m_Proj;

		Matrix4 m_ViewProj;

		Matrix4 m_InverseViewProj;

	public:
		BaseCamera(const Vector3 & Position, const Quaternion & Orientation, float Nz, float Fz)
			: m_Position(Position)
			, m_Orientation(Orientation)
			, m_Nz(Nz)
			, m_Fz(Fz)
		{
		}

		virtual ~BaseCamera(void);

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime) = 0;

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing) = 0;

		std::pair<Vector3, Vector3> CalculateRay(const Vector2 & pt, const CSize & dim);
	};

	typedef boost::shared_ptr<BaseCamera> BaseCameraPtr;

	class OrthoCamera
		: public BaseCamera
	{
	public:
		float m_Width;

		float m_Height;

	public:
		OrthoCamera(const Vector3 & Position, const Quaternion & Orientation, float Width, float Height, float Nz, float Fz)
			: BaseCamera(Position, Orientation, Nz, Fz)
			, m_Width(Width)
			, m_Height(Height)
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

	typedef boost::shared_ptr<OrthoCamera> OrthoCameraPtr;

	class Camera
		: public BaseCamera
	{
	public:
		float m_Fov;

		float m_Aspect;

		ControlEvent EventAlign;

	public:
		Camera(const Vector3 & Position, const Quaternion & Orientation, float Fov, float Aspect, float Nz, float Fz)
			: BaseCamera(Position, Orientation, Nz, Fz)
			, m_Fov(Fov)
			, m_Aspect(Aspect)
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

	typedef boost::shared_ptr<Camera> CameraPtr;

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
			: Camera(Vector3::zero, Quaternion::identity, Fov, Aspect, Nz, Fz)
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
			: Camera(Vector3::zero, Quaternion::identity, Fov, Aspect, Nz, Fz)
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

	class EmitterMgr
	{
	public:
		typedef std::set<EmitterPtr> EmitterPtrSet;

		EmitterPtrSet m_EmitterSet;

	public:
		EmitterMgr(void)
		{
		}

		void Update(
			double fTime,
			float fElapsedTime);

		void Draw(
			EmitterInstance * pInstance,
			const Matrix4 & ViewProj,
			const Quaternion & ViewOrientation,
			double fTime,
			float fElapsedTime);

		void InsertEmitter(EmitterPtr emitter);

		void RemoveEmitter(EmitterPtr emitter);

		void RemoveAllEmitter(void);
	};

	class EffectParameterBase
	{
	public:
		EffectParameterBase(void)
		{
		}

		virtual ~EffectParameterBase(void);

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
		void SetBool(const std::string & Name, bool Value);

		void SetFloat(const std::string & Name, float Value);

		void SetInt(const std::string & Name, int Value);

		void SetVector(const std::string & Name, const Vector4 & Value);

		void SetMatrix(const std::string & Name, const Matrix4 & Value);

		void SetString(const std::string & Name, const std::string & Value);

		void SetTexture(const std::string & Name, BaseTexturePtr Value);
	};

	class Material : public DeviceRelatedObjectBase
	{
	public:
		BaseTexturePtr m_DiffuseTexture;

		BaseTexturePtr m_NormalTexture;

		BaseTexturePtr m_SpecularTexture;

	public:
		Material(void)
		{
		}

		void OnResetDevice(void);

		void OnLostDevice(void);

		void OnDestroyDevice(void);
	};

	typedef boost::shared_ptr<Material> MaterialPtr;

	class ResourceMgr : public AsynchronousResourceMgr
	{
	protected:
		class membuf : public std::streambuf
		{
		public:
			membuf(const char * buff, size_t size)
			{
				char * p = const_cast<char *>(buff);
				setg(p, p, p + size);
			}
		};

		class ResourceCallbackBoundle
		{
		public:
			DeviceRelatedObjectBasePtr m_res;

			IORequest::ResourceCallbackList m_callbacks;

			ResourceCallbackBoundle(DeviceRelatedObjectBasePtr res)
				: m_res(res)
			{
			}

			~ResourceCallbackBoundle(void)
			{
				IORequest::ResourceCallbackList::const_iterator callback_iter = m_callbacks.begin();
				for(; callback_iter != m_callbacks.end(); callback_iter++)
				{
					if(*callback_iter)
					{
						(*callback_iter)(m_res);
					}
				}
			}
		};

		typedef boost::shared_ptr<ResourceCallbackBoundle> ResourceCallbackBoundlePtr;

	public:
		ResourceMgr(void)
		{
		}

		class MaterialIORequest;

		void LoadMaterialAsync(const std::string & path, const ResourceCallback & callback);

		MaterialPtr LoadMaterial(const std::string & path);

		void SaveMaterial(const std::string & path, MaterialPtr material);

		class EmitterIORequest;

		void LoadEmitterAsync(const std::string & path, const ResourceCallback & callback);

		EmitterPtr LoadEmitter(const std::string & path);

		void SaveEmitter(const std::string & path, EmitterPtr emitter);
	};

	typedef boost::function<void (void)> InputEvent;

	class InputMgr
	{
	protected:
		InputPtr m_input;

		KeyboardPtr m_keyboard;

		JoystickPtr m_joystick;

	public:
		InputMgr(void)
		{
		}

		void Create(HINSTANCE hinst);

		void Destroy(void);

		void Update(void);

		static BOOL CALLBACK JoystickFinderCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
	};
}
