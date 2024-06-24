#version 450

precision highp float;

layout(binding = 1) uniform sampler2D texSamplery;
layout(binding = 2) uniform sampler2D texSampleru;
layout(binding = 3) uniform sampler2D texSamplerv;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {

	vec3 rgb;
	rgb.r = texture(texSamplery, fragTexCoord).r;
	rgb.g = texture(texSampleru, fragTexCoord).r;
	rgb.b = texture(texSamplerv, fragTexCoord).r;    

	outColor = vec4(rgb, 1);
}
