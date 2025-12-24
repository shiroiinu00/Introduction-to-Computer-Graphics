#version 330 core



out vec4 FragColor;
in GS_OUT {
    float vAge;
} fs_in;



uniform float time;

void main(){
    float alpha = (1.0 - fs_in.vAge);
    alpha = alpha * alpha;
    
    float flicker = 0.85 + 0.15 * sin(time * 18.0);
    
    float r = 0.5 + 0.5 * sin(time * 1.5);
    float g = 0.5 + 0.5 * sin(time * 2.0 + 1.0);
    float b = 0.5 + 0.5 * sin(time * 2.5 + 2.0);
    
    vec3 baseColor = vec3(r, g, b);
    
    float brightness = 1.0 - fs_in.vAge * 0.5;
    
    FragColor = vec4(baseColor * brightness * flicker, alpha);
}