#version 410 core

layout(location = 0) out vec4 color;
in vec3 normal;
in vec2 texCoord;
in vec4 lightView_Position;
in mat3 normalMatrix;
in vec3 fragPosition;

uniform vec3 lightPosition;
uniform vec3 base_color;
uniform vec3 specular_color;
uniform vec3 ambient;
uniform sampler2D diffuse_tex;
uniform sampler2D specular_tex;
uniform sampler2D shadow;
uniform float shine;
uniform float lightConeAngle;

const vec3 VIEW_DIR = vec3(0.0, 0.0, 1.0);
const vec3 LIGHT_COLOR = vec3(255.0, 245.0, 245.0) * 18.0; // 6200k

const vec3 K_COOL = vec3(0, 0, 1);
const vec3 K_WARM = vec3(1, 1, 0);

// Utility functions

float dot_clamped(vec3 a, vec3 b) {
    return max(0.0, dot(a, b));
}

float distance_squared(vec3 p1, vec3 p2) {
    vec3 diff = p1 - p2;
    return dot(diff, diff);
}

float degrees_between(vec3 u, vec3 v) {
    float cosTheta = dot(u, v);
    cosTheta = clamp(cosTheta, -1.0, 1.0);
    float theta = acos(cosTheta);
    float angleDegrees = degrees(theta);
    return angleDegrees;
}

// Shading functions

vec3 blinn_BRDF(
    vec3 incoming,
    vec3 outgoing,
    vec3 specular,
    vec3 n
) {
    vec3 h = normalize(incoming + outgoing);

    float phi = acos(dot(normalize(n), h));
    float specular_scale = pow(max(0.0, cos(phi)), shine);

    return specular * specular_scale;
}

vec3 blinn_shade(
    vec3 light_color,
    vec3 light_dir,
    vec3 cam_dir,
    vec3 diffuse,
    vec3 specular,
    vec3 n
) {
    vec3 ambient_contrib = ambient * diffuse;
    vec3 reflection = blinn_BRDF(light_dir, cam_dir, specular, n);
    vec3 light_contrib = (diffuse + reflection) * light_color * dot_clamped(n, light_dir);

    return ambient_contrib + light_contrib;
}

// // https://64.github.io/tonemapping/
// vec3 aces_approx_tonemapper(vec3 v) {
//     v *= 0.6f;
//     float a = 2.51f;
//     float b = 0.03f;
//     float c = 2.43f;
//     float d = 0.59f;
//     float e = 0.14f;
//     return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0f, 1.0f);
// }

void main() {
    vec3 light_direction = normalize(lightPosition - fragPosition);

    float attenuation = 1.0 / distance_squared(lightPosition, fragPosition);
    // crude; assumes light is always pointed at (0, 0, 0)
    float visibility = degrees_between(normalize(lightPosition), light_direction)
            < (lightConeAngle * 0.5) ? 1.0 : 0.0;

    light_direction = normalize(
            normalMatrix * light_direction
        );

    vec3 diffuse = texture(diffuse_tex, texCoord).rgb * base_color;
    vec3 specular = texture(specular_tex, texCoord).rgb * specular_color;
    vec3 n = normalize(normal);

    vec3 p = lightView_Position.xyz / lightView_Position.w;
    // textureProj() doesn't seem to be working
    vec3 light_color = LIGHT_COLOR
            * attenuation * visibility
            * (texture(shadow, p.xy).r < p.z ? 0 : 1);

    vec3 shaded_color = blinn_shade(
            light_color,
            light_direction,
            normalize(VIEW_DIR),
            diffuse,
            specular,
            n
        );

    color = vec4(shaded_color, 1);

    //
}
