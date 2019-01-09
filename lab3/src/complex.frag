#version 330 core

// uniforms
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;

#define MAX_STEPS 128
#define SURFACE_DIST 0.000001
#define MAX_DIST 100.0

float displacement(vec3 p)
{
    return sin(3*p.x)*cos(3*p.y);
}

float sdSphere( vec3 p, float s )
{
  return length(p)-s;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return length(max(d,0.0))
         + min(max(d.x,max(d.y,d.z)),0.0); // remove this line for an only partially signed sdf
}

float opDisplace(float d1, vec3 p)
{
    float d2 = displacement(p);
    return d1+d2;
}

float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float getDist(vec3 p)
{
    float dS = opDisplace(sdTorus(p, vec2(1, 0.9)), p);
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
    vec3 lightPos = vec3(2*iMouse.x/iResolution.x-1.0, 0.5+5*(iResolution.y-iMouse.y)/iResolution.y, 4*sin(iTime/2));
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

    vec3 ro = vec3(0, 1, -5);
    vec3 rd = normalize(vec3(uv.xy, 1));

    vec2 res = raymarch(ro, rd);
    float d = res.x;

    vec3 p = ro + rd*d;

    float diffuse = getLight(p, ro);

    col = vec3(diffuse, diffuse, diffuse);

    if (p.y > 0.01 && p.z < 9.99 && abs(p.x)<3.99)
       col *= vec3(0.8, 0.5, 0.2);

    gl_FragColor = vec4(col, 1.);
}
