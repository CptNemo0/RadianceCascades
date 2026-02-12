#version 330 core

out vec4 frag_color;
in vec2 uv;

uniform float time;
uniform float speed;
uniform float size;
uniform sampler2D u_noise_texture;

uniform float percentage;

const float texture_subdivision = 4.0;
const vec2 center = vec2(0.5, 0.5);

const vec3 dark_red = vec3(0.3, 0.0, 0.0);
const vec3 orange = vec3(1.0, 0.6, 0.0);
const vec3 yellow = vec3(1.0, 0.95, 0.25);
const vec3 white = vec3(1.0, 1.0, 1.0);

const float flame_height = 0.1;

void main() {
    float half_percentage = percentage * 0.5;
    vec2 diff = abs(vec2(0.5, 0.5) - uv);

    if (diff.x > half_percentage || diff.y > half_percentage) {
        frag_color = vec4(0.0);
        return;
    }

    vec2 remapped_uvs = vec2((uv.x - (0.5 - half_percentage)) / percentage, (uv.y - (0.5 - half_percentage)) / percentage);

    diff = vec2(0.5, 0.5) - remapped_uvs + vec2(0.0, flame_height);

    float y_intensity = (1.0 / (0.7 * remapped_uvs.y + 0.5)) - 1.0;
    float x_intensity_top = -1.2 * pow(abs(remapped_uvs.x - 0.5), 1.2) + 1;
    float x_intensity_bottom = -1.2 * pow(abs(remapped_uvs.x - 0.5), 0.7) + 1;
    float x_intensity = min(x_intensity_top, x_intensity_bottom);
    float distance_intensity = (1.0 - dot(diff, diff) * 2.5);
    float noise_intensity = texture(u_noise_texture, remapped_uvs + vec2(0.0, -time * speed)).r;

    float intensity = x_intensity * 0.6 * 2.5;
    intensity *= y_intensity * 0.6 * 2.5;
    intensity *= distance_intensity * 0.6 * 2.5;
    intensity *= noise_intensity * 0.2 * 2.5;

    if (intensity < 0.2) {
        frag_color = vec4(0.0);
        return;
    }

    intensity *= 3.0f;
    intensity = pow(intensity, 2.0);

    if (intensity >= 0.625) {
        vec3 color = mix(yellow, white, (intensity - 0.675) / 0.375);
        frag_color = vec4(color, 1.0);
        return;
    }

    if (intensity >= 0.25) {
        vec3 color = mix(orange, yellow, (intensity - 0.25) / 0.5);
        frag_color = vec4(color, 1.0);
        return;
    }

    if (intensity >= 0.15) {
        vec3 color = mix(dark_red, orange, intensity / 0.25);
        frag_color = vec4(color, 1.0);
        return;
    }

    frag_color = vec4(1.0, 1.0, 1.0, 1.0);
    return;
}
