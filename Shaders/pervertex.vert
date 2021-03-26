#version 120

uniform mat4 modelToCameraMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 modelToClipMatrix;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)
uniform vec3 scene_ambient;  // rgb
float epsilon =1.0e-8;

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
float specular_factor(in vec3 n,in vec3 l, in vec3 v, in float m){
	vec3 r;
	float NoL, RoV;

	NoL =dot(n,l);

	r=normalize(2.0 * NoL * n-l); //normalizar

	RoV = dot(r,v);

	if(RoV>epsilon){
		return pow(RoV, m);
	}else{
		return 0.0;
	}
	// COMPROBAR QUE pow(RoV, m)	ES CORRECTO
	// RoV > 0.0 ----> pow(RoV, m)
	// sino devuelvo 0.0 //es mate (tipo de material)



	
}
void luz_direccional (in int i, in vec3 L, in vec3 N, in vec3 V, inout vec3 color_difuso, inout vec3 color_especular){
	
	//Componente difusa
	float NoL = factor_lambert(N,L);
	// si la irradiancia es mayor que 0.0
	if(NoL > epsilon){
		color_difuso += theLights[i].diffuse * theMaterial.diffuse * NoL;

		//Componente especular
		float f_specular = specular_factor(N, L, V, theMaterial.shininess);

		color_especular += theLights[i].specular * theMaterial.specular * f_specular * NoL;
	}
}
void main() {
	float factor = 0.0;
	vec3 L, N, V, color_especular;
	vec4 L4, N4, positionEye, V4;

	// vector 4x1
	N4 = modelToCameraMatrix * vec4(v_normal,0.0);
	N = normalize(N4.xyz);
	// N = vector normalizado (3x1) de N4

	positionEye = modelToCameraMatrix * vec4(v_position, 1.0);

	// Si la primera luz es direccional, w == 0.0, ... 
	// porque es un vector (x, y, z, w)

	V4 = (0, 0, 0, 1) - positionEye;
	V =normalize(V4.xyz);

	// acumulador RGB
	// vec3(0.0) == vec3(0.0, 0.0, 0.0)
	vec3 color_difuso = vec3(0.0, 0.0, 0.0);
	color_especular= vec3(0.0, 0.0, 0.0);
	// N es el vector normal en el vertice (en el espacio de la camara)
	// normalEye en las traspas


	for (int i=0; i<active_lights_n;i++){
		if (theLights[i].position[3] == 0.0) {
			L4=(-1.0) * theLights[i].position;
			L=normalize(L4.xyz);
			luz_direccional(i, L, N, V, color_difuso, color_especular);
			//color_difuso=vec3(0.0,1.0,0.0);
		}// else{
			// color_difuso=vec3(1.0,0.0,0.0);
		// }

	}

	f_color = vec4(color_difuso + color_especular, 1.0);
	f_texCoord = v_texCoord;

	gl_Position = modelToClipMatrix * vec4(v_position, 1);
}

