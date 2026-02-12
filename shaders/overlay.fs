#version 430
layout(location = 0) out vec4 color_;

in vec2 uv;

uniform sampler2D texture_1;
uniform sampler2D texture_2;

uniform vec2 offset_1;
uniform vec2 offset_2;

bool out_of_bounds(vec2 uvs) {
    return uvs.x < 0.0 || uvs.x > 1.0 || uvs.y < 0.0 || uvs.y > 1.0;
}

void main()
{
    vec2 uv1 = uv + offset_1;
    vec2 uv2 = uv + offset_2;

    vec4 color1 = texture(texture_1, uv1);
    vec4 color2 = texture(texture_2, uv2);

    color_ = color1.a < 0.9 ? color2 : color1;
}
