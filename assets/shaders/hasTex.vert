#version 450 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

layout(location = 3) in mat4 iModel;
layout(location = 7) in vec4 iColor;
layout(location = 8) in vec4 iTexParam;

uniform mat4 V;
uniform mat4 P;

uniform mat2 uRotMtx;
uniform vec2 uMcn;
uniform vec2 uTexOffSet;
uniform vec2 uTexScale;

out vec4 vColor;
out vec2 vTexCoord;

void main()
{
    //gl_Position = P * V * M * vec4(aPosition, 0.0, 1.0);
    //gl_Position = vec4(aPosition, 0, 1.0);
    gl_Position = P * V * iModel * vec4(aPosition, 0.0, 1.0);
    vColor = iColor;

    //vTex = uRotMtx * (aTexPos - uMcn) + uMcn;
    //vTex = aTexPos * uTexScale + uTexOffSet;
    vTexCoord = aTexCoord * iTexParam.zw + iTexParam.xy;
}
