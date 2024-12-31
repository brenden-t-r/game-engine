#version 330 core
layout(location = 0) in vec3 aPos;    // Position attribute
layout(location = 1) in vec2 aTexCoord; // Texture coordinate attribute

out vec2 TexCoords;

void main() {
    // Convert to clip space (-1 to 1)
    // Note: For 2D rendering, we keep z at 0.0
    gl_Position = vec4(aPos, 1.0);

    TexCoords = aTexCoord;
}
