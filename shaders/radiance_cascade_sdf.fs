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

bool out_of_bounds(vec2 uv) {
    return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
}

void main() {
    vec2 coord = floor(uv * resolution);

    float spacing_base = sqrt(base_ray_count);
    float spacing = pow(spacing_base, cascade_index);

    vec2 probes_per_dimension = resolution / spacing;
    vec2 probe_coord = mod(coord, probes_per_dimension);
    vec2 probe_center = (probe_coord + 0.5) * spacing;
    vec2 probe_uv = probe_center / resolution;

    float interval_start = base_level ? 0.0 : (pow(base_ray_count, cascade_index)) / resolution.x; // !
    float interval_length = pow(base_ray_count, cascade_index + 1) / resolution.x; // !
    interval_length *= (1 + overlap);

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

        if (out_of_bounds(sample_uv)) {
            continue;
        }

        vec4 radiance_from_ray = vec4(0.0);
        float traveled = 0.0;

        for (int step = 1; step < step_count; step++) {
            float current_distance = texture(sdf_texture, sample_uv).r;
            traveled += current_distance;
            sample_uv += direction * current_distance;

            if (out_of_bounds(sample_uv)) break;

            if (traveled >= interval_length) {
                break;
            }

            if (current_distance <= proximity_epsilon) {
                radiance_from_ray += texture(color_texture, sample_uv);
                break;
            }
        }

        if (cascade_index + 1 != cascade_count && radiance_from_ray.a == 0.0) {
            float upperSpacing = pow(spacing_base, cascade_index + 1.0);
            vec2 upperSize = floor(resolution / upperSpacing);
            vec2 upperPosition = vec2(
                    mod(index, upperSpacing), floor(index / upperSpacing)
                ) * upperSize;

            vec2 offset = (probe_coord + 0.5) / spacing_base;
            vec2 clamped = clamp(offset, vec2(0.5), upperSize - 0.5);

            vec4 upperSample = texture(
                    upper_cascade_texture,
                    (upperPosition + clamped) / resolution
                );

            radiance_from_ray += vec4(upperSample.rgb, upperSample.a);
        }

        radiance += radiance_from_ray;
    }

    color_ = vec4(radiance.rgb / float(base_ray_count), 1.0);
}
