#include "./CameraMatrixBuffer.hlsli"

cbuffer StaticMatrixBuffer : register(b1)
{
	float4x4 World;
	float4 Position;
	float4 Color;
};

cbuffer MiscBuffer : register(b3)
{
	int AlphaTest;
};

#include "./VertexInput.hlsli"

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
	float Fog : FOG;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 worldPosition = mul(float4(input.Position, 1.0f), World);

	output.Position = mul(worldPosition, ViewProjection); 
	output.Normal = input.Normal;
	output.Color = input.Color * Color;
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
	float4 output = Texture.Sample(Sampler, input.UV);
	if (AlphaTest && output.w < 0.01f) {
		discard;
	}
	float3 colorMul = min(input.Color.xyz, 1.0f);
	output.xyz = output.xyz * colorMul.xyz;
	//output.w = 1.0f;

	if (FogMaxDistance != 0)
		output.xyz = lerp(output.xyz, FogColor, input.Fog);

	return output;
}