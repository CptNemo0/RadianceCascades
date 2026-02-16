#version 430
layout(location = 0) out vec2 color_;

in vec2 uv;

uniform sampler2D drawing_texture;

void main()
{
    vec4 current_color = texture(drawing_texture, uv);
    color_ = (current_color.a > 0.0) ? vec2(uv) : vec2(0.0, 0.0);
}
