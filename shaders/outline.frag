#version 410 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

const float EDGE_THRESHOLD = 0.0015;
const vec3 EDGE_COLOR = vec3(0.0, 0.0, 0.0);

const float STROKE_RADIUS = 0.5;

float depthToUnitRange(float depth) {
    float near = 1.0;
    float far = 500.0;
    float linearDepth = (2.0 * near) / (far + near - depth * (far - near));
    return linearDepth;
}

vec4 applyEdges(vec4 base_color) {
    float pixelSizeX = STROKE_RADIUS / textureSize(screenTexture, 0).x;
    float pixelSizeY = STROKE_RADIUS / textureSize(screenTexture, 0).y;

    // Sample neighboring pixels
    float leftDepth = depthToUnitRange(texture(depthTexture, TexCoord + vec2(-pixelSizeX, 0.0)).r);
    float rightDepth = depthToUnitRange(texture(depthTexture, TexCoord + vec2(pixelSizeX, 0.0)).r);
    float topDepth = depthToUnitRange(texture(depthTexture, TexCoord + vec2(0.0, pixelSizeY)).r);
    float bottomDepth = depthToUnitRange(texture(depthTexture, TexCoord + vec2(0.0, -pixelSizeY)).r);

    // Calculate depth differences (gradient)
    float horizontalDiff = abs(rightDepth - leftDepth);
    float verticalDiff = abs(topDepth - bottomDepth) * 0.5;

    // Combine the differences to get the edge intensity
    float edgeIntensity = max(horizontalDiff, verticalDiff);
    //return vec4(vec3(edgeIntensity), 1);

    // Apply edge detection
    if (edgeIntensity > EDGE_THRESHOLD) {

        // This pixel is on an edge - draw it with the edge color
        return vec4(EDGE_COLOR, 1);
    } else {
        return base_color;
    }
}

void main() {
    vec4 base_color = texture(screenTexture, TexCoord);
    FragColor = applyEdges(base_color);
}
