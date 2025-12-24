#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 worldPos;
    vec3 worldNormal;
    vec2 uv;
} f;

uniform sampler2D diffuseTex;
uniform float uTime;
uniform float uBurn;   // 0 -> 1

float hash12(vec2 p){
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

// 2D value noise
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
    vec3 baseCol = texture(diffuseTex, f.uv).rgb;

    float n = noise(f.uv * 8.0 + vec2(0.0, uTime*0.25));

    // burn threshold
    float w = 0.08;                         // 邊界寬度
    float edge = smoothstep(uBurn - w, uBurn + w, n);   // 0=未燒，1=已燒
    float alive = 1.0 - edge;

    if(alive < 0.02) discard;

    float charAmt = smoothstep(0.0, 0.9, edge);
    vec3 charCol = mix(baseCol, vec3(0.05), charAmt);

    float rim = 1.0 - smoothstep(0.0, 0.6, abs(n - uBurn) / w);
    float flicker = 0.6 + 0.4 * noise(f.uv*20.0 + uTime*2.0);
    vec3 glow = rim * flicker * vec3(1.8, 0.7, 0.15);   // 偏橘黃

    FragColor = vec4(charCol + glow, 1.0);
}
