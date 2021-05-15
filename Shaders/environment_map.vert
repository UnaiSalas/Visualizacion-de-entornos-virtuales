#version 120


// DOCUMENTACION SOBRE LA IMPLEMENTACION OPCIONAL
// LAS INFORMACIÓN PARA PODER REALIZAR ESTA SECCIÓN DEL OBJETIVO OPCIONAL SE OBTUVO A TRAVES DE EL DOCUMENTO FACILITADO EN EGELA EN EL QUE SE DESCRIBE LA PRACTICA.
// POR OTRA PARTE, SE REALIZÓ UNA TUTORÍA PARA PODER RESOLVER DUDAS SOBRE TEMAS QUE NO SE TENÍAN PARA NADA CLAROS AL NO HABER PODIDO SEGUIR EL RITMO POR UN LEVE RETRASO
// EN EL PROYECTO.
// FINALMENTE, Y TRAS SEGUIR LAS RECOMENDACIONES DE LA PROFESORA, SE CONSULTARON LAS DOS PAGINAS WEB QUE CONSTABAN COMO BIBLIOGRAFIA EN EL PDF DE LA PRACTICA OPCIONAL
// ESTOS LINKS SON: "EN.WIKIPEDIA.ORG/WIKI/CUBE_MAPPING" Y "KHRONOS.ORG/OPENGL/WIKI/CUBEMAP_TEXTURE"


attribute vec3 v_position;
attribute vec3 v_normal;
attribute vec2 v_texCoord;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)

uniform mat4 modelToCameraMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 modelToClipMatrix;

varying vec3 f_position;       // camera space
varying vec3 f_viewDirection;  // camera space
varying vec3 f_normal;         // camera space
varying vec2 f_texCoord;       // camera space

varying vec3 f_positionw; // world space
varying vec3 f_normalw;   // world space

void main() {
	f_position = (modelToCameraMatrix * (vec4(v_position, 1.0))).xyz;

	f_normal = (modelToCameraMatrix * (vec4(v_normal, 0.0))).xyz;

	f_viewDirection = (0.0, 0.0, 0.0) - (f_position);

	f_texCoord= v_texCoord;

	f_positionw = (modelToWorldMatrix * (vec4(v_position, 1.0))).xyz;

	f_normalw = (modelToWorldMatrix * (vec4(v_normal, 0.0))).xyz;

	gl_Position = modelToClipMatrix * vec4(v_position, 1.0);
}
