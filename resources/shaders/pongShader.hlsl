cbuffer ProjectionBuffer : register(b0)
{
    float4x4 projection;
};

struct VS_INPUT
{
    float4 pos : POSITION0;
    float4x4 world : WORLD;
    float4 color : COLOR0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(input.world, input.pos);
    output.pos = mul(projection, worldPos);
    output.color = input.color;
    return output;
}

float4 PSMain(PS_INPUT input) : SV_Target
{
    return input.color;
}