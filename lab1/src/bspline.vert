#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

uniform mat4 MVP;
uniform vec3 color;

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);
}
