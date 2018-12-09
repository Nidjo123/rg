#version 330 core

varying vec3 v_Position;
varying vec4 v_Color;
varying vec3 v_Normal;

void main()
{
    vec3 lightPos = vec3(0.0, 0.0, 0.0);

    float distance = length(lightPos - v_Position)/10.0;
    vec3 lightVector = normalize(lightPos - v_Position);

    float diffuse = max(dot(v_Normal, lightVector), 0.5);

    diffuse = diffuse * (1.0 / (1.0 + (0.25 * distance * distance)));

    gl_FragColor = v_Color;
}
