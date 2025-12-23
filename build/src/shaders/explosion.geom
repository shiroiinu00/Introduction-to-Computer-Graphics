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
    float intensity; // 燃燒強度
    float dist;      // 距離爆心的距離
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

    // 隨機數生成器 - 為每個三角形生成獨特的隨機值
    vec3 rand1 = hash31(float(id) * 17.0);      // 用於方向
    vec3 rand2 = hash31(float(id) * 31.0 + 7.7); // 用於距離和延遲
    vec3 rand3 = hash31(float(id) * 53.0 + 13.1); // 用於旋轉
    
    // 增強隨機方向：混合徑向、法線和完全隨機方向
    vec3 randomDir = normalize(rand1 * 2.0 - 1.0);
    vec3 faceNormalDir = normalize(faceN + 0.3 * randomDir);
    
    // 權重調整：前期更多徑向，後期更多隨機
    float radialWeight = mix(0.8, 0.1, explode);
    vec3 dir = normalize(mix(radial, faceNormalDir, radialWeight));
    
    // 加入額外的隨機性到方向
    dir = normalize(dir + 0.5 * randomDir);
    
    // 隨機延遲 - 讓碎片在不同時間開始爆炸
    float delay = rand2.x * 0.25; // 增加到 0.25 秒延遲範圍
    
    // 爆炸時間計算 - 使用指數曲線使後期加速
    float t = clamp((explode - delay) / (1.0 - delay), 0.0, 1.0);
    t = pow(t, 1.5); // 使運動更戲劇化
    
    // 隨機基礎距離 - 讓碎片飛得更遠且距離不一
    float minDistance = 10.0; // 最小距離
    float maxDistance = 40.0; // 最大距離
    float baseDistance = minDistance + (maxDistance - minDistance) * rand2.y;
    
    // 距離計算 - 使用更強的曲線
    float travel = baseDistance * (1.0 - exp(-6.0 * t));
    
    // 為碎片添加隨機旋轉
    vec3 rotAxis = normalize(rand3 * 2.0 - 1.0);
    float rotationSpeed = 5.0 + 15.0 * rand3.z; // 隨機旋轉速度
    float rotAngle = rotationSpeed * t + 0.8 * sin(time * (10.0 + 8.0 * rand2.x)) * t;
    
    mat3 R = rotAxisAngle(rotAxis, rotAngle);
    
    // 隨機抖動 - 模擬空氣阻力和不穩定性
    float jitterAmp = mix(0.05, 0.2, t); // 後期抖動更大
    vec3 jitter = (hash31(float(id)*29.0 + time*12.0) * 2.0 - 1.0) * jitterAmp;
    
    // 為每個頂點計算最終位置
    for(int i = 0; i < 3; i++){
        // 相對質心的位置
        vec3 p = gs_in[i].pos - c;
        
        // 應用旋轉
        p = R * p;
        
        // 計算最終世界座標：原始質心 + 旋轉後位置 + 爆炸位移 + 隨機抖動
        vec3 newWorld = c + p + dir * travel + jitter;
        
        // 計算距離爆心的距離（用於顏色）
        float distFromCenter = distance(newWorld, center);
        
        gl_Position = projection * view * vec4(newWorld, 1.0);

        // 傳遞數據到片段著色器
        gs_out.normal = normalize(R * gs_in[i].normal);
        gs_out.uv = gs_in[i].uv;
        gs_out.intensity = 1.0 - t; // 強度隨時間減弱
        gs_out.dist = distFromCenter; // 距離資訊

        EmitVertex();
    }
    EndPrimitive();
}