#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D spriteTexture;
uniform vec4 Color;

void main() {
    // Sample texture and multiply by color for tinting
    vec4 texColor = texture(spriteTexture, TexCoords);
    texColor *= Color;
    FragColor = texColor;
}