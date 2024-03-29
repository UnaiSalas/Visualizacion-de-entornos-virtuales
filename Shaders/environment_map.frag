#version 120


// DOCUMENTACION SOBRE LA IMPLEMENTACION OPCIONAL
// LAS INFORMACIÓN PARA PODER REALIZAR ESTA SECCIÓN DEL OBJETIVO OPCIONAL SE OBTUVO A TRAVES DE EL DOCUMENTO FACILITADO EN EGELA EN EL QUE SE DESCRIBE LA PRACTICA.
// POR OTRA PARTE, SE REALIZÓ UNA TUTORÍA PARA PODER RESOLVER DUDAS SOBRE TEMAS QUE NO SE TENÍAN PARA NADA CLAROS AL NO HABER PODIDO SEGUIR EL RITMO POR UN LEVE RETRASO
// EN EL PROYECTO.
// FINALMENTE, Y TRAS SEGUIR LAS RECOMENDACIONES DE LA PROFESORA, SE CONSULTARON LAS DOS PAGINAS WEB QUE CONSTABAN COMO BIBLIOGRAFIA EN EL PDF DE LA PRACTICA OPCIONAL
// ESTOS LINKS SON: "EN.WIKIPEDIA.ORG/WIKI/CUBE_MAPPING" Y "KHRONOS.ORG/OPENGL/WIKI/CUBEMAP_TEXTURE"



uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)
uniform vec3 scene_ambient; // Scene ambient light

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

uniform vec3 campos; // Camera position in world space

uniform sampler2D texture0;   // Texture
uniform samplerCube envmap;   // Environment map (cubemap)

varying vec3 f_position;      // camera space
varying vec3 f_viewDirection; // camera space
varying vec3 f_normal;        // camera space
varying vec2 f_texCoord;
varying vec3 f_positionw;    // world space
varying vec3 f_normalw;      // world space

vec4 texColor;
float epsilon =1.0e-8;


float factor_lambert(in vec3 n, in vec3 l) {
	return dot(n,l);
}
float specular_factor(in vec3 n,in vec3 l, in vec3 v, in float m){
	vec3 r;
	float NoL, RoV;

	NoL = dot(n,l);

	r = normalize(2.0 * NoL * n-l); //normalizar

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
void luz_spot (in int i, in vec3 L, in vec3 N, in vec3 V, inout vec3 color_difuso, inout vec3 color_especular){
 	vec3 dir_foco = normalize(theLights[i].spotDir);

	
	float fac_int = dot(-L, dir_foco);

	// cSpot como el factor de intensidad del foco
	float cSpot = 0.0;

	//Componente difusa
	float NoL = factor_lambert(N,L);
	if(NoL>epsilon){
		// si el angulo es mayor que 0 y esta dentro del angulo del foco, modificamos el factor
		if(fac_int>=theLights[i].cosCutOff){
			if(fac_int>epsilon){
				cSpot = pow(fac_int, theLights[i].exponent);
			}


			if(cSpot>epsilon){
				color_difuso += theLights[i].diffuse * theMaterial.diffuse * NoL * cSpot;

				//Componente especular
				float f_specular = specular_factor(N, L, V, theMaterial.shininess);

				color_especular += theLights[i].specular * theMaterial.specular * f_specular * NoL * cSpot;
			}
		}
	}


}
void luz_posicional (in int i, in vec3 L, in vec3 N, in vec3 V, inout vec3 color_difuso, inout vec3 color_especular, in float att){
	//Componente difusa
	float NoL = factor_lambert(N,L);
	// si la irradiancia es mayor que 0.0
	if(NoL > epsilon){
		color_difuso += theLights[i].diffuse * theMaterial.diffuse * NoL * att;

		//Componente especular
		float f_specular = specular_factor(N, L, V, theMaterial.shininess);

		color_especular += theLights[i].specular * theMaterial.specular * f_specular * NoL * att;
	}
}

void main() {
	float factor = 0.0;
	float d_posicional=0.0;
	float att=0.0;
	vec3 L, N, V, I, R, color_especular, nw;
	vec4 L4, color, texel;

	// Normal del vertice en el sistema de coordenadas del mundo
	nw = normalize(f_normalw);

	//campos -> posicion de la camara en el sistema de coordenadas del mundo
	//f_positionw -> posicion del vertice en el sistema de coordenadas del mundo
	I = normalize(campos - f_positionw);

	//Reflejo de I
	R = 2*dot(nw, I)*nw - I;

	//Al sistema levógiro
	R.z = -R.z;

	//textura del cubemap
	texel = textureCube(envmap, R);

	// vector 4x1
	N = normalize(f_normal);
	// N = vector normalizado (3x1) de N4

	//positionEye = modelToCameraMatrix * vec4(v_position, 1.0);

	// Si la primera luz es direccional, w == 0.0, ... 
	// porque es un vector (x, y, z, w)

	V =normalize(f_viewDirection);

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
		}else{
			L4= theLights[i].position - vec4(f_position, 1.0);
			d_posicional=length(L4); //distancia desde el punto del foco al positionEye
			L=normalize(L4.xyz);
			if(theLights[i].cosCutOff>0.0){ 									//spotlight
				luz_spot(i, L, N, V, color_difuso, color_especular);
			}else{ 																//posicional
				att=theLights[i].attenuation[0]+theLights[i].attenuation[1]*d_posicional+theLights[i].attenuation[2]*d_posicional*d_posicional;
				if(att>epsilon){
					att=1/att;
					luz_posicional(i, L, N, V, color_difuso, color_especular, att);
					}
				}

		}

	}


	texColor = texture2D(texture0, f_texCoord);
	vec4 mezcla = mix(texColor, texel, 0.8); // Hacemos la mezcla de texturas

	gl_FragColor = vec4(scene_ambient + color_difuso, 1.0) * mezcla + vec4(color_especular, theMaterial.alpha); //color * textura
}
