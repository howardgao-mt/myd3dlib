
#include "CommonHeader.fx"

// ------------------------------------------------------------------------------------------
// global variable
// ------------------------------------------------------------------------------------------

#define WAVE_LENGTH0 1.0
#define WAVE_LENGTH1 1.0
#define WAVE_LENGTH2 1.0
#define PI 3.141596

texture g_WaterNormalTexture;
texture g_CubeTexture;

float FresExp = 3.0;
float ReflStrength = 3.4;
float Amplitude[3] = {0.01, 0.01, 0.03};
float Frequency[3] = {2 * PI / WAVE_LENGTH0, 2 * PI / WAVE_LENGTH1, 2 * PI / WAVE_LENGTH2};
float Phase[3] = {0.2 * 2 * PI / WAVE_LENGTH0, 0.2 * 2 * PI / WAVE_LENGTH1, 0.2 * 2 * PI / WAVE_LENGTH2};
float GerstnerQ[3] = {0.6, 0.6, 0.6};
float3 WaveDir[3] = { {1.0, 0, 1.0}, {1.0, 0, -1.0}, {1.0, 0, 1.0} };

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------

sampler WaterNormalTextureSampler =
sampler_state
{
	Texture = <g_WaterNormalTexture>;
	AddressU = WRAP;
	AddressV = WRAP;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

samplerCUBE CubeTextureSampler =
sampler_state
{
	Texture = <g_CubeTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

// ------------------------------------------------------------------------------------------
// vs
// ------------------------------------------------------------------------------------------

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float2 texCoord0	: TEXCOORD0;
	float2 texCoord1	: TEXCOORD1;
	float2 texCoord2	: TEXCOORD2;
	float3 NormalWS		: TEXCOORD3;
	float3 TangentWS	: TEXCOORD4;
	float3 BinormalWS	: TEXCOORD5;
	float3 ViewWS		: TEXCOORD6;
};

VS_OUTPUT WaterEffectVS( in float4 iPos			: POSITION,
						 in float3 iNormal		: NORMAL,
						 in float3 iBinormal	: BINORMAL,
						 in float3 iTangent		: TANGENT,
						 in float2 iTexCoord	: TEXCOORD0 )
{
	VS_OUTPUT Out;
	
	float angle[3];
	float4 vPosWS = iPos;
	for(uint i = 0; i < 1; i++)
	{
		angle[i] = Frequency[i] * dot(WaveDir[i], iPos.xyz) + Phase[i] * g_fTime;
		float s = sin(angle[i]);
		float c = cos(angle[i]);
		vPosWS.x += WaveDir[i].x * GerstnerQ[i] * Amplitude[i] * c;
		vPosWS.y += Amplitude[i] * s;
		vPosWS.z += WaveDir[i].z * GerstnerQ[i] * Amplitude[i] * c;
	}
	Out.pos = mul(vPosWS, g_mWorldViewProjection);
	
	Out.texCoord0 = iTexCoord + g_fTime * 0.02;
	Out.texCoord1 = iTexCoord * 2.0 + g_fTime * -0.02;
	Out.texCoord2 = iTexCoord / 2.0 + g_fTime * 0.01;
	
	float3 vNormalWS = mul(iNormal, (float3x3)g_mWorld);
	float3 vTangentWS = mul(iTangent, (float3x3)g_mWorld);
	float3 vBinormalWS = cross(vNormalWS, vTangentWS);
	for(uint i = 0; i < 1; i++)
	{
		float wa = Frequency[i] * Amplitude[i];
		float s = sin(angle[i]);
		float c = cos(angle[i]);
		
		vTangentWS.x -= GerstnerQ[i] * WaveDir[i].x * WaveDir[i].x * wa * s;
		vTangentWS.y += WaveDir[i].x * wa * c;
		vTangentWS.z -= GerstnerQ[i] * WaveDir[i].x * WaveDir[i].z * wa * s;
		
		vNormalWS.x -= WaveDir[i].x * wa * c;
		vNormalWS.y -= GerstnerQ[i] * wa * s;
		vNormalWS.z -= WaveDir[i].z * wa * c;
		
		vBinormalWS.x -= GerstnerQ[i] * WaveDir[i].x * WaveDir[i].z * wa * s;
		vBinormalWS.y += WaveDir[i].z * wa * c;
		vBinormalWS.z -= GerstnerQ[i] * WaveDir[i].z * WaveDir[i].z * wa * s;
	}
	Out.NormalWS = vNormalWS;
	Out.TangentWS = vTangentWS;
	Out.BinormalWS = vBinormalWS;
	Out.ViewWS = g_EyePos - vPosWS;
	return Out;
}

// ------------------------------------------------------------------------------------------
// ps
// ------------------------------------------------------------------------------------------

float4 WaterEffectPS( VS_OUTPUT In ) : COLOR
{
	float3x3 mT2W = float3x3(normalize(In.TangentWS), normalize(In.BinormalWS), normalize(In.NormalWS));
	
	float3 vViewWS = normalize(In.ViewWS);
	
	float3 texNormal0 = tex2D(WaterNormalTextureSampler, In.texCoord0).xyz;
	float3 texNormal1 = tex2D(WaterNormalTextureSampler, In.texCoord1).xyz;
	float3 texNormal2 = tex2D(WaterNormalTextureSampler, In.texCoord2).xyz;
	float3 vNormalTS = normalize(2.0 * (texNormal0 + texNormal1 + texNormal2) - 3.0);
	
	float3 vNormalWS = mul(vNormalTS, mT2W);
	
	float3 vReflectionWS = get_reflection(vNormalWS, vViewWS);
	
	float4 cReflection = texCUBE(CubeTextureSampler, vReflectionWS);
	
	float fres = get_fresnel(vNormalWS, vViewWS, FresExp, ReflStrength);
	
	float3 WaterColor = {2 / 255.0, 10 / 255.0, 31 / 255.0};
	
	return float4(cReflection.xyz * fres + WaterColor * (1 - fres), 1);
}

// ------------------------------------------------------------------------------------------
// technique
// ------------------------------------------------------------------------------------------

technique WaterEffect
{
	pass P0
	{
		// AlphaBlendEnable = TRUE;
		// SrcBlend = SRCALPHA;
		// DestBlend = INVSRCALPHA;
		// FillMode = WIREFRAME;
		VertexShader = compile vs_2_0 WaterEffectVS();
		PixelShader = compile ps_2_0 WaterEffectPS();
	}
}
