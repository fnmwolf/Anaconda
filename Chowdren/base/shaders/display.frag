#version 120

varying vec2 texture_coordinate;
uniform float fPeriods;
uniform float fOffset;
uniform float fAmplitude;
uniform sampler2D texture;

void main()
{
    vec4 c = texture2D(texture, texture_coordinate);
    c.rgb += sin(texture_coordinate.y * fPeriods - fOffset) * fAmplitude;
    gl_FragColor = c;
}
