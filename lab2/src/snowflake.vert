#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 MVP;
uniform mat4 MV;

//out vec3 v_Position;
out vec2 TexCoord;

void main()
{
    TexCoord = aTexCoord;
    gl_Position = MVP * vec4(aPos, 1.0);
}
