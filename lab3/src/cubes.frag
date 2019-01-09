#version 330 core

// uniforms
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;

#define MAX_STEPS 128
#define SURFACE_DIST 0.00001
#define MAX_DIST 20.0

float cube(vec3 p)
{
    float side = 0.2;
    vec3 b = vec3(side);

    vec3 d = abs(p)-floor(abs(p)) - b;
    return length(max(d,0.0))
           + min(max(d.x,max(d.y,d.z)),0.0);
}

float getDist(vec3 p)
{
    // cubes are at integer coordinates

    // repeat period
    vec3 c = vec3(1);
    vec3 q = mod(p,c)-0.5*c;
    // repeat cubes
    return cube(q);
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
    vec2 e = vec2(.001, 0);

    vec3 n = d - vec3(
        getDist(p-e.xyy),
        getDist(p-e.yxy),
        getDist(p-e.yyx));

    return normalize(n);
}

float getLight(vec3 p, vec3 eye)
{
    vec3 lightPos = vec3(2*iMouse.x/iResolution.x-1, 5*(iResolution.y-iMouse.y)/iResolution.y-2.5, 0)+eye;
    vec3 lp = lightPos-p;
    vec3 l = normalize(lp);
    vec3 n = getNormal(p);

    vec3 ep = normalize(eye-p);

    float diffuse = clamp(0.1+0.6*dot(l, n)+0.1*pow(dot(ep, l), 60), 0., 1.);

    vec2 res = raymarch(p+0.01*l, l);
    float d = res.x;
    if (d < length(lightPos-p))
        diffuse *= 0.2;

    float lDist = length(lp);
    return diffuse/(lDist*lDist*0.2);
}

void main()
{

    vec2 uv = (gl_FragCoord.xy-0.5*iResolution.xy)/iResolution.y;

    vec3 col = vec3(0);

    vec3 ro = vec3(0.2*cos(iTime), 3*sin(iTime/2), sin(iTime));
    vec3 rd = normalize(vec3(uv.xy, 1));

    vec2 res = raymarch(ro, rd);
    float d = res.x;

    vec3 p = ro + rd*d;

    float diffuse = getLight(p, ro);

    col = vec3(diffuse, diffuse, diffuse)*mod(p, vec3(3,2,3));

    gl_FragColor = vec4(col, 1.);
}
