#version 430
layout(location = 0) out vec4 color_;

in vec2 uv;
uniform sampler2D drawing_texture;

void main()
{
    color_ = texture(drawing_texture, uv);
}
