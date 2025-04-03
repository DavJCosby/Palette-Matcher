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
vec3 gooch_shade(LightData light, vec3 diffuse, vec3 specular, vec3 n);

// FORWARD DECLARATIONS - UTILITY FUNCTIONS
float dot_clamped(vec3 a, vec3 b);
float distance_squared(vec3 p1, vec3 p2);
float degrees_between(vec3 u, vec3 v);

//Convert sRGB to linear RGB
vec3 linear_from_srgb(vec3 rgb) {
    return pow(rgb, vec3(2.2));
}
//Convert linear RGB to sRGB
vec3 srgb_from_linear(vec3 lin) {
    return pow(lin, vec3(1.0 / 2.2));
}

float fresnel(float amount, vec3 n, vec3 v) {
    return pow(
        1.0 - clamp(dot(n, v), 0.0, 1.0),
        amount
    );
}

void main() {
    vec3 k_d = texture(DiffuseTexture, TexCoord).rgb * BaseColor;
    vec3 k_s = texture(SpecularTexture, TexCoord).rgb * SpecularColor;
    LightData spotlight = sample_spotlight();

    vec3 shaded_color = gooch_shade(spotlight, k_d, k_s, normalize(Normal));
    color = vec4(shaded_color, 1);
    //color = vec4(fresnel(3.0, normalize(Normal), VIEW_DIR));
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

vec3 oklab_mix(vec3 k_warm, vec3 k_cool, float gooch);
vec3 oklab_from_linear(vec3 linear);
vec3 linear_from_oklab(vec3 c);

vec3 gooch_shade(LightData light, vec3 diffuse, vec3 specular, vec3 normal) {
    float gooch = (1.0f + dot(light.direction, normal)) / 2.0;
    gooch = gooch * max(light.visibility, 0.33);
    //vec3 gooch = gooch_amount + (fresnel(2.5, normal, VIEW_DIR) * light.color * FRESNEL_SCALE);
    vec3 k_cool = mix(oklab_from_linear(K_COOL), oklab_from_linear(BaseColor), ALPHA);
    vec3 k_warm = mix(oklab_from_linear(K_WARM), oklab_from_linear(BaseColor), BETA);
    vec3 gooch_diffuse = gooch * k_warm + (1 - gooch) * k_cool;
    gooch_diffuse = linear_from_oklab(gooch_diffuse);

    float reflect_amount = blinn_BRDF(light.direction, VIEW_DIR, normal);
    vec3 highlights = reflect_amount * specular
            * light.color
            * dot_clamped(normal, light.direction) * vec3(1 + fresnel(2.5, normal, VIEW_DIR));

    return highlights + gooch_diffuse + fresnel(2.5, normal, VIEW_DIR) * light.color * FRESNEL_SCALE;
}

float blinn_BRDF(vec3 incoming, vec3 outgoing, vec3 normal) {
    vec3 halfway = normalize(incoming + outgoing);
    float specular = pow(dot_clamped(normal, halfway), Shine);
    return specular;
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

vec3 oklab_from_linear(vec3 linear) {
    const mat3 im1 = mat3(0.4121656120, 0.2118591070, 0.0883097947,
            0.5362752080, 0.6807189584, 0.2818474174,
            0.0514575653, 0.1074065790, 0.6302613616);

    const mat3 im2 = mat3(+0.2104542553, +1.9779984951, +0.0259040371,
            +0.7936177850, -2.4285922050, +0.7827717662,
            -0.0040720468, +0.4505937099, -0.8086757660);

    vec3 lms = im1 * linear;

    return im2 * (sign(lms) * pow(abs(lms), vec3(1.0 / 3.0)));
}

vec3 linear_from_oklab(vec3 oklab) {
    const mat3 m1 = mat3(+1.000000000, +1.000000000, +1.000000000,
            +0.396337777, -0.105561346, -0.089484178,
            +0.215803757, -0.063854173, -1.291485548);

    const mat3 m2 = mat3(+4.076724529, -1.268143773, -0.004111989,
            -3.307216883, +2.609332323, -0.703476310,
            +0.230759054, -0.341134429, +1.706862569);
    vec3 lms = m1 * oklab;

    return m2 * (lms * lms * lms);
}

vec3 oklab_mix(vec3 lin1, vec3 lin2, float a)
{
    // https://bottosson.github.io/posts/oklab
    const mat3 kCONEtoLMS = mat3(
            0.4121656120, 0.2118591070, 0.0883097947,
            0.5362752080, 0.6807189584, 0.2818474174,
            0.0514575653, 0.1074065790, 0.6302613616);
    const mat3 kLMStoCONE = mat3(
            4.0767245293, -1.2681437731, -0.0041119885,
            -3.3072168827, 2.6093323231, -0.7034763098,
            0.2307590544, -0.3411344290, 1.7068625689);

    // rgb to cone (arg of pow can't be negative)
    vec3 lms1 = pow(kCONEtoLMS * lin1, vec3(1.0 / 3.0));
    vec3 lms2 = pow(kCONEtoLMS * lin2, vec3(1.0 / 3.0));
    // lerp
    vec3 lms = mix(lms1, lms2, a);
    // gain in the middle (no oklab anymore, but looks better?)
    lms *= 1.0 + 0.2 * a * (1.0 - a);
    // cone to rgb
    return kLMStoCONE * (lms * lms * lms);
}
