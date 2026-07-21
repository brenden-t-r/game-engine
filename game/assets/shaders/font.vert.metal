#include <metal_stdlib>
using namespace metal;

#include <simd/simd.h>
using namespace simd;

struct VertexData {
    float4 position;
    float2 textureCoordinate;
};

struct VertexOut {
    float4 position [[position]];
    float2 textureCoordinate;
};

vertex VertexOut vertex_main(uint vertexID [[vertex_id]],
                              constant VertexData* vertexData) {
    VertexOut out;
    out.position = vertexData[vertexID].position;
    out.textureCoordinate = vertexData[vertexID].textureCoordinate;
    return out;
}