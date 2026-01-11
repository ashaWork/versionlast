#version 450 core

in vec4 vColor;
out vec4 FragColor;


void main()
{
    FragColor = vColor;
	//FragColor = vec4(1.0,0.0,1.0,1.0);
}
