#version 330 core

// uniforms
uniform vec2 iResolution;
uniform float iTime;

#define MAX_STEPS 128
#define SURFACE_DIST 0.000001
#define MAX_DIST 100.0

float getDist(vec3 p)
{
    vec4 sphere = vec4(cos(iTime), 1.5+0.5*sin(iTime), 6.0+sin(iTime)*cos(iTime), 1);
    float dS = length(p-sphere.xyz)-sphere.w;
    float dP = p.y;
    float d = min(dS, dP);
    return d;
}

vec2 raymarch(vec3 ro, vec3 rd)
{
    float dO = 0.0;
    float mind = MAX_DIST;
    for (int i = 0; i < MAX_STEPS; i++) {
    	vec3 p = ro + dO * rd;
        float dS = getDist(p);
        dO += dS;
        mind = min(mind, 16.*dO);
        if (dS < SURFACE_DIST || dO > MAX_DIST)
            break;
    }
    return vec2(dO, mind);
}

vec3 getNormal(vec3 p)
{
    float d = getDist(p);
    vec2 e = vec2(.01, 0);

    vec3 n = d - vec3(
        getDist(p-e.xyy),
        getDist(p-e.yxy),
        getDist(p-e.yyx));

    return normalize(n);
}

float getLight(vec3 p)
{
    vec3 lightPos = vec3(0, 5, 6);
    //vec3 lightPos = vec3(iMouse.x/iResolution.x, 5.*iMouse.y/iResolution.y, 6);
    vec3 l = normalize(lightPos-p);
    vec3 n = getNormal(p);

    float diffuse = clamp(dot(l, n), 0., 1.);

    vec2 res = raymarch(p+0.1*l, l);
    float d = res.x;
    if (d < length(lightPos-p))
        diffuse *= 0.15*d;

    return diffuse;
}

void main()
{

    vec2 uv = (gl_FragCoord.xy-0.5*iResolution.xy)/iResolution.y;

    vec3 col = vec3(0);

    vec3 ro = vec3(0, 1, 0);
    vec3 rd = normalize(vec3(uv.xy, 1));

    vec2 res = raymarch(ro, rd);
    float d = res.x;

    vec3 p = ro + rd*d;

    float diffuse = getLight(p);

    col = vec3(diffuse, diffuse, diffuse);

    gl_FragColor = vec4(col, 1.);
}
