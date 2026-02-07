#version 430

out vec4 color_;

in vec2 uv;

// Const
uniform vec2 resolution;

// Rarely changing
uniform float cascadeCount;
uniform float base_ray_count;
uniform int step_count;
uniform float proximity_epsilon;

uniform int cascade_index;
uniform bool base_level;
uniform float overlap;
uniform float magic;

// Textures
uniform sampler2D sceneTexture;
uniform sampler2D distanceTexture;
uniform sampler2D lastTexture;

// Real const
const float srgb = 2.1;
const float whole = 3.141592 * 2.0;

bool out_of_bounds(vec2 uv) {
    return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
}

void main() {
    vec2 coord = floor(uv * resolution);

    float ray_count = pow(base_ray_count, cascade_index + 1.0);
    float one_over_ray_count = 1.0 / float(ray_count);
    float angle_step = whole * one_over_ray_count;

    float spacing_base = sqrt(base_ray_count);
    float spacing = pow(spacing_base, cascade_index);

    vec2 probes_per_dimension = resolution / spacing;
    vec2 probe_coord = mod(coord, probes_per_dimension);
    vec2 probe_center = (probe_coord + 0.5) * spacing;
    vec2 probe_uv = probe_center / resolution;

    float intervalStart = base_level ? 0.0 : (pow(base_ray_count, cascade_index - 1.0)) / resolution.x; // !
    float intervalLength = pow(base_ray_count, cascade_index) / resolution.x; // !

    // Multiply by base_ray_count to further subdivide.
    // This trick spreads out the underlying indices.
    vec2 ray_id = floor(coord / probes_per_dimension);
    float base_ray_index = float(base_ray_count) * (ray_id.x + (spacing * ray_id.y));

    vec4 radiance = vec4(0.0);
    for (float i = 0.0; i < base_ray_count; i += 1) {
        float index = base_ray_index + i;
        float angleStep = index + 0.5;
        float angle = angle_step * angleStep;
        vec2 rayDirection = vec2(cos(angle), -sin(angle));

        vec2 sampleUv = probe_uv + intervalStart * rayDirection;

        if (out_of_bounds(sampleUv)) {
            continue;
        }

        vec4 radDelta = vec4(0.0);
        float traveled = 0.0;

        // We tested uv already (we know we aren't an object), so skip step 0.
        for (int step = 1; step < step_count; step++) {

            // How far away is the nearest object?
            float dist = texture(distanceTexture, sampleUv).r;

            // Go the direction we're traveling
            sampleUv += rayDirection * dist;

            if (out_of_bounds(sampleUv)) break;

            if (dist <= proximity_epsilon) {
                radDelta += texture(sceneTexture, sampleUv);
                break;
            }

            traveled += dist;
            if (traveled >= intervalLength) break;
        }

        // Only merge on non-opaque areas
        if (cascade_index + 1 != cascadeCount && radDelta.a == 0.0) {
            float upperSpacing = pow(spacing_base, cascade_index + 1.0);
            vec2 upperSize = floor(resolution / upperSpacing);
            vec2 upperPosition = vec2(
                    mod(index, upperSpacing), floor(index / upperSpacing)
                ) * upperSize;

            vec2 offset = (probe_coord + 0.5) / spacing_base;
            vec2 clamped = clamp(offset, vec2(0.5), upperSize - 0.5);

            vec4 upperSample = texture(
                    lastTexture,
                    (upperPosition + clamped) / resolution
                );

            radDelta += vec4(upperSample.rgb, upperSample.a);
        }

        // Accumulate total radiance
        radiance += radDelta;
    }

    color_ = vec4(radiance.rgb / float(base_ray_count), 1.0);
}
