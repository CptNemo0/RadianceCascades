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

const float whole = 3.141592 * 2.0;

bool out_of_bounds(vec2 sample_uv) {
    return sample_uv.x < 0.0 || sample_uv.y < 0.0 || sample_uv.x > 1.0 || sample_uv.y > 1.0;
}

void main() {
    vec2 coord = floor(uv * resolution); // Example (125, 250)

    float spacing_base = sqrt(base_ray_count); // 2
    float spacing = pow(spacing_base, cascade_index); // 32, 16, 8, 4, 2, 1

    // (768x768)
    vec2 probes_per_dimension = resolution / spacing; // 24, 48, 96, 192, 384, 768
    vec2 probe_coord = mod(coord, probes_per_dimension); // (5, 10) | (29, 10) | (29, 58) | (125, 58) | (125, 250) | (125, 250)
    vec2 probe_center = (probe_coord + 0.5) * spacing; // (176, 336) | (472, 168) | (236, 468) | (502, 234) | (251, 501) | (125.5, 250.5)
    vec2 probe_uv = probe_center / resolution; // (0.2291, 0.4375) | (0.6146, 0.2186) | (0.3073, 0.6094)

    float interval_start = base_level ? 0.0 : (pow(base_ray_count, (cascade_index - 1)) / resolution.x); // !
    //0.04166,
    float interval_end = ((
        (1.0 + 3.0 * overlap) * pow(base_ray_count, cascade_index) - pow(cascade_index, 2.0)
        ) / resolution.x); // !
    float interval_length = interval_end - interval_start;

    float base_cascade_length = 2 / resolution.x;
    interval_start = (pow(2.0, cascade_index - 1.0)) * base_cascade_length;
    interval_end = (pow(2.0, cascade_index) * base_cascade_length) * (1.0 + overlap);

    interval_length = (interval_end - interval_start);

    // Multiply by base_ray_count to further subdivide.
    // This trick spreads out the underlying indices.
    float ray_count = pow(base_ray_count, cascade_index + 1.0); // 4096, 1024, 256, 64, 16, 4
    float angle_step = whole / ray_count;
    vec2 ray_id = floor(coord / probes_per_dimension); // (5, 10), (2, 5), (1, 2), (0, 1), (0, 0), (0, 0)
    float base_ray_index = float(base_ray_count) * (ray_id.x + (spacing * ray_id.y));
    // 960, 968, 772, 768, 4, 4

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
            float upper_spacing = pow(spacing_base, cascade_index + 1.0); // 64, 32, 16, 8, 4, 2
            vec2 upper_size = floor(resolution / upper_spacing); // 12, 24, 48, 96, 192, 384
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
