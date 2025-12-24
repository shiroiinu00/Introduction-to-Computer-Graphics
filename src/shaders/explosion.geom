#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT{
    vec3 pos;
    vec3 normal;
    vec2 uv;
} gs_in[];

out GS_OUT{
    vec3 normal;
    vec2 uv;
    float intensity;
    float dist;
} gs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float explode;
uniform float time;

float hash11(float p){
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float hash12(vec2 p){
    vec3 p3  = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

vec3 hash31(float p){
    return vec3(
        hash11(p * 1.7 + 0.1),
        hash11(p * 2.3 + 0.7),
        hash11(p * 3.1 + 1.3)
    );
}

vec3 hash33(vec3 p){
    return vec3(
        hash12(p.xy + p.z),
        hash12(p.yz + p.x),
        hash12(p.zx + p.y)
    );
}

mat3 rotAxisAngle(vec3 a, float ang){
    a = normalize(a);
    float s = sin(ang), c = cos(ang), oc = 1.0 - c;
    return mat3(
        oc*a.x*a.x + c,        oc*a.x*a.y - a.z*s,  oc*a.z*a.x + a.y*s,
        oc*a.x*a.y + a.z*s,    oc*a.y*a.y + c,      oc*a.y*a.z - a.x*s,
        oc*a.z*a.x - a.y*s,    oc*a.y*a.z + a.x*s,  oc*a.z*a.z + c
    );
}

void main(){
    int id = gl_PrimitiveID;
    
    vec3 center = (model * vec4(0,0,0,1)).xyz;
    
    vec3 c = (gs_in[0].pos + gs_in[1].pos + gs_in[2].pos) / 3.0;
    
    vec3 edge1 = gs_in[1].pos - gs_in[0].pos;
    vec3 edge2 = gs_in[2].pos - gs_in[0].pos;
    float triArea = length(cross(edge1, edge2)) * 0.5;
    
    vec3 radial = normalize(c - center);
    
    vec3 faceN = normalize(cross(edge1, edge2));
    
    float seed = float(id);
    vec3 rand1 = hash31(seed * 17.241);
    vec3 rand2 = hash31(seed * 31.873);
    vec3 rand3 = hash31(seed * 53.127);
    vec3 rand4 = hash31(seed * 71.459);
    vec3 rand5 = hash31(seed * 97.631);
    vec3 rand6 = hash33(c + vec3(seed));
    
    float theta = rand1.x * 6.28318;
    float phi = acos(2.0 * rand1.y - 1.0);
    vec3 sphericalRandom = vec3(
        sin(phi) * cos(theta),
        sin(phi) * sin(theta),
        cos(phi)
    );
    
    vec3 randomDir = normalize(rand2 * 2.0 - 1.0);
    
    float mixFactor = explode;
    vec3 dir = normalize(
        radial * (0.3 - mixFactor * 0.25) +
        faceN * (0.2 * (1.0 - mixFactor * 0.5)) +
        sphericalRandom * (0.5 + mixFactor * 1.0) +
        randomDir * (0.3 + mixFactor * 0.8)
    );
    
    vec3 perturbation = (rand6 * 2.0 - 1.0) * (0.4 + explode * 0.6);
    dir = normalize(dir + perturbation);
    
    float delay = rand2.z * 0.4 + rand3.x * 0.2;
    float t = clamp((explode - delay) / (1.0 - delay + 0.01), 0.0, 1.0);
    t = pow(t, 1.5 + rand4.x * 0.8);
    
    float distBase = 15.0 + rand2.y * 50.0;
    float distMult = 0.5 + rand3.y * 2.0;
    float finalDist = distBase * distMult;
    
    float expParam = 4.0 + rand4.y * 4.0;
    float travel = finalDist * (1.0 - exp(-expParam * t));
    
    vec3 rotAxis = normalize(rand3 * 2.0 - 1.0);
    float rotSpeed = 5.0 + rand3.z * 25.0;
    
    float baseRotation = rotSpeed * t * exp(-t * 5.0); 
    float timeOscillation = sin(time * (8.0 + rand4.z * 12.0)) * (0.5 + t * 0.5) * exp(-t * 3.0);
    float rotAngle = baseRotation + timeOscillation;
    
    mat3 R = rotAxisAngle(rotAxis, rotAngle);
    
    vec3 timeJitter = hash33(vec3(seed, time * 10.0, explode));
    float jitterAmp = (0.3 + rand5.x * 0.8) * exp(-t * 10.0);
    vec3 jitter = (timeJitter * 2.0 - 1.0) * jitterAmp * finalDist * 0.15;
    
    float baseScale = 0.3 + rand4.x * 1.4;
    
    float areaFactor = clamp(triArea * 100.0, 0.1, 2.0);
    baseScale *= (0.7 + areaFactor * 0.3);
    
    float timeScale = 1.0 + 0.3 * sin(t * 3.14159 + rand5.y * 6.28) * (1.0 - t * 0.5);
    
    float shrinkGrow = 1.0;
    if (rand5.z > 0.5) {
        shrinkGrow = 1.0 - t * (0.3 + rand6.x * 0.4);
    } else {
        shrinkGrow = 1.0 + t * (0.2 + rand6.y * 0.3);
    }
    
    float finalScale = baseScale * timeScale * shrinkGrow;
    
    for(int i = 0; i < 3; i++){
        vec3 p = gs_in[i].pos - c;
        
        p *= finalScale;
        p = R * p;
        
        vec3 newWorld = c + p + dir * travel + jitter;
        
        float distFromCenter = distance(newWorld, center);
        
        gl_Position = projection * view * vec4(newWorld, 1.0);
        
        gs_out.normal = normalize(R * gs_in[i].normal);
        gs_out.uv = gs_in[i].uv;
        gs_out.intensity = 1.0 - t;
        gs_out.dist = distFromCenter;
        
        EmitVertex();
    }
    EndPrimitive();
}