#version 330 core
in vec4 channelCol;
in vec2 channelTex;

out vec4 outCol;

uniform sampler2D uTex;
uniform bool useTex;

void main()
{
    if (!useTex) {
        outCol = channelCol;
    }
    else {
        outCol = texture(uTex, channelTex);
        // Za crosshair želimo transparentnost
        if (outCol.a < 0.1) {
            discard;
        }
    }
}