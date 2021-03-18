#version 120

uniform mat4 modelToCameraMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 modelToClipMatrix;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)
uniform vec3 scene_ambient;  // rgb

uniform struct light_t {
	vec4 position;    // Camera space
	vec3 diffuse;     // rgb
	vec3 specular;    // rgb
	vec3 attenuation; // (constant, lineal, quadratic)
	vec3 spotDir;     // Camera space
	float cosCutOff;  // cutOff cosine
	float exponent;
} theLights[4];     // MG_MAX_LIGHTS

uniform struct material_t {
	vec3  diffuse;
	vec3  specular;
	float alpha;
	float shininess;
} theMaterial;

attribute vec3 v_position; // Model space
attribute vec3 v_normal;   // Model space
attribute vec2 v_texCoord;

varying vec4 f_color;
varying vec2 f_texCoord;

// Implementa la ley de Lambert para obtener el color difuso del vertice
float factor_lambert(in vec3 n, in vec3 l) {
	return dot(n,l);
}

//void luz_direccional(in ...) {
//	
//}

void main() {
	float factor = 0.0;
	vec3 L, N;

	// Si la primera luz es direccional, w == 0.0, ... 
	// porque es un vector (x, y, z, w)

	// acumulador RGB
	// vec3(0.0) == vec3(0.0, 0.0, 0.0)
	vec3 color_difuso = vec3(0.0, 0.0, 0.0);

	// N es el vector normal en el vertice (en el espacio de la camara)
	// normalEye en las traspas

	// vector 4x1
	vec4 N4 = modelToCameraMatrix * vec4(v_normal, 0.0);
	N = normalize(N4.xyz);
	// N = vector normalizado (3x1) de N4

	if (theLights[0].position[3] == 0) {
		// La luz es direccional

		// factor de Lambert (n, l) = coseno del angulo que forman n y l = 
		//      = dot(n, l) siendo n y l vectores unitarios

		// L es el vector luz (saliente)
		// theLights[0].position vec4... L es un vec3 normalizado
		L = normalize(-theLights[0].position.xyz);
		// VA ENTRE [-1, 1]

		factor = factor_lambert(N, L);

		if (factor > 0.0) {
			color_difuso += theLights[0].diffuse * theMaterial.diffuse * factor;
		}

	}

	f_color = vec4(color_difuso, 1.0);
	f_texCoord = v_texCoord;

	gl_Position = modelToClipMatrix * vec4(v_position, 1);
}

