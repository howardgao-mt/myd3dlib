
//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

shared float g_fTime;
shared float4x4 g_mWorld;
shared float4x4 g_mWorldViewProjection;
shared float3 g_EyePos;
shared float3 g_LightDir;
shared float4 g_LightDiffuse;
shared texture  g_ShadowTexture;
shared row_major float2x4 g_dualquat[96];

//--------------------------------------------------------------------------------------
// Vertex shader input structure
//--------------------------------------------------------------------------------------

struct SKINED_VS_INPUT
{
    float4 Pos				: POSITION;
    float4 BlendWeights		: BLENDWEIGHT;
    float4 BlendIndices		: BlendIndices;
    float3 Normal			: NORMAL;
	float3 Binormal			: BINORMAL;
	float3 Tangent			: TANGENT;
    float2 Tex0				: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// This shader computes skined transform
//--------------------------------------------------------------------------------------

void get_skined_dual(SKINED_VS_INPUT i, out float2x4 dual)
{
    float2x4 m = g_dualquat[i.BlendIndices.x];
    float4 dq0 = (float1x4)m;
	dual = i.BlendWeights.x * m;
	
    m = g_dualquat[i.BlendIndices.y];
    float4 dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= i.BlendWeights.y * m;
    else
		dual += i.BlendWeights.y * m;
		
	m = g_dualquat[i.BlendIndices.z];
    dq = (float1x4)m;
    if (dot( dq0, dq ) < 0)
		dual -= i.BlendWeights.z * m;
	else
		dual += i.BlendWeights.z * m;
		
	m = g_dualquat[i.BlendIndices.w];
    dq = (float1x4)m;
    if (dot( dq0, dq ) < 0)
		dual -= i.BlendWeights.w * m;
	else
		dual += i.BlendWeights.w * m;
		
    float length = sqrt(dual[0].w * dual[0].w + dual[0].x * dual[0].x + dual[0].y * dual[0].y + dual[0].z * dual[0].z);
    dual = dual / length;
}

void get_skined_vs(SKINED_VS_INPUT i, out float4 Pos)
{
    float2x4 dual = (float2x4)0;
	get_skined_dual(i, dual);
	
    float3 position = i.Pos.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, i.Pos.xyz) + dual[0].w * i.Pos.xyz);
    float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
    position += translation;
    Pos = float4(position, 1);
}

void get_skined_vsnormal(SKINED_VS_INPUT i, out float4 Pos, out float3 Normal, out float3 Tangent)
{
	float2x4 dual = (float2x4)0;
	get_skined_dual(i, dual);
	
    float3 position = i.Pos.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, i.Pos.xyz) + dual[0].w * i.Pos.xyz);
    float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
    position += translation;
    Pos = float4(position, 1);
	
    Normal = i.Normal.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, i.Normal.xyz) + dual[0].w * i.Normal.xyz);
	Tangent = i.Tangent.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, i.Tangent.xyz) + dual[0].w * i.Tangent.xyz);;
}