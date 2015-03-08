#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "Animator.h"

class ActorComponent
	: public my::AABBComponent
{
public:
	my::Matrix4 m_World;

public:
	ActorComponent(const my::AABB & aabb)
		: AABBComponent(aabb)
		, m_World(my::Matrix4::Identity())
	{
	}
};

class RenderComponent
	: public ActorComponent
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(const my::AABB & aabb)
		: ActorComponent(aabb)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage) = 0;
};

class MeshComponent
	: public RenderComponent
{
public:
	class LOD
	{
	public:
		MeshComponent * m_owner;
	
	public:
		LOD(MeshComponent * owner)
			: m_owner(owner)
		{
		}

		virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type) = 0;

		virtual void OnSetShader(my::Effect * shader, DWORD AttribId) = 0;
	};

	typedef boost::shared_ptr<LOD> LODPtr;

	class MeshLOD
		: public LOD
	{
	public:
		my::OgreMeshPtr m_Mesh;

		DWORD m_AttribId;

		bool m_bInstance;

		MaterialPtr m_Material;

	public:
		MeshLOD(MeshComponent * owner)
			: LOD(owner)
			, m_AttribId(0)
			, m_bInstance(false)
		{
		}

		virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type);

		virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
	};

	typedef boost::shared_ptr<MeshLOD> MeshLODPtr;

	class IndexdPrimitiveUPLOD
		: public my::DeviceRelatedObjectBase
		, public LOD
	{
	public:
		std::vector<D3DXATTRIBUTERANGE> m_AttribTable;

		CComPtr<IDirect3DVertexDeclaration9> m_Decl;

		my::Cache m_VertexData;

		DWORD m_VertexStride;

		std::vector<unsigned short> m_IndexData;

		MaterialPtrList m_MaterialList;

	public:
		IndexdPrimitiveUPLOD(MeshComponent * owner)
			: LOD(owner)
			, m_VertexStride(0)
		{
		}

		virtual void OnResetDevice(void);

		virtual void OnLostDevice(void);

		virtual void OnDestroyDevice(void);

		virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type);

		virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
	};

	typedef std::vector<LODPtr> LODPtrList;

	LODPtrList m_lods;

	DWORD m_lodId;

public:
	MeshComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
		, m_lodId(0)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class SkeletonMeshComponent
	: public MeshComponent
{
public:
	AnimatorPtr m_Animator;

public:
	SkeletonMeshComponent(const my::AABB & aabb)
		: MeshComponent(aabb)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;

class EmitterMeshComponent
	: public RenderComponent
{
public:
	enum WorldType
	{
		WorldTypeWorld,
		WorldTypeLocal,
	};

	WorldType m_WorldType;

	enum DirectionType
	{
		DirectionTypeCamera,
		DirectionTypeVertical,
		DirectionTypeHorizontal,
	};

	DirectionType m_DirectionType;

	typedef std::vector<my::EmitterPtr> EmitterPtrList;

	EmitterPtrList m_EmitterList;

	MaterialPtrList m_MaterialList;

public:
	EmitterMeshComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
		, m_WorldType(WorldTypeWorld)
		, m_DirectionType(DirectionTypeCamera)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<EmitterMeshComponent> EmitterMeshComponentPtr;