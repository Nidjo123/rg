#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 MVP;
uniform vec2 iResolution;
uniform float iTime;

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);
}
