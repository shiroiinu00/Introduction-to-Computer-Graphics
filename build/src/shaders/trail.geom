#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices=4) out;

in VS_OUT{
    vec3 worldPos;
} gs_in[];

out GS_OUT{
    float vAge;   // 0 (新) -> 1 (舊)
} gs_out;

uniform mat4 view;
uniform mat4 projection;
uniform int pointCount;

void main(){
    // 將兩點轉到 view space
    vec3 v0 = (view * vec4(gs_in[0].worldPos, 1.0)).xyz;
    vec3 v1 = (view * vec4(gs_in[1].worldPos, 1.0)).xyz;

    vec3 dir = normalize(v1 - v0);

    // view space 下相機朝 -Z，看向前方；用 z 軸當參考做 billboard right
    vec3 viewAxis = vec3(0.0, 0.0, 1.0);
    vec3 right = cross(dir, viewAxis);
    float rlen = length(right);
    if (rlen < 1e-5) right = vec3(1.0, 0.0, 0.0);
    else right /= rlen;

    // 依 segment index 算 age（pointCount=N -> segments = N-1）
    float seg = float(gl_PrimitiveIDIn);
    float maxSeg = max(float(pointCount - 2), 1.0); // 最後一段的 index 約 N-2
    float age0 = seg / maxSeg;
    float age1 = (seg + 1.0) / maxSeg;

    // 粗細：新比較粗，舊比較細
    float w0 = mix(1.0, 0.5, age0); // 你可以調
    float w1 = mix(1.0, 0.5, age1);

    vec3 o0 = right * w0;
    vec3 o1 = right * w1;

    vec4 P0L = projection * vec4(v0 - o0, 1.0);
    vec4 P0R = projection * vec4(v0 + o0, 1.0);
    vec4 P1L = projection * vec4(v1 - o1, 1.0);
    vec4 P1R = projection * vec4(v1 + o1, 1.0);

    gs_out.vAge = age0; gl_Position = P0L; EmitVertex();
    gs_out.vAge = age0; gl_Position = P0R; EmitVertex();
    gs_out.vAge = age1; gl_Position = P1L; EmitVertex();
    gs_out.vAge = age1; gl_Position = P1R; EmitVertex();
    EndPrimitive();
}
