#include <metal_stdlib>
using namespace metal;

struct ConstantBufferData {
    float4 Color1;
    float4 Color2;
};

fragment float4 fragment_main(constant ConstantBufferData& uniforms [[ buffer(0) ]]) {
    return (uniforms.Color1 + uniforms.Color2) * 0.5;
}