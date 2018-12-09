#version 330 core

in vec3 v_Position;
in vec4 v_Color;

void main()
{
    gl_FragColor = v_Color;
}
