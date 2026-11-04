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
    float X;
    float Y;
    float Width;
    float Height;
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
    float2 pos = input.position.xy;

    // Center of the bounding box (your rect goes from Y down to Y - Height)
    float2 center = float2(X + Width * 0.5, Y - Height * 0.5);

    // Normalize to -1..1 inside the bounding box
    float2 p = (pos - center) / float2(Width * 0.5, Height * 0.5);
    p = abs(p); // hexagon is symmetric on both axes, so fold into one quadrant


    // Pointy-top hexagon:
    //   vertical sides:  |x| <= 1   (for |y| <= 0.5)
    //   slanted sides:   |y| + 0.5*|x| <= 1  (reaches the points at (0, ±1))
    bool inside = (p.x <= 1.0) && (p.y + 0.5 * p.x <= 1.0);

    if (!inside)
    {
        return float4(1, 1, 1, 0);
    }

    return Color;
}
// float4 PSMain(VS_OUTPUT input) : SV_TARGET
// {
//     float2 pos = input.position.xy;
//
//
//
// if (pos.x < X || pos.x > X + Width ||
//           pos.y > Y || pos.y < Y - Height)
//       {
//           return float4(0, 1, 0, 1);
// }
//
//     return Color;
// }