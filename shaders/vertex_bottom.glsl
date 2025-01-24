#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out mat3 TBN;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Calculate normal matrix
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * vec3(0.0, 1.0, 0.0));
    
    // Calculate tangent space matrix
    vec3 T = normalize(normalMatrix * vec3(1.0, 0.0, 0.0));
    vec3 N = Normal;
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
} 