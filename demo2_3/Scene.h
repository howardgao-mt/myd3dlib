#pragma once

#include <myd3dlib.h>
#include "Camera.h"

class BaseScene
{
public:
	static void DrawLine(
		IDirect3DDevice9 * pd3dDevice,
		const my::Vector3 & v0,
		const my::Vector3 & v1,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawSphere(
		IDirect3DDevice9 * pd3dDevice,
		float radius,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawBox(
		IDirect3DDevice9 * pd3dDevice,
		const my::Vector3 & halfSize,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawTriangle(
		IDirect3DDevice9 * pd3dDevice,
		const my::Vector3 & v0,
		const my::Vector3 & v1,
		const my::Vector3 & v2,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawSpereStage(
		IDirect3DDevice9 * pd3dDevice,
		float radius,
		int VSTAGE_BEGIN,
		int VSTAGE_END,
		float offsetY,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawCylinderStage(
		IDirect3DDevice9 * pd3dDevice,
		float radius,
		float y0,
		float y1,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

	static void DrawCapsule(
		IDirect3DDevice9 * pd3dDevice,
		float radius,
		float height,
		D3DCOLOR Color,
		const my::Matrix4 & world = my::Matrix4::identity);

public:
	virtual ~BaseScene(void)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime) = 0;

	virtual void OnRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime) = 0;
};

typedef boost::shared_ptr<BaseScene> BaseScenePtr;

class Scene
	: public BaseScene
{
public:
	BaseCameraPtr m_Camera;

public:
	Scene(void)
	{
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);

	virtual void OnRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);
};
