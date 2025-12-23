#version 330 core
in GS_OUT {
    vec3 normal;
    vec2 uv;
    float heat;
}fs_in;
out vec4 FragColor;
uniform sampler2D outTexture;

uniform float time;
uniform bool isExplosion;

vec3 fireRamp(float h){
    vec3 red = vec3(1.0, 0.15, 0.05);
    vec3 orange = vec3(1.0, 0.55, 0.05);
    vec3 yellow = vec3(1.0, 0.95, 0.40);

    vec3 a = mix(red, orange, smoothstep(0.0, 0.6, h));
    vec3 b = mix(orange, yellow, smoothstep(0.6, 1.0, h));
    return mix(a, b, step(0.6, h));
}

void main(){

    if(isExplosion){
        vec3 albedo = texture(outTexture, fs_in.uv).rgb;

        float h = clamp(fs_in.heat, 0.0, 1.0);

        float flicker = 0.85 + 0.15 * sin(time * 20.0 + fs_in.uv.x * 40.0);

        vec3 fire = fireRamp(h) * flicker;

        float emissive = pow(h, 1.5) * 1.8;

        vec3 color = mix(albedo, fire, 0.75) + fire * emissive;
        color += fire * (0.2 * h);

        FragColor = vec4(color, 1.0);
    }else{
        FragColor = vec4(texture(outTexture, fs_in.uv).rgb, 1.0);
    }

    

}