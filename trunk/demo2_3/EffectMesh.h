#pragma once

class EffectParameterBase
{
public:
	EffectParameterBase(void)
	{
	}

	virtual ~EffectParameterBase(void)
	{
	}

	virtual void SetParameter(my::Effect * effect, const std::string & name) const = 0;
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

	virtual void SetParameter(my::Effect * effect, const std::string & name) const;
};

class EffectParameterMap : public std::map<std::string, EffectParameterBasePtr>
{
public:
	void SetBool(const std::string & name, bool value);

	void SetFloat(const std::string & name, float value);

	void SetInt(const std::string & name, int value);

	void SetVector(const std::string & name, const my::Vector4 & value);

	void SetMatrix(const std::string & name, const my::Matrix4 & value);

	void SetString(const std::string & name, const std::string & value);

	void SetTexture(const std::string & name, my::BaseTexturePtr value);
};

class Material : public EffectParameterMap
{
public:
	my::EffectPtr m_Effect;

public:
	Material(void)
	{
	}

	void ApplyParameterBlock(void);
};

typedef boost::shared_ptr<Material> MaterialPtr;

class EffectMesh
{
public:
	my::OgreMeshPtr m_Mesh;

	typedef std::vector<MaterialPtr> MaterialPtrList;

	MaterialPtrList m_materials;

public:
	EffectMesh(void)
	{
	}

	virtual ~EffectMesh(void)
	{
	}

	void InsertMaterial(MaterialPtr material)
	{
		m_materials.push_back(material);
	}

	void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

	void DrawSubset(DWORD i, my::Effect * effect);
};

typedef boost::shared_ptr<EffectMesh> EffectMeshPtr;
