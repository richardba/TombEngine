#include "./CameraMatrixBuffer.hlsli"
#include "./Blending.hlsli"
#include "./VertexInput.hlsli"
#include "./Math.hlsli"

// NOTE: This shader is used for all 3D and alpha blended sprites, because we send aleady transformed vertices to the GPU 
// instead of instances

#define FADE_FACTOR .789f

cbuffer SpriteBuffer : register(b9)
{
	float IsSoftParticle;
	float isTr5Laser;
	float secondsUniform;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float  Fog : FOG;
	float4 PositionCopy: TEXCOORD2;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D DepthMap : register(t6);
SamplerState DepthMapSampler : register(s6);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 worldPosition = float4(input.Position, 1.0f);

	output.Position = mul(worldPosition, ViewProjection);
	output.PositionCopy = output.Position;	
	output.Normal = input.Normal;
	output.Color = input.Color;
	output.UV = input.UV;

	float4 d = length(CamPositionWS - worldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV) * input.Color;
	
	DoAlphaTest(output);

	if (IsSoftParticle == 1)
	{
		float particleDepth = input.PositionCopy.z / input.PositionCopy.w;
		input.PositionCopy.xy /= input.PositionCopy.w;
		float2 texCoord = 0.5f * (float2(input.PositionCopy.x, -input.PositionCopy.y) + 1);
		float sceneDepth = DepthMap.Sample(DepthMapSampler, texCoord).r;

		sceneDepth = LinearizeDepth(sceneDepth, NearPlane, FarPlane);
		particleDepth = LinearizeDepth(particleDepth, NearPlane, FarPlane);

		if (particleDepth - sceneDepth > 0.01f)
			discard;

		float fade = (sceneDepth - particleDepth) * 1024.0f;
		output.w = min(output.w, fade);
	}

	if(isTr5Laser == 1)
	{
		output = DoLasers(input.Position, output, input.UV, FADE_FACTOR, secondsUniform);
	}

	output = DoFog(output, float4(0.0f, 0.0f, 0.0f, 0.0f), input.Fog);

	return output;
}
