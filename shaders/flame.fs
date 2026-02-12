#version 330 core

out vec4 frag_color;
in vec2 uv;

uniform float time;
uniform float speed;
uniform float size;
uniform sampler2D u_noise_texture;

void main() {
    if (distance(uv, vec2(0.5, 0.5)) > size) {
        frag_color = vec4(0.0);
        return;
    }

    vec2 noise_uv = uv + vec2(0.0, -time * speed);
    float color_heat = texture(u_noise_texture, noise_uv).r;

    vec3 col = vec3(0.0);
    vec3 blue_base = vec3(0.0, 0.3, 0.9);
    vec3 dark_red = vec3(0.6, 0.0, 0.0);
    vec3 orange = vec3(1.0, 0.4, 0.0);
    vec3 yellow = vec3(1.0, 0.9, 0.2);
    vec3 white = vec3(1.0, 1.0, 1.0);

    col = mix(dark_red, orange, color_heat);

    frag_color = vec4(col, 1.0);
}
