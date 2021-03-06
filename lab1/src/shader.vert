#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 1) in vec3 aColor;

uniform mat4 MVP;
uniform mat4 MV;

varying vec3 v_Position;
varying vec4 v_Color;
varying vec3 v_Normal;

void main()
{
    v_Position = vec3(MV * vec4(aPos, 1.0));
    v_Color = vec4(aColor, 1.0); //vec4(normalize(MVP * vec4(aPos*100, 1.0)).xyz, 1.0);
    v_Normal = vec3(MV * vec4(aNormal, 1.0));

    gl_Position = MVP * vec4(aPos, 1.0);
}
