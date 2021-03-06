#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 MVP;
uniform vec2 iResolution;
uniform float iTime;

void main()
{
    gl_Position = MVP * vec4(aPos, 0.0, 1.0);
}
