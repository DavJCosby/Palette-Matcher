#version 410 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexTexCoord;

out vec3 Normal;
out vec2 TexCoord;
out vec4 LightViewPosition;
out vec3 FragPosition;
out mat3 NormalMatrix;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 LightSpaceMatrix;
uniform vec3 LightPosition;

void main() {
    FragPosition = VertexPosition;
    LightViewPosition = LightSpaceMatrix * vec4(VertexPosition, 1);
    // TODO: calculate in cpp and pass as uniform
    NormalMatrix = transpose(inverse(mat3(MV)));
    Normal = normalize(NormalMatrix * VertexNormal);
    TexCoord = VertexTexCoord;

    gl_Position = MVP * vec4(VertexPosition, 1.0);
}
