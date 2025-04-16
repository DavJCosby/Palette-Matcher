#version 410 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ScreenTexture;
uniform sampler2D DepthTexture;

const float EDGE_THRESHOLD = 0.003;
const float EDGE_THRESHOLD_FEATHER = 0.00225;
const vec3 EDGE_COLOR = vec3(0.1, 0.0, 0.2);

const float STROKE_RADIUS = 0.55;

float linearize_depth(float depth) {
    float near = 1.0;
    float far = 500.0;
    float linear_depth = (2.0 * near) / (far + near - depth * (far - near));
    return linear_depth;
}

vec4 apply_edges(vec4 base_color) {
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
        return vec4(EDGE_COLOR, 1);
    } else {
        if (edge_intensity > EDGE_THRESHOLD_FEATHER) {
            float amount = (edge_intensity - EDGE_THRESHOLD_FEATHER) / (EDGE_THRESHOLD - EDGE_THRESHOLD_FEATHER);
            return mix(base_color, vec4(EDGE_COLOR, 1), amount);
        } else {
            return base_color;
        }
    }
}

void main() {
    vec4 base_color = texture(ScreenTexture, TexCoord);
    FragColor = apply_edges(base_color);
}
