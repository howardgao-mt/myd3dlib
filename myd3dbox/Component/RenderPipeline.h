#pragma once

class RenderPipeline;

class Material
	: public my::DeviceRelatedObjectBase
{
public:
	enum MeshType
	{
		MeshTypeStatic			= 0,
		MeshTypeAnimation		= 1,
		MeshTypeParticle		= 2,
		MeshTypeNum
	};

	enum PassType
	{
		PassTypeShadow			= 0,
		PassTypeNormalDepth		= 1,
		PassTypeDiffuseSpec		= 2,
		PassTypeTextureColor	= 3,
		PassTypeTransparent		= 4,
		PassTypeNum
	};

	enum TextureType
	{
		TextureTypeDiffuse		= 0,
		TextureTypeNormal		= 1,
		TextureTypeSpecular		= 2,
		TextureTypeNum
	};

	unsigned int m_PassMask;

	std::pair<std::string, boost::shared_ptr<my::BaseTexture> > m_DiffuseTexture;

	std::pair<std::string, boost::shared_ptr<my::BaseTexture> > m_NormalTexture;

	std::pair<std::string, boost::shared_ptr<my::BaseTexture> > m_SpecularTexture;

	unsigned int m_TextureMask;

public:
	Material(void)
		: m_PassMask(0)
		, m_TextureMask(0)
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

	static unsigned int PassTypeToMask(unsigned int pass_type)
	{
		_ASSERT(pass_type >= 0 && pass_type < PassTypeNum); return 1 << pass_type;
	}

	static unsigned int TextureTypeToMask(unsigned int texture_type)
	{
		_ASSERT(texture_type >= 0 && texture_type < TextureTypeNum); return 1 << texture_type;
	}

	void BuildTextureMask(void)
	{
		m_TextureMask = 0;
		m_TextureMask |= m_DiffuseTexture.first.empty() ? 0 : TextureTypeToMask(TextureTypeDiffuse);
		m_TextureMask |= m_NormalTexture.first.empty() ? 0 : TextureTypeToMask(TextureTypeNormal);
		m_TextureMask |= m_SpecularTexture.first.empty() ? 0 : TextureTypeToMask(TextureTypeSpecular);
	}

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_PassMask);
		ar & boost::serialization::make_nvp(BOOST_PP_STRINGIZE(m_DiffuseTexture), m_DiffuseTexture.first);
		ar & boost::serialization::make_nvp(BOOST_PP_STRINGIZE(m_NormalTexture), m_NormalTexture.first);
		ar & boost::serialization::make_nvp(BOOST_PP_STRINGIZE(m_SpecularTexture), m_SpecularTexture.first);

		BuildTextureMask();
	}

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<Material> MaterialPtr;

typedef std::vector<MaterialPtr> MaterialPtrList;

class RenderPipeline
{
public:
	my::D3DVertexElementSet m_ParticleVertexElems;

	my::D3DVertexElementSet m_ParticleInstanceElems;

	std::vector<D3DVERTEXELEMENT9> m_ParticleVEList;

	DWORD m_ParticleVertexStride;

	DWORD m_ParticleInstanceStride;

	CComPtr<IDirect3DVertexDeclaration9> m_ParticleDecl;

	my::VertexBuffer m_ParticleVertexBuffer;

	my::IndexBuffer m_ParticleIndexBuffer;

	my::VertexBuffer m_ParticleInstanceData;

	my::D3DVertexElementSet m_MeshInstanceElems;

	std::vector<D3DVERTEXELEMENT9> m_MeshIEList;

	DWORD m_MeshInstanceStride;

	my::VertexBuffer m_MeshInstanceData;

	class IShaderSetter
	{
	public:
		virtual void OnSetShader(my::Effect * shader, DWORD AttribId) = 0;
	};

	struct MeshAtom
	{
		my::Mesh * mesh;
		DWORD AttribId;
		my::Effect * shader;
		IShaderSetter * setter;
	};

	typedef std::vector<MeshAtom> MeshAtomList;

	struct MeshInstanceAtom
	{
		IShaderSetter * setter;
		std::vector<D3DXATTRIBUTERANGE> m_AttribTable;
		std::vector<D3DVERTEXELEMENT9> m_velist;
		DWORD m_VertexStride;
		CComPtr<IDirect3DVertexDeclaration9> m_Decl;
		my::TransformList m_TransformList;
	};

	typedef boost::tuple<my::Mesh *, DWORD, my::Effect *> MeshInstanceAtomKey;

	typedef boost::unordered_map<MeshInstanceAtomKey, MeshInstanceAtom> MeshInstanceAtomMap;

	struct IndexedPrimitiveUPAtom
	{
		IDirect3DVertexDeclaration9* pDecl;
		D3DPRIMITIVETYPE PrimitiveType;
		UINT MinVertexIndex;
		UINT NumVertices;
		UINT PrimitiveCount;
		CONST void* pIndexData;
		D3DFORMAT IndexDataFormat;
		CONST void* pVertexStreamZeroData;
		UINT VertexStreamZeroStride;
		DWORD AttribId;
		my::Effect * shader;
		IShaderSetter * setter;
	};

	typedef std::vector<IndexedPrimitiveUPAtom> IndexedPrimitiveUPAtomList;

	struct EmitterAtom
	{
		my::Emitter * emitter;
		DWORD AttribId;
		my::Effect * shader;
		IShaderSetter * setter;
	};

	typedef std::vector<EmitterAtom> EmitterAtomList;

	struct Pass
	{
		MeshAtomList m_MeshList;
		MeshInstanceAtomMap m_MeshInstanceMap;
		IndexedPrimitiveUPAtomList m_IndexedPrimitiveUPList;
		EmitterAtomList m_EmitterList;
	};

	boost::array<Pass, Material::PassTypeNum> m_Pass;

public:
	RenderPipeline(void)
		: m_ParticleVertexStride(0)
		, m_ParticleInstanceStride(0)
		, m_MeshInstanceStride(0)
	{
	}

	virtual my::Effect * QueryShader(Material::MeshType mesh_type, unsigned int PassID, bool bInstance, const Material * material) = 0;

	HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	void OnLostDevice(void);

	void OnDestroyDevice(void);

	void RenderAllObjects(
		unsigned int PassID,
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	void ClearAllObjects(void);

	void DrawMesh(my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void DrawMeshInstance(
		IDirect3DDevice9 * pd3dDevice,
		my::Mesh * mesh,
		DWORD AttribId,
		my::Effect * shader,
		IShaderSetter * setter,
		MeshInstanceAtom & atom);

	void DrawIndexedPrimitiveUP(
		IDirect3DDevice9 * pd3dDevice,
		IDirect3DVertexDeclaration9* pDecl,
		D3DPRIMITIVETYPE PrimitiveType,
		UINT MinVertexIndex,
		UINT NumVertices,
		UINT PrimitiveCount,
		CONST void* pIndexData,
		D3DFORMAT IndexDataFormat,
		CONST void* pVertexStreamZeroData,
		UINT VertexStreamZeroStride,
		DWORD AttribId,
		my::Effect * shader,
		IShaderSetter * setter);

	void DrawEmitter(IDirect3DDevice9 * pd3dDevice, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void PushMesh(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void PushMeshInstance(unsigned int PassID, my::Mesh * mesh, DWORD AttribId, const my::Matrix4 & World, my::Effect * shader, IShaderSetter * setter);

	void PushIndexedPrimitiveUP(
		unsigned int PassID,
		IDirect3DVertexDeclaration9* pDecl,
		D3DPRIMITIVETYPE PrimitiveType,
		UINT MinVertexIndex,
		UINT NumVertices,
		UINT PrimitiveCount,
		CONST void* pIndexData,
		D3DFORMAT IndexDataFormat,
		CONST void* pVertexStreamZeroData,
		UINT VertexStreamZeroStride,
		DWORD AttribId,
		my::Effect * shader,
		IShaderSetter * setter);

	void PushEmitter(unsigned int PassID, my::Emitter * emitter, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);
};
