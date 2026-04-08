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

cbuffer FontConstBuffer : register(b0)
{
    float4 Color;
    float4 OutlineColor;
    float4 OutlineWidth;
};

// Texture and sampler declaration
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

// Median function
float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}
static const float pxRange = 16.0;

// Pixel Shader
float4 PSMain(VS_OUTPUT input) : SV_TARGET {
    float4 texColor = texture0.Sample(sampler0, input.texCoord);
    float sd = median(texColor.r, texColor.g, texColor.b);

    uint w, h;
    texture0.GetDimensions(w, h);
    float2 texSize = float2(w, h);
    float2 unitRange = pxRange / texSize;
    float2 fwidthUV = abs(ddx(input.texCoord)) + abs(ddy(input.texCoord));
    float screenPxRangeVal = max(0.5 * dot(unitRange, 1.0 / fwidthUV), 1.0);
    float screenPxDist = screenPxRangeVal * (sd - 0.5);

    float outlineWidth = OutlineWidth.r;
    float4 outlineColor = OutlineColor;
    float fillAlpha    = clamp(screenPxDist + 0.5, 0.0, 1.0);
    float rawOutline   = clamp(screenPxDist + outlineWidth + 0.5, 0.0, 1.0) - fillAlpha;

    // Gate outline on SDF data presence.
    // Dead atlas background: all channels = 0. Glyph padding: at least one channel > 0.
    // smoothstep gives a soft transition at the padding boundary (via bilinear filtering).
    float maxChannel  = max(texColor.r, max(texColor.g, texColor.b));
    float dataPresent = smoothstep(0.0, 0.05, maxChannel);
    float outlineAlpha = rawOutline * dataPresent;

    // Premultiplied alpha composite (blend state: One, InvSrcAlpha)
    float4 color = float4(Color.rgb, Color.a) * fillAlpha
                 + float4(outlineColor.rgb, outlineColor.a) * outlineAlpha;

    return color;
}