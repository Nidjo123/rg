#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 MVP;

out vec3 v_Position;
out vec4 v_Color;

void main()
{
    v_Position = aPos;
    v_Color = vec4(aColor, 1.0);

    gl_Position = MVP * vec4(aPos, 1.0);
}
