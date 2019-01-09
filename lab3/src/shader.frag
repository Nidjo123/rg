#version 330 core

// uniforms
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;

#define MAX_STEPS 128
#define SURFACE_DIST 0.000001
#define MAX_DIST 100.0

float getDist(vec3 p)
{
    vec4 sphere = vec4(cos(iTime), 1.5+0.5*sin(iTime), 6.0+sin(iTime)*cos(iTime), 1);
    float dS = length(p-sphere.xyz)-sphere.w;
    float dP = min(p.y, 10.0-p.z);
    dP = min(dP, 4.0-abs(p.x));
    float d = min(dS, dP);
    return d;
}

vec2 raymarch(vec3 ro, vec3 rd)
{
    float dO = 0.0;
    float mind = 1.0;
    for (int i = 0; i < MAX_STEPS; i++) {
    	vec3 p = ro + dO * rd;
        float dS = getDist(p);
        dO += dS;
        mind = min(mind, dS);
        if (dS < SURFACE_DIST || dO > MAX_DIST)
            break;
    }
    return vec2(dO, mind);
}

vec3 getNormal(vec3 p)
{
    float d = getDist(p);
    vec2 e = vec2(.005, 0);

    vec3 n = d - vec3(
        getDist(p-e.xyy),
        getDist(p-e.yxy),
        getDist(p-e.yyx));

    return normalize(n);
}

float getLight(vec3 p, vec3 eye)
{
    vec3 lightPos = vec3(2*iMouse.x/iResolution.x-1.0, 0.5+5*(iResolution.y-iMouse.y)/iResolution.y, 4);
    vec3 l = normalize(lightPos-p);
    vec3 n = getNormal(p);

    vec3 ep = normalize(eye-p);

    float diffuse = clamp(0.1+0.6*dot(l, n)+0.1*pow(dot(ep, l), 50), 0., 1.);

    vec2 res = raymarch(p+0.1*l, l);
    float d = res.x;
    vec2 lRes = raymarch(lightPos-0.01*l, -l);
    if (d < length(lightPos-p))
        diffuse *= 0.2;
    else if (lRes.y > 0.1)
        diffuse *= 0.5*lRes.y;

    return diffuse*0.9;
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

    float diffuse = getLight(p, ro);

    col = vec3(diffuse, diffuse, diffuse);

    gl_FragColor = vec4(col, 1.);
}
