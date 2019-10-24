#version 330 core

in V_OUT_ {
    vec2 mUV;
} v_out_;

uniform sampler2D texture_main;

out vec4 color_;

void main()
{
    color_ = texture(texture_main, v_out_.mUV);
}