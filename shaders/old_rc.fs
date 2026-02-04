#version 430
layout(location = 0) out vec4 color_;
precision highp float;

in vec2 uv;

// Resolution of the frame
uniform float width;
uniform float height;

// Cascade level
// Level with the highest linear resolution and
// the lowest angular resolution is the first level.
uniform float cascade_level;
uniform float cascade_count;
// Amount of rays at the first level.
// Each next level has two times more.
uniform float base_ray_count;

// Max amount of steps when marching along a single ray.
uniform int step_count;
// How close to get to the object to register collision.
uniform float proximity_epsilon;

// The start and the end of the ray.
uniform float interval_start;
uniform float interval_end;

// Textures
uniform sampler2D color_texture;
uniform sampler2D sdf_texture;
uniform sampler2D previous_frame;

float pi = 3.141592;

float whole = pi * 2.0;

bool uv_out_of_bounds(vec2 sample_uv) {
    return sample_uv.x < 0.0 || sample_uv.x > 1.0 || sample_uv.y < 0.0 || sample_uv.y > 1.0;
}

void main()
{
    // IMPORTANT !!!
    // All variables prefixed with u_ are for the upper cascade.

    vec2 resolution = vec2(width, height);
    vec2 coord = uv * resolution;
    float interval_length = interval_end - interval_start;

    float sqrt_base_ray_count = sqrt(base_ray_count);
    // Number of segments in each axis
    float subdivisions = pow(sqrt_base_ray_count, cascade_level - 1);
    float u_subdivisions = subdivisions * sqrt_base_ray_count;
    // Resolution of a single segment
    vec2 level_resolution = floor(resolution / subdivisions);
    vec2 u_level_resolution = floor(resolution / u_subdivisions);
    // Starting coords of the segment
    vec2 root_coords = floor(coord / level_resolution);
    // Coords withing a segment, relative to the root
    vec2 root_relative_coords = mod(coord, level_resolution);
    // UVs of the segment
    vec2 level_uv = root_relative_coords / level_resolution;

    // For each subdivisions shoot base_ray_count rays.
    // This rau count is equal to the number of segments.
    // Each segment covers some angle.
    float ray_count = subdivisions * subdivisions;
    float u_ray_count = u_subdivisions * u_subdivisions;

    float one_over_ray_count = 1.0 / ray_count;
    float one_over_base_ray_count = 1.0 / base_ray_count;
    // Angle step for those rays
    float ray_angle_step = whole * one_over_ray_count;
    float u_ray_angle_step = whole * (1.0 / u_ray_count);
    // Base ray index that stems out of the segment's position
    float base_ray_index = (root_coords.y * subdivisions + root_coords.x);

    vec4 result_color = vec4(0.0, 0.0, 0.0, 0.0);

    // Each segment covers some angle. However, this is achieved
    // via subdividing this angle into base_ray_count separate rays.
    for (float i = 0.0; i < 1.0; i += one_over_base_ray_count) {
        // Each segment calculates light in a different direction.
        // Each segment calculates light in a direction rotated by angle_step_size.
        float ray_index = i + base_ray_index;
        // Current angle
        float ray_angle = (ray_index + one_over_base_ray_count * 0.5) * ray_angle_step;
        //color_ = vec4(ray_angle / whole, 0.0, 0.0, 1.0);
        //return;
        // Direction based on angle
        vec2 ray_direction = normalize(vec2(cos(ray_angle), -sin(ray_angle)));
        //color_ = vec4(ray_direction, 0.0, 1.0);
        //return;
        // Ray starts in some distance from the probe
        vec2 sample_uv = level_uv + interval_start * ray_direction;

        if (uv_out_of_bounds(sample_uv)) {
            continue;
        }

        // Ray can only travel certain distance.
        float max_travel_distance = length(sample_uv + ray_direction * interval_end);
        float traveled = 0;
        vec4 radiance_delta = vec4(0.0, 0.0, 0.0, 0.0);

        // Raymarching
        bool hit = false;
        bool out_of_bounds = false;

        for (int step = 0; step < step_count; step++) {
            float current_distance = texture(sdf_texture, sample_uv).r;
            traveled += current_distance;
            sample_uv += current_distance * ray_direction;

            if (uv_out_of_bounds(sample_uv)) {
                out_of_bounds = true;
                break;
            }

            // Ray can travel only certain distance.
            if (traveled > interval_length) {
                break;
            }

            if (current_distance < proximity_epsilon) {
                radiance_delta += texture(color_texture, sample_uv);
                hit = true;
                break;
            }
        }

        //if (cascade_level != cascade_count && !out_of_bounds && radiance_delta.a == 0.0) {
        //    float upper_segment = (ray_angle / whole) * u_ray_count;
        //    float segment_x = mod(upper_segment, u_subdivisions);
        //    float segment_y = floor(upper_segment / u_subdivisions);

        //    float segment_x_n = segment_x / u_subdivisions;
        //    float segment_y_n = segment_y / u_subdivisions;
        //    vec2 upper_cascade_start_uv = vec2(segment_x_n, segment_y_n);

        //    vec2 upper_level_sample = upper_cascade_start_uv + sample_uv;

        //    if (!uv_out_of_bounds(upper_level_sample)) {
        //        radiance_delta += texture(previous_frame, upper_level_sample);
        //    }
        //}

        {
            float upper_segment = (ray_angle / whole) * u_ray_count;
            float segment_x = mod(upper_segment, u_subdivisions);
            float segment_y = floor(upper_segment / u_subdivisions);

            float segment_x_n = segment_x / u_subdivisions;
            float segment_y_n = segment_y / u_subdivisions;
            vec2 upper_cascade_start_uv = vec2(segment_x_n, segment_y_n);

            // 4. Return previous frame sampled with those UVs
            if (cascade_level == cascade_count) {
                radiance_delta += vec4(uv, 0.0, 1.0);
            } else {
                // radiance_delta += vec4(segment_x_n, segment_y_n, 0.0, 1.0);
                //radiance_delta += texture(previous_frame, upper_cascade_start_uv + sample_uv);
                radiance_delta += vec4(upper_cascade_start_uv + level_uv, 0.0, 1.0);
            }
        }

        result_color += radiance_delta;
    }

    // Normalize color by dividing it with the number of base rays.
    // It was subdivided into the same number in the loop.
    result_color *= one_over_base_ray_count;

    // Return
    color_ = result_color;
}
