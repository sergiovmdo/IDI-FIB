#version 330 core

out vec4 FragColor;
in vec4 out_colors;

void main() {
    
	//el viewport mide 700x700

/* Esta linea de debajo hace que hayan franjas vacias en nuestro dibujo
	si hacemos .x las franjas son verticales y si hacemos .y (como aqui debajo)
	obtenemos franjas transparentes horizontales.
	Contra mas mayor es el numero por el que dividimos, las franjas son mas gordas
	el else sirve para las franjas que tienen color*/
/*	
if (int(gl_FragCoord.y/5) %2 == 0) discard;
	else{

   	 // arriba izq
	if (gl_FragCoord.x < 354 && gl_FragCoord.y > 354)
		FragColor = vec4(1, 0, 0, 1);
	// arriba der
	if (gl_FragCoord.x > 354 && gl_FragCoord.y > 354)
		FragColor = vec4(0, 0, 1, 1);
	// abajo izq
	if (gl_FragCoord.x < 354 && gl_FragCoord.y < 354)
		FragColor = vec4(1, 1, 0, 1);
	// abajo der
	if (gl_FragCoord.x > 354 && gl_FragCoord.y < 354)
		FragColor = vec4(0, 1, 0, 1);
}*/
	//PARA INTERPOLAR COLORES
 	FragColor = out_colors;
}

