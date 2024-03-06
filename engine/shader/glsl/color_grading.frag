#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput in_color;

layout(set = 0, binding = 1) uniform sampler2D color_grading_lut_texture_sampler;

layout(location = 0) out highp vec4 out_color;

void main()
{
    highp ivec2 lut_tex_size = textureSize(color_grading_lut_texture_sampler, 0);
	highp float lenX 		= float(lut_tex_size.x);
    highp float lenY      = float(lut_tex_size.y);
    highp vec4 color       = subpassLoad(in_color).rgba;

	highp float block1 		= floor(color.b * lenY);
	highp float block2 		= block1 + 1.0;

	highp float u 			= ((block1 + color.r) * lenY) / lenX;
	highp float v 			= color.g;
	
	highp vec4 color1 = texture(color_grading_lut_texture_sampler, vec2(u, v));
	
	u = ((block2 + color.r) * lenY) / lenX;
	highp vec4 color2 = texture(color_grading_lut_texture_sampler, vec2(u, v));

	highp float blend = fract(color.b * lenY);
	out_color = mix(color1, color2, blend);
}
