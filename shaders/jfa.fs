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

const ivec2 directions[8] = ivec2[](
        ivec2(-1, -1), ivec2(0, -1), ivec2(1, -1),
        ivec2(-1, 0), ivec2(1, 0),
        ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1)
    );

void main()
{
    ivec2 start_pixel_position = ivec2(gl_FragCoord.x, gl_FragCoord.y);
    ivec2 texture_size = textureSize(uv_colorspace_texture, 0);
    vec2 current_seed = texelFetch(uv_colorspace_texture, start_pixel_position, 0).xy;

    float nearest_distance = 99999.9;
    vec2 nearest_seed = current_seed;

    if (nearest_seed.x > 0.0 || nearest_seed.y > 0.0) {
        vec2 diff = nearest_seed - uv;
        nearest_distance = dot(diff, diff);
    }

    for (int i = 0; i < 8; i++) {
        ivec2 current_position = start_pixel_position + step_size * directions[i];
        current_position = clamp(current_position, ivec2(0.0), texture_size - 1);

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

    color_ = vec4(nearest_seed, 0.0, 1.0);
}
