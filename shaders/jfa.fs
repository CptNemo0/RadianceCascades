#version 430
layout(location = 0) out vec4 color_;
precision highp float;

in vec2 uv;

uniform sampler2D uv_colorspace_texture;

uniform float width;
uniform float height;

uniform float one_over_width;
uniform float one_over_height;

uniform int step_size;

void main()
{
    ivec2 start_pixel_position = ivec2(gl_FragCoord.x, gl_FragCoord.y);

    float nearest_distance = 99999.9;
    vec2 nearest_seed = vec2(-9999.0, -9999.0);

    vec2 current_seed = texelFetch(uv_colorspace_texture, start_pixel_position, 0).xy;

    for (int x = -1; x < 2; x++) {
        for (int y = -1; y < 2; y++) {
            ivec2 current_position = ivec2(start_pixel_position.x + step_size * x, start_pixel_position.y + step_size * y);
            vec2 current_uv = vec2(current_position.x * one_over_width, current_position.y * one_over_height);

            if (current_uv.x < 0.0 || current_uv.x > 1.0 || current_uv.y < 0.0 || current_uv.y > 1.0) {
                continue;
            }

            current_seed = texelFetch(uv_colorspace_texture, current_position, 0).xy;

            if (current_seed.x > 0.0 || current_seed.y > 0.0) {
                vec2 diff = current_seed - uv;
                float current_distance = dot(diff, diff);

                if (nearest_distance > current_distance) {
                    nearest_distance = current_distance;
                    nearest_seed = current_seed;
                }
            }
        }
    }

    color_ = vec4(nearest_seed, 0.0, 1.0);
}
