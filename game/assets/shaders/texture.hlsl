// Vertex Shader
struct VS_INPUT
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = input.position;
    output.texCoord = input.texCoord;
    return output;
}

cbuffer ConstBuffer : register(b0)
{
    float4 Color; // RGBA tint
};

// Texture and sampler declaration
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    // Sample the texture using texture coordinates
    float4 texColor = texture0.Sample(sampler0, input.texCoord);
    float4 color = texColor * Color;
    return color;
}
