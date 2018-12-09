#version 330 core

in vec3 v_Position;
in vec2 TexCoord;

uniform sampler2D tex;

void main()
{
    gl_FragColor = texture(tex, TexCoord);
    if (gl_FragColor.rgb == vec3(0, 0, 0))
       discard;
}
