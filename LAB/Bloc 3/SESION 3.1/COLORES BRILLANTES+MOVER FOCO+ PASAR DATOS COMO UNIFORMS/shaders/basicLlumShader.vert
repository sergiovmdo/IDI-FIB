#version 330 core

in vec3 vertex;
in vec3 normal;

in vec3 matamb;
in vec3 matdiff;
in vec3 matspec;
in float matshin;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 TG;

//Necesario si queremos pasarlo como uniforms
uniform vec3 posFocus;
uniform vec3 colFocus;

/*Estan comentados pq se los paso como uniforms a traves del MYGLWidget.cpp
    Si no tendrian que estar descomentados */
    
//vec3 colFocus = vec3(0.8, 0.8, 0.8);
vec3 llumAmbient = vec3(0.2, 0.2, 0.2);
//vec3 posFocus = vec3(1, 0, 1);  // en SCA

out vec3 fcolor;

vec3 Lambert (vec3 NormSCO, vec3 L) 
{
    // S'assumeix que els vectors que es reben com a paràmetres estan normalitzats

    // Inicialitzem color a component ambient
    vec3 colRes = llumAmbient * matamb;

    // Afegim component difusa, si n'hi ha
    if (dot (L, NormSCO) > 0)
      colRes = colRes + colFocus * matdiff * dot (L, NormSCO);
    return (colRes);
}

vec3 Phong (vec3 NormSCO, vec3 L, vec4 vertSCO) 
{
    // Els vectors estan normalitzats

    // Inicialitzem color a Lambert
    vec3 colRes = Lambert (NormSCO, L);

    // Calculem R i V
    if (dot(NormSCO,L) < 0)
      return colRes;  // no hi ha component especular

    vec3 R = reflect(-L, NormSCO); // equival a: normalize (2.0*dot(NormSCO,L)*NormSCO - L);
    vec3 V = normalize(-vertSCO.xyz);

    if ((dot(R, V) < 0) || (matshin == 0))
      return colRes;  // no hi ha component especular
    
    // Afegim la component especular
    float shine = pow(max(0.0, dot(R, V)), matshin);
    return (colRes + matspec * colFocus * shine); 
}

void main()
{	
    //Pasar posicion del vertice al SCO
    vec3 vertSCO = (view*TG*vec4(vertex,1.0)).xyz;

    //Pasamos posicion del foco de luz al SCO
    vec3 posF = (view * vec4(posFocus, 1.0)).xyz;
    
    // Direccio llum
    vec3 L = posF - vertSCO;
    
    // Passar vector normal a SCO
    mat3 NormalMatrix = (inverse(transpose(mat3(view * TG))));
    vec3 NM = NormalMatrix * normal;
    
    // Nomes la Normal i la L son "vcectors", els unics que es normalitzen
    L = normalize(L);
    NM = normalize(NM);


    /* Si queremos que tenga taca especular pues hay que usar Phong*/
    fcolor = Phong(NM, L, vec4(vertSCO, 1.));
    
    /*Si no pues usamos al parguel este, lo demas del codigo mejor copy pastear todo
        pq npi de lo q hace iluminacion, solo hay q tocar cosas del VS a no ser que lo quieran en el FS
        en cuyo caso tendremos otra guia a parte*/

    //fcolor = Lambert(NM, L);
    
    gl_Position = proj * view * TG * vec4 (vertex, 1.0);


}
