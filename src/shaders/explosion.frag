#version 330 core
in GS_OUT {
    vec3 normal;
    vec2 uv;
    float intensity;
    float dist;
} fs_in;

out vec4 FragColor;

uniform sampler2D outTexture;
uniform float time;

vec3 lerpColor(vec3 a, vec3 b, float t) {
    return a * (1.0 - t) + b * t;
}

float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float perlinNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    float a = noise(i);
    float b = noise(i + vec2(1.0, 0.0));
    float c = noise(i + vec2(0.0, 1.0));
    float d = noise(i + vec2(1.0, 1.0));
    
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

void main() {
    vec4 texColor = texture(outTexture, fs_in.uv);
    
    vec3 centerColor = vec3(1.0, 0.9, 0.6);
    vec3 middleColor = vec3(1.0, 0.4, 0.1);
    vec3 edgeColor = vec3(0.3, 0.1, 0.05);
    
    float distFactor = clamp(fs_in.dist / 50.0, 0.0, 1.0);
    float heat = fs_in.intensity * (1.0 - distFactor * 0.7);
    
    vec2 noiseCoord = fs_in.uv * 5.0 + vec2(time * 2.0, time * 1.5);
    float flameNoise = perlinNoise(noiseCoord) * 0.3;
    
    heat = clamp(heat + flameNoise, 0.0, 1.0);
    
    vec3 fireColor;
    if (heat > 0.7) {
        float t = (heat - 0.7) / 0.3;
        fireColor = lerpColor(middleColor, centerColor, t);
    } else if (heat > 0.3) {
        float t = (heat - 0.3) / 0.4;
        fireColor = lerpColor(edgeColor, middleColor, t);
    } else {
        float t = heat / 0.3;
        fireColor = lerpColor(vec3(0.05, 0.02, 0.01), edgeColor, t);
    }
    
    float smoke = clamp(distFactor * 2.0 - 0.5, 0.0, 1.0);
    smoke *= (1.0 - fs_in.intensity);
    
    vec3 smokeColor = vec3(0.2, 0.2, 0.2);
    
    float fireAmount = fs_in.intensity;
    float texAmount = (1.0 - fireAmount) * 0.3;
    
    vec3 finalColor = fireColor * fireAmount +
                     smokeColor * smoke +
                     texColor.rgb * texAmount;
    
    float edgeDarken = 1.0 - smoothstep(0.7, 1.0, length(fs_in.uv - vec2(0.5)) * 2.0);
    finalColor *= edgeDarken;
    
    float alpha = clamp(texColor.a * (0.5 + fireAmount * 0.5), 0.1, 1.0);
    
    FragColor = vec4(finalColor, alpha);
    
    FragColor.rgb += fireColor * fireAmount * 0.3;
}
