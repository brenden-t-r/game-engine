// Vertex Shader
struct VS_INPUT
{
    float4 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
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
    // Return a solid light green color
    return float4(0.5f, 1.0f, 0.5f, 1.0f);  // RGBA for light green
}
