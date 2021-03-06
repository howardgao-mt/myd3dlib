#include "stdafx.h"
#include "LuaExtension.h"

using namespace luabind;

void ExportResource2Lua(lua_State * L)
{
	module(L)
	[
		class_<my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("BaseTexture")

		, class_<my::Texture2D, my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("Texture2D")

		, class_<my::CubeTexture, my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("CubeTexture")

		, class_<my::Mesh, boost::shared_ptr<my::Mesh> >("Mesh")

		, class_<my::OgreMesh, my::Mesh, boost::shared_ptr<my::OgreMesh> >("OgreMesh")
			.def("GetMaterialNum", &my::OgreMesh::GetMaterialNum)
			.def("GetMaterialName", &my::OgreMesh::GetMaterialName)

		, class_<my::OgreMeshSet, boost::shared_ptr<my::OgreMeshSet> >("OgreMeshSet")

		, class_<my::OgreSkeletonAnimation, boost::shared_ptr<my::OgreSkeletonAnimation> >("OgreSkeletonAnimation")

		// ! many methods of my::BaseEffect, my::Effect cannot be use in lua
		, class_<my::BaseEffect, boost::shared_ptr<my::BaseEffect> >("BaseEffect")
			//.def("GetAnnotation", &my::BaseEffect::GetAnnotation)
			//.def("GetAnnotationByName", &my::BaseEffect::GetAnnotationByName)
			//.def("GetBool", &my::BaseEffect::GetBool)
			//.def("GetBoolArray", &my::BaseEffect::GetBoolArray)
			//.def("GetDesc", &my::BaseEffect::GetDesc)
			//.def("GetFloat", &my::BaseEffect::GetFloat)
			//.def("GetFloatArray", &my::BaseEffect::GetFloatArray)
			//.def("GetFunction", &my::BaseEffect::GetFunction)
			//.def("GetFunctionByName", &my::BaseEffect::GetFunctionByName)
			//.def("GetFunctionDesc", &my::BaseEffect::GetFunctionDesc)
			//.def("GetInt", &my::BaseEffect::GetInt)
			//.def("GetIntArray", &my::BaseEffect::GetIntArray)
			//.def("GetMatrix", &my::BaseEffect::GetMatrix)
			//.def("GetMatrixArray", &my::BaseEffect::GetMatrixArray)
			//.def("GetMatrixPointerArray", &my::BaseEffect::GetMatrixPointerArray)
			//.def("GetMatrixTranspose", &my::BaseEffect::GetMatrixTranspose)
			//.def("GetMatrixTransposeArray", &my::BaseEffect::GetMatrixTransposeArray)
			//.def("GetMatrixTransposePointerArray", &my::BaseEffect::GetMatrixTransposePointerArray)
			//.def("GetParameter", &my::BaseEffect::GetParameter)
			//.def("GetParameterByName", &my::BaseEffect::GetParameterByName)
			//.def("GetParameterBySemantic", &my::BaseEffect::GetParameterBySemantic)
			//.def("GetParameterDesc", &my::BaseEffect::GetParameterDesc)
			//.def("GetParameterElement", &my::BaseEffect::GetParameterElement)
			//.def("GetPass", &my::BaseEffect::GetPass)
			//.def("GetPassByName", &my::BaseEffect::GetPassByName)
			//.def("GetPassDesc", &my::BaseEffect::GetPassDesc)
			//.def("GetPixelShader", &my::BaseEffect::GetPixelShader)
			//.def("GetString", &my::BaseEffect::GetString)
			//.def("GetTechnique", &my::BaseEffect::GetTechnique)
			//.def("GetTechniqueByName", &my::BaseEffect::GetTechniqueByName)
			//.def("GetTechniqueDesc", &my::BaseEffect::GetTechniqueDesc)
			//.def("GetTexture", &my::BaseEffect::GetTexture)
			//.def("GetValue", &my::BaseEffect::GetValue)
			//.def("GetVector", &my::BaseEffect::GetVector)
			//.def("GetVectorArray", &my::BaseEffect::GetVectorArray)
			//.def("GetVertexShader", &my::BaseEffect::GetVertexShader)
			//.def("SetArrayRange", &my::BaseEffect::SetArrayRange)
			//.def("SetBool", &my::BaseEffect::SetBool)
			//.def("SetBoolArray", &my::BaseEffect::SetBoolArray)
			//.def("SetFloat", &my::BaseEffect::SetFloat)
			//.def("SetFloatArray", &my::BaseEffect::SetFloatArray)
			//.def("SetInt", &my::BaseEffect::SetInt)
			//.def("SetIntArray", &my::BaseEffect::SetIntArray)
			//.def("SetMatrix", &my::BaseEffect::SetMatrix)
			//.def("SetMatrixArray", &my::BaseEffect::SetMatrixArray)
			//.def("SetMatrixPointerArray", &my::BaseEffect::SetMatrixPointerArray)
			//.def("SetMatrixTranspose", &my::BaseEffect::SetMatrixTranspose)
			//.def("SetMatrixTransposeArray", &my::BaseEffect::SetMatrixTransposeArray)
			//.def("SetMatrixTransposePointerArray", &my::BaseEffect::SetMatrixTransposePointerArray)
			//.def("SetString", &my::BaseEffect::SetString)
			//// ! luabind cannot convert boost::shared_ptr<Derived Class> to base ptr
			//.def("SetTexture", &my::BaseEffect::SetTexture)
			//.def("SetValue", &my::BaseEffect::SetValue)
			//.def("SetVector", (void (my::BaseEffect::*)(D3DXHANDLE, const my::Vector4 &))&my::BaseEffect::SetVector)
			//.def("SetVector", (void (my::BaseEffect::*)(D3DXHANDLE, const my::Vector3 &))&my::BaseEffect::SetVector)
			//.def("SetVectorArray", &my::BaseEffect::SetVectorArray)

		, class_<my::Effect, my::BaseEffect, boost::shared_ptr<my::Effect> >("Effect")
			//.def("ApplyParameterBlock", &my::Effect::ApplyParameterBlock)
			//.def("Begin", &my::Effect::Begin)
			//.def("BeginParameterBlock", &my::Effect::BeginParameterBlock)
			//.def("BeginPass", &my::Effect::BeginPass)
			//.def("CloneEffect", &my::Effect::CloneEffect)
			//.def("CommitChanges", &my::Effect::CommitChanges)
			//.def("DeleteParameterBlock", &my::Effect::DeleteParameterBlock)
			//.def("End", &my::Effect::End)
			//.def("EndParameterBlock", &my::Effect::EndParameterBlock)
			//.def("EndPass", &my::Effect::EndPass)
			//.def("FindNextValidTechnique", &my::Effect::FindNextValidTechnique)
			//.def("GetCurrentTechnique", &my::Effect::GetCurrentTechnique)
			//.def("GetDevice", &my::Effect::GetDevice)
			//.def("GetPool", &my::Effect::GetPool)
			//.def("GetStateManager", &my::Effect::GetStateManager)
			//.def("IsParameterUsed", &my::Effect::IsParameterUsed)
			//.def("SetRawValue", &my::Effect::SetRawValue)
			//.def("SetStateManager", &my::Effect::SetStateManager)
			//.def("SetTechnique", &my::Effect::SetTechnique)
			//.def("ValidateTechnique", &my::Effect::ValidateTechnique)

		, class_<my::Font, boost::shared_ptr<my::Font> >("Font")
			.enum_("Align")
			[
				value("AlignLeft", my::Font::AlignLeft),
				value("AlignCenter", my::Font::AlignCenter),
				value("AlignRight", my::Font::AlignRight),
				value("AlignTop", my::Font::AlignTop),
				value("AlignMiddle", my::Font::AlignMiddle),
				value("AlignBottom", my::Font::AlignBottom),
				value("AlignLeftTop", my::Font::AlignLeftTop),
				value("AlignCenterTop", my::Font::AlignCenterTop),
				value("AlignRightTop", my::Font::AlignRightTop),
				value("AlignLeftMiddle", my::Font::AlignLeftMiddle),
				value("AlignCenterMiddle", my::Font::AlignCenterMiddle),
				value("AlignRightMiddle", my::Font::AlignRightMiddle),
				value("AlignLeftBottom", my::Font::AlignLeftBottom),
				value("AlignCenterBottom", my::Font::AlignCenterBottom),
				value("AlignRightBottom", my::Font::AlignRightBottom)
			]
			.def_readonly("Height", &my::Font::m_Height)
			.property("Scale", &my::Font::GetScale, &my::Font::SetScale)
			.def_readonly("LineHeight", &my::Font::m_LineHeight)

		, class_<my::ResourceCallback>("ResourceCallback")

		, class_<my::ResourceMgr>("ResourceMgr")
			.def("LoadTextureAsync", &my::ResourceMgr::LoadTextureAsync)
			.def("LoadTexture", &my::ResourceMgr::LoadTexture)
			.def("LoadMeshAsync", &my::ResourceMgr::LoadMeshAsync)
			.def("LoadMesh", &my::ResourceMgr::LoadMesh)
			.def("LoadMeshSetAsync", &my::ResourceMgr::LoadMeshSetAsync)
			.def("LoadMeshSet", &my::ResourceMgr::LoadMeshSet)
			.def("LoadSkeletonAsync", &my::ResourceMgr::LoadSkeletonAsync)
			.def("LoadSkeleton", &my::ResourceMgr::LoadSkeleton)
			.def("LoadEffectAsync", &my::ResourceMgr::LoadEffectAsync)
			.def("LoadEffect", &my::ResourceMgr::LoadEffect)
			.def("LoadFontAsync", &my::ResourceMgr::LoadFontAsync)
			.def("LoadFont", &my::ResourceMgr::LoadFont)
			.def("SaveMesh", &my::ResourceMgr::SaveMesh)
			.def("SaveSimplyMesh", &my::ResourceMgr::SaveSimplyMesh)
	];
}
