#include "SkyBox.h"

SkyBox::SkyBox(LPDIRECT3DDEVICE9 pD3DDevice)
	: m_Device(pD3DDevice)
{
	my::CachePtr cache = my::ResourceMgr::getSingleton().OpenArchiveStream("uffizi_cross.dds")->GetWholeCache();
	m_cubeTexture = my::CubeTexture::CreateCubeTextureFromFileInMemory(pD3DDevice, &(*cache)[0], cache->size());

	my::D3DVERTEXELEMENT9Set elems;
	elems.insert(my::D3DVERTEXELEMENT9Set::CreateCustomElement(0, D3DDECLUSAGE_POSITION, 0, 0, D3DDECLTYPE_FLOAT4));
	m_vertBuffer = my::VertexBuffer::CreateVertexBuffer(pD3DDevice, elems, 0);

	m_vertDecl = elems.CreateVertexDeclaration(pD3DDevice);

	cache = my::ResourceMgr::getSingleton().OpenArchiveStream("SkyBox.fx")->GetWholeCache();
	m_effect = my::Effect::CreateEffect(pD3DDevice, &(*cache)[0], cache->size());
}

SkyBox::~SkyBox(void)
{
}

void SkyBox::OnResetDevice(void)
{
	m_cubeTexture->OnResetDevice();

	const D3DSURFACE_DESC * pBackBufferSurfaceDesc = DXUTGetD3D9BackBufferSurfaceDesc();
    float fHighW = -1.0f - ( 1.0f / ( float )pBackBufferSurfaceDesc->Width );
    float fHighH = -1.0f - ( 1.0f / ( float )pBackBufferSurfaceDesc->Height );
    float fLowW = 1.0f + ( 1.0f / ( float )pBackBufferSurfaceDesc->Width );
    float fLowH = 1.0f + ( 1.0f / ( float )pBackBufferSurfaceDesc->Height );

	m_vertBuffer->ResizeVertexBufferLength(4);
	m_vertBuffer->SetCustomType(0, my::Vector4(fLowW,  fLowH,  1.0f, 1.0f), D3DDECLUSAGE_POSITION, 0);
	m_vertBuffer->SetCustomType(1, my::Vector4(fLowW,  fHighH, 1.0f, 1.0f), D3DDECLUSAGE_POSITION, 0);
	m_vertBuffer->SetCustomType(2, my::Vector4(fHighW, fLowH,  1.0f, 1.0f), D3DDECLUSAGE_POSITION, 0);
	m_vertBuffer->SetCustomType(3, my::Vector4(fHighW, fHighH, 1.0f, 1.0f), D3DDECLUSAGE_POSITION, 0);
	m_vertBuffer->OnResetDevice();

	m_effect->OnResetDevice();
}

void SkyBox::OnLostDevice(void)
{
	m_cubeTexture->OnLostDevice();

	m_vertBuffer->OnLostDevice();

	m_effect->OnLostDevice();
}

void SkyBox::OnDestroyDevice(void)
{
	m_cubeTexture->OnDestroyDevice();

	m_vertBuffer->OnDestroyDevice();

	m_effect->OnDestroyDevice();

	m_vertDecl.Release();

	m_Device.Release();
}

void SkyBox::Render(float fElapsedTime, const my::Matrix4 & mWorldViewProj)
{
	m_effect->SetMatrix("g_mInvWorldViewProjection", mWorldViewProj.inverse());
	m_effect->SetTechnique("Skybox");
	m_effect->SetTexture("g_EnvironmentTexture", m_cubeTexture->m_ptr);

	m_Device->SetStreamSource(m_vertBuffer->m_Stream, m_vertBuffer->m_VertexBuffer, 0, m_vertBuffer->m_vertexStride);
	m_Device->SetVertexDeclaration(m_vertDecl);

    UINT uiNumPasses = m_effect->Begin();
	for(UINT uiPass = 0; uiPass < uiNumPasses; uiPass++)
	{
		m_effect->BeginPass(uiPass);
		m_Device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_effect->EndPass();
	}
	m_effect->End();
}
