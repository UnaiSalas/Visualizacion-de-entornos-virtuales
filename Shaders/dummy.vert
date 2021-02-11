#version 120

uniform mat4 modelToCameraMatrix; // M
uniform mat4 cameraToClipMatrix;  // P

attribute vec3 v_position;

varying vec4 f_color;

void main() {

	f_color = vec4(1,1,1,1);
	gl_Position = cameraToClipMatrix * modelToCameraMatrix * vec4(v_position, 1); // posicion del vertice paso del sistema de coordenadas local al de la camara, y despues con p lo proyecto
}
