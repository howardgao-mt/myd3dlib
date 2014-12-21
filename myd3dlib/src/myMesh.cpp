#include "stdafx.h"
#include "myMesh.h"
#include "myResource.h"
#include "libc.h"

using namespace my;

#define MESH_INSTANCE_MAX 1024u

void D3DVertexElementSet::InsertVertexElement(WORD Offset, D3DDECLTYPE Type, D3DDECLUSAGE Usage, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	_ASSERT(Type != D3DDECLTYPE_UNUSED);

	_ASSERT(Usage < MAX_USAGE && UsageIndex < MAX_USAGE_INDEX);

	elems[Usage][UsageIndex] = D3DVertexElement(Offset, Type, Method);
}

std::vector<D3DVERTEXELEMENT9> D3DVertexElementSet::BuildVertexElementList(WORD Stream)
{
	std::vector<D3DVERTEXELEMENT9> ret;
	for(unsigned int Usage = 0; Usage < MAX_USAGE; Usage++)
	{
		for(unsigned int UsageIndex = 0; UsageIndex < MAX_USAGE_INDEX; UsageIndex++)
		{
			const D3DVertexElement & elem = elems[Usage][UsageIndex];
			if(elem.Type != D3DDECLTYPE_UNUSED)
			{
				D3DVERTEXELEMENT9 ve = {Stream, elem.Offset, elem.Type, elem.Method, Usage, UsageIndex};
				ret.push_back(ve);
			}
		}
	}
	return ret;
}

unsigned int D3DVertexElementSet::CalcTextureCoords(void)
{
	unsigned int texture_coords = 0;
	for(unsigned int UsageIndex = 0; UsageIndex < MAX_USAGE_INDEX; UsageIndex++)
	{
		const D3DVertexElement & elem = elems[D3DDECLUSAGE_TEXCOORD][UsageIndex];
		if(elem.Type != D3DDECLTYPE_UNUSED)
		{
			texture_coords++;
		}
	}
	return texture_coords;
}

void D3DVertexElementSet::InsertPositionElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION, UsageIndex, Method);
}

Vector3 & D3DVertexElementSet::GetPosition(void * pVertex, BYTE UsageIndex)
{
	_ASSERT(elems[D3DDECLUSAGE_POSITION][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return GetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_POSITION, UsageIndex);
}

void D3DVertexElementSet::SetPosition(void * pVertex, const Vector3 & Position, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_POSITION][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return SetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_POSITION, UsageIndex, Position);
}

void D3DVertexElementSet::InsertBlendWeightElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex, Method);
}

Vector4 & D3DVertexElementSet::GetBlendWeight(void * pVertex, BYTE UsageIndex)
{
	_ASSERT(elems[D3DDECLUSAGE_BLENDWEIGHT][UsageIndex].Type == D3DDECLTYPE_FLOAT4);

	return GetVertexValue<Vector4>(pVertex, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex);
}

void D3DVertexElementSet::SetBlendWeight(void * pVertex, const Vector4 & BlendWeight, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_BLENDWEIGHT][UsageIndex].Type == D3DDECLTYPE_FLOAT4);

	return SetVertexValue<Vector4>(pVertex, D3DDECLUSAGE_BLENDWEIGHT, UsageIndex, BlendWeight);
}

void D3DVertexElementSet::InsertBlendIndicesElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_UBYTE4, D3DDECLUSAGE_BLENDINDICES, UsageIndex, Method);
}

DWORD & D3DVertexElementSet::GetBlendIndices(void * pVertex, BYTE UsageIndex)
{
	_ASSERT(elems[D3DDECLUSAGE_BLENDINDICES][UsageIndex].Type == D3DDECLTYPE_UBYTE4);

	return GetVertexValue<DWORD>(pVertex, D3DDECLUSAGE_BLENDINDICES, UsageIndex);
}

void D3DVertexElementSet::SetBlendIndices(void * pVertex, const DWORD & BlendIndices, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_BLENDINDICES][UsageIndex].Type == D3DDECLTYPE_UBYTE4);

	return SetVertexValue<DWORD>(pVertex, D3DDECLUSAGE_BLENDINDICES, UsageIndex, BlendIndices);
}

void D3DVertexElementSet::InsertNormalElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL, UsageIndex, Method);
}

Vector3 & D3DVertexElementSet::GetNormal(void * pVertex, BYTE UsageIndex)
{
	_ASSERT(elems[D3DDECLUSAGE_NORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return GetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_NORMAL, UsageIndex);
}

void D3DVertexElementSet::SetNormal(void * pVertex, const Vector3 & Normal, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_NORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return SetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_NORMAL, UsageIndex, Normal);
}

void D3DVertexElementSet::InsertTexcoordElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, UsageIndex, Method);
}

Vector2 & D3DVertexElementSet::GetTexcoord(void * pVertex, BYTE UsageIndex)
{
	_ASSERT(elems[D3DDECLUSAGE_TEXCOORD][UsageIndex].Type == D3DDECLTYPE_FLOAT2);

	return GetVertexValue<Vector2>(pVertex, D3DDECLUSAGE_TEXCOORD, UsageIndex);
}

void D3DVertexElementSet::SetTexcoord(void * pVertex, const Vector2 & Texcoord, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_TEXCOORD][UsageIndex].Type == D3DDECLTYPE_FLOAT2);

	return SetVertexValue<Vector2>(pVertex, D3DDECLUSAGE_TEXCOORD, UsageIndex, Texcoord);
}

void D3DVertexElementSet::InsertTangentElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_TANGENT, UsageIndex, Method);
}

Vector3 & D3DVertexElementSet::GetTangent(void * pVertex, BYTE UsageIndex)
{
	_ASSERT(elems[D3DDECLUSAGE_TANGENT][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return GetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_TANGENT, UsageIndex);
}

void D3DVertexElementSet::SetTangent(void * pVertex, const Vector3 & Tangent, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_TANGENT][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return SetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_TANGENT, UsageIndex, Tangent);
}

void D3DVertexElementSet::InsertBinormalElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_BINORMAL, UsageIndex, Method);
}

Vector3 & D3DVertexElementSet::GetBinormal(void * pVertex, BYTE UsageIndex)
{
	_ASSERT(elems[D3DDECLUSAGE_BINORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return GetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_BINORMAL, UsageIndex);
}

void D3DVertexElementSet::SetBinormal(void * pVertex, const Vector3 & Binormal, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_BINORMAL][UsageIndex].Type == D3DDECLTYPE_FLOAT3);

	return SetVertexValue<Vector3>(pVertex, D3DDECLUSAGE_BINORMAL, UsageIndex, Binormal);
}

void D3DVertexElementSet::InsertColorElement(WORD Offset, BYTE UsageIndex, D3DDECLMETHOD Method)
{
	InsertVertexElement(Offset, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR, UsageIndex, Method);
}

D3DCOLOR & D3DVertexElementSet::GetColor(void * pVertex, BYTE UsageIndex)
{
	_ASSERT(elems[D3DDECLUSAGE_COLOR][UsageIndex].Type == D3DDECLTYPE_D3DCOLOR);

	return GetVertexValue<D3DCOLOR>(pVertex, D3DDECLUSAGE_COLOR, UsageIndex);
}

void D3DVertexElementSet::SetColor(void * pVertex, const D3DCOLOR & Color, BYTE UsageIndex) const
{
	_ASSERT(elems[D3DDECLUSAGE_COLOR][UsageIndex].Type == D3DDECLTYPE_D3DCOLOR);

	return SetVertexValue<D3DCOLOR>(pVertex, D3DDECLUSAGE_COLOR, UsageIndex, Color);
}

void VertexBuffer::Create(IDirect3DVertexBuffer9 * ptr)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;
}

void VertexBuffer::CreateVertexBuffer(
	LPDIRECT3DDEVICE9 pDevice,
	UINT Length,
	DWORD Usage,
	DWORD FVF,
	D3DPOOL Pool)
{
	IDirect3DVertexBuffer9 * pVB;
	if(FAILED(hr = pDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, &pVB, 0)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pVB);
}

D3DVERTEXBUFFER_DESC VertexBuffer::GetDesc(void)
{
	D3DVERTEXBUFFER_DESC desc;
	V(m_ptr->GetDesc(&desc));
	return desc;
}

void * VertexBuffer::Lock(UINT OffsetToLock, UINT SizeToLock, DWORD Flags)
{
	void * ret;
	V(m_ptr->Lock(OffsetToLock, SizeToLock, &ret, Flags));
	return ret;
}

void VertexBuffer::Unlock(void)
{
	V(m_ptr->Unlock());
}

void IndexBuffer::Create(IDirect3DIndexBuffer9 * ptr)
{
	_ASSERT(!m_ptr);

	m_ptr = ptr;
}

void IndexBuffer::CreateIndexBuffer(
	LPDIRECT3DDEVICE9 pDevice,
	UINT Length,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool)
{
	IDirect3DIndexBuffer9 * pIB;
	if(FAILED(hr = pDevice->CreateIndexBuffer(Length, Usage, Format, Pool, &pIB, 0)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	Create(pIB);
}

D3DINDEXBUFFER_DESC IndexBuffer::GetDesc(void)
{
	D3DINDEXBUFFER_DESC desc;
	V(m_ptr->GetDesc(&desc));
	return desc;
}

void * IndexBuffer::Lock(UINT OffsetToLock, UINT SizeToLock, DWORD Flags)
{
	void * ret;
	V(m_ptr->Lock(OffsetToLock, SizeToLock, &ret, Flags));
	return ret;
}

void IndexBuffer::Unlock(void)
{
	V(m_ptr->Unlock());
}

void Mesh::Create(ID3DXMesh * pMesh)
{
	_ASSERT(!m_ptr);

	m_ptr = pMesh;
}

void Mesh::CreateMesh(
	LPDIRECT3DDEVICE9 pD3DDevice,
	DWORD NumFaces,
	DWORD NumVertices,
	CONST LPD3DVERTEXELEMENT9 pDeclaration,
	DWORD Options)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateMesh(NumFaces, NumVertices, Options, pDeclaration, pD3DDevice, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateMeshFVF(
	LPDIRECT3DDEVICE9 pD3DDevice,
	DWORD NumFaces,
	DWORD NumVertices,
	DWORD FVF,
	DWORD Options)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateMeshFVF(NumFaces, NumVertices, Options, FVF, pD3DDevice, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateMeshFromX(
	LPDIRECT3DDEVICE9 pD3DDevice,
	LPCSTR pFilename,
	DWORD Options,
	LPD3DXBUFFER * ppAdjacency,
	LPD3DXBUFFER * ppMaterials,
	LPD3DXBUFFER * ppEffectInstances,
	DWORD * pNumMaterials)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXLoadMeshFromXA(
		pFilename, Options, pD3DDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateMeshFromXInMemory(
	LPDIRECT3DDEVICE9 pD3DDevice,
	LPCVOID Memory,
	DWORD SizeOfMemory,
	DWORD Options,
	LPD3DXBUFFER * ppAdjacency,
	LPD3DXBUFFER * ppMaterials,
	LPD3DXBUFFER * ppEffectInstances,
	DWORD * pNumMaterials)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXLoadMeshFromXInMemory(
		Memory, SizeOfMemory, Options, pD3DDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, &pMesh);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateBox(
	LPDIRECT3DDEVICE9 pd3dDevice,
	FLOAT Width,
	FLOAT Height,
	FLOAT Depth,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateBox(pd3dDevice, Width, Height, Depth, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateCylinder(
	LPDIRECT3DDEVICE9 pd3dDevice,
	FLOAT Radius1,
	FLOAT Radius2,
	FLOAT Length,
	UINT Slices,
	UINT Stacks,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateCylinder(pd3dDevice, Radius1, Radius2, Length, Slices, Stacks, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreatePolygon(
	LPDIRECT3DDEVICE9 pDevice,
	FLOAT Length,
	UINT Sides,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreatePolygon(pDevice, Length, Sides, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateSphere(
	LPDIRECT3DDEVICE9 pDevice,
	FLOAT Radius,
	UINT Slices,
	UINT Stacks,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateSphere(pDevice, Radius, Slices, Stacks, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateTeapot(
	LPDIRECT3DDEVICE9 pDevice,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateTeapot(pDevice, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

void Mesh::CreateTorus(
	LPDIRECT3DDEVICE9 pDevice,
	FLOAT InnerRadius,
	FLOAT OuterRadius,
	UINT Sides,
	UINT Rings,
	LPD3DXBUFFER * ppAdjacency)
{
	LPD3DXMESH pMesh = NULL;
	HRESULT hres = D3DXCreateTorus(pDevice, InnerRadius, OuterRadius, Sides, Rings, &pMesh, ppAdjacency);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	Create(pMesh);
}

CComPtr<ID3DXMesh> Mesh::CloneMesh(DWORD Options, CONST D3DVERTEXELEMENT9 * pDeclaration, LPDIRECT3DDEVICE9 pDevice)
{
	CComPtr<ID3DXMesh> ret;
	V(m_ptr->CloneMesh(Options, pDeclaration, pDevice, &ret));
	return ret;
}

CComPtr<ID3DXMesh> Mesh::CloneMeshFVF(DWORD Options, DWORD FVF, LPDIRECT3DDEVICE9 pDevice)
{
	CComPtr<ID3DXMesh> ret;
	V(m_ptr->CloneMeshFVF(Options, FVF, pDevice, &ret));
	return ret;
}

CComPtr<ID3DXMesh> Mesh::CleanMesh(D3DXCLEANTYPE CleanType, const DWORD *pAdjacencyIn, DWORD *pAdjacencyOut)
{
	CComPtr<ID3DXMesh> ret;
	CComPtr<ID3DXBuffer> ErrorMsgs;
	HRESULT hres = D3DXCleanMesh(CleanType, m_ptr, pAdjacencyIn, &ret, pAdjacencyOut, &ErrorMsgs);
	if (FAILED(hres))
	{
		if (ErrorMsgs)
		{
			THROW_CUSEXCEPTION(ms2ts((char *)ErrorMsgs->GetBufferPointer()));
		}
		THROW_D3DEXCEPTION(hres);
	}
	return ret;
}

CComPtr<ID3DXMesh> Mesh::SimplifyMesh(
	const DWORD *pAdjacency,
	DWORD MinValue,
	DWORD Options,
	const D3DXATTRIBUTEWEIGHTS *pVertexAttributeWeights,
	const FLOAT *pVertexWeights)
{
	std::vector<DWORD> outAdjacency(GetNumFaces() * 3);
	CComPtr<ID3DXMesh> mesh = CleanMesh(D3DXCLEAN_BOWTIES, pAdjacency, &outAdjacency[0]);
	CComPtr<ID3DXMesh> ret = NULL;
	hr = D3DXSimplifyMesh(mesh, &outAdjacency[0], pVertexAttributeWeights, pVertexWeights, MinValue, Options, &ret);
	if (FAILED(hr))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return ret;
}

void Mesh::ConvertAdjacencyToPointReps(CONST DWORD * pAdjacency, DWORD * pPRep)
{
	V(m_ptr->ConvertAdjacencyToPointReps(pAdjacency, pPRep));
}

void Mesh::ConvertPointRepsToAdjacency(CONST DWORD* pPRep, DWORD* pAdjacency)
{
	V(m_ptr->ConvertPointRepsToAdjacency(pPRep, pAdjacency));
}

void Mesh::DrawSubset(DWORD AttribId)
{
	V(m_ptr->DrawSubset(AttribId));
}

void Mesh::GenerateAdjacency(FLOAT Epsilon, DWORD * pAdjacency)
{
	V(m_ptr->GenerateAdjacency(Epsilon, pAdjacency));
}

void Mesh::GetAttributeTable(D3DXATTRIBUTERANGE * pAttribTable, DWORD * pAttribTableSize)
{
	V(m_ptr->GetAttributeTable(pAttribTable, pAttribTableSize));
}

void Mesh::GetDeclaration(D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE])
{
	V(m_ptr->GetDeclaration(Declaration));
}

CComPtr<IDirect3DDevice9> Mesh::GetDevice(void)
{
	CComPtr<IDirect3DDevice9> Device;
	V(m_ptr->GetDevice(&Device));
	return Device;
}

DWORD Mesh::GetFVF(void)
{
	return m_ptr->GetFVF();
}

CComPtr<IDirect3DIndexBuffer9> Mesh::GetIndexBuffer(void)
{
	CComPtr<IDirect3DIndexBuffer9> IndexBuffer;
	V(m_ptr->GetIndexBuffer(&IndexBuffer));
	return IndexBuffer;
}

DWORD Mesh::GetNumBytesPerVertex(void)
{
	return m_ptr->GetNumBytesPerVertex();
}

DWORD Mesh::GetNumFaces(void)
{
	return m_ptr->GetNumFaces();
}

DWORD Mesh::GetNumVertices(void)
{
	return m_ptr->GetNumVertices();
}

DWORD Mesh::GetOptions(void)
{
	return m_ptr->GetOptions();
}

CComPtr<IDirect3DVertexBuffer9> Mesh::GetVertexBuffer(void)
{
	CComPtr<IDirect3DVertexBuffer9> VertexBuffer;
	V(m_ptr->GetVertexBuffer(&VertexBuffer));
	return VertexBuffer;
}

LPVOID Mesh::LockIndexBuffer(DWORD Flags)
{
	LPVOID pData = NULL;
	if(FAILED(hr = m_ptr->LockIndexBuffer(Flags, &pData)))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return pData;
}

LPVOID Mesh::LockVertexBuffer(DWORD Flags)
{
	LPVOID pData = NULL;
	if(FAILED(hr = m_ptr->LockVertexBuffer(Flags, &pData)))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return pData;
}

void Mesh::UnlockIndexBuffer(void)
{
	V(m_ptr->UnlockIndexBuffer());
}

void Mesh::UnlockVertexBuffer(void)
{
	V(m_ptr->UnlockVertexBuffer());
}

void Mesh::UpdateSemantics(D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE])
{
	V(m_ptr->UpdateSemantics(Declaration));
}

DWORD * Mesh::LockAttributeBuffer(DWORD Flags)
{
	DWORD * pData = NULL;
	if(FAILED(hr = m_ptr->LockAttributeBuffer(Flags, &pData)))
	{
		THROW_D3DEXCEPTION(hr);
	}
	return pData;
}

CComPtr<ID3DXMesh> Mesh::Optimize(
	DWORD Flags,
	CONST DWORD * pAdjacencyIn,
	DWORD * pAdjacencyOut,
	DWORD * pFaceRemap,
	LPD3DXBUFFER * ppVertexRemap)
{
	CComPtr<ID3DXMesh> ret;
	V(m_ptr->Optimize(Flags, pAdjacencyIn, pAdjacencyOut, pFaceRemap, ppVertexRemap, &ret));
	return ret;
}

void Mesh::OptimizeInplace(DWORD Flags,
	CONST DWORD * pAdjacencyIn,
	DWORD * pAdjacencyOut,
	DWORD * pFaceRemap,
	LPD3DXBUFFER * ppVertexRemap)
{
	V(m_ptr->OptimizeInplace(Flags, pAdjacencyIn, pAdjacencyOut, pFaceRemap, ppVertexRemap));
}

void Mesh::SetAttributeTable(CONST D3DXATTRIBUTERANGE * pAttribTable, DWORD cAttribTableSize)
{
	V(m_ptr->SetAttributeTable(pAttribTable, cAttribTableSize));
}

void Mesh::UnlockAttributeBuffer(void)
{
	V(m_ptr->UnlockAttributeBuffer());
}

MeshInstance::MeshInstance(void)
	: m_VertexStride(0)
	, m_InstanceStride(0)
{
}

MeshInstance::~MeshInstance(void)
{
}

void MeshInstance::OnResetDevice(void)
{
	Mesh::OnResetDevice();

	if (m_InstanceStride && !m_InstanceData.m_ptr)
	{
		m_InstanceData.CreateVertexBuffer(m_Device, m_InstanceStride * MESH_INSTANCE_MAX, 0, 0, D3DPOOL_DEFAULT);
	}
}

void MeshInstance::OnLostDevice(void)
{
	if (m_InstanceData.m_ptr)
	{
		m_InstanceData.OnDestroyDevice();
	}

	Mesh::OnLostDevice();
}

void MeshInstance::OnDestroyDevice(void)
{
	m_Device.Release();
	m_Decl.Release();
	if (m_InstanceData.m_ptr)
	{
		m_InstanceData.OnDestroyDevice();
	}

	Mesh::OnDestroyDevice();
}

void MeshInstance::CreateInstance(IDirect3DDevice9 * pd3dDevice)
{
	m_Device = GetDevice();
	_ASSERT(m_Device);

	DWORD submeshes = 0;
	GetAttributeTable(NULL, &submeshes);
	m_AttribTable.resize(submeshes);
	GetAttributeTable(&m_AttribTable[0], &submeshes);
	_ASSERT(m_AttribTable.size() == submeshes);

	WORD offset = 0;
	m_InstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 1);
	offset += sizeof(Vector4);
	m_InstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 2);
	offset += sizeof(Vector4);
	m_InstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 3);
	offset += sizeof(Vector4);
	m_InstanceElems.InsertVertexElement(offset, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION, 4);
	offset += sizeof(Vector4);

	m_velist.resize(MAX_FVF_DECL_SIZE);
	GetDeclaration(&m_velist[0]);

	unsigned int i = 0;
	for (; i < m_velist.size(); i++)
	{
		if (m_velist[i].Stream == 0xff || m_velist[i].Type == D3DDECLTYPE_UNUSED)
		{
			break;
		}
	}

	if (i >= m_velist.size())
	{
		THROW_CUSEXCEPTION(_T("invalid vertex declaration"));
	}

	std::vector<D3DVERTEXELEMENT9> ielist = m_InstanceElems.BuildVertexElementList(1);
	m_velist.insert(m_velist.begin() + i, ielist.begin(), ielist.end());

	m_VertexStride = D3DXGetDeclVertexSize(&m_velist[0], 0);
	m_InstanceStride = D3DXGetDeclVertexSize(&m_velist[0], 1);

	_ASSERT(!m_InstanceData.m_ptr);

	if (FAILED(hr = m_Device->CreateVertexDeclaration(&m_velist[0], &m_Decl)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	if (m_InstanceStride && !m_InstanceData.m_ptr)
	{
		m_InstanceData.CreateVertexBuffer(m_Device, m_InstanceStride * MESH_INSTANCE_MAX, 0, 0, D3DPOOL_DEFAULT);
	}
}

Matrix4 * MeshInstance::LockInstanceData(DWORD NumInstances)
{
	_ASSERT(NumInstances <= MESH_INSTANCE_MAX);

	Matrix4 * ret = (Matrix4 *)m_InstanceData.Lock(0, m_InstanceStride * NumInstances, 0);
	_ASSERT(ret);
	return ret;
}

void MeshInstance::UnlockInstanceData(void)
{
	m_InstanceData.Unlock();
}

void MeshInstance::DrawSubsetInstance(DWORD AttribId, DWORD NumInstances)
{
	_ASSERT(AttribId <= m_AttribTable.size());
	_ASSERT(NumInstances <= MESH_INSTANCE_MAX);

	_ASSERT(m_Device);

	CComPtr<IDirect3DVertexBuffer9> vb = GetVertexBuffer();
	CComPtr<IDirect3DIndexBuffer9> ib = GetIndexBuffer();

	V(m_Device->SetStreamSource(0, vb, 0, m_VertexStride));
	V(m_Device->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | NumInstances));

	V(m_Device->SetStreamSource(1, m_InstanceData.m_ptr, 0, m_InstanceStride));
	V(m_Device->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));

	V(m_Device->SetVertexDeclaration(m_Decl));
	V(m_Device->SetIndices(ib));
	V(m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,
		m_AttribTable[AttribId].VertexStart, m_AttribTable[AttribId].VertexCount, m_AttribTable[AttribId].FaceStart, m_AttribTable[AttribId].FaceCount));

	V(m_Device->SetStreamSourceFreq(0,1));
	V(m_Device->SetStreamSourceFreq(1,1));
}

void OgreMesh::CreateMeshFromOgreXmlInFile(
	LPDIRECT3DDEVICE9 pd3dDevice,
	LPCTSTR pFilename,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
{
	CachePtr cache = FileIStream::Open(pFilename)->GetWholeCache();
	cache->push_back(0);

	CreateMeshFromOgreXmlInMemory(pd3dDevice, (char *)&(*cache)[0], cache->size(), bComputeTangentFrame, dwMeshOptions);
}

void OgreMesh::CreateMeshFromOgreXmlInMemory(
	LPDIRECT3DDEVICE9 pd3dDevice,
	LPSTR pSrcData,
	UINT srcDataLen,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
{
	_ASSERT(0 == pSrcData[srcDataLen-1]);

	rapidxml::xml_document<char> doc;
	try
	{
		doc.parse<0>(pSrcData);
	}
	catch(rapidxml::parse_error & e)
	{
		THROW_CUSEXCEPTION(ms2ts(e.what()));
	}

	CreateMeshFromOgreXml(pd3dDevice, &doc, bComputeTangentFrame, dwMeshOptions);
}

void OgreMesh::CreateMeshFromOgreXml(
	LPDIRECT3DDEVICE9 pd3dDevice,
	const rapidxml::xml_node<char> * node_root,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
{
	DEFINE_XML_NODE_SIMPLE(mesh, root);
	DEFINE_XML_NODE_SIMPLE(sharedgeometry, mesh);
	DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
	rapidxml::xml_node<char> * node_submesh = node_submeshes->first_node("submesh");
	rapidxml::xml_node<char> * node_boneassignments = node_mesh->first_node("boneassignments");

	CreateMeshFromOgreXmlNodes(pd3dDevice, node_sharedgeometry, node_boneassignments, node_submesh, true, bComputeTangentFrame, dwMeshOptions);
}

void OgreMesh::CreateMeshFromOgreXmlNodes(
	LPDIRECT3DDEVICE9 pd3dDevice,
	const rapidxml::xml_node<char> * node_geometry,
	const rapidxml::xml_node<char> * node_boneassignments,
	const rapidxml::xml_node<char> * node_submesh,
	const bool bUseSharedGeometry,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
{
	DEFINE_XML_ATTRIBUTE_INT_SIMPLE(vertexcount, geometry);
	DEFINE_XML_NODE_SIMPLE(vertexbuffer, geometry);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(positions, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(normals, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(colours_diffuse, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(colours_specular, vertexbuffer);
	DEFINE_XML_ATTRIBUTE_INT_SIMPLE(texture_coords, vertexbuffer);

	if((dwMeshOptions & ~D3DXMESH_32BIT) && vertexcount >= USHRT_MAX)
	{
		//THROW_CUSEXCEPTION(_T("facecount overflow ( >= 65535 )"));
		dwMeshOptions |= D3DXMESH_32BIT;
	}

	if(!positions)
	{
		THROW_CUSEXCEPTION(_T("cannot process non-position vertex"));
	}

	m_VertexElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);

	if(normals || bComputeTangentFrame)
	{
		m_VertexElems.InsertNormalElement(offset);
		offset += sizeof(Vector3);
	}

	if(bComputeTangentFrame)
	{
		m_VertexElems.InsertTangentElement(offset);
		offset += sizeof(Vector3);
	}

	if(texture_coords > MAXBYTE)
	{
		THROW_CUSEXCEPTION(_T("texture coords overflow ( > 255 )"));
	}

	for(int i = 0; i < texture_coords; i++)
	{
		m_VertexElems.InsertTexcoordElement(offset, i);
		offset += sizeof(Vector2);
	}

	WORD indicesOffset = 0, weightsOffset = 0;
	if(node_boneassignments != NULL && node_boneassignments->first_node("vertexboneassignment"))
	{
		m_VertexElems.InsertBlendIndicesElement(offset);
		offset += sizeof(DWORD);

		m_VertexElems.InsertBlendWeightElement(offset);
		offset += sizeof(Vector4);
	}

	int facecount = 0;
	const rapidxml::xml_node<char> * node_submesh_iter = node_submesh;
	for(; node_submesh_iter != NULL; node_submesh_iter = bUseSharedGeometry ? node_submesh_iter->next_sibling() : NULL)
	{
		DEFINE_XML_NODE_SIMPLE(faces, submesh_iter);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);
		facecount += count;
	}

	std::vector<D3DVERTEXELEMENT9> velist = m_VertexElems.BuildVertexElementList(0);
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	velist.push_back(ve_end);

	CreateMesh(pd3dDevice, facecount, vertexcount, (D3DVERTEXELEMENT9 *)&velist[0], dwMeshOptions);

	m_aabb.Min = Vector3(FLT_MAX,FLT_MAX,FLT_MAX);
	m_aabb.Max = Vector3(FLT_MIN,FLT_MIN,FLT_MIN);

	VOID * pVertices = LockVertexBuffer();
	DEFINE_XML_NODE_SIMPLE(vertex, vertexbuffer);
	for(int vertex_i = 0; node_vertex != NULL && vertex_i < vertexcount; node_vertex = node_vertex->next_sibling(), vertex_i++)
	{
		unsigned char * pVertex = (unsigned char *)pVertices + vertex_i * offset;
		if(positions)
		{
			DEFINE_XML_NODE_SIMPLE(position, vertex);
			Vector3 & Position = m_VertexElems.GetPosition(pVertex);
			rapidxml::xml_attribute<char> * attr_tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(Position.x, attr_tmp, node_position, x);
			DEFINE_XML_ATTRIBUTE_FLOAT(Position.y, attr_tmp, node_position, y);
			DEFINE_XML_ATTRIBUTE_FLOAT(Position.z, attr_tmp, node_position, z);

			m_aabb.Min.x = Min(m_aabb.Min.x, Position.x);
			m_aabb.Min.y = Min(m_aabb.Min.y, Position.y);
			m_aabb.Min.z = Min(m_aabb.Min.z, Position.z);

			m_aabb.Max.x = Max(m_aabb.Max.x, Position.x);
			m_aabb.Max.y = Max(m_aabb.Max.y, Position.y);
			m_aabb.Max.z = Max(m_aabb.Max.z, Position.z);
		}

		if(normals)
		{
			DEFINE_XML_NODE_SIMPLE(normal, vertex);
			Vector3 & Normal = m_VertexElems.GetNormal(pVertex);
			rapidxml::xml_attribute<char> * attr_tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(Normal.x, attr_tmp, node_normal, x);
			DEFINE_XML_ATTRIBUTE_FLOAT(Normal.y, attr_tmp, node_normal, y);
			DEFINE_XML_ATTRIBUTE_FLOAT(Normal.z, attr_tmp, node_normal, z);
		}

		rapidxml::xml_node<char> * node_texcoord = node_vertex->first_node("texcoord");
		for(int i = 0; i < texture_coords && node_texcoord != NULL; i++, node_texcoord = node_texcoord->next_sibling())
		{
			Vector2 & Texcoord = m_VertexElems.GetTexcoord(pVertex, i);
			rapidxml::xml_attribute<char> * attr_tmp;
			DEFINE_XML_ATTRIBUTE_FLOAT(Texcoord.x, attr_tmp, node_texcoord, u);
			DEFINE_XML_ATTRIBUTE_FLOAT(Texcoord.y, attr_tmp, node_texcoord, v);
		}

		if(node_boneassignments != NULL && node_boneassignments->first_node("vertexboneassignment"))
		{
			m_VertexElems.SetBlendIndices(pVertex, 0);
			m_VertexElems.SetBlendWeight(pVertex, Vector4::zero);
		}
	}

	if(node_boneassignments != NULL)
	{
		rapidxml::xml_node<char> * node_vertexboneassignment = node_boneassignments->first_node("vertexboneassignment");
		for(; node_vertexboneassignment != NULL; node_vertexboneassignment = node_vertexboneassignment->next_sibling())
		{
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(vertexindex, vertexboneassignment);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(boneindex, vertexboneassignment);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(weight, vertexboneassignment);

			if(vertexindex >= vertexcount)
			{
				THROW_CUSEXCEPTION(str_printf(_T("invalid vertex index: %d"), vertexindex));
			}

			if(boneindex >= 0xff)
			{
				THROW_CUSEXCEPTION(str_printf(_T("invalid bone index: %d"), boneindex));
			}

			unsigned char * pVertex = (unsigned char *)pVertices + vertexindex * offset;
			unsigned char * pIndices = (unsigned char *)&m_VertexElems.GetBlendIndices(pVertex);
			float * pWeights = (float *)&m_VertexElems.GetBlendWeight(pVertex);

			int i = 0;
			for(; i < D3DVertexElementSet::MAX_BONE_INDICES; i++)
			{
				if(pWeights[i] == 0)
				{
					pIndices[i] = boneindex;
					pWeights[i] = weight;
					break;
				}
			}

			if(i >= D3DVertexElementSet::MAX_BONE_INDICES)
			{
				THROW_CUSEXCEPTION(_T("too much bone assignment"));
			}
		}
	}
	UnlockVertexBuffer();

	VOID * pIndices = LockIndexBuffer();
	DWORD * pAttrBuffer = LockAttributeBuffer();
	int submesh_i = 0;
	node_submesh_iter = node_submesh;
	for(int face_i = 0; node_submesh_iter != NULL; node_submesh_iter = bUseSharedGeometry ? node_submesh_iter->next_sibling() : NULL, submesh_i++)
	{
		DEFINE_XML_ATTRIBUTE_SIMPLE(material, submesh_iter);
		DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(use32bitindexes, submesh_iter);
		DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(usesharedvertices, submesh_iter);
		DEFINE_XML_ATTRIBUTE_SIMPLE(operationtype, submesh_iter);
		if(usesharedvertices != bUseSharedGeometry || 0 != _stricmp(attr_operationtype->value(), "triangle_list"))
		{
			THROW_CUSEXCEPTION(_T("!usesharedvertices || !triangle_list"));
		}

		DEFINE_XML_NODE_SIMPLE(faces, submesh_iter);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(count, faces);

		DEFINE_XML_NODE_SIMPLE(face, faces);
		for(; node_face != NULL && face_i < facecount; node_face = node_face->next_sibling(), face_i++)
		{
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v1, face);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v2, face);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(v3, face);

			if(dwMeshOptions & D3DXMESH_32BIT)
			{
				*((DWORD *)pIndices + face_i * 3 + 0) = v1;
				*((DWORD *)pIndices + face_i * 3 + 1) = v2;
				*((DWORD *)pIndices + face_i * 3 + 2) = v3;
			}
			else
			{
				*((WORD *)pIndices + face_i * 3 + 0) = v1;
				*((WORD *)pIndices + face_i * 3 + 1) = v2;
				*((WORD *)pIndices + face_i * 3 + 2) = v3;
			}
			pAttrBuffer[face_i] = submesh_i;
		}

		m_MaterialNameList.push_back(attr_material->value());
	}
	UnlockAttributeBuffer();
	UnlockIndexBuffer();

	std::vector<DWORD> adjacency(GetNumFaces() * 3);
	GenerateAdjacency((float)EPSILON_E6, &adjacency[0]);
	if(bComputeTangentFrame)
	{
		//DWORD dwOptions = D3DXTANGENT_GENERATE_IN_PLACE;
		//if(!normals)
		//	dwOptions |= D3DXTANGENT_CALCULATE_NORMALS;
		//HRESULT hres = D3DXComputeTangentFrameEx(
		//	m_ptr, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLUSAGE_TANGENT, 0, D3DX_DEFAULT, 0, D3DDECLUSAGE_NORMAL, 0, dwOptions, &m_Adjacency[0], -1.01f, -0.01f, -1.01f, NULL, NULL);
		//if(FAILED(hres))
		//{
		//	THROW_D3DEXCEPTION(hres);
		//}
		ComputeTangentFrame();
	}
	m_Adjacency.resize(adjacency.size());
	OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, &adjacency[0], &m_Adjacency[0], NULL, NULL);
}

void OgreMesh::SaveMesh(std::ostream & ostr)
{
	ostr << "<mesh>\n";
	ostr << "\t<sharedgeometry vertexcount=\"" << GetNumVertices() << "\">\n";
	bool normals = m_VertexElems.elems[D3DDECLUSAGE_NORMAL][0].Type == D3DDECLTYPE_FLOAT3;
	ostr << "\t\t<vertexbuffer positions=\"true\" normals=";
	if (normals)
		ostr << "\"true\"";
	else
		ostr << "\"false\"";
	ostr << " colours_diffuse=\"false\" colours_specular=\"false\" texture_coords=\"";
	unsigned int texture_coords = m_VertexElems.CalcTextureCoords();
	if (texture_coords)
		ostr << texture_coords << "\">\n";
	else
		ostr << 0 << "\">\n";
	void * pVertices = LockVertexBuffer();
	DWORD VertexStride = GetNumBytesPerVertex();
	// write vertex data
	for (DWORD i=0; i < GetNumVertices(); i++)
	{
		ostr << "\t\t\t<vertex>\n";
		unsigned char * pVertex = (unsigned char *)pVertices + i * VertexStride;
		const Vector3 vertex = m_VertexElems.GetPosition(pVertex);
		//write vertex position
		ostr << "\t\t\t\t<position x=\"" << vertex.x << "\" y=\"" << vertex.y << "\" " << "z=\"" << vertex.z << "\"/>\n";
		//write vertex normal
		if (normals)
		{
			const Vector3 normal = m_VertexElems.GetNormal(pVertex);
			ostr << "\t\t\t\t<normal x=\"" << normal.x << "\" y=\"" << normal.y << "\" " << "z=\"" << normal.z << "\"/>\n";
		}
		//write vertex texture coordinates
		if (texture_coords)
		{
			for (DWORD j=0; j<texture_coords; j++)
			{
				const Vector2 texcoord = m_VertexElems.GetTexcoord(pVertex, (BYTE)j);
				ostr << "\t\t\t\t<texcoord u=\"" << texcoord.x << "\" v=\"" << texcoord.y << "\"/>\n";
			}
		}
		ostr << "\t\t\t</vertex>\n";
	}
	ostr << "\t\t</vertexbuffer>\n";
	ostr << "\t</sharedgeometry>\n";
	DWORD submeshes = 0;
	GetAttributeTable(NULL, &submeshes);
	std::vector<D3DXATTRIBUTERANGE> rang(submeshes);
	GetAttributeTable(&rang[0], &submeshes);
	VOID * pIndices = LockIndexBuffer();
	// write submeshes data
	ostr << "\t<submeshes>\n";
	for (DWORD i=0; i < submeshes; i++)
	{
		// Start submesh description
		ostr << "\t\t<submesh ";
		// Write material name
		ostr << "material=\"" << m_MaterialNameList[rang[i].AttribId] << "\" ";
		DWORD use32bitindexes = GetOptions() & D3DXMESH_32BIT;
		// Write use32bitIndexes flag
		ostr << "use32bitindexes=\"";
		if (use32bitindexes)
			ostr << "true";
		else
			ostr << "false";
		ostr << "\" ";
		// Write operation type flag
		ostr << "usesharedvertices=\"true\" operationtype=\"triangle_list\">\n";

		// Write submesh polygons
		ostr << "\t\t\t<faces count=\"" << rang[i].FaceCount << "\">\n";
		for (DWORD face_i=rang[i].FaceStart; face_i<rang[i].FaceStart+rang[i].FaceCount; face_i++)
		{
			if (use32bitindexes)
			{
				ostr << "\t\t\t\t<face v1=\"" << *((DWORD *)pIndices + face_i * 3 + 0) << "\" v2=\"" << *((DWORD *)pIndices + face_i * 3 + 1) << "\" v3=\"" << *((DWORD *)pIndices + face_i * 3 + 2) << "\"/>\n";
			}
			else
			{
				ostr << "\t\t\t\t<face v1=\"" << *((WORD *)pIndices + face_i * 3 + 0) << "\" v2=\"" << *((WORD *)pIndices + face_i * 3 + 1) << "\" v3=\"" << *((WORD *)pIndices + face_i * 3 + 2) << "\"/>\n";
			}
		}
		ostr << "\t\t\t</faces>\n";
		ostr << "\t\t</submesh>\n";
	}
	UnlockIndexBuffer();
	ostr << "\t</submeshes>\n";
	// write skeleton link
	ostr << "\t<skeletonlink name=\"\"/>\n";
	// Write shared geometry bone assignments
	ostr << "\t<boneassignments>\n";
	if (m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
	{
		for (DWORD i=0; i<GetNumVertices(); i++)
		{
			unsigned char * pVertex = (unsigned char *)pVertices + i * VertexStride;
			for (int j=0; j<D3DVertexElementSet::MAX_BONE_INDICES; j++)
			{
				if (m_VertexElems.GetBlendWeight(pVertex)[j] > 0.001)
				{
					ostr << "\t\t<vertexboneassignment vertexindex=\"" << i << "\" boneindex=\"" << (int)((unsigned char *)&m_VertexElems.GetBlendIndices(pVertex))[j] << "\" weight=\"" << m_VertexElems.GetBlendWeight(pVertex)[j] <<"\"/>\n";
				}
			}
		}
	}
	UnlockVertexBuffer();
	ostr << "\t</boneassignments>\n";
	// write submesh names
	ostr << "\t<submeshnames>\n";
	ostr << "\t</submeshnames>\n";
	// end mesh description
	ostr << "</mesh>\n";
}

void OgreMesh::ComputeTangentFrame(void)
{
	std::vector<Vector3> tan1(GetNumVertices(), Vector3::zero);
	std::vector<Vector3> tan2(GetNumVertices(), Vector3::zero);
	DWORD VertexStride = GetNumBytesPerVertex();

	VOID * pIndices = LockIndexBuffer();
	const VOID * pVertices = LockVertexBuffer();

	for(DWORD face_i = 0; face_i < GetNumFaces(); face_i++)
	{
		int i1, i2, i3;
		if(GetOptions() & D3DXMESH_32BIT)
		{
			i1 = *((DWORD *)pIndices + face_i * 3 + 0);
			i2 = *((DWORD *)pIndices + face_i * 3 + 1);
			i3 = *((DWORD *)pIndices + face_i * 3 + 2);
		}
		else
		{
			i1 = *((WORD *)pIndices + face_i * 3 + 0);
			i2 = *((WORD *)pIndices + face_i * 3 + 1);
			i3 = *((WORD *)pIndices + face_i * 3 + 2);
		}

		unsigned char * pv1 = (unsigned char *)pVertices + i1 * VertexStride;
		unsigned char * pv2 = (unsigned char *)pVertices + i2 * VertexStride;
		unsigned char * pv3 = (unsigned char *)pVertices + i3 * VertexStride;

		const Vector3 & v1 = m_VertexElems.GetPosition(pv1);
		const Vector3 & v2 = m_VertexElems.GetPosition(pv2);
		const Vector3 & v3 = m_VertexElems.GetPosition(pv3);

		const Vector2 & w1 = m_VertexElems.GetTexcoord(pv1);
		const Vector2 & w2 = m_VertexElems.GetTexcoord(pv2);
		const Vector2 & w3 = m_VertexElems.GetTexcoord(pv3);

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0F / (s1 * t2 - s2 * t1);
		Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for(DWORD vertex_i = 0; vertex_i < GetNumVertices(); vertex_i++)
	{
		unsigned char * pVertex = (unsigned char *)pVertices + vertex_i * VertexStride;
		const Vector3 & n = m_VertexElems.GetNormal(pVertex);
		const Vector3 & t = tan1[vertex_i];

		// Gram-Schmidt orthogonalize
		m_VertexElems.GetTangent(pVertex) = (t - n * n.dot(t)).normalize();
	}

	UnlockVertexBuffer();
	UnlockIndexBuffer();
}

UINT OgreMesh::GetMaterialNum(void) const
{
	return m_MaterialNameList.size();
}

const std::string & OgreMesh::GetMaterialName(DWORD AttribId) const
{
	return m_MaterialNameList[AttribId];
}

void OgreMeshSet::OnResetDevice(void)
{
	iterator mesh_iter = begin();
	for(; mesh_iter != end(); mesh_iter++)
	{
		(*mesh_iter)->OnResetDevice();
	}
}

void OgreMeshSet::OnLostDevice(void)
{
	iterator mesh_iter = begin();
	for(; mesh_iter != end(); mesh_iter++)
	{
		(*mesh_iter)->OnLostDevice();
	}
}

void OgreMeshSet::OnDestroyDevice(void)
{
	iterator mesh_iter = begin();
	for(; mesh_iter != end(); mesh_iter++)
	{
		(*mesh_iter)->OnDestroyDevice();
	}
}

void OgreMeshSet::CreateMeshSetFromOgreXmlInFile(
	LPDIRECT3DDEVICE9 pd3dDevice,
	LPCTSTR pFilename,
	bool bComputeTangentFrame,
	DWORD dwMeshSetOptions)
{
	CachePtr cache = FileIStream::Open(pFilename)->GetWholeCache();
	cache->push_back(0);

	CreateMeshSetFromOgreXmlInMemory(pd3dDevice, (char *)&(*cache)[0], cache->size(), bComputeTangentFrame, dwMeshSetOptions);
}

void OgreMeshSet::CreateMeshSetFromOgreXmlInMemory(
	LPDIRECT3DDEVICE9 pd3dDevice,
	LPSTR pSrcData,
	UINT srcDataLen,
	bool bComputeTangentFrame,
	DWORD dwMeshSetOptions)
{
	_ASSERT(0 == pSrcData[srcDataLen-1]);

	rapidxml::xml_document<char> doc;
	try
	{
		doc.parse<0>(pSrcData);
	}
	catch(rapidxml::parse_error & e)
	{
		THROW_CUSEXCEPTION(ms2ts(e.what()));
	}

	CreateMeshSetFromOgreXml(pd3dDevice, &doc, bComputeTangentFrame, dwMeshSetOptions);
}

void OgreMeshSet::CreateMeshSetFromOgreXml(
	LPDIRECT3DDEVICE9 pd3dDevice,
	const rapidxml::xml_node<char> * node_root,
	bool bComputeTangentFrame,
	DWORD dwMeshOptions)
{
	DEFINE_XML_NODE_SIMPLE(mesh, root);
	DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
	rapidxml::xml_node<char> * node_submesh = node_submeshes->first_node("submesh");
	for(; node_submesh != NULL; node_submesh = node_submesh->next_sibling())
	{
		DEFINE_XML_NODE_SIMPLE(geometry, submesh);
		rapidxml::xml_node<char> * node_boneassignments = node_submesh->first_node("boneassignments");

		OgreMeshPtr mesh_ptr(new OgreMesh);
		mesh_ptr->CreateMeshFromOgreXmlNodes(
			pd3dDevice, node_geometry, node_boneassignments, node_submesh, false, bComputeTangentFrame, dwMeshOptions);

		push_back(mesh_ptr);
	}
}
