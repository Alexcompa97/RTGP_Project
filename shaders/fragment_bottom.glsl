#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

uniform vec3 lightPos = vec3(0.0, 10.0, 0.0);

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 ip = floor(p);
    vec2 u = fract(p);
    u = u * u * (3.0 - 2.0 * u);
    
    float res = mix(
        mix(rand(ip), rand(ip+vec2(1.0,0.0)), u.x),
        mix(rand(ip+vec2(0.0,1.0)), rand(ip+vec2(1.0,1.0)), u.x), u.y);
    return res * res;
}

vec3 calculateNormal(vec2 pos, float scale) {
    float eps = 0.01;
    float nx = (noise(pos + vec2(eps, 0.0)) - noise(pos - vec2(eps, 0.0))) / (2.0 * eps);
    float ny = (noise(pos + vec2(0.0, eps)) - noise(pos - vec2(0.0, eps))) / (2.0 * eps);
    
    return normalize(vec3(nx * scale, 1.0, ny * scale));
}

void main() {
    float scale = 1.5;
    vec2 pos = FragPos.xz * scale;
    
    float n = noise(pos);
    n += 0.5 * noise(pos * 2.0);
    n += 0.25 * noise(pos * 4.0);
    n += 0.125 * noise(pos * 8.0);
    n = n / (1.0 + 0.5 + 0.25 + 0.125);
    
    float marble = abs(sin(pos.x + pos.y + 6.0 * n));
    
    // Brighter base colors
    vec3 baseColor = vec3(0.98, 0.96, 0.93);
    vec3 veinColor = vec3(0.60, 0.57, 0.55);
    
    vec3 albedo = mix(veinColor, baseColor, marble);
    
    vec3 baseNormal = vec3(0.0, 1.0, 0.0);
    vec3 normalOffset = calculateNormal(pos, 0,15);
    vec3 normal = normalize(TBN * mix(baseNormal, normalOffset, 0,4));
    
    vec3 lightDir = normalize(vec3(0.0, -1.0, 0.0));
    vec3 viewDir = normalize(-FragPos);
    
    // Increased ambient light
    float ambientStrength = 0.7;
    vec3 ambient = ambientStrength * albedo;
    
    // Increased diffuse intensity
    float diff = max(dot(-lightDir, normal), 0.0) * 1.3; // Multiplied by 1.3 for more intensity
    vec3 diffuse = diff * albedo;
    
    // Adjusted specular
    float specularStrength = 0.4;
    vec3 halfwayDir = normalize(-lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * vec3(1.0);
    
    // Combine with increased overall brightness
    vec3 finalColor = (ambient + diffuse + specular) * 1.2;
    
    // Ensure we don't exceed maximum brightness
    finalColor = min(finalColor, vec3(1.0));
    
    FragColor = vec4(finalColor, 1.0);
}