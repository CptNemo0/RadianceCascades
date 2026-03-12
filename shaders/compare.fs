#version 430
layout(location = 0) out vec4 color_;

in vec2 uv;
uniform sampler2D ground_truth;
uniform sampler2D prediction;

uniform float remap;

void main()
{
    vec4 diff = texture(ground_truth, uv).rgba - texture(prediction, uv);
    float d2 = dot(diff, diff) / remap;
    color_ = vec4(d2, d2, d2, 1.0);
}
