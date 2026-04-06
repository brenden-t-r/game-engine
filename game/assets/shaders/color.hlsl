// Vertex Shader
struct VS_INPUT
{
    float4 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

cbuffer ColorBuffer : register(b0)
{
    float4 Color;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = input.position;
    return output;
}

// Simple pixel shader for solid color
float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    return Color;
}
