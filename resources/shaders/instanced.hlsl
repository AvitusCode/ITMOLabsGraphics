cbuffer Camera : register(b0)
{
    float4x4 View;
    float4x4 Projection;
};

struct VSInput
{
    float4 Position : POSITION;
    float4x4 World : WORLD;
    float4 Color : COLOR;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4x4 viewProj = mul(Projection, View);
    float4 worldPos = mul(input.World, input.Position);
    output.Position = mul(viewProj, worldPos);
    output.Color = input.Color;
    return output;
}

float4 PSMain(VSOutput input) : SV_Target
{
    return input.Color;
}