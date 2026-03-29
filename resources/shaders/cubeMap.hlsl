cbuffer ConstantBuffer : register(b0)
{
    float4x4 view;
    float4x4 projection;
};

struct VSInput
{
    float3 pos : POSITION;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 texCoord : TEXCOORD;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPos = float4(input.pos, 1.0f);
    output.pos = mul(view, worldPos);
    output.pos = mul(projection, output.pos);
    output.texCoord = input.pos;
    return output;
}

TextureCube gCubeMap : register(t0);
SamplerState gSampler : register(s0);

float4 PSMain(VSOutput input) : SV_TARGET
{
    return gCubeMap.Sample(gSampler, input.texCoord);
}