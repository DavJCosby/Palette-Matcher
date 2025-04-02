#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 txc;
out vec3 normal;
// out vec3 light_direction;
out vec2 texCoord;
out vec4 lightView_Position;

out vec3 fragPosition;
out mat3 normalMatrix;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 lightSpaceMatrix;
uniform vec3 lightPosition;

void main() {
    fragPosition = pos;
    normalMatrix = transpose(inverse(mat3(mv)));
    normal = normalize(normalMatrix * norm);
    texCoord = txc;
    gl_Position = mvp * vec4(pos, 1.0);
    lightView_Position = lightSpaceMatrix * vec4(pos, 1);
}
