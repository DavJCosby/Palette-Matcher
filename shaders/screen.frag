#version 410 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main() {
    vec4 base_color = texture(screenTexture, TexCoord);
    FragColor = vec4(base_color.rgb, 1);
}
