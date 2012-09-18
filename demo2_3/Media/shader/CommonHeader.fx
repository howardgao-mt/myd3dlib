
//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

shared float g_fTime;
shared float4x4 g_mWorld;
shared float4x4 g_mWorldViewProjection;
shared float4x4 g_mLightViewProjection;
shared float3 g_EyePos;
shared float3 g_EyePosOS;
shared float3 g_LightDir;
shared float4 g_LightDiffuse;
shared texture g_ShadowTexture;
shared texture g_ReflectTexture;
shared texture g_RefractTexture;
shared row_major float2x4 g_dualquat[96];

//--------------------------------------------------------------------------------------
// SKINED_VS_INPUT
//--------------------------------------------------------------------------------------

struct SKINED_VS_INPUT
{
	float4 Pos				: POSITION;
	float4 BlendWeights		: BLENDWEIGHT;
	float4 BlendIndices		: BlendIndices;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float2 Tex0				: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// get_skined_dual
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

//--------------------------------------------------------------------------------------
// get_skined_vs
//--------------------------------------------------------------------------------------

void get_skined_vs(SKINED_VS_INPUT i, out float4 Pos)
{
	float2x4 dual = (float2x4)0;
	get_skined_dual(i, dual);
	
	float3 position = i.Pos.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, i.Pos.xyz) + dual[0].w * i.Pos.xyz);
	float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
	position += translation;
	Pos = float4(position, 1);
}

//--------------------------------------------------------------------------------------
// get_skined_vsnormal
//--------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------
// get_reflection
//--------------------------------------------------------------------------------------

float3 get_reflection(float3 Normal, float3 View)
{
	return normalize(2 * dot(Normal, View) * Normal - View);
}

//--------------------------------------------------------------------------------------
// get_fresnel
//--------------------------------------------------------------------------------------

float get_fresnel(float3 Normal, float3 View, float FresExp, float ReflStrength)
{
	return pow(1.0 - abs(dot(Normal, View)), FresExp) * ReflStrength;
}