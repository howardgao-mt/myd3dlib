#include "StdAfx.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include "myUtility.h"
#include "libc.h"
#include "myCollision.h"
#include "myDxutApp.h"
#include "rapidxml.hpp"
#include <boost/bind.hpp>
#include <fstream>

using namespace my;

void DrawHelper::BeginLine(void)
{
	m_vertices.clear();
}

void DrawHelper::EndLine(IDirect3DDevice9 * pd3dDevice, const Matrix4 & Transform)
{
	if (!m_vertices.empty())
	{
		pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Transform);
		pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, m_vertices.size() / 2, &m_vertices[0], sizeof(m_vertices[0]));
	}
}

void DrawHelper::PushLine(const Vector3 & v0, const Vector3 & v1, D3DCOLOR Color)
{
	m_vertices.push_back(Vertex(v0, Color));
	m_vertices.push_back(Vertex(v1, Color));
}

void DrawHelper::PushWireAABB(const AABB & aabb, D3DCOLOR Color)
{
	Vector3 v[8] = {
		Vector3(aabb.Min.x, aabb.Min.y, aabb.Min.z),
		Vector3(aabb.Min.x, aabb.Min.y, aabb.Max.z),
		Vector3(aabb.Min.x, aabb.Max.y, aabb.Max.z),
		Vector3(aabb.Min.x, aabb.Max.y, aabb.Min.z),
		Vector3(aabb.Max.x, aabb.Min.y, aabb.Min.z),
		Vector3(aabb.Max.x, aabb.Min.y, aabb.Max.z),
		Vector3(aabb.Max.x, aabb.Max.y, aabb.Max.z),
		Vector3(aabb.Max.x, aabb.Max.y, aabb.Min.z),
	};
	PushLine(v[0], v[1], Color); PushLine(v[1], v[2], Color); PushLine(v[2], v[3], Color); PushLine(v[3], v[0], Color);
	PushLine(v[4], v[5], Color); PushLine(v[5], v[6], Color); PushLine(v[6], v[7], Color); PushLine(v[7], v[4], Color);
	PushLine(v[0], v[4], Color); PushLine(v[1], v[5], Color); PushLine(v[2], v[6], Color); PushLine(v[3], v[7], Color);
}

void DrawHelper::PushWireAABB(const AABB & aabb, D3DCOLOR Color, const Matrix4 & Transform)
{
	Vector3 v[8] = {
		Vector3(aabb.Min.x, aabb.Min.y, aabb.Min.z).transformCoord(Transform),
		Vector3(aabb.Min.x, aabb.Min.y, aabb.Max.z).transformCoord(Transform),
		Vector3(aabb.Min.x, aabb.Max.y, aabb.Max.z).transformCoord(Transform),
		Vector3(aabb.Min.x, aabb.Max.y, aabb.Min.z).transformCoord(Transform),
		Vector3(aabb.Max.x, aabb.Min.y, aabb.Min.z).transformCoord(Transform),
		Vector3(aabb.Max.x, aabb.Min.y, aabb.Max.z).transformCoord(Transform),
		Vector3(aabb.Max.x, aabb.Max.y, aabb.Max.z).transformCoord(Transform),
		Vector3(aabb.Max.x, aabb.Max.y, aabb.Min.z).transformCoord(Transform),
	};
	PushLine(v[0], v[1], Color); PushLine(v[1], v[2], Color); PushLine(v[2], v[3], Color); PushLine(v[3], v[0], Color);
	PushLine(v[4], v[5], Color); PushLine(v[5], v[6], Color); PushLine(v[6], v[7], Color); PushLine(v[7], v[4], Color);
	PushLine(v[0], v[4], Color); PushLine(v[1], v[5], Color); PushLine(v[2], v[6], Color); PushLine(v[3], v[7], Color);
}

void DrawHelper::PushGrid(float length, float linesEvery, unsigned subLines, D3DCOLOR GridColor, D3DCOLOR AxisColor)
{
	PushLine(Vector3(-length, 0, 0), Vector3( length, 0, 0), AxisColor);
	PushLine(Vector3(0, 0, -length), Vector3(0, 0,  length), AxisColor);

	float stage = linesEvery / subLines;
	for (float incre = stage; incre < length; incre += stage)
	{
		PushLine(Vector3(-length, 0,  incre), Vector3( length, 0,  incre), GridColor);
		PushLine(Vector3(-length, 0, -incre), Vector3( length, 0, -incre), GridColor);
		PushLine(Vector3( incre, 0, -length), Vector3( incre, 0,  length), GridColor);
		PushLine(Vector3(-incre, 0, -length), Vector3(-incre, 0,  length), GridColor);
	}

	PushLine(Vector3(-length, 0,  length), Vector3( length, 0,  length), GridColor);
	PushLine(Vector3(-length, 0, -length), Vector3( length, 0, -length), GridColor);
	PushLine(Vector3( length, 0, -length), Vector3( length, 0,  length), GridColor);
	PushLine(Vector3(-length, 0, -length), Vector3(-length, 0,  length), GridColor);
}

void Timer::Step(float fElapsedTime, int MaxIter)
{
	m_RemainingTime = Min(4 * m_Interval, m_RemainingTime + fElapsedTime);
	for(int i = 0; m_RemainingTime >= m_Interval && m_Managed; i++)
	{
		if(m_EventTimer)
			m_EventTimer(m_Interval);

		m_RemainingTime -= m_Interval;
	}
}

TimerPtr TimerMgr::AddTimer(float Interval, TimerEvent EventTimer)
{
	TimerPtr timer(new Timer(Interval, Interval));
	timer->m_EventTimer = EventTimer;
	InsertTimer(timer);
	return timer;
}

void TimerMgr::InsertTimer(TimerPtr timer)
{
	if(timer)
	{
		_ASSERT(timer->m_Managed);

		m_timerSet.insert(timer);

		timer->m_Managed = true;
	}
}

void TimerMgr::RemoveTimer(TimerPtr timer)
{
	if(timer)
	{
		_ASSERT(!timer->m_Managed);

		m_timerSet.erase(timer);

		timer->m_Managed = false;
	}
}

void TimerMgr::RemoveAllTimer(void)
{
	m_timerSet.clear();
}

void TimerMgr::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	TimerPtrSet::const_iterator timer_iter = m_timerSet.begin();
	for(; timer_iter != m_timerSet.end(); )
	{
		// ! must step iterator before Update, because TimerEvent will remove self from set
		(*timer_iter++)->Step(fElapsedTime, m_MaxIterCount);
	}
}

void ModelViewerCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_Eular.y, m_Eular.x, m_Eular.z);

	m_Eye = m_LookAt + Rotation[2].xyz * m_Distance;

	m_View = Matrix4::LookAtRH(m_Eye, m_LookAt, Rotation[1].xyz);

	m_Proj = Matrix4::PerspectiveAovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT ModelViewerCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		m_bDrag = true;
		m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
		SetCapture(hWnd);
		*pbNoFurtherProcessing = true;
		return 0;

	case WM_LBUTTONUP:
		if(m_bDrag)
		{
			m_bDrag = false;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			m_Eular.x -= D3DXToRadian(HIWORD(lParam) - m_DragPos.y);
			m_Eular.y -= D3DXToRadian(LOWORD(lParam) - m_DragPos.x);
			m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEWHEEL:
		m_Distance -= (short)HIWORD(wParam) / WHEEL_DELTA;
		*pbNoFurtherProcessing = true;
		return 0;
	}
	return 0;
}

std::pair<Vector3, Vector3> ModelViewerCamera::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	return IntersectionTests::CalculateRay(m_InverseViewProj, m_Eye, pt, Vector2((float)dim.cx, (float)dim.cy));
}

void FirstPersonCamera::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_Eular.y, m_Eular.x, m_Eular.z);

	m_Eye += (m_LocalVel * 5.0f * fElapsedTime).transform(Rotation).xyz;

	m_View = (Rotation * Matrix4::Translation(m_Eye)).inverse();

	m_Proj = Matrix4::PerspectiveFovRH(m_Fov, m_Aspect, m_Nz, m_Fz);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();
}

LRESULT FirstPersonCamera::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		m_bDrag = true;
		m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
		SetCapture(hWnd);
		*pbNoFurtherProcessing = true;
		return 0;

	case WM_LBUTTONUP:
		if(m_bDrag)
		{
			m_bDrag = false;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			m_Eular.x -= D3DXToRadian(HIWORD(lParam) - m_DragPos.y);
			m_Eular.y -= D3DXToRadian(LOWORD(lParam) - m_DragPos.x);
			m_DragPos.SetPoint(LOWORD(lParam),HIWORD(lParam));
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case 'W':
			m_LocalVel.z = -1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'S':
			m_LocalVel.z = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'A':
			m_LocalVel.x = -1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'D':
			m_LocalVel.x = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'E':
			m_LocalVel.y = 1;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'Q':
			m_LocalVel.y = -1;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;

	case WM_KEYUP:
		switch(wParam)
		{
		case 'W':
			if(m_LocalVel.z < 0)
				m_LocalVel.z = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'S':
			if(m_LocalVel.z > 0)
				m_LocalVel.z = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'A':
			if(m_LocalVel.x < 0)
				m_LocalVel.x = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'D':
			if(m_LocalVel.x > 0)
				m_LocalVel.x = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'E':
			if(m_LocalVel.y > 0)
				m_LocalVel.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;

		case 'Q':
			if(m_LocalVel.y < 0)
				m_LocalVel.y = 0;
			*pbNoFurtherProcessing = true;
			return 0;
		}
		break;
	}
	return 0;
}

std::pair<Vector3, Vector3> FirstPersonCamera::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	return IntersectionTests::CalculateRay(m_InverseViewProj, m_Eye, pt, Vector2((float)dim.cx, (float)dim.cy));
}

void EmitterMgr::Update(
	double fTime,
	float fElapsedTime)
{
	EmitterPtrSet::iterator emitter_iter = m_EmitterSet.begin();
	for(; emitter_iter != m_EmitterSet.end(); emitter_iter++)
	{
		(*emitter_iter)->Update(fTime, fElapsedTime);
	}
}

void EmitterMgr::Draw(
	EmitterInstance * pInstance,
	const Matrix4 & ViewProj,
	const Matrix4 & View,
	double fTime,
	float fElapsedTime)
{
	pInstance->SetViewProj(ViewProj);

	EmitterPtrSet::iterator emitter_iter = m_EmitterSet.begin();
	for(; emitter_iter != m_EmitterSet.end(); emitter_iter++)
	{
		(*emitter_iter)->Draw(pInstance, View, fTime, fElapsedTime);
	}
}

void EmitterMgr::InsertEmitter(EmitterPtr emitter)
{
	_ASSERT(m_EmitterSet.end() == m_EmitterSet.find(emitter));

	m_EmitterSet.insert(emitter);
}

void EmitterMgr::RemoveEmitter(EmitterPtr emitter)
{
	m_EmitterSet.erase(emitter);
}

void EmitterMgr::RemoveAllEmitter(void)
{
	m_EmitterSet.clear();
}

EffectParameterBase::~EffectParameterBase(void)
{
}

template <>
void EffectParameter<bool>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetBool(Name.c_str(), m_Value);
}

template <>
void EffectParameter<float>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetFloat(Name.c_str(), m_Value);
}

template <>
void EffectParameter<int>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetInt(Name.c_str(), m_Value);
}

template <>
void EffectParameter<Vector4>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetVector(Name.c_str(), m_Value);
}

template <>
void EffectParameter<Matrix4>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetMatrix(Name.c_str(), m_Value);
}

template <>
void EffectParameter<std::string>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetString(Name.c_str(), m_Value.c_str());
}

template <>
void EffectParameter<BaseTexturePtr>::SetParameter(Effect * pEffect, const std::string & Name) const
{
	pEffect->SetTexture(Name.c_str(), m_Value);
}

void EffectParameterMap::SetBool(const std::string & Name, bool Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<bool>(Value));
}

void EffectParameterMap::SetFloat(const std::string & Name, float Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<float>(Value));
}

void EffectParameterMap::SetInt(const std::string & Name, int Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<int>(Value));
}

void EffectParameterMap::SetVector(const std::string & Name, const Vector4 & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<Vector4>(Value));
}

void EffectParameterMap::SetMatrix(const std::string & Name, const Matrix4 & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<Matrix4>(Value));
}

void EffectParameterMap::SetString(const std::string & Name, const std::string & Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<std::string>(Value));
}

void EffectParameterMap::SetTexture(const std::string & Name, BaseTexturePtr Value)
{
	operator[](Name) = EffectParameterBasePtr(new EffectParameter<BaseTexturePtr>(Value));
}

void Material::OnResetDevice(void)
{
}

void Material::OnLostDevice(void)
{
}

void Material::OnDestroyDevice(void)
{
}

class ResourceMgr::MaterialIORequest : public IORequest
{
protected:
	std::string m_path;

	ResourceMgr * m_arc;

	CachePtr m_cache;

public:
	MaterialIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
		}
	}

	static void OnDiffuseTextureLoaded(ResourceCallbackBoundlePtr boundle, DeviceRelatedObjectBasePtr tex)
	{
		boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_DiffuseTexture = boost::dynamic_pointer_cast<BaseTexture>(tex);
	}

	static void OnNormalTextureLoaded(ResourceCallbackBoundlePtr boundle, DeviceRelatedObjectBasePtr tex)
	{
		boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_NormalTexture = boost::dynamic_pointer_cast<BaseTexture>(tex);
	}

	static void OnSpecularTextureLoaded(ResourceCallbackBoundlePtr boundle, DeviceRelatedObjectBasePtr tex)
	{
		boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_SpecularTexture = boost::dynamic_pointer_cast<BaseTexture>(tex);
	}

	virtual void DoLoadDiffuseTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
	{
		m_arc->LoadTextureAsync(path, boost::bind(&MaterialIORequest::OnDiffuseTextureLoaded, boundle, _1));
	}

	virtual void DoLoadNormalTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
	{
		m_arc->LoadTextureAsync(path, boost::bind(&MaterialIORequest::OnNormalTextureLoaded, boundle, _1));
	}

	virtual void DoLoadSpecularTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
	{
		m_arc->LoadTextureAsync(path, boost::bind(&MaterialIORequest::OnSpecularTextureLoaded, boundle, _1));
	}

	virtual void PostBuildResource(ResourceCallbackBoundlePtr boundle)
	{
		boundle->m_callbacks = m_callbacks;
		m_callbacks.clear();
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		MaterialPtr res(new Material());
		ResourceCallbackBoundlePtr boundle(new ResourceCallbackBoundle(res));
		membuf mb((char *)&(*m_cache)[0], m_cache->size());
		std::istream ims(&mb);
		boost::archive::xml_iarchive ia(ims);
		std::string path;
		ia >> boost::serialization::make_nvp("m_DiffuseTexture", path);
		if (!path.empty())
		{
			DoLoadDiffuseTexture(boundle, path);
		}
		ia >> boost::serialization::make_nvp("m_NormalTexture", path);
		if (!path.empty())
		{
			DoLoadNormalTexture(boundle, path);
		}
		ia >> boost::serialization::make_nvp("m_SpecularTexture", path);
		if (!path.empty())
		{
			DoLoadSpecularTexture(boundle, path);
		}
		m_res = res;
		PostBuildResource(boundle);
	}
};

void ResourceMgr::LoadMaterialAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new MaterialIORequest(callback, path, this)), false);
}

MaterialPtr ResourceMgr::LoadMaterial(const std::string & path)
{
	class SyncMaterialIORequest : public MaterialIORequest
	{
	public:
		SyncMaterialIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
			: MaterialIORequest(callback, path, arc)
		{
		}

		virtual void DoLoadDiffuseTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
		{
			boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_DiffuseTexture = m_arc->LoadTexture(path);
		}

		virtual void DoLoadNormalTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
		{
			boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_NormalTexture = m_arc->LoadTexture(path);
		}

		virtual void DoLoadSpecularTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
		{
			boost::dynamic_pointer_cast<Material>(boundle->m_res)->m_SpecularTexture = m_arc->LoadTexture(path);
		}

		virtual void PostBuildResource(ResourceCallbackBoundlePtr boundle)
		{
		}
	};

	return LoadResource<Material>(path, IORequestPtr(new SyncMaterialIORequest(ResourceCallback(), path, this)));
}

void ResourceMgr::SaveMaterial(const std::string & path, MaterialPtr material)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("m_DiffuseTexture", GetResourceKey(material->m_DiffuseTexture));
	oa << boost::serialization::make_nvp("m_NormalTexture", GetResourceKey(material->m_NormalTexture));
	oa << boost::serialization::make_nvp("m_SpecularTexture", GetResourceKey(material->m_SpecularTexture));
}

class ResourceMgr::EmitterIORequest : public IORequest
{
public:
	std::string m_path;

	ResourceMgr * m_arc;

	CachePtr m_cache;

public:
	EmitterIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	static void OnTextureLoaded(ResourceCallbackBoundlePtr boundle, DeviceRelatedObjectBasePtr tex)
	{
		boost::dynamic_pointer_cast<Emitter>(boundle->m_res)->m_Texture = boost::dynamic_pointer_cast<BaseTexture>(tex);
	}

	virtual void DoLoadTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
	{
		m_arc->LoadTextureAsync(path, boost::bind(&EmitterIORequest::OnTextureLoaded, boundle, _1));
	}

	virtual void PostBuildResource(ResourceCallbackBoundlePtr boundle)
	{
		boundle->m_callbacks = m_callbacks;
		m_callbacks.clear();
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		EmitterPtr res;
		ResourceCallbackBoundlePtr boundle;
		membuf mb((char *)&(*m_cache)[0], m_cache->size());
		std::istream ims(&mb);
		boost::archive::xml_iarchive ia(ims);
		ia >> boost::serialization::make_nvp("Emitter", res);
		boundle.reset(new ResourceCallbackBoundle(res));
		std::string path;
		ia >> boost::serialization::make_nvp("m_Texture", path);
		if (!path.empty())
		{
			DoLoadTexture(boundle, path);
		}
		m_res = res;
		PostBuildResource(boundle);
	}
};

void ResourceMgr::LoadEmitterAsync(const std::string & path, const ResourceCallback & callback)
{
	LoadResourceAsync(path, IORequestPtr(new EmitterIORequest(callback, path, this)), false);
}

EmitterPtr ResourceMgr::LoadEmitter(const std::string & path)
{
	class SyncEmitterIORequest : public EmitterIORequest
	{
	public:
		SyncEmitterIORequest(const ResourceCallback & callback, const std::string & path, ResourceMgr * arc)
			: EmitterIORequest(callback, path, arc)
		{
		}

		virtual void DoLoadTexture(ResourceCallbackBoundlePtr boundle, const std::string & path)
		{
			boost::dynamic_pointer_cast<Emitter>(boundle->m_res)->m_Texture = m_arc->LoadTexture(path);
		}

		virtual void PostBuildResource(ResourceCallbackBoundlePtr boundle)
		{
		}
	};

	return LoadResource<Emitter>(path, IORequestPtr(new SyncEmitterIORequest(ResourceCallback(), path, this)));
}

void ResourceMgr::SaveEmitter(const std::string & path, EmitterPtr emitter)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("Emitter", emitter);
	oa << boost::serialization::make_nvp("m_Texture", GetResourceKey(emitter->m_Texture));
}

void ResourceMgr::SaveMesh(const std::string & path, OgreMeshPtr mesh)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	mesh->SaveMesh(ofs);
}

void ResourceMgr::SaveSimplyMesh(const std::string & path, OgreMeshPtr mesh, DWORD MinFaces)
{
	OgreMeshPtr mesh_sim(new OgreMesh());
	mesh_sim->Create(mesh->SimplifyMesh(&mesh->m_Adjacency[0], MinFaces, D3DXMESHSIMP_FACE).Detach());
	mesh_sim->m_aabb = mesh->m_aabb;
	mesh_sim->m_Adjacency = mesh->m_Adjacency;
	mesh_sim->m_MaterialNameList = mesh->m_MaterialNameList;
	mesh_sim->m_VertexElems = mesh->m_VertexElems;
	SaveMesh(path, mesh_sim);
}

void InputMgr::Create(HINSTANCE hinst, HWND hwnd)
{
	m_input.reset(new Input);
	m_input->CreateInput(hinst);

	JoystickEnumDesc desc;
	desc.input = m_input->m_ptr;
	desc.hwnd = hwnd;
	desc.min_x = -255;
	desc.max_x =  255;
	desc.min_y = -255;
	desc.max_y =  255;
	desc.min_z = -255;
	desc.max_z =  255;
	desc.dead_zone = 10;
	m_input->EnumDevices(DI8DEVCLASS_GAMECTRL, JoystickFinderCallback, &desc, DIEDFL_ATTACHEDONLY);
	m_joystick = desc.joystick;

	::GetCursorPos(&m_MousePos);
	::ScreenToClient(hwnd, &m_MousePos);
}

void InputMgr::Destroy(void)
{
	m_joystick.reset();

	m_input.reset();
}

void InputMgr::Update(double fTime, float fElapsedTime)
{
	if (m_joystick)
	{
		m_joystick->Capture();
	}
}

bool InputMgr::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_KEYDOWN:
		if (m_KeyPressedEvent)
		{
			KeyboardEventArg arg(wParam);
			m_KeyPressedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_SYSKEYDOWN:
		if (m_KeyPressedEvent)
		{
			KeyboardEventArg arg(wParam);
			m_KeyPressedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_KEYUP:
		if (m_KeyReleasedEvent)
		{
			KeyboardEventArg arg(wParam);
			m_KeyReleasedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_SYSKEYUP:
		if (m_KeyReleasedEvent)
		{
			KeyboardEventArg arg(wParam);
			m_KeyReleasedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_MOUSEMOVE:
		if (m_MouseMovedEvent)
		{
			CPoint OldMousePos(m_MousePos);
			m_MousePos.SetPoint(LOWORD(lParam), HIWORD(lParam));
			MouseMoveEventArg arg(m_MousePos.x - OldMousePos.x, m_MousePos.y - OldMousePos.y, 0);
			m_MouseMovedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_LBUTTONDOWN:
		if (m_MousePressedEvent)
		{
			MouseBtnEventArg arg(0);
			m_MousePressedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_LBUTTONUP:
		if (m_MouseReleasedEvent)
		{
			MouseBtnEventArg arg(0);
			m_MouseReleasedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_MBUTTONDOWN:
		if (m_MousePressedEvent)
		{
			MouseBtnEventArg arg(2);
			m_MousePressedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_MBUTTONUP:
		if (m_MouseReleasedEvent)
		{
			MouseBtnEventArg arg(2);
			m_MouseReleasedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_RBUTTONDOWN:
		if (m_MousePressedEvent)
		{
			MouseBtnEventArg arg(1);
			m_MousePressedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_RBUTTONUP:
		if (m_MouseReleasedEvent)
		{
			MouseBtnEventArg arg(1);
			m_MouseReleasedEvent(&arg);
			return arg.handled;
		}
		break;
	case WM_MOUSEWHEEL:
		if (m_MouseMovedEvent)
		{
			MouseMoveEventArg arg(0, 0, (short)HIWORD(wParam) / WHEEL_DELTA);
			m_MouseMovedEvent(&arg);
			return arg.handled;
		}
		break;
	}
	return false;
}

BOOL CALLBACK InputMgr::JoystickFinderCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	_ASSERT(lpddi && pvRef);

	if(lpddi->dwDevType & DI8DEVTYPE_JOYSTICK)
	{
		JoystickEnumDesc * desc = static_cast<JoystickEnumDesc *>(pvRef);
		JoystickPtr joystick(new Joystick);
		joystick->CreateJoystick(
			desc->input, desc->hwnd, lpddi->guidInstance, desc->min_x, desc->max_x, desc->min_y, desc->max_y, desc->min_z, desc->max_z, desc->dead_zone);
		_ASSERT(!desc->joystick);
		desc->joystick = joystick;
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}
