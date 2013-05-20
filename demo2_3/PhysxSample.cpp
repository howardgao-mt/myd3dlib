#include "StdAfx.h"
#include "PhysxSample.h"

#define PHYSX_SAFE_RELEASE(p) if(p) { p->release(); p=NULL; }

using namespace my;

void * PhysxSampleAllocator::allocate(size_t size, const char * typeName, const char * filename, int line)
{
#ifdef _DEBUG
	return _aligned_malloc_dbg(size, 16, filename, line);
#else
	return _aligned_malloc(size, 16);	
#endif
}

void PhysxSampleAllocator::deallocate(void * ptr)
{
#ifdef _DEBUG
	_aligned_free_dbg(ptr);
#else
	_aligned_free(ptr);
#endif
}

void PhysxSampleErrorCallback::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	switch(code)
	{
	case PxErrorCode::eNO_ERROR:
		break;

	case PxErrorCode::eDEBUG_INFO:
		OutputDebugStringA(str_printf("%s (%d) : info: %s\n", file, line, message).c_str());
		break;

	case PxErrorCode::eDEBUG_WARNING:
	case PxErrorCode::ePERF_WARNING:
		OutputDebugStringA(str_printf("%s (%d) : warning: %s\n", file, line, message).c_str());
		break;

	case PxErrorCode::eINVALID_PARAMETER:
	case PxErrorCode::eINVALID_OPERATION:
	case PxErrorCode::eOUT_OF_MEMORY:
	case PxErrorCode::eINTERNAL_ERROR:
	case PxErrorCode::eABORT:
	case PxErrorCode::eEXCEPTION_ON_STARTUP:
		OutputDebugStringA(str_printf("%s (%d) : error: %s\n", file, line, message).c_str());
		break;
	}
}

bool PhysxSample::OnInit(void)
{
	if(!(m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback)))
	{
		THROW_CUSEXCEPTION("PxCreateFoundation failed");
	}

	if(!(m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(),
#ifdef _DEBUG
		true
#else
		false
#endif
		)))
	{
		THROW_CUSEXCEPTION("PxCreatePhysics failed");
	}

	if(!PxInitExtensions(*m_Physics))
	{
		THROW_CUSEXCEPTION("PxInitExtensions failed");
	}

	if(!(m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams())))
	{
		THROW_CUSEXCEPTION("PxCreateCooking failed");
	}

	if(!(m_CpuDispatcher = PxDefaultCpuDispatcherCreate(1, NULL)))
	{
		THROW_CUSEXCEPTION("PxDefaultCpuDispatcherCreate failed");
	}

	physx::apex::NxApexSDKDesc apexDesc;
	apexDesc.outputStream = &PhysxSample::getSingleton().m_ErrorCallback;
	apexDesc.physXSDKVersion = PX_PHYSICS_VERSION;
	apexDesc.physXSDK = PhysxSample::getSingleton().m_Physics;
	apexDesc.cooking = PhysxSample::getSingleton().m_Cooking;
	apexDesc.renderResourceManager = &m_ApexUserRenderResMgr;
	if(!(m_ApexSDK = NxCreateApexSDK(apexDesc)))
	{
		THROW_CUSEXCEPTION("NxCreateApexSDK failed");
	}

	if(!(m_ModuleDestructible = static_cast<physx::apex::NxModuleDestructible *>(m_ApexSDK->createModule("Destructible"))))
	{
		THROW_CUSEXCEPTION("m_ApexSDK->createModule failed");
	}

	NxParameterized::Interface * moduleDesc = m_ModuleDestructible->getDefaultModuleDesc();
	m_ModuleDestructible->init(*moduleDesc);

	return true;
}

PhysxSample::SingleInstance * SingleInstance<PhysxSample>::s_ptr = NULL;

void PhysxSample::OnShutdown(void)
{
	PHYSX_SAFE_RELEASE(m_ModuleDestructible);

	PHYSX_SAFE_RELEASE(m_ApexSDK);

	PHYSX_SAFE_RELEASE(m_CpuDispatcher);

	PHYSX_SAFE_RELEASE(m_Cooking);

	if(m_Physics)
		PxCloseExtensions();

	PHYSX_SAFE_RELEASE(m_Physics);

	PHYSX_SAFE_RELEASE(m_Foundation);
}

void StepperTask::run(void)
{
	m_Scene->SubstepDone(this);
	release();
}

const char * StepperTask::getName(void) const
{
	return "Stepper Task";
}

bool PhysxScene::OnInit(void)
{
	PxSceneDesc sceneDesc(PhysxSample::getSingleton().m_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = PhysxSample::getSingleton().m_CpuDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	if(!(m_Scene = PhysxSample::getSingleton().m_Physics->createScene(sceneDesc)))
	{
		THROW_CUSEXCEPTION("PhysxSample::getSingleton().m_Physics->createScene failed");
	}

	if(!(m_Material = PhysxSample::getSingleton().m_Physics->createMaterial(0.5f, 0.5f, 0.1f)))
	{
		THROW_CUSEXCEPTION("PhysxSample::getSingleton().m_Physics->createMaterial failed");
	}

	if(!(m_Sphere = PxCreateDynamic(*PhysxSample::getSingleton().m_Physics, PxTransform(PxVec3(0,10,0)), PxSphereGeometry(1), *m_Material, 1)))
	{
		THROW_CUSEXCEPTION("PxCreateDynamic failed");
	}
	m_Scene->addActor(*m_Sphere);

	if(!(m_Plane = PxCreatePlane(*PhysxSample::getSingleton().m_Physics, PxPlane(PxVec3(0,1,0), 0), *m_Material)))
	{
		THROW_CUSEXCEPTION("PxCreatePlane failed");
	}
	m_Scene->addActor(*m_Plane);

	m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);

	m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);

	physx::apex::NxApexSceneDesc apexSceneDesc;
	apexSceneDesc.scene = m_Scene;
	if(!(m_ApexScene = PhysxSample::getSingleton().m_ApexSDK->createScene(apexSceneDesc)))
	{
		THROW_CUSEXCEPTION("m_ApexSDK->createScene failed");
	}

	return true;
}

void PhysxScene::OnShutdown(void)
{
	PHYSX_SAFE_RELEASE(m_ApexScene);

	PHYSX_SAFE_RELEASE(m_Plane);

	PHYSX_SAFE_RELEASE(m_Sphere);

	PHYSX_SAFE_RELEASE(m_Material);

	PHYSX_SAFE_RELEASE(m_Scene);
}

void PhysxScene::OnTickPreRender(float dtime)
{
	m_Sync.ResetEvent();

	m_WaitForResults = Advance(dtime);
}

void PhysxScene::OnTickPostRender(float dtime)
{
	if(m_WaitForResults)
	{
		m_Sync.WaitEvent(INFINITE);
	}
}

bool PhysxScene::Advance(float dtime)
{
	m_Timer.m_RemainingTime = Min(0.1f, m_Timer.m_RemainingTime + dtime);

	if(m_Timer.m_RemainingTime < m_Timer.m_Interval)
	{
		return false;
	}

	m_Timer.m_RemainingTime -= m_Timer.m_Interval;

	m_Completion0.setContinuation(*m_Scene->getTaskManager(), NULL);

	Substep(m_Completion0);

	m_Completion0.removeReference();

	return true;
}

void PhysxScene::Substep(StepperTask & completionTask)
{
	m_Scene->simulate(m_Timer.m_Interval, &completionTask, 0, 0, true);
}

void PhysxScene::SubstepDone(StepperTask * ownerTask)
{
	m_Scene->fetchResults(true);

	if(m_Timer.m_RemainingTime < m_Timer.m_Interval)
	{
		m_Sync.SetEvent();
		return;
	}

	m_Timer.m_RemainingTime -= m_Timer.m_Interval;

	StepperTask & task = (ownerTask == &m_Completion0 ? m_Completion1 : m_Completion0);

	task.setContinuation(*m_Scene->getTaskManager(), NULL);

	Substep(task);

	task.removeReference();
}

void PhysxScene::DrawRenderBuffer(IDirect3DDevice9 * pd3dDevice, const PxRenderBuffer & debugRenderable)
{
	const PxU32 numPoints = debugRenderable.getNbPoints();
	if(numPoints)
	{
		const PxDebugPoint* PX_RESTRICT points = debugRenderable.getPoints();
		for(PxU32 i=0; i<numPoints; i++)
		{
			const PxDebugPoint& point = points[i];
		}
	}

	const PxU32 numLines = debugRenderable.getNbLines();
	if(numLines)
	{
		const PxDebugLine* PX_RESTRICT lines = debugRenderable.getLines();
		for(PxU32 i=0; i<numLines; i++)
		{
			const PxDebugLine& line = lines[i];
			DrawLine(pd3dDevice, (Vector3 &)line.pos0, (Vector3 &)line.pos1, line.color0);
		}
	}

	const PxU32 numTriangles = debugRenderable.getNbTriangles();
	if(numTriangles)
	{
		const PxDebugTriangle* PX_RESTRICT triangles = debugRenderable.getTriangles();
		for(PxU32 i=0; i<numTriangles; i++)
		{
			const PxDebugTriangle& triangle = triangles[i];
			DrawTriangle(pd3dDevice, (Vector3 &)triangle.pos0, (Vector3 &)triangle.pos1, (Vector3 &)triangle.pos2, triangle.color0);
		}
	}
}
