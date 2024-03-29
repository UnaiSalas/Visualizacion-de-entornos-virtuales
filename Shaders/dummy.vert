#version 120

uniform mat4 modelToCameraMatrix; // M
uniform mat4 cameraToClipMatrix;  // P

uniform float sc;

attribute vec3 v_position;

varying vec4 f_color;

void main() {
	if (sc<0.5)
		f_color=vec4(1,0,0,1);
	else
		f_color = vec4(0,1,0,1);
	
	gl_Position = cameraToClipMatrix * modelToCameraMatrix * vec4(v_position, 1); // posicion del vertice paso del sistema de coordenadas local al de la camara, y despues con p lo proyecto
}
