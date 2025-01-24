#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;

// Hash function for cellular noise
vec3 hash3(vec3 p) {
    p = vec3(dot(p,vec3(127.1,311.7, 74.7)),
             dot(p,vec3(269.5,183.3,246.1)),
             dot(p,vec3(113.5,271.9,124.6)));
    return -1.0 + 2.0 * fract(sin(p)*43758.5453123);
}

// Cellular (Worley) noise
float cellular(vec3 p) {
    vec3 i_p = floor(p);
    vec3 f_p = fract(p);
    
    float min_dist = 1.0;
    
    // Search neighboring cells
    for(int k=-1; k<=1; k++)
    for(int j=-1; j<=1; j++)
    for(int i=-1; i<=1; i++) {
        vec3 neighbor = vec3(float(i), float(j), float(k));
        vec3 point = hash3(i_p + neighbor);
        point = 0.5 + 0.5 * sin(point * 6.2831853); // Animate points
        vec3 diff = neighbor + point - f_p;
        float dist = length(diff);
        min_dist = min(min_dist, dist);
    }
    
    return min_dist;
}

// Enhanced cellular noise with multiple layers
float enhancedCellular(vec3 p, float scale, float intensity) {
    float n = cellular(p * scale);
    float n2 = cellular(p * scale * 2.0 + 5.0);
    
    // Mix different frequencies
    return mix(n, n2, intensity);
}

uniform float noiseScale;
uniform float noiseIntensity;
uniform vec3 baseColor1;
uniform vec3 baseColor2;
uniform float edgeThreshold;
uniform float glowStrength;

void main() {
    // Generate cellular noise
    float noise = enhancedCellular(FragPos, noiseScale, noiseIntensity);
    
    // Create cell edges effect
    float edge = 1.0 - smoothstep(0.0, edgeThreshold, noise);
    
    // Mix colors based on noise
    vec3 cellColor = mix(baseColor1, baseColor2, noise);
    vec3 edgeColor = vec3(1.0); // White edges
    vec3 finalColor = mix(cellColor, edgeColor, edge);
    
    // Basic lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Ambient
    vec3 ambient = 0.3 * finalColor;
    
    // Diffuse
    vec3 diffuse = diff * finalColor;
    
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.5 * spec * vec3(1.0);
    
    // Add glow to edges
    vec3 glow = edge * glowStrength * baseColor2;
    
    vec3 result = ambient + diffuse + specular + glow;
    FragColor = vec4(result, 1.0);
} 