#version 450 core

in vec4 vColor;
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTex2d;

void main()
{
    vec4 texColor1 = texture(uTex2d, vTexCoord);
    if(texColor1.a <= 0.01)
    {
        discard;
    }
    FragColor = texColor1;
	//FragColor = vec4(1.0,0.0,1.0,1.0);
}
