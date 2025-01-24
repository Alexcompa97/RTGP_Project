#version 330 core
out vec4 FragColor;
in vec3 FragPos;

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

void main() {
    // Wood grain parameters
    float scale = 15.0;
    vec2 pos = FragPos.xy * scale;  // Use xy for front and back walls
    
    // Create wood grain layers
    float grain = noise(pos);
    grain += 0.5 * noise(pos * 3.0);
    grain += 0.25 * noise(pos * 6.0);
    
    // Create wood rings with natural variation
    float ringFreq = 3.0;
    float distortion = grain * 0.4;
    float rings = sin(pos.x * ringFreq + distortion * 8.0) * 0.5 + 0.5;
    rings = pow(rings, 1.5);
    
    // Add fine grain detail
    float detail = noise(pos * 12.0) * 0.1;
    rings = mix(rings, detail, 0.15);
    
    // Warm wood colors
    vec3 lightWood = vec3(0.85, 0.55, 0.27);    // Light oak color
    vec3 darkWood = vec3(0.45, 0.25, 0.12);     // Dark oak color
    vec3 midWood = vec3(0.65, 0.40, 0.20);      // Medium oak color
    
    // Create final wood color with three-way mix
    vec3 woodColor = mix(darkWood, midWood, rings);
    woodColor = mix(woodColor, lightWood, grain * 0.5);
    
    // Add subtle variation
    woodColor += vec3(noise(pos * 24.0) * 0.03);
    
    FragColor = vec4(woodColor, 1.0);
} 