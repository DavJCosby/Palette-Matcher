#version 410 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ScreenTexture;
uniform sampler2D DepthTexture;
uniform int TogglePalette;
uniform float Dither;

const float EDGE_THRESHOLD = 0.003;
const float EDGE_THRESHOLD_FEATHER = 0.00225;
const vec3 EDGE_COLOR = vec3(0, 0, 0);
const float STROKE_RADIUS = 0.55;

// On program startup, a palette text file is provided
// and is parsed into a constant that is placed below.

const vec3[] PALETTE = vec3[](
	vec3(0.591267, 0.007236, -0.031458),
	vec3(0.661239, 0.003510, -0.051761),
	vec3(0.707165, 0.006057, -0.065566),
	vec3(0.763112, 0.003352, -0.057338),
	vec3(0.790501, 0.008863, -0.047014),
	vec3(0.825614, 0.017907, -0.038741),
	vec3(0.851626, 0.014828, -0.031222),
	vec3(0.914045, 0.024313, -0.016000),
	vec3(0.939287, 0.028037, 0.008229),
	vec3(0.973370, 0.007475, 0.014161),
	vec3(0.997624, 0.000327, -0.001112)
);


const int BAYER_N = 4;
const int BAYER_N_SQ = BAYER_N * BAYER_N;
const int BAYER_MATRIX[BAYER_N_SQ] = int[BAYER_N_SQ](
        0, 8, 2, 10,
        12, 4, 14, 6,
        3, 11, 1, 9,
        15, 7, 13, 5
    );

// FORWARD DECLARATIONS - OUTLINES
void apply_edges();
float linearize_depth(float depth);

// FORWARD DECLARATIONS - UTILITY FUNCTIONS
vec3 oklab_from_rgb(vec3 rgb);
vec3 rgb_from_oklab(vec3 oklab);

// FORWARD DECLARATIONS - PALETTE MATCHING
vec3 closest_candiate(vec3 target) {
    vec3 closest;
    float dist_of_closest = 100000000.0;

    for (int i = 0; i < PALETTE.length(); i++) {
        vec3 color = PALETTE[i];
        vec3 delta = color - target;
        float d = dot(delta, delta); // magnitude squared
        if (d < dist_of_closest) {
            dist_of_closest = d;
            closest = color;
        }
    }
    return closest;
}

void lock_to_palette() {
    ivec2 pixel_coordinate = ivec2(gl_FragCoord.xy);
    vec3 original_color = FragColor.rgb;
    vec3 original_oklab = oklab_from_rgb(FragColor.rgb);

    vec3 error = vec3(0, 0, 0);
    vec3[BAYER_N_SQ] candidates;

    for (int j = 0; j < BAYER_N_SQ; j++) {
        vec3 sample_c = original_oklab + error * Dither;
        vec3 candidate = closest_candiate(sample_c);
        candidates[j] = candidate;
        error += (original_color - candidate);
    }

    // sort by ascending lightness
    for (int i = 0; i < BAYER_N_SQ - 1; i++) {
        for (int j = 0; j < BAYER_N_SQ - i - 1; j++) {
            if (candidates[j].x > candidates[j + 1].x) {
                vec3 temp = candidates[j];
                candidates[j] = candidates[j + 1];
                candidates[j + 1] = temp;
            }
        }
    }

    int index_row = pixel_coordinate.x % BAYER_N;
    int index_col = pixel_coordinate.y % BAYER_N;
    int index = (index_row * BAYER_N) + index_col;

    vec3 closest_match_rgb = rgb_from_oklab(candidates[BAYER_MATRIX[index]]);

    FragColor = vec4(closest_match_rgb, 1);
}

void main() {
    FragColor = texture(ScreenTexture, TexCoord);
    apply_edges();
    if (TogglePalette == 1) {
        lock_to_palette();
    }
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
