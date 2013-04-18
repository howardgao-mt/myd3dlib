#include "StdAfx.h"
#include "EffectMesh.h"

using namespace my;

template <>
void EffectParameter<bool>::SetParameter(Effect * effect, const std::string & name) const
{
	effect->SetBool(name.c_str(), m_Value);
}

template <>
void EffectParameter<float>::SetParameter(Effect * effect, const std::string & name) const
{
	effect->SetFloat(name.c_str(), m_Value);
}

template <>
void EffectParameter<int>::SetParameter(Effect * effect, const std::string & name) const
{
	effect->SetInt(name.c_str(), m_Value);
}

template <>
void EffectParameter<Vector4>::SetParameter(Effect * effect, const std::string & name) const
{
	effect->SetVector(name.c_str(), m_Value);
}

template <>
void EffectParameter<Matrix4>::SetParameter(Effect * effect, const std::string & name) const
{
	effect->SetMatrix(name.c_str(), m_Value);
}

template <>
void EffectParameter<std::string>::SetParameter(Effect * effect, const std::string & name) const
{
	effect->SetString(name.c_str(), m_Value.c_str());
}

template <>
void EffectParameter<BaseTexturePtr>::SetParameter(Effect * effect, const std::string & name) const
{
	effect->SetTexture(name.c_str(), m_Value ? m_Value->m_ptr : NULL);
}

void EffectParameterMap::SetBool(const std::string & name, bool value)
{
	operator[](name) = EffectParameterBasePtr(new EffectParameter<bool>(value));
}

void EffectParameterMap::SetFloat(const std::string & name, float value)
{
	operator[](name) = EffectParameterBasePtr(new EffectParameter<float>(value));
}

void EffectParameterMap::SetInt(const std::string & name, int value)
{
	operator[](name) = EffectParameterBasePtr(new EffectParameter<int>(value));
}

void EffectParameterMap::SetVector(const std::string & name, const Vector4 & value)
{
	operator[](name) = EffectParameterBasePtr(new EffectParameter<Vector4>(value));
}

void EffectParameterMap::SetMatrix(const std::string & name, const Matrix4 & value)
{
	operator[](name) = EffectParameterBasePtr(new EffectParameter<Matrix4>(value));
}

void EffectParameterMap::SetString(const std::string & name, const std::string & value)
{
	operator[](name) = EffectParameterBasePtr(new EffectParameter<std::string>(value));
}

void EffectParameterMap::SetTexture(const std::string & name, BaseTexturePtr value)
{
	operator[](name) = EffectParameterBasePtr(new EffectParameter<BaseTexturePtr>(value));
}

void Material::ApplyParameterBlock(void)
{
	const_iterator param_iter = begin();
	for(; param_iter != end(); param_iter++)
	{
		param_iter->second->SetParameter(m_Effect.get(), param_iter->first);
	}
}

void EffectMesh::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	_ASSERT(m_Mesh);

	for(DWORD i = 0; i < m_materials.size(); i++)
	{
		Material * mat = m_materials[i].get();
		if(mat->m_Effect && mat->m_Effect->m_ptr)
		{
			mat->ApplyParameterBlock();

			DrawSubset(i, mat->m_Effect.get());
		}
	}
}

void EffectMesh::DrawSubset(DWORD i, Effect * effect)
{
	_ASSERT(m_Mesh);

	UINT cPasses = effect->Begin();
	for(UINT p = 0; p < cPasses; p++)
	{
		effect->BeginPass(p);
		m_Mesh->DrawSubset(i);
		effect->EndPass();
	}
	effect->End();
}
