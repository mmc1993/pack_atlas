#version 330 core

layout(location = 0) in vec2 a_v_;
layout(location = 1) in vec2 a_uv_;

out V_OUT_ {
    vec2 mUV;
} v_out_;

uniform mat4 matrix_proj;
uniform mat4 matrix_view;
uniform mat4 matrix_model;

void main()
{
    v_out_.mUV   = a_uv_;
    // gl_Position = vec4(a_v_, 0, 1);
    gl_Position  = matrix_proj 
                 * matrix_view 
                 * matrix_model
                 * vec4(a_v_, 0, 1);
}