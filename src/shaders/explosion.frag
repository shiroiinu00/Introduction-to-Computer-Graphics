#version 330 core
in GS_OUT {
    vec3 normal;
    vec2 uv;
    float intensity; // 燃燒強度 (1.0 -> 0.0)
    float dist;      // 距離爆心的距離
} fs_in;

out vec4 FragColor;

uniform sampler2D outTexture;
uniform float time; // 添加時間參數用於動態效果

// 顏色混合函數
vec3 lerpColor(vec3 a, vec3 b, float t) {
    return a * (1.0 - t) + b * t;
}

// 噪聲函數 - 用於火焰效果
float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

// 簡單柏林噪聲
float perlinNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    // 四個角點的隨機值
    float a = noise(i);
    float b = noise(i + vec2(1.0, 0.0));
    float c = noise(i + vec2(0.0, 1.0));
    float d = noise(i + vec2(1.0, 1.0));
    
    // 平滑插值
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

void main() {
    // 原始紋理顏色
    vec4 texColor = texture(outTexture, fs_in.uv);
    
    // 燃燒顏色梯度：中心白熱 -> 橙紅 -> 黑煙
    vec3 centerColor = vec3(1.0, 0.9, 0.6);    // 白黃色（最熱）
    vec3 middleColor = vec3(1.0, 0.4, 0.1);    // 橙紅色
    vec3 edgeColor = vec3(0.3, 0.1, 0.05);     // 暗紅色/黑色
    
    // 根據距離和強度計算顏色權重
    float distFactor = clamp(fs_in.dist / 50.0, 0.0, 1.0);
    float heat = fs_in.intensity * (1.0 - distFactor * 0.7);
    
    // 火焰動態效果
    vec2 noiseCoord = fs_in.uv * 5.0 + vec2(time * 2.0, time * 1.5);
    float flameNoise = perlinNoise(noiseCoord) * 0.3;
    
    // 調整熱度值
    heat = clamp(heat + flameNoise, 0.0, 1.0);
    
    // 根據熱度混合顏色
    vec3 fireColor;
    if (heat > 0.7) {
        // 高溫區域：白熱到橙紅
        float t = (heat - 0.7) / 0.3;
        fireColor = lerpColor(middleColor, centerColor, t);
    } else if (heat > 0.3) {
        // 中溫區域：橙紅到暗紅
        float t = (heat - 0.3) / 0.4;
        fireColor = lerpColor(edgeColor, middleColor, t);
    } else {
        // 低溫區域：暗紅到黑色
        float t = heat / 0.3;
        fireColor = lerpColor(vec3(0.05, 0.02, 0.01), edgeColor, t);
    }
    
    // 根據距離增加煙霧效果
    float smoke = clamp(distFactor * 2.0 - 0.5, 0.0, 1.0);
    smoke *= (1.0 - fs_in.intensity); // 後期更多煙霧
    
    // 煙霧顏色
    vec3 smokeColor = vec3(0.2, 0.2, 0.2);
    
    // 最終顏色混合：火焰 + 煙霧 + 原始紋理
    float fireAmount = fs_in.intensity;
    float texAmount = (1.0 - fireAmount) * 0.3; // 保留少量原始紋理
    
    vec3 finalColor = fireColor * fireAmount + 
                     smokeColor * smoke + 
                     texColor.rgb * texAmount;
    
    // 邊緣暗化效果
    float edgeDarken = 1.0 - smoothstep(0.7, 1.0, length(fs_in.uv - vec2(0.5)) * 2.0);
    finalColor *= edgeDarken;
    
    // 根據強度調整透明度
    float alpha = clamp(texColor.a * (0.5 + fireAmount * 0.5), 0.1, 1.0);
    
    FragColor = vec4(finalColor, alpha);
    
    // 可選：添加發光效果（用於後處理）
    FragColor.rgb += fireColor * fireAmount * 0.3;
}