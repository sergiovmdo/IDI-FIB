/*EN ESTE ARCHIVO VEMOS COMO CREAR DOS VAOS Y 
  CREAMOS DOS VIEWPORTS */


//#include <GL/glew.h>
#include "MyGLWidget.h"

#include <iostream>

MyGLWidget::MyGLWidget (QWidget* parent) : QOpenGLWidget(parent), program(NULL)
{
  setFocusPolicy(Qt::StrongFocus);  // per rebre events de teclat
}

MyGLWidget::~MyGLWidget ()
{
  if (program != NULL)
    delete program;
}

void MyGLWidget::initializeGL ()
{
  // Cal inicialitzar l'ús de les funcions d'OpenGL
  initializeOpenGLFunctions();
  
  glClearColor (0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  carregaShaders();
  //Llamamos a las funciones para pintar los objetos
  creaBuffers1();
  creaBuffers2();
}

void MyGLWidget::paintGL ()
{
// Aquest codi és necessari únicament per a MACs amb pantalla retina.
#ifdef __APPLE__
  GLint vp[4];
  glGetIntegerv (GL_VIEWPORT, vp);
  ample = vp[2];
  alt = vp[3];
#endif

  
  glClear (GL_COLOR_BUFFER_BIT);  // Esborrem el frame-buffer


  /*El primer viewport*/

   glViewport (0, 0, ample/2, alt/2);

  // Activem l'Array a pintar VAO1
  glBindVertexArray(VAO1);
 
  // Pintem l'escena VAO1
  glDrawArrays(GL_TRIANGLES, 0, 3); //Aqui espeecificamos el numero de vertices a pintar
  

  // Activem l'Array a pintar VAO2
  glBindVertexArray(VAO2);
 
  // Pintem l'escena VAO2
  glDrawArrays(GL_TRIANGLES, 0, 3); //Aqui espeecificamos el numero de vertices a pintar

  /* Aqui hacemos que haya otro viewport dentro de nuestra pantalla, los dos primeros parametros
    nos definen donde empezará la escena*/

  glViewport (ample/2, alt/2, ample/2, alt/2);
  
  // Activem l'Array a pintar VAO1
  glBindVertexArray(VAO1);
 
  // Pintem l'escena VAO1
  glDrawArrays(GL_TRIANGLES, 0, 3); //Aqui espeecificamos el numero de vertices a pintar
  
  // Activem l'Array a pintar VAO2
  glBindVertexArray(VAO2);
 
  // Pintem l'escena VAO2
  glDrawArrays(GL_TRIANGLES, 0, 3); //Aqui espeecificamos el numero de vertices a pintar

  // Desactivem el VAO
  glBindVertexArray(0);
}

void MyGLWidget::resizeGL (int w, int h)
{
  ample = w;
  alt = h;
}

void MyGLWidget::creaBuffers1 ()
{

  /* Aqui estoy dibujando un cuadrado centrado
    simplemente es pasarle las coordenadas y tocar el num de la linea 48
  */

  glm::vec3 Vertices[3];  // Tres vèrtexs amb X, Y i Z
  Vertices[0] = glm::vec3(-1, -1, 0.0);
  Vertices[1] = glm::vec3(0, -1, 0.0);
  Vertices[2] = glm::vec3(-0.5, 0.75, 0.0);

  // Creació del Vertex Array Object (VAO) que usarem per pintar
  glGenVertexArrays(1, &VAO1);
  glBindVertexArray(VAO1);

  // Creació del buffer amb les dades dels vèrtexs
  GLuint VBO1;
  glGenBuffers(1, &VBO1);
  glBindBuffer(GL_ARRAY_BUFFER, VBO1);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
  // Activem l'atribut que farem servir per vèrtex (només el 0 en aquest cas)	
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0); //El 3 no se que es por si acaso no tocar
  glEnableVertexAttribArray(vertexLoc);

  // Desactivem el VAO
  glBindVertexArray(0);


}

//Creamos una funcion identica a la anterior para usar un nuevo VAO con datos dentro de un VBO

void MyGLWidget::creaBuffers2 ()
{

  /* Aqui estoy dibujando un cuadrado centrado
    simplemente es pasarle las coordenadas y tocar el num de la linea 48
  */

  glm::vec3 Vertices[3];  // Tres vèrtexs amb X, Y i Z
  Vertices[0] = glm::vec3(0, -1, 0.0);
  Vertices[1] = glm::vec3(1, -1, 0.0);
  Vertices[2] = glm::vec3(0.5, 0.75, 0.0);

  // Creació del Vertex Array Object (VAO) que usarem per pintar
  glGenVertexArrays(1, &VAO2);
  glBindVertexArray(VAO2);

  // Creació del buffer amb les dades dels vèrtexs
  GLuint VBO2;
  glGenBuffers(1, &VBO2);
  glBindBuffer(GL_ARRAY_BUFFER, VBO2);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
  // Activem l'atribut que farem servir per vèrtex (només el 0 en aquest cas) 
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0); //El 3 no se que es por si acaso no tocar
  glEnableVertexAttribArray(vertexLoc);

  // Desactivem el VAO
  glBindVertexArray(0);


}

void MyGLWidget::carregaShaders()
{
  // Creem els shaders per al fragment shader i el vertex shader
  QOpenGLShader fs (QOpenGLShader::Fragment, this);
  QOpenGLShader vs (QOpenGLShader::Vertex, this);
  // Carreguem el codi dels fitxers i els compilem
  fs.compileSourceFile("shaders/basicShader.frag");
  vs.compileSourceFile("shaders/basicShader.vert");
  // Creem el program
  program = new QOpenGLShaderProgram(this);
  // Li afegim els shaders corresponents
  program->addShader(&fs);
  program->addShader(&vs);
  // Linkem el program
  program->link();
  // Indiquem que aquest és el program que volem usar
  program->bind();

  // Obtenim identificador per a l'atribut “vertex” del vertex shader
  vertexLoc = glGetAttribLocation (program->programId(), "vertex");
}
