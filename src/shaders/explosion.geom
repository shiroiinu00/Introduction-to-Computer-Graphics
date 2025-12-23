#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT{
    vec3 pos;     // world
    vec3 normal;  // world
    vec2 uv;
} gs_in[];

out GS_OUT{
    vec3 normal;
    vec2 uv;
} gs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float explode; // 0~1
uniform float time;

// ----- hash -----
float hash11(float p){
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}
vec3 hash31(float p){
    return vec3(hash11(p+1.1), hash11(p+2.2), hash11(p+3.3));
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

    // 爆心：模型原點 (0,0,0) 經 model 變到 world
    vec3 center = (model * vec4(0,0,0,1)).xyz;

    // triangle centroid (world)
    vec3 c = (gs_in[0].pos + gs_in[1].pos + gs_in[2].pos) / 3.0;

    // 爆炸方向：從爆心指向這片碎片
    vec3 radial = normalize(c - center);

    // face normal (world)
    vec3 faceN = normalize(cross(gs_in[1].pos - gs_in[0].pos, gs_in[2].pos - gs_in[0].pos));

    // random per-tri
    vec3 r = hash31(float(id) * 17.0);
    vec3 randDir = normalize(r * 2.0 - 1.0);

    vec3 dir = normalize(mix(radial, normalize(faceN + 0.35 * randDir), 0.35));

    float delay = r.x * 0.18; // 0~0.18
    float t = clamp((explode - delay) / (1.0 - delay), 0.0, 1.0);

    float base = mix(8.0, 12.0, r.y);          // can adjust the distance
    float travel = base * (1.0 - exp(-4.0*t)); // 0..base

    vec3 axis = normalize(hash31(float(id) * 91.0) * 2.0 - 1.0);
    float ang = (5.0 + 10.0 * r.z) * t
              + 0.6 * sin(time * (8.0 + 6.0*r.y)) * t;

    mat3 R = rotAxisAngle(axis, ang);

    float jitterAmp = mix(0.03, 0.005, t);
    vec3 jitter = (hash31(float(id)*13.0 + time*10.0) * 2.0 - 1.0) * jitterAmp;

    for(int i=0;i<3;i++){
        vec3 p = gs_in[i].pos - c;     // pivot at centroid
        p = R * p;

        vec3 newWorld = c + p + dir * travel + jitter;

        gl_Position = projection * view * vec4(newWorld, 1.0);

        gs_out.normal = normalize(R * gs_in[i].normal); 
        gs_out.uv = gs_in[i].uv;

        EmitVertex();
    }
    EndPrimitive();
}
