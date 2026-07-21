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
float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    float4 texColor = texture0.Sample(sampler0, input.texCoord);

    if (IsMSDF)
    {
        uint textureWidth, textureHeight;
        texture0.GetDimensions(textureWidth, textureHeight);
        float2 textureSize = float2(textureWidth, textureHeight);
        float2 uvRangePerPixel = PxRange / textureSize;
        float2 uvChangePerScreenPixel = abs(ddx(input.texCoord)) + abs(ddy(input.texCoord));
        float screenPixelRange = max(
            0.5 * dot(uvRangePerPixel, 1.0 / uvChangePerScreenPixel),
            1.0
        );
        float signedDistance = median(texColor.r, texColor.g, texColor.b);
        float distanceInScreenPixels = screenPixelRange * (signedDistance - 0.5);
        float fillAlpha = clamp(distanceInScreenPixels + 0.5, 0.0, 1.0);
        float4 color = Color * fillAlpha;

        if (OutlineWidth > 0)
        {
            float maxPossibleOutlineWidth = (screenPixelRange * 0.5) - 0.5;
            float clampedOutlineWidth = min(OutlineWidth.r, maxPossibleOutlineWidth);
            float outlineAndFillAlpha = clamp(
                distanceInScreenPixels + clampedOutlineWidth + 0.5,
                0.0, 1.0
            );
            float rawOutlineAlpha = outlineAndFillAlpha - fillAlpha;
            float maxChannel = max(texColor.r, max(texColor.g, texColor.b));
            float hasSDFData = smoothstep(0.0, 0.05, maxChannel);
            float outlineAlpha = rawOutlineAlpha * hasSDFData;
            color += OutlineColor * outlineAlpha;
        }

        return color;
    }
    else
    {
        float4 color = texColor * Color;
        // r-alpha for 1 bit texture
        color.a = texColor.r * Color.a;
        return color;
    }
}