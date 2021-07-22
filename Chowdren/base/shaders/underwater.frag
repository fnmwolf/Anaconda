#version 120

uniform sampler2D texture;
varying vec2 texture_coordinate;
uniform float fBlur;
uniform float fAmplitudeX;
uniform float fPeriodsX;
uniform float fFreqX;
uniform float fAmplitudeY;
uniform float fPeriodsY;
uniform float fFreqY;

void main()
{
    vec2 i = texture_coordinate;
    i.y = i.y + (sin((i.x+fFreqX)*fPeriodsX)*fAmplitudeX);// + 1.0f)%1.0f;
    //SiY
    i.x = i.x + (sin((i.y+fFreqY)*fPeriodsY)*fAmplitudeY);// + 1.0f)%1.0f;

    // Output pixel
    vec4 texTL;
    vec4 texBL;
    vec4 texBR;
    vec4 texTR;
    texTL = texture2D(texture, vec2(i.x-fBlur, i.y-fBlur));
    texBL = texture2D(texture, vec2(i.x-fBlur, i.y+fBlur));
    texBR = texture2D(texture, vec2(i.x+fBlur, i.y+fBlur));
    texTR = texture2D(texture, vec2(i.x+fBlur, i.y-fBlur));

    vec4 col = texture2D(texture, i);
    col = (col+ texTL + texBL + texBR + texTR)/5.0;
    gl_FragColor = col * gl_Color;
}