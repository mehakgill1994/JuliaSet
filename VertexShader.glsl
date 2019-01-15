#version 410

layout (location = 0) in vec2 position;
out vec2 coord;

uniform mat4 matProjection;


void main() 
{
    gl_Position = vec4(position, 0.0f, 1.0f);
    coord = vec2(position.x, position.y);
}