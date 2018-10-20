#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

uniform mat4 MVP;

out vec4 vertexColor; // specify a color output to the fragment shader

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);
    vertexColor = normalize(MVP * vec4(aPos*100, 1.0)); // set the output variable to a dark-red color
}
