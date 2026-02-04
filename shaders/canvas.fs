#version 430
out vec4 color_;

in vec2 uv;

uniform sampler2D drawing_texture;

uniform float width;
uniform float height;

uniform vec3 brush_color;
uniform vec2 position;
uniform float brush_radius;

void main()
{
    vec2 coords = vec2(gl_FragCoord.x, gl_FragCoord.y);
    vec2 diff = position - coords;
    float dist = dot(diff, diff);

    if (dist < (brush_radius * brush_radius + 1)) {
        color_ = vec4(brush_color, 1.0);
        return;
    }

    vec4 color = texture(drawing_texture, uv);
    if (color.a > 0.0) {
        color_ = color;
        return;
    }

    color_ = vec4(0.0, 0.0, 0.0, 0.0);
}
