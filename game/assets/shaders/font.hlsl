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
    float OutlineWidth;
    float PxRange;
    bool IsMSDF;
};

// Texture and sampler declaration
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

// Median function
float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

// Pixel Shader
float4 PSMain(VS_OUTPUT input) : SV_TARGET {
    float4 texColor = texture0.Sample(sampler0, input.texCoord);

    if (IsMSDF) {
        uint w, h;
        texture0.GetDimensions(w, h);
        float2 texSize = float2(w, h);
        float2 unitRange = PxRange / texSize;
        float2 fwidthUV = abs(ddx(input.texCoord)) + abs(ddy(input.texCoord));
        float screenPxRangeVal = max(0.5 * dot(unitRange, 1.0 / fwidthUV), 1.0);
        float sd = median(texColor.r, texColor.g, texColor.b);
        float screenPxDist = screenPxRangeVal * (sd - 0.5);

        float fillAlpha = clamp(screenPxDist + 0.5, 0.0, 1.0);
        float4 color = float4(Color.rgb, Color.a) * fillAlpha;

        if (OutlineWidth > 0) {
            float maxOutline = (screenPxRangeVal * 0.5) - 0.5; // -0.5 for half-pixel antialiasing band at glyph edge
            float outlineWidth = min(OutlineWidth.r, maxOutline);
            float rawOutline = clamp(screenPxDist + outlineWidth + 0.5, 0.0, 1.0) - fillAlpha;
            // Gate outline on SDF data presence.
            float maxChannel = max(texColor.r, max(texColor.g, texColor.b));
            float dataPresent = smoothstep(0.0, 0.05, maxChannel);
            float outlineAlpha = rawOutline * dataPresent;
            color += float4(OutlineColor.rgb, OutlineColor.a) * outlineAlpha;
        }

        return color;
    } else {
        float4 color = texColor * Color;
        // r-alpha for 1 bit texture
        color.a = texColor.r * Color.a;
        return color;
    }
}