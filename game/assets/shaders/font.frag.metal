#include <metal_stdlib>
using namespace metal;

struct ConstantBufferData {
    float4 Color;
    float4 OutlineColor;
    float OutlineWidth;
    float PxRange;
    bool IsMSDF;
};

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

fragment float4 fragment_main(
    VertexOut in            [[stage_in]],
    texture2d<float> tex    [[texture(0)]],
    sampler textureSampler  [[sampler(0)]],
    constant ConstantBufferData& uniforms [[buffer(0)]])
{
    float4 texColor = tex.sample(textureSampler, in.textureCoordinate);

    if (uniforms.IsMSDF)
    {
        float2 textureSize = float2(tex.get_width(), tex.get_height());
        float2 uvRangePerPixel = float2(uniforms.PxRange) / textureSize;

        float2 uvChangePerScreenPixel =
            abs(dfdx(in.textureCoordinate)) + abs(dfdy(in.textureCoordinate));

        float screenPixelRange = max(
            0.5 * dot(uvRangePerPixel, 1.0 / uvChangePerScreenPixel),
            1.0
        );

        float signedDistance = median(texColor.r, texColor.g, texColor.b);
        float distanceInScreenPixels = screenPixelRange * (signedDistance - 0.5);
        float fillAlpha = clamp(distanceInScreenPixels + 0.5, 0.0, 1.0);
        float4 color = uniforms.Color * fillAlpha;

        if (uniforms.OutlineWidth > 0.0)
        {
            float maxPossibleOutlineWidth = (screenPixelRange * 0.5) - 0.5;
            float clampedOutlineWidth = min(uniforms.OutlineWidth, maxPossibleOutlineWidth);

            float outlineAndFillAlpha = clamp(
                distanceInScreenPixels + clampedOutlineWidth + 0.5,
                0.0, 1.0
            );

            float rawOutlineAlpha = outlineAndFillAlpha - fillAlpha;
            float maxChannel = max(texColor.r, max(texColor.g, texColor.b));
            float hasSDFData = smoothstep(0.0, 0.05, maxChannel);
            float outlineAlpha = rawOutlineAlpha * hasSDFData;

            color += uniforms.OutlineColor * outlineAlpha;
        }

        return color;
    }
    else
    {
        float4 color = texColor * uniforms.Color;
        // r-channel used as alpha for 1-bit textures
        color.a = texColor.r * uniforms.Color.a;
        return color;
    }
}