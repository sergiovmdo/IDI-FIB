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
  createBuffers();
}

void MyGLWidget::paintGL ()
{
  glClear (GL_COLOR_BUFFER_BIT);  // Esborrem el frame-buffer

  modelTransform(); //SUPER IMPORTANTE AÑADIR ESTO SI NO NO SE VE NADA

  glViewport (0, 0, width(), height()); // Aquesta crida no cal, Qt la fa de forma automàtica amb aquests paràmetres

  // Activem l'Array a pintar 
  glBindVertexArray(VAO1);
 
  // Pintem l'escena
  glDrawArrays(GL_TRIANGLES, 0, 3);
  
  // Desactivem el VAO
  glBindVertexArray(0);
}

void MyGLWidget::resizeGL (int w, int h)
{
  // Aquí anirà el codi que cal fer quan es redimensiona la finestra
}


/*En esta funcion realizamos las TGs que queramos*/ 
void MyGLWidget::modelTransform()
{
	glm::mat4 TG(1.0); //Matriz de tg, inicializada en la identidad
	TG = glm::translate(TG,glm::vec3(tx,ty,0.0)); //aplicamos la transformacion
	glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);//esto lo hacemos xq si

}

void MyGLWidget::keyPressEvent (QKeyEvent *e) {
	makeCurrent ();
	switch ( e->key() ) {
		case Qt::Key_Left :
		tx -= 0.1;
		modelTransform(); //Llamamos a la funcion para que se aplique la TGs
		break;
		
		case Qt::Key_Right :
		tx += 0.1;
		modelTransform();
		break;

		case Qt::Key_Up :
		ty += 0.1;
		modelTransform();
		break;

		case Qt::Key_Down :
		ty -= 0.1;
		modelTransform();
		break;

		
		default: e->ignore (); // propagar al pare
}
	update ();
}


void MyGLWidget::createBuffers()
{
  glm::vec3 Vertices[3];  // Tres vèrtexs amb X, Y i Z
  Vertices[0] = glm::vec3(-1.0, -1.0, 0.0);
  Vertices[1] = glm::vec3(1.0, -1.0, 0.0);
  Vertices[2] = glm::vec3(0.0, 1.0, 0.0);
  
  // Creació del Vertex Array Object (VAO) que usarem per pintar
  glGenVertexArrays(1, &VAO1);
  glBindVertexArray(VAO1);

  // Creació del buffer amb les dades dels vèrtexs
  GLuint VBO1;
  glGenBuffers(1, &VBO1);
  glBindBuffer(GL_ARRAY_BUFFER, VBO1);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
  // Activem l'atribut que farem servir per vèrtex  
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);


/* A partir de aqui nos creamos otro VBO para interpolar los colores
  las 3 lineas de abajo sirven para la interpolacion de colores*/
  glm:: vec3 colors[3];
  colors[0] = glm::vec3(1.0, 0.0, 0.0);
  colors[1] = glm::vec3(0.0, 1.0, 0.0);
  colors[2] = glm::vec3(0.0, 0.0, 1.0);
  
  GLuint VBO2;
  glGenBuffers(1, &VBO2);
  glBindBuffer(GL_ARRAY_BUFFER, VBO2);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
  // Activem l'atribut que farem servir per vèrtex 

  /* Estas dos lineas son necesarias para los shaders no entiendo muy bien pq
    pero hay que ponerlas y punto*/

  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);


  // Desactivem el VAO
  glBindVertexArray(0);
}

void MyGLWidget::carregaShaders()
{
  // Creem els shaders per al fragment shader i el vertex shader
  QOpenGLShader fs (QOpenGLShader::Fragment, this);
  QOpenGLShader vs (QOpenGLShader::Vertex, this);
  // Carreguem el codi dels fitxers i els compilem
  fs.compileSourceFile("shaders/fragshad.frag");
  vs.compileSourceFile("shaders/vertshad.vert");
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
  //creamos el identificador para el colorLoc que le pasaremos al vertexShader
  colorLoc= glGetAttribLocation (program->programId(), "colors");
  //creamos el identificador para las TG que le pasaremos al vertexShader
  transLoc = glGetUniformLocation (program->programId(), "TG");
  
}