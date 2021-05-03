#version 120

attribute vec3 v_position;
attribute vec3 v_normal;
attribute vec2 v_texCoord;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)

uniform mat4 modelToCameraMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 modelToClipMatrix;

varying vec3 f_position;
varying vec3 f_viewDirection;
varying vec3 f_normal;
varying vec2 f_texCoord;

void main() {

	f_position = (modelToCameraMatrix * (vec4(v_position, 1.0))).xyz;  //positionEye

	f_viewDirection = (0.0, 0.0, 0.0) - (f_position).xyz;  //V

	f_normal = (modelToCameraMatrix * (vec4(v_normal, 0.0))).xyz;  //N
	
	f_texCoord= v_texCoord;

	gl_Position = modelToClipMatrix * vec4(v_position, 1.0);
}

