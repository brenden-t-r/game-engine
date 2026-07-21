#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D texture0;
uniform vec4 Color;
uniform vec4 OutlineColor;
uniform float OutlineWidth;
uniform float PxRange;
uniform bool IsMSDF;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec4 texColor = texture(texture0, TexCoords);

    if (IsMSDF)
    {
        vec2 textureSize = vec2(textureSize(texture0, 0));
        vec2 uvRangePerPixel = vec2(PxRange) / textureSize;

        vec2 uvChangePerScreenPixel = abs(dFdx(TexCoords)) + abs(dFdy(TexCoords));

        float screenPixelRange = max(
            0.5 * dot(uvRangePerPixel, 1.0 / uvChangePerScreenPixel),
            1.0
        );

        float signedDistance = median(texColor.r, texColor.g, texColor.b);
        float distanceInScreenPixels = screenPixelRange * (signedDistance - 0.5);
        float fillAlpha = clamp(distanceInScreenPixels + 0.5, 0.0, 1.0);
        vec4  color = Color * fillAlpha;

        if (OutlineWidth > 0.0)
        {
            float maxPossibleOutlineWidth = (screenPixelRange * 0.5) - 0.5;
            float clampedOutlineWidth = min(OutlineWidth, maxPossibleOutlineWidth);

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

        FragColor = color;
    }
    else
    {
        vec4 color = texColor * Color;
        // r-channel used as alpha for 1-bit textures
        color.a = texColor.r * Color.a;
        FragColor = color;
    }
}