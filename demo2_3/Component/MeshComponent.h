#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "MeshAnimator.h"

class RenderComponent
	: public my::AABBComponent
	, public my::DeviceRelatedObjectBase
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(const my::AABB & aabb)
		: AABBComponent(aabb)
	{
	}

	virtual void OnResetDevice(void)
	{
	}

	virtual void OnLostDevice(void)
	{
	}

	virtual void OnDestroyDevice(void)
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
		MaterialPtrList m_Materials;
	
		my::OgreMeshPtr m_Mesh;
	
	public:
		LOD(void)
		{
		}
	};

	typedef boost::shared_ptr<LOD> LODPtr;

	typedef std::vector<LODPtr> LODPtrList;

	LODPtrList m_lods;

	DWORD m_lodId;

	my::Matrix4 m_World;

public:
	MeshComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
		, m_lodId(0)
		, m_World(my::Matrix4::Identity())
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
	MeshAnimatorPtr m_Animator;

public:
	SkeletonMeshComponent(const my::AABB & aabb)
		: MeshComponent(aabb)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;

class DeformationMeshComponent
	: public RenderComponent
{
public:
	MaterialPtrList m_Materials;

	my::Cache m_VertexData;

	std::vector<unsigned short> m_IndexData;

	std::vector<D3DXATTRIBUTERANGE> m_AttribTable;

	my::D3DVertexElementSet m_VertexElems;

	DWORD m_VertexStride;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	my::Matrix4 m_World;

public:
	DeformationMeshComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
		, m_VertexStride(0)
		, m_World(my::Matrix4::Identity())
	{
	}

	virtual void OnResetDevice(void);

	virtual void OnLostDevice(void);

	virtual void OnDestroyDevice(void);

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	void CreateFromOgreMeshWithoutMaterials(IDirect3DDevice9 * pd3dDevice, my::OgreMeshPtr mesh);
};

typedef boost::shared_ptr<DeformationMeshComponent> DeformationMeshComponentPtr;
