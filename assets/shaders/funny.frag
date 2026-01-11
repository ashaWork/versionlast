#version 450 core

in vec3 vColor;
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D uTex2d;
uniform bool uHasTex;

void main()
{
    vec4 texColor1 = texture(uTex2d, vTex);
    FragColor = uHasTex? texColor1 : vec4(vColor, 1.0);
}
