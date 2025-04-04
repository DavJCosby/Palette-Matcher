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
uniform sampler2DShadow ShadowMap;
uniform float Shine;
uniform float LightConeAngle;

// gooch shading constants
const vec3 K_COOL = vec3(64.0, 6.0, 191.0) / 255.0;
const vec3 K_WARM = vec3(255.0, 223.0, 97.0) / 255.0;
const float ALPHA = 0.6;
const float BETA = 0.2;
const float FRESNEL_SCALE = 0.3;

const vec3 VIEW_DIR = vec3(0.0, 0.0, 1.0);
const vec3 LIGHT_COLOR = vec3(255.0, 255.0, 255.0) * 8.0; // 6200k

struct LightData {
    vec3 color; // color of light
    vec3 direction; // direction to light from fragment (in viewspace)
    float visibility;
};

// FORWARD DECLARATIONS - LIGHTING FUNCTIONS
LightData sample_spotlight();
float blinn_BRDF(vec3 incoming, vec3 outgoing, vec3 n);
vec3 gooch_shade(LightData light, vec3 albedo, vec3 specular, vec3 n);
float fresnel(float amount, vec3 n, vec3 v);

// FORWARD DECLARATIONS - UTILITY FUNCTIONS
float dot_clamped(vec3 a, vec3 b);
float distance_squared(vec3 p1, vec3 p2);
float degrees_between(vec3 u, vec3 v);

vec3 oklab_from_rgb(vec3 rgb);
vec3 rgb_from_oklab(vec3 oklab);

void main() {
    vec3 k_d = texture(DiffuseTexture, TexCoord).rgb * BaseColor;
    vec3 k_s = texture(SpecularTexture, TexCoord).rgb * SpecularColor;
    LightData spotlight = sample_spotlight();

    vec3 shaded_color = gooch_shade(spotlight, k_d, k_s, normalize(Normal));
    color = vec4(shaded_color, 1);
}

// LIGHTING FUNCTIONS

LightData sample_spotlight() {
    vec3 direction_to_light = normalize(LightPosition - FragPosition);
    vec3 fragpos_lightspace = LightViewPosition.xyz / LightViewPosition.w;
    vec3 spotlight_look_dir = normalize(LightPosition); // Crude; assumes light is always pointed at origin. Fix later.
    float angle = degrees_between(spotlight_look_dir, direction_to_light);

    float crop = angle < (LightConeAngle * 0.5) ? 1.0 : 0.0;
    float attenuation = 1.0 / distance_squared(LightPosition, FragPosition);
    float shadowed = textureProj(ShadowMap, LightViewPosition);
    float visibility = crop * shadowed;

    vec3 color = LIGHT_COLOR * attenuation * visibility;
    vec3 direction_viewspace = normalize(NormalMatrix * direction_to_light);

    return LightData(color, direction_viewspace, visibility);
}

vec3 gooch_shade(LightData light, vec3 albedo, vec3 specular, vec3 normal) {
    float gooch = (1.0f + dot(light.direction, normal)) / 2.0;
    gooch = gooch * max(light.visibility, 0.33); // darken in shadowed areas

    // oklab produces more natural color interpolations
    vec3 albedo_oklab = oklab_from_rgb(albedo);
    vec3 k_cool = mix(oklab_from_rgb(K_COOL), albedo_oklab, ALPHA); // TODO: precompute K_COOL/K_WARM oklabs once
    vec3 k_warm = mix(oklab_from_rgb(K_WARM), albedo_oklab, BETA);
    vec3 gooch_diffuse = gooch * k_warm + (1 - gooch) * k_cool;
    gooch_diffuse = rgb_from_oklab(gooch_diffuse);

    float reflect_amount = blinn_BRDF(light.direction, VIEW_DIR, normal);
    vec3 highlights = reflect_amount * specular
            * light.color
            * dot_clamped(normal, light.direction)
            * vec3(1 + fresnel(2.5, normal, VIEW_DIR) * FRESNEL_SCALE);

    return highlights + gooch_diffuse + fresnel(2.5, normal, VIEW_DIR) * light.color * FRESNEL_SCALE;
}

float blinn_BRDF(vec3 incoming, vec3 outgoing, vec3 normal) {
    vec3 halfway = normalize(incoming + outgoing);
    float specular = pow(dot_clamped(normal, halfway), Shine);
    return specular;
}

float fresnel(float amount, vec3 n, vec3 v) {
    return pow(
        1.0 - clamp(dot(n, v), 0.0, 1.0),
        amount
    );
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

vec3 oklab_from_rgb(vec3 rgb) {
    // https://bottosson.github.io/posts/oklab
    const mat3 im1 = mat3(0.4121656120, 0.2118591070, 0.0883097947,
            0.5362752080, 0.6807189584, 0.2818474174,
            0.0514575653, 0.1074065790, 0.6302613616);

    const mat3 im2 = mat3(+0.2104542553, +1.9779984951, +0.0259040371,
            +0.7936177850, -2.4285922050, +0.7827717662,
            -0.0040720468, +0.4505937099, -0.8086757660);

    vec3 lms = im1 * rgb;

    return im2 * (sign(lms) * pow(abs(lms), vec3(1.0 / 3.0)));
}

vec3 rgb_from_oklab(vec3 oklab) {
    // https://bottosson.github.io/posts/oklab
    const mat3 m1 = mat3(+1.000000000, +1.000000000, +1.000000000,
            +0.396337777, -0.105561346, -0.089484178,
            +0.215803757, -0.063854173, -1.291485548);

    const mat3 m2 = mat3(+4.076724529, -1.268143773, -0.004111989,
            -3.307216883, +2.609332323, -0.703476310,
            +0.230759054, -0.341134429, +1.706862569);
    vec3 lms = m1 * oklab;

    return m2 * (lms * lms * lms);
}
