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

// Median function
float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}
static const float pxRange = 4.0;

// Pixel Shader
float4 PSMain(VS_OUTPUT input) : SV_TARGET {
    float4 texColor = texture0.Sample(sampler0, input.texCoord);
    float sd = median(texColor.r, texColor.g, texColor.b);

    // Correct screenPxRange calculation (inline)
    float2 texSize;
    uint w, h;
    texture0.GetDimensions(w, h);
    texSize = float2(w, h);
    float2 unitRange = pxRange / texSize; // pxRange
    float2 fwidthUV = abs(ddx(input.texCoord)) + abs(ddy(input.texCoord));
    float screenPxRangeVal = max(0.5 * dot(unitRange, 1.0 / fwidthUV), 1.0);
    float screenPxDist = screenPxRangeVal * (sd - 0.5);
    float opacity = clamp(screenPxDist + 0.5, 0.0, 1.0);

    float4 bgColor = float4(0, 0, 0, 0);
    float4 fgColor = float4(Color.rgb, Color.a * opacity);
    return fgColor;
}