#include "StdAfx.h"
#include "myEmitter.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Emitter)

Emitter::~Emitter(void)
{
}

void Emitter::OnResetDevice(void)
{
}

void Emitter::OnLostDevice(void)
{
}

void Emitter::OnDestroyDevice(void)
{
}

void Emitter::Reset(void)
{
	m_ParticleList.clear();
}

void Emitter::Spawn(const Vector3 & Position, const Vector3 & Velocity)
{
	if(m_ParticleList.size() < PARTICLE_INSTANCE_MAX)
	{
		m_ParticleList.push_back(std::make_pair(0.0f, Particle(Position, Velocity)));
	}
}

void Emitter::Update(double fTime, float fElapsedTime)
{
	ParticlePairList::reverse_iterator part_iter = m_ParticleList.rbegin();
	for(; part_iter != m_ParticleList.rend(); part_iter++)
	{
		if((part_iter->first += fElapsedTime) < m_ParticleLifeTime)
		{
			UpdateParticle(part_iter->second, part_iter->first, fElapsedTime);
		}
		else
		{
			m_ParticleList.erase(m_ParticleList.begin(), m_ParticleList.begin() + (m_ParticleList.rend() - part_iter));
			break;
		}
	}
}

void Emitter::UpdateParticle(Particle & particle, float time, float fElapsedTime)
{
	particle.m_Position += particle.m_Velocity * fElapsedTime;

	particle.m_Color = D3DCOLOR_ARGB(
		(int)m_ParticleColorA.Interpolate(time, 255),
		(int)m_ParticleColorR.Interpolate(time, 255),
		(int)m_ParticleColorG.Interpolate(time, 255),
		(int)m_ParticleColorB.Interpolate(time, 255));

	particle.m_Texcoord1 = Vector4(
		m_ParticleSizeX.Interpolate(time, 1), m_ParticleSizeY.Interpolate(time, 1), m_ParticleAngle.Interpolate(time, 0), 1);

	const unsigned int AnimFrame = (unsigned int)(time * m_ParticleAnimFPS) % ((unsigned int)m_ParticleAnimColumn * m_ParticleAnimRow);
	particle.m_Texcoord2 = Vector4(
		0, 0, (float)(AnimFrame / m_ParticleAnimRow), (float)(AnimFrame % m_ParticleAnimColumn));
}
//
//DWORD Emitter::BuildInstance(
//	ParticleInstance * pInstance,
//	double fTime,
//	float fElapsedTime)
//{
//	_ASSERT(m_ParticleList.size() <= PARTICLE_INSTANCE_MAX);
//
//	unsigned char * pVertices = pInstance->LockInstanceData(m_ParticleList.size());
//	_ASSERT(pVertices);
//	for(DWORD i = 0; i < m_ParticleList.size(); i++)
//	{
//		// ! Can optimize, because all point offset are constant
//		unsigned char * pVertex = pVertices + pInstance->m_InstanceStride * i;
//		pInstance->m_InstanceElems.SetPosition(pVertex, m_ParticleList[i].first->m_Position);
//		pInstance->m_InstanceElems.SetColor(pVertex, m_ParticleList[i].first->m_Color);
//		pInstance->m_InstanceElems.SetVertexValue(pVertex, D3DDECLUSAGE_TEXCOORD, 1, m_ParticleList[i].first->m_Texcoord1);
//		pInstance->m_InstanceElems.SetVertexValue(pVertex, D3DDECLUSAGE_TEXCOORD, 2, m_ParticleList[i].first->m_Texcoord2);
//	}
//	pInstance->UnlockInstanceData();
//
//	return m_ParticleList.size();
//}
//
//void Emitter::Draw(
//	ParticleInstance * pInstance,
//	const Matrix4 & View,
//	double fTime,
//	float fElapsedTime)
//{
//	switch(m_WorldType)
//	{
//	case WorldTypeWorld:
//		pInstance->SetWorld(Matrix4::Identity());
//		break;
//
//	default:
//		pInstance->SetWorld(Matrix4::RotationQuaternion(m_Orientation) * Matrix4::Translation(m_Position));
//		break;
//	}
//
//	switch(m_DirectionType)
//	{
//	case DirectionTypeCamera:
//		pInstance->SetDirection(
//			Vector3(View._13,View._23,View._33),
//			Vector3(View._12,View._22,View._32),
//			Vector3(View._11,View._21,View._31));
//		break;
//
//	case DirectionTypeVertical:
//		{
//			Vector3 Up(0,1,0);
//			Vector3 Right = Up.cross(Vector3(View._13,View._23,View._33));
//			Vector3 Dir = Right.cross(Up);
//			pInstance->SetDirection(Dir, Up, Right);
//		}
//		break;
//
//	case DirectionTypeHorizontal:
//		pInstance->SetDirection(Vector3(0,1,0), Vector3(0,0,1), Vector3(-1,0,0));
//		break;
//	}
//
//	DWORD ParticleCount = BuildInstance(pInstance, fTime, fElapsedTime);
//
//	pInstance->SetTexture(m_Texture);
//
//	pInstance->SetAnimationColumnRow(m_ParticleAnimColumn, m_ParticleAnimRow);
//
//	pInstance->DrawInstance(ParticleCount);
//}

BOOST_CLASS_EXPORT(SphericalEmitter)

void SphericalEmitter::Update(double fTime, float fElapsedTime)
{
	Emitter::Update(fTime, fElapsedTime);

	m_Time += fElapsedTime;

	m_RemainingSpawnTime += fElapsedTime;

	_ASSERT(m_SpawnInterval > 0);

	Vector3 SpawnPos;
	Quaternion SpawnOri;
	switch(m_WorldType)
	{
	case WorldTypeWorld:
		SpawnPos = m_Position;
		SpawnOri = m_Orientation;
		break;

	default:
		SpawnPos = Vector3::zero;
		SpawnOri = Quaternion::identity;
		break;
	}

	while(m_RemainingSpawnTime >= 0)
	{
		Spawn(
			Vector3(
				Random(SpawnPos.x - m_HalfSpawnArea.x, SpawnPos.x + m_HalfSpawnArea.x),
				Random(SpawnPos.y - m_HalfSpawnArea.y, SpawnPos.y + m_HalfSpawnArea.y),
				Random(SpawnPos.z - m_HalfSpawnArea.z, SpawnPos.z + m_HalfSpawnArea.z)),

			Vector3::SphericalToCartesian(
				m_SpawnSpeed,
				m_SpawnInclination.Interpolate(fmod(m_Time, m_SpawnLoopTime), 0),
				m_SpawnAzimuth.Interpolate(fmod(m_Time, m_SpawnLoopTime), 0)).transform(SpawnOri));

		m_RemainingSpawnTime -= m_SpawnInterval;
	}
}
//
//ParticleInstance::ParticleInstance(void)
//{
//}
//
//ParticleInstance::~ParticleInstance(void)
//{
//}
//
//void ParticleInstance::OnResetDevice(void)
//{
//	_ASSERT(!m_VertexBuffer.m_ptr);
//	m_VertexBuffer.CreateVertexBuffer(m_Device, m_VertexStride * 4);
//	unsigned char * pVertices = (unsigned char *)m_VertexBuffer.Lock(0, m_VertexStride * 4);
//	m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 0, Vector2(0,0));
//	m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 1, Vector2(1,0));
//	m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 2, Vector2(1,1));
//	m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 3, Vector2(0,1));
//	m_VertexBuffer.Unlock();
//
//	_ASSERT(!m_IndexData.m_ptr);
//	m_IndexData.CreateIndexBuffer(m_Device, sizeof(WORD) * 4);
//	WORD * pIndices = (WORD *)m_IndexData.Lock(0, sizeof(WORD) * 4);
//	pIndices[0] = 0;
//	pIndices[1] = 1;
//	pIndices[2] = 2;
//	pIndices[3] = 3;
//	m_IndexData.Unlock();
//
//	_ASSERT(!m_InstanceData.m_ptr);
//	m_InstanceData.CreateVertexBuffer(m_Device, m_InstanceStride * PARTICLE_INSTANCE_MAX, 0, 0, D3DPOOL_DEFAULT);
//}
//
//void ParticleInstance::OnLostDevice(void)
//{
//	m_VertexBuffer.OnDestroyDevice();
//	m_IndexData.OnDestroyDevice();
//	m_InstanceData.OnDestroyDevice();
//}
//
//void ParticleInstance::OnDestroyDevice(void)
//{
//	_ASSERT(!m_VertexBuffer.m_ptr);
//	_ASSERT(!m_IndexData.m_ptr);
//	_ASSERT(!m_InstanceData.m_ptr);
//
//	m_Decl.Release();
//	m_Device.Release();
//}
//
//void ParticleInstance::CreateInstance(IDirect3DDevice9 * pd3dDevice)
//{
//	_ASSERT(!m_Device);
//	m_Device = pd3dDevice;
//
//	m_VertexElems.InsertTexcoordElement(0);
//
//	m_InstanceElems.InsertPositionElement(0);
//	WORD offset = sizeof(Vector3);
//	m_InstanceElems.InsertColorElement(offset);
//	offset += sizeof(D3DCOLOR);
//	m_InstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 1);
//	offset += sizeof(Vector4);
//	m_InstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2);
//	offset += sizeof(Vector4);
//
//	m_velist = m_VertexElems.BuildVertexElementList(0);
//	std::vector<D3DVERTEXELEMENT9> ielist = m_InstanceElems.BuildVertexElementList(1);
//	m_velist.insert(m_velist.end(), ielist.begin(), ielist.end());
//	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
//	m_velist.push_back(ve_end);
//
//	m_VertexStride = D3DXGetDeclVertexSize(&m_velist[0], 0);
//	m_InstanceStride = D3DXGetDeclVertexSize(&m_velist[0], 1);
//
//	_ASSERT(!m_VertexBuffer.m_ptr);
//	_ASSERT(!m_IndexData.m_ptr);
//	_ASSERT(!m_InstanceData.m_ptr);
//
//	if(FAILED(hr = pd3dDevice->CreateVertexDeclaration(&m_velist[0], &m_Decl)))
//	{
//		THROW_D3DEXCEPTION(hr);
//	}
//}
//
//unsigned char * ParticleInstance::LockInstanceData(DWORD NumInstances)
//{
//	_ASSERT(NumInstances <= PARTICLE_INSTANCE_MAX);
//
//	unsigned char * ret = (unsigned char *)m_InstanceData.Lock(0, m_InstanceStride * NumInstances, 0);
//	_ASSERT(ret);
//	return ret;
//}
//
//void ParticleInstance::UnlockInstanceData(void)
//{
//	m_InstanceData.Unlock();
//}
//
//void ParticleInstance::DrawInstance(DWORD NumInstances)
//{
//	V(m_Device->SetStreamSource(0, m_VertexBuffer.m_ptr, 0, m_VertexStride));
//	V(m_Device->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));
//
//	V(m_Device->SetStreamSource(1, m_InstanceData.m_ptr, 0, m_InstanceStride));
//	V(m_Device->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));
//
//	V(m_Device->SetVertexDeclaration(m_Decl));
//	V(m_Device->SetIndices(m_IndexData.m_ptr));
//	V(m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, 4, 0, 2));
//
//	V(m_Device->SetStreamSourceFreq(0,1));
//	V(m_Device->SetStreamSourceFreq(1,1));
//}
