#include "StdAfx.h"
#include "PhysXContext.h"

const my::Vector3 PhysXContext::Gravity(0.0f, -9.81f, 0.0f);

void * PhysXAllocator::allocate(size_t size, const char * typeName, const char * filename, int line)
{
#ifdef _DEBUG
	return _aligned_malloc_dbg(size, 16, filename, line);
#else
	return _aligned_malloc(size, 16);	
#endif
}

void PhysXAllocator::deallocate(void * ptr)
{
#ifdef _DEBUG
	_aligned_free_dbg(ptr);
#else
	_aligned_free(ptr);
#endif
}

bool PhysXContext::OnInit(void)
{
	if(!(m_Foundation.reset(PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, *this)), m_Foundation))
	{
		THROW_CUSEXCEPTION(_T("PxCreateFoundation failed"));
	}

	if(!(m_sdk.reset(PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(),
#ifdef _DEBUG
		true,
#else
		false,
#endif
		NULL)), m_sdk))
	{
		THROW_CUSEXCEPTION(_T("PxCreatePhysics failed"));
	}

	if(!(m_Cooking.reset(PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams())), m_Cooking))
	{
		THROW_CUSEXCEPTION(_T("PxCreateCooking failed"));
	}

	if(!PxInitExtensions(*m_sdk))
	{
		THROW_CUSEXCEPTION(_T("PxInitExtensions failed"));
	}

	if(!(m_CpuDispatcher.reset(PxDefaultCpuDispatcherCreate(1, NULL)), m_CpuDispatcher))
	{
		THROW_CUSEXCEPTION(_T("PxDefaultCpuDispatcherCreate failed"));
	}

	if(!(m_ControllerMgr.reset(PxCreateControllerManager(*m_Foundation)), m_ControllerMgr))
	{
		THROW_CUSEXCEPTION(_T("PxCreateControllerManager failed"));
	}

	if(!(m_PxMaterial.reset(m_sdk->createMaterial(0.5f, 0.5f, 0.1f)), m_PxMaterial))
	{
		THROW_CUSEXCEPTION(_T("m_sdk->createMaterial failed"));
	}
	return true;
}

void PhysXContext::OnShutdown(void)
{
	m_PxMaterial.reset();

	if(m_sdk)
	{
		PxCloseExtensions();
	}

	m_ControllerMgr.reset();

	m_CpuDispatcher.reset();

	m_Cooking.reset();

	m_sdk.reset();

	m_Foundation.reset();
}

class PhysXOStream : public PxOutputStream
{
public:
	my::OStreamPtr ostream;

	PhysXOStream(my::OStreamPtr _ostream)
		: ostream(_ostream)
	{
	}

	virtual PxU32 write(const void* src, PxU32 count)
	{
		return ostream->write(src, count);
	}
};

class PhysXIStream : public PxInputStream
{
public:
	my::IStreamPtr istream;

	PhysXIStream(my::IStreamPtr _istream)
		: istream(_istream)
	{
	}

	virtual PxU32 read(void* dest, PxU32 count)
	{
		return istream->read(dest, count);
	}
};

void PhysXContext::CookTriangleMesh(my::OStreamPtr ostream, my::MeshPtr mesh)
{
	PxTriangleMeshDesc desc;
	desc.points.count = mesh->GetNumVertices();
	desc.points.stride = mesh->GetNumBytesPerVertex();
	desc.points.data = mesh->LockVertexBuffer();
	desc.triangles.count = mesh->GetNumFaces();
	if (mesh->GetOptions() & D3DXMESH_32BIT)
	{
		desc.triangles.stride = 3 * sizeof(DWORD);
	}
	else
	{
		desc.triangles.stride = 3 * sizeof(WORD);
		desc.flags |= PxMeshFlag::e16_BIT_INDICES;
	}
	desc.triangles.data = mesh->LockIndexBuffer();
	desc.materialIndices.stride = sizeof(DWORD);
	desc.materialIndices.data = (PxMaterialTableIndex *)mesh->LockAttributeBuffer();
	m_Cooking->cookTriangleMesh(desc, PhysXOStream(ostream));
	mesh->UnlockVertexBuffer();
	mesh->UnlockIndexBuffer();
	mesh->UnlockAttributeBuffer();
}

void PhysXContext::CookTriangleMeshToFile(std::string path, my::MeshPtr mesh)
{
	CookTriangleMesh(my::FileOStream::Open(ms2ts(path).c_str()), mesh);
}

PxTriangleMesh * PhysXContext::CreateTriangleMesh(my::IStreamPtr istream)
{
	// ! should be call at resource thread
	PxTriangleMesh * ret = m_sdk->createTriangleMesh(PhysXIStream(istream));
	return ret;
}

void PhysXContext::CookClothFabric(my::OStreamPtr ostream, my::MeshPtr mesh, WORD PositionOffset)
{
	PxClothMeshDesc desc;
	desc.points.data = (unsigned char *)mesh->LockVertexBuffer() + PositionOffset;
	desc.points.count = mesh->GetNumVertices();
	desc.points.stride = mesh->GetNumBytesPerVertex();

	desc.triangles.data = mesh->LockIndexBuffer();
	desc.triangles.count = mesh->GetNumFaces();
	if (mesh->GetOptions() & D3DXMESH_32BIT)
	{
		desc.triangles.stride = 3 * sizeof(DWORD);
	}
	else
	{
		desc.triangles.stride = 3 * sizeof(WORD);
		desc.flags |= PxMeshFlag::e16_BIT_INDICES;
	}

	m_Cooking->cookClothFabric(desc, (PxVec3&)Gravity, PhysXOStream(ostream));
	mesh->UnlockVertexBuffer();
	mesh->UnlockIndexBuffer();
}

void PhysXContext::CookClothFabricToFile(std::string path, my::MeshPtr mesh, WORD PositionOffset)
{
	CookClothFabric(my::FileOStream::Open(ms2ts(path).c_str()), mesh, PositionOffset);
}

PxClothFabric * PhysXContext::CreateClothFabric(my::IStreamPtr istream)
{
	PxClothFabric * ret = m_sdk->createClothFabric(PhysXIStream(istream));
	return ret;
}

void PhysXContext::InitClothParticles(
	std::vector<PxClothParticle> & particles,
	my::MeshPtr mesh,
	WORD PositionOffset)
{
	particles.resize(mesh->GetNumVertices());
	unsigned char * pVertices = (unsigned char *)mesh->LockVertexBuffer();
	for(unsigned int i = 0; i < particles.size(); i++) {
		unsigned char * pVertex = pVertices + i * mesh->GetNumBytesPerVertex();
		particles[i].pos = *(PxVec3 *)(pVertex + PositionOffset);
		particles[i].invWeight = 1 / 1.0f;
	}
	mesh->UnlockVertexBuffer();
}

void PhysXContext::InitClothParticles(
	std::vector<PxClothParticle> & particles,
	my::MeshPtr mesh,
	WORD PositionOffset,
	WORD IndicesOffset,
	const my::BoneHierarchy & hierarchy,
	DWORD root_i)
{
	particles.resize(mesh->GetNumVertices());
	unsigned char * pVertices = (unsigned char *)mesh->LockVertexBuffer();
	for(unsigned int i = 0; i < particles.size(); i++) {
		unsigned char * pVertex = pVertices + i * mesh->GetNumBytesPerVertex();
		particles[i].pos = *(PxVec3 *)(pVertex + PositionOffset);
		unsigned char * pIndices = (unsigned char *)(pVertex + IndicesOffset);
		BOOST_STATIC_ASSERT(4 == my::D3DVertexElementSet::MAX_BONE_INDICES);
		particles[i].invWeight = (
			pIndices[0] == root_i || hierarchy.HaveChild(root_i, pIndices[0]) ||
			pIndices[1] == root_i || hierarchy.HaveChild(root_i, pIndices[1]) ||
			pIndices[2] == root_i || hierarchy.HaveChild(root_i, pIndices[2]) ||
			pIndices[3] == root_i || hierarchy.HaveChild(root_i, pIndices[3])) ? 1 / 1.0f : 0.0f;
	}
	mesh->UnlockVertexBuffer();
}

bool PhysXContext::UpdateClothParticles(
	PxCloth * cloth,
	unsigned char * pVertices,
	DWORD PositionOffset,
	DWORD Stride)
{
	/* 这里需要传入 std::vector<PxClothParticle>，因为初始化之后这个数组就不再使用了，PxClothParticle 还需要扩展 Indices、Weights用以简化mesh->Lock
	正好可以利用再次变换，那么这里同时也要传入 dualquat列表，用以对invWeight==0的节点进行变换，
	变换之后会产生新的顶点列表，并写入cloth。
	那么当OnRender的时候，还需要从cloth中读出readdata，并且再将其转换为VertexBuffer，同时计算Normal
	计算Normal的方式可以用PxBuildSmoothNormals，参考：PhysX-3.2.3_PC_SDK_Core\Samples\SampleBase\RenderClothActor.cpp
	*/

	PxClothReadData * readData = cloth->lockClothReadData();
	if (readData)
	{
		std::vector<PxClothParticle> NewParticles(cloth->getNbParticles());
		for (unsigned int i = 0; i < cloth->getNbParticles(); i++)
		{
			NewParticles[i].invWeight = readData->particles[i].invWeight;
			if (0 == NewParticles[i].invWeight)
			{
				NewParticles[i].pos = *(PxVec3 *)(pVertices + i * Stride + PositionOffset);
			}
			else
			{
				NewParticles[i].pos = readData->particles[i].pos;
			}
		}
		readData->unlock();
		cloth->setParticles(&NewParticles[0], NULL);
		return true;
	}
	return false;
}

bool PhysXContext::ReadClothParticles(
	my::MeshPtr mesh,
	WORD PositionOffset,
	const PxCloth * cloth)
{
	PxClothReadData * readData = cloth->lockClothReadData();
	if (readData)
	{
		unsigned char * pVertices = (unsigned char *)mesh->LockVertexBuffer();
		for (unsigned int i = 0; i < cloth->getNbParticles(); i++)
		{
			unsigned char * pVertex = pVertices + i * mesh->GetNumBytesPerVertex();
			*(my::Vector3 *)(pVertex + PositionOffset) = (my::Vector3&)readData->particles[i].pos;
		}
		mesh->UnlockVertexBuffer();
		readData->unlock();
	}
	return false;
}

void PhysXSceneContext::StepperTask::run(void)
{
	m_PxScene->SubstepDone(this);
	release();
}

const char * PhysXSceneContext::StepperTask::getName(void) const
{
	return "Stepper Task";
}

bool PhysXSceneContext::OnInit(PxPhysics * sdk, PxDefaultCpuDispatcher * dispatcher)
{
	PxSceneDesc sceneDesc(sdk->getTolerancesScale());
	sceneDesc.gravity = (PxVec3&)PhysXContext::Gravity;
	sceneDesc.cpuDispatcher = dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	if(!(m_PxScene.reset(sdk->createScene(sceneDesc)), m_PxScene))
	{
		THROW_CUSEXCEPTION(_T("sdk->createScene failed"));
	}

	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

	return true;
}

void PhysXSceneContext::OnShutdown(void)
{
	//_ASSERT(!m_PxScene || 0 == m_PxScene->getNbActors(PxActorTypeSelectionFlags(0xff)));

	m_PxScene.reset();
}

void PhysXSceneContext::OnTickPreRender(float dtime)
{
	m_Sync.ResetEvent();

	m_WaitForResults = Advance(dtime);
}

void PhysXSceneContext::OnTickPostRender(float dtime)
{
	if(m_WaitForResults)
	{
		m_Sync.Wait(INFINITE);
	}
}

bool PhysXSceneContext::Advance(float dtime)
{
	m_Timer.m_RemainingTime = my::Min(0.1f, m_Timer.m_RemainingTime + dtime);

	if(m_Timer.m_RemainingTime < m_Timer.m_Interval)
	{
		return false;
	}

	m_Timer.m_RemainingTime -= m_Timer.m_Interval;

	m_Completion0.setContinuation(*m_PxScene->getTaskManager(), NULL);

	Substep(m_Completion0);

	m_Completion0.removeReference();

	return true;
}

void PhysXSceneContext::Substep(StepperTask & completionTask)
{
	m_PxScene->simulate(m_Timer.m_Interval, &completionTask, 0, 0, true);
}

void PhysXSceneContext::SubstepDone(StepperTask * ownerTask)
{
	m_PxScene->fetchResults(true, &m_ErrorState);

	_ASSERT(0 == m_ErrorState);

	OnPxThreadSubstep(m_Timer.m_Interval);

	if(m_Timer.m_RemainingTime < m_Timer.m_Interval)
	{
		m_Sync.SetEvent();
		return;
	}

	m_Timer.m_RemainingTime -= m_Timer.m_Interval;

	StepperTask & task = (ownerTask == &m_Completion0 ? m_Completion1 : m_Completion0);

	task.setContinuation(*m_PxScene->getTaskManager(), NULL);

	Substep(task);

	task.removeReference();
}

void PhysXSceneContext::OnPxThreadSubstep(float dtime)
{
}

void PhysXSceneContext::PushRenderBuffer(my::DrawHelper * drawHelper)
{
	const PxRenderBuffer & debugRenderable = m_PxScene->getRenderBuffer();

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
			drawHelper->PushLine((my::Vector3 &)line.pos0, (my::Vector3 &)line.pos1, line.color0);
		}
	}

	//const PxU32 numTriangles = debugRenderable.getNbTriangles();
	//if(numTriangles)
	//{
	//	const PxDebugTriangle* PX_RESTRICT triangles = debugRenderable.getTriangles();
	//	for(PxU32 i=0; i<numTriangles; i++)
	//	{
	//		const PxDebugTriangle& triangle = triangles[i];
	//	}
	//}
}
