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

    vec3 color = vec3(1.0, 0.35, 0.15) * flicker;

    FragColor = vec4(color, alpha);
}