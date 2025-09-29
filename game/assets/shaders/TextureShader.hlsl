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

// Texture and sampler declaration
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    // Sample the texture using texture coordinates
    float4 color =  texture0.Sample(sampler0, input.texCoord);
float alpha = color.r;
float3 cl = float3(1.0f,1.0f,1.0f);
return float4(cl, alpha);
}
