#pragma once

#include "physx_ptr.hpp"
#include "ApexRenderResourceMgr.h"

class PhysXAllocator : public PxAllocatorCallback
{
public:
	PhysXAllocator(void)
	{
	}

	void * allocate(size_t size, const char * typeName, const char * filename, int line);

	void deallocate(void * ptr);
};

class PhysXErrorCallback : public PxErrorCallback
{
public:
	PhysXErrorCallback(void)
	{
	}

	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);
};

class PhysXContext
{
protected:
	PhysXAllocator m_Allocator;

	PhysXErrorCallback m_ErrorCallback;

	ApexRenderer m_ApexRenderer;

	ApexRenderResourceMgr m_ApexUserRenderResMgr;

	ApexResourceCallback m_ApexResourceCallback;

	physx_ptr<PxFoundation> m_Foundation;

	physx_ptr<PxPhysics> m_Physics;

	physx_ptr<PxCooking> m_Cooking;

	physx_ptr<PxDefaultCpuDispatcher> m_CpuDispatcher;

	physx_ptr<physx::apex::NxApexSDK> m_ApexSDK;

	physx_ptr<physx::apex::NxModuleDestructible> m_ModuleDestructible;

public:
	PhysXContext(void)
	{
	}

	bool OnInit(void);

	void OnShutdown(void);
};

class PhysXSceneContext;

class StepperTask
	: public physx::pxtask::LightCpuTask
{
public:
	PhysXSceneContext * m_Scene;

	StepperTask(PhysXSceneContext * Scene)
		: m_Scene(Scene)
	{
	}

	virtual void run(void);

	virtual const char * getName(void) const;
};

class PhysXSceneContext
	: public PhysXContext
{
protected:
	physx_ptr<PxScene> m_Scene;

	physx_ptr<physx::apex::NxApexScene> m_ApexScene;

	my::Timer m_Timer;

	StepperTask m_Completion0, m_Completion1;

	my::Event m_Sync;

	bool m_WaitForResults;

	physx::PxU32 m_ErrorState;

	physx_ptr<PxMaterial> m_Material;

	physx::PxU32 m_ViewMatrixID;

	physx::PxU32 m_ProjMatrixID;

public:
	PhysXSceneContext(void)
		: m_Timer(1/60.0f,0)
		, m_Completion0(this)
		, m_Completion1(this)
		, m_Sync(NULL, FALSE, FALSE, NULL)
		, m_WaitForResults(false)
		, m_ErrorState(0)
	{
	}

	bool OnInit(void);

	void OnShutdown(void);

	void SetViewMatrix(const my::Matrix4 & View);

	void SetViewParams(const my::Vector3 & EyePos, const my::Vector3 & EyeDir, const my::Vector3 & WorldUp = my::Vector3(0,1,0));

	void SetProjMatrix(const my::Matrix4 & Proj);

	void SetProjParams(float nz, float fz, float fov, DWORD ViewportWidth, DWORD ViewportHeight);

	void OnTickPreRender(float dtime);

	void OnTickPostRender(float dtime);

	bool Advance(float dtime);

	void Substep(StepperTask & completionTask);

	void SubstepDone(StepperTask * ownerTask);

	void DrawRenderBuffer(IDirect3DDevice9 * pd3dDevice);
};