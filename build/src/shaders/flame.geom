#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices=4) out;

in VS_OUT {
    vec3 worldPos;
    vec2 uv;
} gs_in[];

uniform mat4 view;
uniform mat4 projection;

uniform float uTime;
uniform float uBurn;     // 0..1
uniform vec3  uUp;       // (0,1,0)

out GS_OUT {
    vec2 fUV;        // quad UV
    float strength;  // 0..1
} gs_out;

float hash1(float n){ return fract(sin(n)*43758.5453); }

float hash12(vec2 p){
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}
float noise(vec2 p){
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash12(i);
    float b = hash12(i + vec2(1,0));
    float c = hash12(i + vec2(0,1));
    float d = hash12(i + vec2(1,1));
    vec2 u = f*f*(3.0-2.0*f);
    return mix(mix(a,b,u.x), mix(c,d,u.x), u.y);
}

void main(){
    // triangle center (world)
    vec3 c = (gs_in[0].worldPos + gs_in[1].worldPos + gs_in[2].worldPos) / 3.0;
    vec2 uv = (gs_in[0].uv + gs_in[1].uv + gs_in[2].uv) / 3.0;

    float n = noise(uv * 8.0 + vec2(0.0, uTime*0.25));
    float w = 0.08;
    float band = 1.0 - smoothstep(0.0, 1.0, abs(n - uBurn) / w);

    float r = hash1(float(gl_PrimitiveIDIn) * 12.9898);

    float width  = 3.0;
    float height = 10.0;

    float sway = (noise(uv*20.0 + uTime*2.0) - 0.5) * 0.8;
    vec3 side = normalize(cross(uUp, vec3(0.0,0.0,1.0)));
    if(length(side) < 0.01) side = vec3(1,0,0);

    vec3 p0 = c + side * (-width) + uUp * 0.00;
    vec3 p1 = c + side * ( width) + uUp * 0.00;
    vec3 p2 = c + side * (-width * 0.4) + uUp * (height) + side * sway;
    vec3 p3 = c + side * ( width * 0.4) + uUp * (height) + side * sway;

    gs_out.strength = 1.0;

    gs_out.fUV = vec2(0,0);
    gl_Position = projection * view * vec4(p0, 1.0);
    EmitVertex();

    gs_out.fUV = vec2(1,0);
    gl_Position = projection * view * vec4(p1, 1.0);
    EmitVertex();

    gs_out.fUV = vec2(0,1);
    gl_Position = projection * view * vec4(p2, 1.0);
    EmitVertex();

    gs_out.fUV = vec2(1,1);
    gl_Position = projection * view * vec4(p3, 1.0);
    EmitVertex();

    EndPrimitive();
}
