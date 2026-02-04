#version 430
layout(location = 0) out vec4 color_;

in vec2 uv;
uniform sampler2D jfa_texture;
uniform float diameter;

void main()
{
    vec2 seed = texture(jfa_texture, uv).xy;
    float dist = distance(seed, uv);

    color_ = vec4(dist, dist, dist, 1.0);
}
