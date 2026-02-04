#version 430
layout(location = 0) out vec4 color_;
precision highp float;

in vec2 uv;

uniform int step_count;
uniform float proximity_epsilon;

uniform int ray_count;
uniform float one_over_ray_count;
uniform float angle_step;

uniform float width;
uniform float height;

uniform sampler2D color_texture;
uniform sampler2D sdf_texture;
uniform sampler2D previous_frame;
uniform sampler2D noise_texture;

uniform float time;

float pi = 3.141592;
float whole = pi * 2.0;
float emission_threshold = 1.0;

bool out_of_bounds(vec2 sample_uv) {
    return sample_uv.x < 0.0 || sample_uv.x > 1.0 || sample_uv.y < 0.0 || sample_uv.y > 1.0;
}

void main()
{
    vec4 this_pixel_color = texture(color_texture, uv);
    vec2 angle_jitter = texture(noise_texture, uv + vec2(time, time)).xy;
    vec4 result_color = this_pixel_color;

    if (this_pixel_color.a > 0.2) {
        color_ = this_pixel_color;
        return;
    }

    bool hit = false;
    for (float ray_angle = 0; ray_angle < whole; ray_angle += angle_step) {
        vec2 direction = vec2(cos(ray_angle), -sin(ray_angle));
        direction = normalize(mix(angle_jitter, direction, 0.5));
        vec2 sample_uv = uv;

        for (int i = 1; i < step_count; i++) {
            float current_distance = texture(sdf_texture, sample_uv).r;
            sample_uv += current_distance * direction;

            if (out_of_bounds(sample_uv)) {
                break;
            }

            if (current_distance < proximity_epsilon) {
                result_color += texture(color_texture, sample_uv);
                hit = true;
                break;
            }
        }
    }

    result_color *= one_over_ray_count;
    color_ = mix(texture(previous_frame, uv), result_color, 0.1);
}
