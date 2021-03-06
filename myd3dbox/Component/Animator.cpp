#include "stdafx.h"
#include "Animator.h"

using namespace my;

void SimpleAnimator::Update(float fElapsedTime)
{
	if (m_Skeleton)
	{
		OgreSkeletonAnimation::OgreAnimationNameMap::const_iterator anim_iter = m_Skeleton->m_animationMap.begin();
		if (anim_iter == m_Skeleton->m_animationMap.end())
		{
			return;
		}

		m_Time = fmod(m_Time + fElapsedTime, m_Skeleton->GetAnimation(anim_iter->first).GetTime());;

		BoneList animPose(m_Skeleton->m_boneBindPose.size());
		BoneList bindPoseHier(m_Skeleton->m_boneBindPose.size());
		BoneList animPoseHier(m_Skeleton->m_boneBindPose.size());
		my::BoneIndexSet::const_iterator root_iter = m_Skeleton->m_boneRootSet.begin();
		for (; root_iter != m_Skeleton->m_boneRootSet.end(); root_iter++)
		{
			m_Skeleton->BuildAnimationPose(
				animPose, m_Skeleton->m_boneHierarchy, *root_iter, anim_iter->first, m_Time);

			m_Skeleton->m_boneBindPose.BuildHierarchyBoneList(
				bindPoseHier, m_Skeleton->m_boneHierarchy, *root_iter, Quaternion(0,0,0,1), Vector3(0,0,0));

			animPose.BuildHierarchyBoneList(
				animPoseHier, m_Skeleton->m_boneHierarchy, *root_iter, Quaternion(0,0,0,1), Vector3(0,0,0));
		}
		m_DualQuats.resize(m_Skeleton->m_boneBindPose.size());
		bindPoseHier.BuildDualQuaternionList(m_DualQuats, animPoseHier);
	}
}
