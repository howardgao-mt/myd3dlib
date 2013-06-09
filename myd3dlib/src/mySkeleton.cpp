#include "StdAfx.h"
#include "mySkeleton.h"
#include "rapidxml.hpp"
#include "myException.h"
#include "libc.h"
#include "myResource.h"
#include <tchar.h>

using namespace my;

void BoneHierarchy::InsertSibling(int root_i, int sibling_i)
{
	_ASSERT(root_i >= 0 && root_i < (int)size());
	_ASSERT(sibling_i >= 0 && sibling_i < (int)size());

	reference node = operator[](root_i);
	if(node.m_sibling >= 0)
	{
		InsertSibling(node.m_sibling, sibling_i);
	}
	else
	{
		node.m_sibling = sibling_i;
	}
}

void BoneHierarchy::InsertChild(int root_i, int child_i)
{
	_ASSERT(root_i >= 0 && root_i < (int)size());
	_ASSERT(child_i >= 0 && child_i < (int)size());

	reference node = operator[](root_i);
	if(node.m_child >= 0)
	{
		InsertSibling(node.m_child, child_i);
	}
	else
	{
		node.m_child = child_i;
	}
}

BoneHierarchy & BoneHierarchy::BuildLeafedHierarchy(
	BoneHierarchy & leafedBoneHierarchy,
	int root_i,
	const BoneIndexSet & leafNodeIndices)
{
	_ASSERT(leafedBoneHierarchy.size() == size());

	reference node = leafedBoneHierarchy[root_i] = operator[](root_i);

	if(leafNodeIndices.end() == leafNodeIndices.find(root_i))
	{
		int node_i = node.m_child;
		for(; node_i >= 0; node_i = operator[](node_i).m_sibling)
		{
			BuildLeafedHierarchy(leafedBoneHierarchy, node_i, leafNodeIndices);
		}
	}
	else
		leafedBoneHierarchy[root_i].m_child = -1;

	return leafedBoneHierarchy;
}

TransformList & TransformList::Transform(
	TransformList & hierarchyTransformList,
	const TransformList & rhs,
	const BoneHierarchy & boneHierarchy,
	int root_i) const
{
	_ASSERT(hierarchyTransformList.size() == size());
	_ASSERT(hierarchyTransformList.size() == rhs.size());

	hierarchyTransformList[root_i] = operator[](root_i) * rhs[root_i];

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		Transform(hierarchyTransformList, rhs, boneHierarchy, node_i);
	}

	return hierarchyTransformList;
}

TransformList & TransformList::TransformSelf(
	const TransformList & rhs,
	const BoneHierarchy & boneHierarchy,
	int root_i)
{
	_ASSERT(size() == rhs.size());

	operator[](root_i) *= rhs[root_i];

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		TransformSelf(rhs, boneHierarchy, node_i);
	}

	return *this;
}

BoneList & BoneList::Increment(
	BoneList & boneList,
	const BoneList & rhs,
	const BoneHierarchy & boneHierarchy,
	int root_i) const
{
	_ASSERT(boneList.size() == size());
	_ASSERT(boneList.size() == rhs.size());

	boneList[root_i] = operator[](root_i).Increment(rhs[root_i]);

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		Increment(boneList, rhs, boneHierarchy, node_i);
	}

	return boneList;
}

BoneList & BoneList::IncrementSelf(
	const BoneList & rhs,
	const BoneHierarchy & boneHierarchy,
	int root_i)
{
	_ASSERT(size() == rhs.size());

	operator[](root_i).IncrementSelf(rhs[root_i]);

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		IncrementSelf(rhs, boneHierarchy, node_i);
	}

	return *this;
}

BoneList & BoneList::Lerp(
	BoneList & boneList,
	const BoneList & rhs,
	const BoneHierarchy & boneHierarchy,
	int root_i,
	float t) const
{
	_ASSERT(boneList.size() == size());
	_ASSERT(boneList.size() == rhs.size());

	boneList[root_i] = operator[](root_i).Lerp(rhs[root_i], t);

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		Lerp(boneList, rhs, boneHierarchy, node_i, t);
	}

	return boneList;
}

BoneList & BoneList::LerpSelf(
	const BoneList & rhs,
	const BoneHierarchy & boneHierarchy,
	int root_i,
	float t)
{
	_ASSERT(size() == rhs.size());

	operator[](root_i).LerpSelf(rhs[root_i], t);

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		LerpSelf(rhs, boneHierarchy, node_i, t);
	}

	return *this;
}

BoneList & BoneList::BuildHierarchyBoneList(
	BoneList & hierarchyBoneList,
	const BoneHierarchy & boneHierarchy,
	int root_i,
	const Quaternion & rootRotation,
	const Vector3 & rootPosition)
{
	_ASSERT(hierarchyBoneList.size() == size());
	_ASSERT(hierarchyBoneList.size() == boneHierarchy.size());

	BoneHierarchy::const_reference node = boneHierarchy[root_i];
	const_reference bone = operator[](root_i);
	reference hier_bone = hierarchyBoneList[root_i];
	hier_bone.m_rotation = bone.m_rotation * rootRotation;
	hier_bone.m_position = bone.m_position.transform(rootRotation) + rootPosition;

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		BuildHierarchyBoneList(hierarchyBoneList, boneHierarchy, node_i, hier_bone.m_rotation, hier_bone.m_position);
	}

	return hierarchyBoneList;
}

BoneList & BoneList::BuildInverseHierarchyBoneList(
	BoneList & inverseHierarchyBoneList,
	const BoneHierarchy & boneHierarchy,
	int root_i,
	const Quaternion & inverseRootRotation,
	const Vector3 & inverseRootPosition)
{
	_ASSERT(inverseHierarchyBoneList.size() == size());
	_ASSERT(inverseHierarchyBoneList.size() == boneHierarchy.size());

	BoneHierarchy::const_reference node = boneHierarchy[root_i];
	const_reference bone = operator[](root_i);
	reference hier_bone = inverseHierarchyBoneList[root_i];
	hier_bone.m_rotation = inverseRootRotation * bone.m_rotation.conjugate();
	hier_bone.m_position = -(bone.m_position.transform(inverseRootRotation.conjugate()) - inverseRootPosition);

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		BuildInverseHierarchyBoneList(inverseHierarchyBoneList, boneHierarchy, node_i, hier_bone.m_rotation, hier_bone.m_position);
	}

	return inverseHierarchyBoneList;
}

TransformList & BoneList::BuildTransformList(
	TransformList & transformList) const
{
	_ASSERT(transformList.size() == size());

	for(size_t i = 0; i < size(); i++)
	{
		transformList[i] = operator[](i).BuildTransform();
	}

	return transformList;
}

TransformList & BoneList::BuildTransformListTF(
	TransformList & inverseTransformList) const
{
	_ASSERT(inverseTransformList.size() == size());

	for(size_t i = 0; i < size(); i++)
	{
		const_reference bone = operator[](i);
		inverseTransformList[i] = Matrix4::Translation(bone.m_position) * Matrix4::RotationQuaternion(bone.m_rotation);
	}

	return inverseTransformList;
}

TransformList & BoneList::BuildInverseTransformList(
	TransformList & inverseTransformList) const
{
	_ASSERT(inverseTransformList.size() == size());

	for(size_t i = 0; i < size(); i++)
	{
		inverseTransformList[i] = operator[](i).BuildInverseTransform();
	}

	return inverseTransformList;
}

TransformList & BoneList::BuildDualQuaternionList(
	TransformList & dualQuaternionList,
	const BoneList & rhs) const
{
	_ASSERT(dualQuaternionList.size() == size());
	_ASSERT(dualQuaternionList.size() == rhs.size());

	for(size_t i = 0; i < size(); i++)
	{
		const_reference local_bone = operator [](i);
		BoneList::const_reference trans_bone = rhs[i];
		TransformList::reference dual = dualQuaternionList[i];

		Quaternion quat = local_bone.m_rotation.conjugate() * trans_bone.m_rotation;
		Vector3 tran = (-local_bone.m_position).transform(quat) + trans_bone.m_position;

		dual._11 = quat.x ; 
		dual._12 = quat.y ; 
		dual._13 = quat.z ; 
		dual._14 = quat.w ; 
		dual._21 = 0.5f * ( tran.x * quat.w + tran.y * quat.z - tran.z * quat.y ) ; 
		dual._22 = 0.5f * (-tran.x * quat.z + tran.y * quat.w + tran.z * quat.x ) ; 
		dual._23 = 0.5f * ( tran.x * quat.y - tran.y * quat.x + tran.z * quat.w ) ; 
		dual._24 = -0.5f * (tran.x * quat.x + tran.y * quat.y + tran.z * quat.z ) ; 
	}

	return dualQuaternionList;
}

TransformList & BoneList::BuildHierarchyTransformList(
	TransformList & hierarchyTransformList,
	const BoneHierarchy & boneHierarchy,
	int root_i,
	const Matrix4 & rootTransform)
{
	_ASSERT(hierarchyTransformList.size() == size());
	_ASSERT(hierarchyTransformList.size() == boneHierarchy.size());

	BoneHierarchy::const_reference node = boneHierarchy[root_i];
	const_reference bone = operator[](root_i);
	hierarchyTransformList[root_i] = Matrix4::RotationQuaternion(bone.m_rotation) * Matrix4::Translation(bone.m_position) * rootTransform;

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		BuildHierarchyTransformList(hierarchyTransformList, boneHierarchy, node_i, hierarchyTransformList[root_i]);
	}

	return hierarchyTransformList;
}

TransformList & BoneList::BuildInverseHierarchyTransformList(
	TransformList & inverseHierarchyTransformList,
	const BoneHierarchy & boneHierarchy,
	int root_i,
	const Matrix4 & inverseRootTransform)
{
	_ASSERT(inverseHierarchyTransformList.size() == size());
	_ASSERT(inverseHierarchyTransformList.size() == boneHierarchy.size());

	BoneHierarchy::const_reference node = boneHierarchy[root_i];
	const_reference bone = operator[](root_i);
	inverseHierarchyTransformList[root_i] = inverseRootTransform * Matrix4::Translation(-bone.m_position) * Matrix4::RotationQuaternion(bone.m_rotation.conjugate());

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		BuildInverseHierarchyTransformList(inverseHierarchyTransformList, boneHierarchy, node_i, inverseHierarchyTransformList[root_i]);
	}

	return inverseHierarchyTransformList;
}

Bone BoneTrack::GetPoseBone(float time) const
{
	_ASSERT(!empty());

	const_iterator key_iter = begin();
	if(time < key_iter->m_time)
	{
		return *key_iter;
	}

	key_iter++;
	for(; key_iter != end(); key_iter++)
	{
		if(time < key_iter->m_time)
		{
			const_iterator prev_key_iter = key_iter - 1;
			return prev_key_iter->Lerp(*key_iter, (time - prev_key_iter->m_time) / (key_iter->m_time - prev_key_iter->m_time));
		}
	}

	return back();
}

BoneList & BoneTrackList::GetPose(
	BoneList & boneList,
	const BoneHierarchy & boneHierarchy,
	int root_i,
	float time) const
{
	_ASSERT(boneList.size() == size());
	_ASSERT(boneList.size() == boneHierarchy.size());

	boneList[root_i] = operator[](root_i).GetPoseBone(time);

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		GetPose(boneList, boneHierarchy, node_i, time);
	}

	return boneList;
}

void OgreSkeletonAnimation::CreateOgreSkeletonAnimation(
	LPCSTR pSrcData,
	UINT srcDataLen)
{
	Clear();

	std::string xmlStr(pSrcData, srcDataLen);

	rapidxml::xml_document<char> doc;
	try
	{
		doc.parse<0>(&xmlStr[0]);
	}
	catch(rapidxml::parse_error & e)
	{
		THROW_CUSEXCEPTION(e.what());
	}

	rapidxml::xml_node<char> * node_root = &doc;
	DEFINE_XML_NODE_SIMPLE(skeleton, root);
	DEFINE_XML_NODE_SIMPLE(bones, skeleton);
	DEFINE_XML_NODE_SIMPLE(bone, bones);

	int bone_i = 0;
	for(; node_bone != NULL; node_bone = node_bone->next_sibling(), bone_i++)
	{
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(id, bone);
		if(id != bone_i)
		{
			THROW_CUSEXCEPTION(str_printf("invalid bone id: %d", id));
		}

		DEFINE_XML_ATTRIBUTE_SIMPLE(name, bone);
		if(m_boneNameMap.end() != m_boneNameMap.find(attr_name->value()))
		{
			THROW_CUSEXCEPTION(str_printf("bone name \"%s\"have already existed", attr_name->value()));
		}

		m_boneNameMap.insert(std::make_pair(attr_name->value(), id));

		DEFINE_XML_NODE_SIMPLE(position, bone);
		DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(x, position);
		DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(y, position);
		DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(z, position);

		DEFINE_XML_NODE_SIMPLE(rotation, bone);
		DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(angle, rotation);
		DEFINE_XML_NODE_SIMPLE(axis, rotation);
		float axis_x, axis_y, axis_z;
		rapidxml::xml_attribute<char> * attr_axis_x, * attr_axis_y, * attr_axis_z;
		DEFINE_XML_ATTRIBUTE_FLOAT(axis_x, attr_axis_x, node_axis, x);
		DEFINE_XML_ATTRIBUTE_FLOAT(axis_y, attr_axis_y, node_axis, y);
		DEFINE_XML_ATTRIBUTE_FLOAT(axis_z, attr_axis_z, node_axis, z);

		m_boneBindPose.push_back(
			Bone(Quaternion::RotationAxis(Vector3(axis_x, axis_y, axis_z), angle), Vector3(x, y, z)));

		_ASSERT(id == m_boneBindPose.size() - 1);
	}

	DEFINE_XML_NODE_SIMPLE(bonehierarchy, skeleton);
	DEFINE_XML_NODE_SIMPLE(boneparent, bonehierarchy);

	m_boneHierarchy.resize(m_boneBindPose.size());
	for(; node_boneparent != NULL; node_boneparent = node_boneparent->next_sibling())
	{
		DEFINE_XML_ATTRIBUTE_SIMPLE(bone, boneparent);
		if(m_boneNameMap.end() == m_boneNameMap.find(attr_bone->value()))
		{
			THROW_CUSEXCEPTION(str_printf("invalid bone name: %s", attr_bone->value()));
		}

		DEFINE_XML_ATTRIBUTE_SIMPLE(parent, boneparent);
		if(m_boneNameMap.end() == m_boneNameMap.find(attr_parent->value()))
		{
			THROW_CUSEXCEPTION(str_printf("invalid bone parent name: %s", attr_parent->value()));
		}

		m_boneHierarchy.InsertChild(
			m_boneNameMap[attr_parent->value()], m_boneNameMap[attr_bone->value()]);
	}

	DEFINE_XML_NODE_SIMPLE(animations, skeleton);
	DEFINE_XML_NODE_SIMPLE(animation, animations);

	for(; node_animation != NULL; node_animation = node_animation->next_sibling())
	{
		DEFINE_XML_ATTRIBUTE_SIMPLE(name, animation);
		if(m_animationMap.end() != m_animationMap.find(attr_name->value()))
		{
			THROW_CUSEXCEPTION(str_printf("animation \"%s\" have already existed", attr_name->value()));
		}

		m_animationMap.insert(std::make_pair(attr_name->value(), OgreAnimation()));
		OgreAnimation & anim = m_animationMap[attr_name->value()];

		DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(length, animation);
		anim.m_time = length;
		anim.resize(m_boneBindPose.size());

		DEFINE_XML_NODE_SIMPLE(tracks, animation);
		DEFINE_XML_NODE_SIMPLE(track, tracks);
		for(; node_track != NULL; node_track = node_track->next_sibling())
		{
			DEFINE_XML_ATTRIBUTE_SIMPLE(bone, track);
			if(m_boneNameMap.end() == m_boneNameMap.find(attr_bone->value()))
			{
				THROW_CUSEXCEPTION(str_printf("invalid bone name: %s", attr_bone->value()));
			}

			BoneTrack & bone_track = anim[m_boneNameMap[attr_bone->value()]];

			DEFINE_XML_NODE_SIMPLE(keyframes, track);
			DEFINE_XML_NODE_SIMPLE(keyframe, keyframes);
			for(; node_keyframe != NULL; node_keyframe = node_keyframe->next_sibling())
			{
				DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(time, keyframe);

				rapidxml::xml_attribute<char> * attr_translate_x, * attr_translate_y, * attr_translate_z;
				float translate_x, translate_y, translate_z;
				DEFINE_XML_NODE_SIMPLE(translate, keyframe);
				DEFINE_XML_ATTRIBUTE_FLOAT(translate_x, attr_translate_x, node_translate, x);
				DEFINE_XML_ATTRIBUTE_FLOAT(translate_y, attr_translate_y, node_translate, y);
				DEFINE_XML_ATTRIBUTE_FLOAT(translate_z, attr_translate_z, node_translate, z);

				DEFINE_XML_NODE_SIMPLE(rotate, keyframe);
				DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(angle, rotate);

				rapidxml::xml_attribute<char> * attr_axis_x, * attr_axis_y, * attr_axis_z;
				float axis_x, axis_y, axis_z;
				DEFINE_XML_NODE_SIMPLE(axis, rotate);
				DEFINE_XML_ATTRIBUTE_FLOAT(axis_x, attr_axis_x, node_axis, x);
				DEFINE_XML_ATTRIBUTE_FLOAT(axis_y, attr_axis_y, node_axis, y);
				DEFINE_XML_ATTRIBUTE_FLOAT(axis_z, attr_axis_z, node_axis, z);

				bone_track.push_back(
					BoneKeyframe(Quaternion::RotationAxis(Vector3(axis_x, axis_y, axis_z), angle), Vector3(translate_x, translate_y, translate_z), time));
			}
		}
	}
}

void OgreSkeletonAnimation::CreateOgreSkeletonAnimationFromFile(
	LPCTSTR pFilename)
{
	FILE * fp;
	if(0 != _tfopen_s(&fp, pFilename, _T("rb")))
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %s", pFilename));
	}

	CachePtr cache = ArchiveStreamPtr(new FileArchiveStream(fp))->GetWholeCache();

	CreateOgreSkeletonAnimation((LPCSTR)&(*cache)[0], cache->size());
}
