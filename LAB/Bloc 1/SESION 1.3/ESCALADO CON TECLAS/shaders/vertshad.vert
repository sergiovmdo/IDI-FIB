#version 330 core

in vec3 vertex;

uniform float scale;


/*Estas dos lineas son para interpolar colores. La primera es lo que recibimos del
	cpp, y la segunda es lo que le mandamos al fragment shader*/
in vec3 colors;
out vec4 out_colors;

void main()  {
	
    //Si queremos escalar, multiplicamos el numero por el vertex	
    gl_Position = vec4 (vertex*scale, 1.0);

//En caso de querer interpolar colores
	out_colors = vec4(colors,1.0);
}
