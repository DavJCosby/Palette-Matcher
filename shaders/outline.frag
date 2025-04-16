#version 410 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ScreenTexture;
uniform sampler2D DepthTexture;

const float EDGE_THRESHOLD = 0.003;
const float EDGE_THRESHOLD_FEATHER = 0.00225;
const vec3 EDGE_COLOR = vec3(0.1, 0.0, 0.2);
const float STROKE_RADIUS = 0.55;

const vec3[5] PALETTE = vec3[5](vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1), vec3(1, 1, 1));
const int BAYER_N = 2;
const int BAYER_N_SQ = BAYER_N * BAYER_N;
const int BAYER_MATRIX[BAYER_N_SQ] = int[BAYER_N_SQ](0, 2, 3, 1);

// FORWARD DECLARATIONS - OUTLINES
void apply_edges();
float linearize_depth(float depth);

// FORWARD DECLARATIONS - PALETTE MATCHING
vec3 closest_candiate(vec3 target) {
    float dist = 10000.0;
    vec3 closest = vec3(1, 0, 1);

    for (int i = 0; i < PALETTE.length(); i++) {
        vec3 color = PALETTE[i];
        float current_dist = length(target - color);
        if (current_dist < dist) {
            dist = current_dist;
            closest = color;
        }
    }
    return closest;
}
// FORWARD DECLARATIONS - UTILITY FUNCTIONS
vec3 oklab_from_rgb(vec3 rgb);
vec3 rgb_from_oklab(vec3 oklab);

void main() {
    FragColor = texture(ScreenTexture, TexCoord);
    apply_edges();
    vec2 pixel_coordinate = floor(TexCoord * textureSize(ScreenTexture, 0));
    highp int index = int(pixel_coordinate.x + pixel_coordinate.y);

    FragColor = vec4(closest_candiate(FragColor.rgb), 1);
}

// OUTLINES

void apply_edges() {
    float pixel_size_x = STROKE_RADIUS / textureSize(ScreenTexture, 0).x;
    float pixel_size_y = STROKE_RADIUS / textureSize(ScreenTexture, 0).y;

    float left_depth = linearize_depth(texture(DepthTexture, TexCoord + vec2(-pixel_size_x, 0.0)).r);
    float right_depth = linearize_depth(texture(DepthTexture, TexCoord + vec2(pixel_size_x, 0.0)).r);
    float top_depth = linearize_depth(texture(DepthTexture, TexCoord + vec2(0.0, pixel_size_y)).r);
    float bottom_depth = linearize_depth(texture(DepthTexture, TexCoord + vec2(0.0, -pixel_size_y)).r);

    float horizontal_diff = abs(right_depth - left_depth);
    float vertical_diff = abs(top_depth - bottom_depth);

    float edge_intensity = max(horizontal_diff, vertical_diff);

    // Apply edge detection
    if (edge_intensity > EDGE_THRESHOLD) {
        FragColor = vec4(EDGE_COLOR, 1);
    } else {
        if (edge_intensity > EDGE_THRESHOLD_FEATHER) {
            float amount = (edge_intensity - EDGE_THRESHOLD_FEATHER) / (EDGE_THRESHOLD - EDGE_THRESHOLD_FEATHER);
            FragColor = mix(FragColor, vec4(EDGE_COLOR, 1), amount);
        }
    }
}

float linearize_depth(float depth) {
    float near = 1.0;
    float far = 500.0;
    float linear_depth = (2.0 * near) / (far + near - depth * (far - near));
    return linear_depth;
}

// PALETTE MATCHING

// UTILITY FUNCTIONS
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
