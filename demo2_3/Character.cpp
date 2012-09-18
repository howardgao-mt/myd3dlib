#include "StdAfx.h"
#include "Character.h"
#include "GameState.h"

void Character::OnFrameMove(double fTime, float fElapsedTime)
{
	const float totalTime = m_skeletonLOD[m_LODLevel]->GetAnimation("walk").GetTime();

	m_StateTime = fmod(m_StateTime + fElapsedTime, totalTime);

	// ��ȡ��ǰ����
	int root_i = m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01");
	m_animPose.clear();
	m_animPose.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size(), my::Bone(my::Quaternion::Identity(), my::Vector3(0,0,0)));
	m_skeletonLOD[m_LODLevel]->BuildAnimationPose(m_animPose, root_i, "walk", m_StateTime);

	m_animPose[root_i].m_position.z = 0;

	// ����ǰ�����Ͱ󶨶�������
	m_incrementedPose.clear();
	m_incrementedPose.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_animPose.Increment(
		m_incrementedPose, m_skeletonLOD[m_LODLevel]->m_boneBindPose, m_skeletonLOD[m_LODLevel]->m_boneHierarchy, root_i);

	// Ϊ�󶨶������ɲ�λ��Ĺ����б����б����ӹ��������ݽ������������ı任��
	m_hierarchyBoneList.clear();
	m_hierarchyBoneList.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_skeletonLOD[m_LODLevel]->m_boneBindPose.BuildHierarchyBoneList(
		m_hierarchyBoneList, m_skeletonLOD[m_LODLevel]->m_boneHierarchy, root_i);

	// ΪĿ�궯�����ɲ�λ��Ĺ����б�
	m_hierarchyBoneList2.clear();
	m_hierarchyBoneList2.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_incrementedPose.BuildHierarchyBoneList(
		m_hierarchyBoneList2, m_skeletonLOD[m_LODLevel]->m_boneHierarchy, root_i);

	// ���󶨶�����Ŀ�궯���Ĺ����б�����˫��Ԫʽ���󶨶���������Ҫ��任��˫��Ԫʽ����д�����
	m_dualQuaternionList.clear();
	m_dualQuaternionList.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_hierarchyBoneList.BuildDualQuaternionList(
		m_dualQuaternionList, m_hierarchyBoneList2);
}

void Character::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	Game::getSingleton().m_SimpleSample->SetMatrixArray("g_dualquat", &m_dualQuaternionList[0], m_dualQuaternionList.size());

	m_meshLOD[m_LODLevel]->Draw(pd3dDevice, fElapsedTime);
}