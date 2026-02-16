#version 430
layout(location = 0) out float color_;

in vec2 uv;
uniform sampler2D jfa_texture;
uniform float diameter;

void main()
{
    color_ = distance(texture(jfa_texture, uv).rg, uv);
}
