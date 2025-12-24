#version 330 core
out vec4 FragColor;

in GS_OUT {
    vec2 fUV;
    float strength;
} g;

uniform float uTime;

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
    float x = g.fUV.x * 2.0 - 1.0;   // -1..1
    float y = g.fUV.y;              // 0..1

    float core = 1.0 - abs(x) * (0.9 + 0.6*y); 
    float n = noise(vec2(x*3.0, y*6.0) + vec2(0.0, uTime*3.0));
    float rag = smoothstep(0.2, 0.9, core + (n-0.5)*0.6);

    float flick = 0.6 + 0.4 * noise(vec2(y*5.0, uTime*6.0));
    float alpha = rag * (1.0 - y) * flick * g.strength;

    vec3 col = mix(vec3(2.0, 1.2, 0.2), vec3(1.2, 0.2, 0.05), y);

    FragColor = vec4(col, alpha);
}
