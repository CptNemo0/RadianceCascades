#version 430
out vec4 color_;

in vec2 uv;

uniform sampler2D drawing_texture;

uniform float width;
uniform float height;

uniform vec3 brush_color;
uniform vec2 position;
uniform float brush_radius;
uniform bool eraser;

void main()
{
    vec2 diff = position - uv;
    float dist = dot(diff, diff);
    vec4 color = texture(drawing_texture, uv);

    if (dist < brush_radius * 0.9) {
        color_ = vec4(brush_color, !eraser);
        return;
    } else {
        color_ = color;
    }
}
