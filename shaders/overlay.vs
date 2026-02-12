#version 430
layout(location = 0) in vec3 position_;
layout(location = 1) in vec2 uv_;

out vec2 uv;

void main()
{
    gl_Position = vec4(position_, 1.0);
    uv = uv_;
}
