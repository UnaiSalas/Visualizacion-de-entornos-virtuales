#version 120

uniform mat4 modelToCameraMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 modelToClipMatrix;

attribute vec3 v_position;


// DOCUMENTACION SOBRE LA IMPLEMENTACION OPCIONAL
// LAS INFORMACIÓN PARA PODER REALIZAR ESTA SECCIÓN DEL OBJETIVO OPCIONAL SE OBTUVO A TRAVES DE EL DOCUMENTO FACILITADO EN EGELA EN EL QUE SE DESCRIBE LA PRACTICA.
// ADEMAS, SE REALIZÓ UNA BREVE TUTORÍA PARA PODER ACLARAR DUDAS MUY PUNTUALES QUE AUNQUE NO TENIAN UNA GRAN REPERCUSIÓN A LA HORA DE SACAR ESTE SHADER FUERON 
// DE GRAN AYUDA PARA ENTENDER LO QUE SE REALIZABA A TRAVES DEL CODIGO.


varying vec3 f_texCoord; // Note: texture coordinates is vec3

void main() {

	f_texCoord.xyz = v_position.xyz;

	f_texCoord.z = -f_texCoord.z;

	gl_Position = modelToClipMatrix * vec4(v_position, 1.0);
}
