
#include "stdafx.h"
#include "Game.h"
#include "MeshComponent.h"

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo : public Game
{
public:
	//MeshPtr m_static_mesh;
	//MeshPtr m_skined_mesh;
	//OgreSkeletonAnimationPtr m_skel_anim;
	//BoneList m_skel_pose;
	//BoneList m_skel_pose_heir1;
	//BoneList m_skel_pose_heir2;
	//TransformList m_dualquat;
	//EffectPtr m_static_mesh_effect;
	//EffectPtr m_skined_mesh_effect;
	//MaterialPtr m_material;

	EffectPtr m_SimpleSample;

	typedef std::vector<MeshComponentBasePtr> MeshComponentBasePtrList;

	MeshComponentBasePtrList m_Meshes;

	void DrawTextAtWorld(
		const Vector3 & pos,
		LPCWSTR lpszText,
		D3DCOLOR Color,
		Font::Align align = Font::AlignCenterMiddle)
	{
		Vector3 ptProj = pos.transformCoord(m_Camera->m_ViewProj);
		Vector2 vp = DialogMgr::GetDlgViewport();
		Vector2 ptVp(Lerp(0.0f, vp.x, (ptProj.x + 1) / 2), Lerp(0.0f, vp.y, (1 - ptProj.y) / 2));
		m_Font->DrawString(m_UIRender.get(), lpszText, my::Rectangle(ptVp, ptVp), Color, align);
	}

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Game::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		ExecuteCode("dofile \"GameStateMain.lua\"");

		//// ����mesh
		//m_static_mesh = LoadMesh("mesh/tube.mesh.xml");
		//m_skined_mesh = LoadMesh("mesh/tube.mesh.xml");
		//m_skel_anim = LoadSkeleton("mesh/tube.skeleton.xml");
		//m_static_mesh_effect = LoadEffect("shader/SimpleSample.fx", EffectMacroPairList());
		//EffectMacroPairList macros;
		//macros.push_back(EffectMacroPair("VS_SKINED_DQ",""));
		//m_skined_mesh_effect = LoadEffect("shader/SimpleSample.fx", macros);
		//m_material = LoadMaterial("material/lambert1.txt");

		m_SimpleSample = LoadEffect("shader/SimpleSample.fx", EffectMacroPairList());

		MeshComponentPtr mesh_cmp(new MeshComponent());
		mesh_cmp->m_Mesh = LoadMesh("mesh/casual19_m_highpoly.mesh.xml");
		std::vector<std::string>::const_iterator mat_name_iter = mesh_cmp->m_Mesh->m_MaterialNameList.begin();
		for(; mat_name_iter != mesh_cmp->m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
		{
			MaterialPtr mat = LoadMaterial(str_printf("material/%s.txt", mat_name_iter->c_str()));
			MeshComponent::MaterialPair mat_pair(mat, LoadEffect("shader/SimpleSample.fx", EffectMacroPairList()));
			mesh_cmp->m_Materials.push_back(mat_pair);
		}
		mesh_cmp->m_World = Matrix4::identity;
		m_Meshes.push_back(mesh_cmp);

		return S_OK;
	}

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Game::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}
		return S_OK;
	}

	virtual void OnLostDevice(void)
	{
		Game::OnLostDevice();
	}

	virtual void OnDestroyDevice(void)
	{
		m_Meshes.clear();

		Game::OnDestroyDevice();
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		Game::OnFrameMove(fTime, fElapsedTime);

		//// ���ö���
		//static float anim_time = 0;
		//anim_time = fmod(anim_time + fElapsedTime, m_skel_anim->GetAnimation("clip1").GetTime());
		//m_skel_pose.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_anim->BuildAnimationPose(
		//	m_skel_pose,
		//	m_skel_anim->m_boneHierarchy,
		//	m_skel_anim->GetBoneIndex("joint1"),
		//	"clip1",
		//	anim_time);
		//m_skel_pose_heir1.clear();
		//m_skel_pose_heir1.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_anim->m_boneBindPose.BuildHierarchyBoneList(
		//	m_skel_pose_heir1,
		//	m_skel_anim->m_boneHierarchy,
		//	m_skel_anim->GetBoneIndex("joint1"));
		//m_skel_pose_heir2.clear();
		//m_skel_pose_heir2.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_pose.BuildHierarchyBoneList(
		//	m_skel_pose_heir2,
		//	m_skel_anim->m_boneHierarchy,
		//	m_skel_anim->GetBoneIndex("joint1"));
		//m_dualquat.clear();
		//m_dualquat.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_pose_heir1.BuildDualQuaternionList(m_dualquat, m_skel_pose_heir2);
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);

		DrawHelper::DrawGrid(pd3dDevice);

		//// ��Ⱦ��̬mesh
		//if (m_static_mesh_effect) 
		//{
		//	Matrix4 World = Matrix4::Translation(Vector3(2,0,0));
		//	m_static_mesh_effect->SetTexture("g_MeshTexture", m_material->m_DiffuseTexture);
		//	m_static_mesh_effect->SetMatrix("g_World", World);
		//	m_static_mesh_effect->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
		//	UINT passes = m_static_mesh_effect->Begin();
		//	for(UINT p = 0; p < passes; p++)
		//	{
		//		m_static_mesh_effect->BeginPass(p);
		//		m_static_mesh->DrawSubset(0);
		//		m_static_mesh_effect->EndPass();
		//	}
		//	m_static_mesh_effect->End();
		//}

		//// ��Ⱦ����mesh
		//if (m_skined_mesh_effect)
		//{
		//	Matrix4 World = Matrix4::Translation(Vector3(-2,0,0));
		//	m_skined_mesh_effect->SetTexture("g_MeshTexture", m_material->m_DiffuseTexture);
		//	m_skined_mesh_effect->SetMatrix("g_World", World);
		//	m_skined_mesh_effect->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
		//	m_skined_mesh_effect->SetMatrixArray("g_dualquat", &m_dualquat[0], m_dualquat.size());
		//	UINT passes = m_skined_mesh_effect->Begin();
		//	for(UINT p = 0; p < passes; p++)
		//	{
		//		m_skined_mesh_effect->BeginPass(p);
		//		m_skined_mesh->DrawSubset(0);
		//		m_skined_mesh_effect->EndPass();
		//	}
		//	m_skined_mesh_effect->End();
		//}

		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
		MeshComponentBasePtrList::iterator mesh_cmp_iter = m_Meshes.begin();
		for(; mesh_cmp_iter != m_Meshes.end(); mesh_cmp_iter++)
		{
			(*mesh_cmp_iter)->Draw(MeshComponentBase::DrawStateOpaque, Matrix4::Scaling(0.05f,0.05f,0.05f));
		}

		m_EmitterInst->Begin();
		EmitterMgr::Draw(m_EmitterInst.get(), m_Camera.get(), fTime, fElapsedTime);
		m_EmitterInst->End();

		m_UIRender->Begin();
		m_UIRender->SetWorld(Matrix4::identity);
		m_UIRender->SetViewProj(DialogMgr::m_Camera.m_ViewProj);
		DrawTextAtWorld(Vector3(12,0,0), L"x", D3DCOLOR_ARGB(255,255,255,0));
		DrawTextAtWorld(Vector3(0,0,12), L"z", D3DCOLOR_ARGB(255,255,255,0));
		DialogMgr::Draw(m_UIRender.get(), fTime, fElapsedTime);
		_ASSERT(m_Font);
		m_UIRender->SetWorld(Matrix4::identity);
		m_Font->DrawString(m_UIRender.get(), m_strFPS, Rectangle::LeftTop(5,5,500,10), D3DCOLOR_ARGB(255,255,255,0));
		m_UIRender->End();
	}

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		LRESULT lr;
		if(lr = Game::MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
		{
			return lr;
		}
		return 0;
	}
};

// ------------------------------------------------------------------------------------------
// wWinMain
// ------------------------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPWSTR lpCmdLine,
					int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	// ����crtdbg�����ڴ�й©
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return Demo().Run();
}
