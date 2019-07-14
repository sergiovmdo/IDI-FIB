#version 330 core

in vec3 fcolor;
out vec4 FragColor;

uniform int franges;

void main()
{	

	if (franges == 1){

		int tam = int(gl_FragCoord.y)/10;

		if (tam%2 == 0) FragColor = vec4(1.0,1.0,1.0,1.0);
		else FragColor = vec4(0.0,0.0,0.0,0.0);

	}
	else FragColor = vec4(fcolor,1);	
}
