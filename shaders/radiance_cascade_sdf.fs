#version 430

out vec4 color_;

in vec2 uv;

// Const
uniform vec2 resolution;

// Rarely changing
uniform float cascade_count;
uniform float base_ray_count;
uniform int step_count;
uniform float proximity_epsilon;

uniform int cascade_index;
uniform bool base_level;
uniform float overlap;

// Textures
uniform sampler2D color_texture;
uniform sampler2D sdf_texture;
uniform sampler2D upper_cascade_texture;

// Real const
const float srgb = 2.1;
const float whole = 3.141592 * 2.0;

bool out_of_bounds(vec2 sample_uv) {
    return sample_uv.x < 0.0 || sample_uv.y < 0.0 || sample_uv.x > 1.0 || sample_uv.y > 1.0;
}

void main() {
    vec4 this_pixel_color = texture(color_texture, uv);
    vec2 coord = floor(uv * resolution);

    float spacing_base = sqrt(base_ray_count);
    float spacing = pow(spacing_base, cascade_index);

    vec2 probes_per_dimension = resolution / spacing;
    vec2 probe_coord = mod(coord, probes_per_dimension);
    vec2 probe_center = (probe_coord + 0.5) * spacing;
    vec2 probe_uv = probe_center / resolution;

    float interval_start = base_level ? 0.0 : (pow(base_ray_count, (cascade_index - 1)) / resolution.x); // !
    float interval_end = ((
        (1.0 + 3.0 * overlap) * pow(base_ray_count, cascade_index) - pow(cascade_index, 2.0)
        ) / resolution.x); // !
    float interval_length = interval_end - interval_start;

    // Multiply by base_ray_count to further subdivide.
    // This trick spreads out the underlying indices.
    float ray_count = pow(base_ray_count, cascade_index + 1.0);
    float angle_step = whole / ray_count;
    vec2 ray_id = floor(coord / probes_per_dimension);
    float base_ray_index = float(base_ray_count) * (ray_id.x + (spacing * ray_id.y));

    vec4 radiance = vec4(0.0);

    for (float i = 0.0; i < base_ray_count; i += 1) {
        float index = base_ray_index + i;
        float angleStep = index + 0.5;
        float angle = angle_step * angleStep;
        vec2 direction = vec2(cos(angle), -sin(angle));

        vec2 sample_uv = probe_uv + interval_start * direction;
        float current_distance = texture(sdf_texture, sample_uv).r;

        if (out_of_bounds(sample_uv)) {
            continue;
        }

        vec4 radiance_from_ray = vec4(0.0);
        float traveled = 0.0;

        for (int step = 0; step < step_count; step++) {
            sample_uv += direction * current_distance;

            if (out_of_bounds(sample_uv)) {
                break;
            }

            traveled += current_distance;
            if (traveled > interval_length) {
                break;
            }

            if (current_distance < proximity_epsilon) {
                radiance_from_ray += texture(color_texture, sample_uv);
                break;
            }
            current_distance = texture(sdf_texture, sample_uv).r;
        }

        if (cascade_index + 1 != cascade_count && radiance_from_ray.a == 0.0) {
            float upper_spacing = pow(spacing_base, cascade_index + 1.0);
            vec2 upper_size = floor(resolution / upper_spacing);
            vec2 upper_position = vec2(
                    mod(index, upper_spacing), floor(index / upper_spacing)
                ) * upper_size;

            vec2 offset = (probe_coord + 0.5) / spacing_base;
            vec2 clamped = clamp(offset, vec2(0.5), upper_size - 0.5);

            vec4 upper_sample = texture(
                    upper_cascade_texture,
                    (upper_position + clamped) / resolution
                );

            radiance_from_ray += vec4(upper_sample.rgb, upper_sample.a);
        }

        radiance += radiance_from_ray;
    }

    color_ = vec4(radiance.rgb / float(base_ray_count), 1.0);
}
