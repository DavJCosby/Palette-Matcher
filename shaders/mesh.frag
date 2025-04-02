#version 410 core

layout(location = 0) out vec4 color;

in vec3 Normal;
in vec2 TexCoord;
in vec4 LightViewPosition;
in vec3 FragPosition;
in mat3 NormalMatrix;

uniform vec3 LightPosition;
uniform vec3 BaseColor;
uniform vec3 SpecularColor;
uniform vec3 Ambient;
uniform sampler2D DiffuseTexture;
uniform sampler2D SpecularTexture;
uniform sampler2D ShadowMap;
uniform float Shine;
uniform float LightConeAngle;

const vec3 VIEW_DIR = vec3(0.0, 0.0, 1.0);
const vec3 LIGHT_COLOR = vec3(255.0, 245.0, 245.0) * 18.0; // 6200k
// gooch shading constants
const vec3 K_COOL = vec3(0, 0, 1);
const vec3 K_WARM = vec3(1, 1, 0);

struct LightData {
    vec3 color; // color of light
    vec3 direction; // direction to light from fragment (in viewspace)
};

// FORWARD DECLARATIONS - LIGHTING FUNCTIONS
LightData sample_spotlight();
vec3 blinn_BRDF(vec3 incoming, vec3 outgoing, vec3 specular, vec3 n);
vec3 blinn_shade(LightData light, vec3 diffuse, vec3 specular, vec3 n);

// FORWARD DECLARATIONS - UTILITY FUNCTIONS
float dot_clamped(vec3 a, vec3 b);
float distance_squared(vec3 p1, vec3 p2);
float degrees_between(vec3 u, vec3 v);

void main() {
    vec3 k_d = texture(DiffuseTexture, TexCoord).rgb * BaseColor;
    vec3 k_s = texture(SpecularTexture, TexCoord).rgb * SpecularColor;
    LightData spotlight = sample_spotlight();

    vec3 shaded_color = blinn_shade(spotlight, k_d, k_s, normalize(Normal));
    color = vec4(shaded_color, 1);
    //color = vec4(1, 1, 1, 1);
}

// LIGHTING FUNCTIONS

LightData sample_spotlight() {
    vec3 direction_to_light = normalize(LightPosition - FragPosition);
    vec3 fragpos_lightspace = LightViewPosition.xyz / LightViewPosition.w;
    vec3 spotlight_look_dir = normalize(LightPosition); // Crude; assumes light is always pointed at origin. Fix later.
    float angle = degrees_between(spotlight_look_dir, direction_to_light);

    float crop = angle < (LightConeAngle * 0.5) ? 1.0 : 0.0;
    float attenuation = 1.0 / distance_squared(LightPosition, FragPosition);
    float shadowed = texture(ShadowMap, fragpos_lightspace.xy).r < fragpos_lightspace.z ? 0 : 1;

    vec3 color = LIGHT_COLOR * crop * attenuation * shadowed;
    vec3 direction_viewspace = normalize(NormalMatrix * direction_to_light);

    return LightData(color, direction_viewspace);
}

vec3 blinn_shade(LightData light, vec3 diffuse, vec3 specular, vec3 normal) {
    vec3 ambient_contrib = Ambient * diffuse;

    vec3 reflection = blinn_BRDF(light.direction, VIEW_DIR, specular, normal);
    vec3 light_contrib = (diffuse + reflection)
            * light.color
            * dot_clamped(normal, light.direction);

    return ambient_contrib + light_contrib;
}

vec3 blinn_BRDF(vec3 incoming, vec3 outgoing, vec3 specular, vec3 normal) {
    vec3 halfway = normalize(incoming + outgoing);
    float specular_scale = pow(dot_clamped(normal, halfway), Shine);
    return specular * specular_scale;
}

// UTILITY FUNCTIONS

float dot_clamped(vec3 a, vec3 b) {
    return max(0.0, dot(a, b));
}

float distance_squared(vec3 p1, vec3 p2) {
    vec3 diff = p1 - p2;
    return dot(diff, diff);
}

// TODO: just use radians bro
float degrees_between(vec3 u, vec3 v) {
    float cos_theta = dot(u, v);
    cos_theta = clamp(cos_theta, -1.0, 1.0);
    float theta = acos(cos_theta);
    float angle_degrees = degrees(theta);
    return angle_degrees;
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
