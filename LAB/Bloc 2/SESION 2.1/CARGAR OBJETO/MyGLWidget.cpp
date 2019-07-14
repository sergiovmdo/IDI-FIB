/*Cargamos y hacemos que rote y escale mediante teclas un Homer
  Inicializamos la camara, todo detallado debajo, es importante modificar el .pro
  y el .h.
  Para finalizar añadimos un suelo, que tiene que tener transformaciones diferentes al HOmer para
  que no rote tambien al pulsar la R.*/

#include "MyGLWidget.h"

#include <iostream>

MyGLWidget::MyGLWidget (QWidget* parent) : QOpenGLWidget(parent), program(NULL)
{
  setFocusPolicy(Qt::StrongFocus);  // per rebre events de teclat
  scale = 1.0f;
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

  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  carregaShaders();
  creaBuffers();

  /*ACTIVAR EL Z-BUFFER, NOMES UN COP! En paintGL() molt incorrecte, 
    esto es necesario para la carga del Homer no se pq pero lo es */
  glEnable(GL_DEPTH_TEST); 

  //projectTransform(); //SUPER IMPORTANTE AÑADIR ESTO

  /*Lo de arriba no hace falta en este caso pq hemos creado una funcion que ya inicializa
    la project matrix y la view matrix que se llama iniCamera */
  iniCamera();
  rotate = 0.0f;
  scale = 1.0f;
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

// En cas de voler canviar els paràmetres del viewport, descomenteu la crida següent i
// useu els paràmetres que considereu (els que hi ha són els de per defecte)
  glViewport (0, 0, ample, alt);
  
  // Esborrar el buffer de profunditats a la vegada que el frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //ESTO ES IMPORTANTE va ligado a lo de la linea 34

  // Carreguem la transformació de model
  modelTransform ();

  // Activem el VAO per a pintar el Homer
  glBindVertexArray (VAO_HomerProves);

  // pintem l'objecte
  glDrawArrays(GL_TRIANGLES, 0, HomerProves.faces().size()*3);

  glBindVertexArray (0);
}

/*Inicializamos todos los parametros de la camara */
void MyGLWidget::iniCamera () 
{
	FOV = float(M_PI)/2.0f;
	raw = 1.0f;
	znear = 0.4f;
	zfar = 3.0f;
	projectTransform();

	OBS = glm::vec3(0,0,1);
	VRP= glm::vec3(0,0,0);
	UP = glm::vec3(0,1,0);
	viewTransform();

}

void MyGLWidget::modelTransform () 
{
  // Matriu de transformació de model
  glm::mat4 transform (1.0f);
  transform = glm::rotate(transform,rotate,glm::vec3(0.0,1.0,0.0));
  transform = glm::scale(transform, glm::vec3(scale));
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}

/*Metodo para definir la project matrix*/ 

void MyGLWidget::projectTransform () {
	
	/*Debajo tenemos un ejemplo de como creaer una transformacio de projeccio*/
	// glm::perspective (FOV en radians, ra window, znear, zfar)
	glm::mat4 Proj = glm::perspective (FOV, raw, znear, zfar);
	glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}


/*metodo para definir la view matrix*/
void MyGLWidget::viewTransform () {
	
	// glm::lookAt (OBS, VRP, UP)
	glm::mat4 View = glm::lookAt (OBS,VRP, UP);
	glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);
}



void MyGLWidget::resizeGL (int w, int h) 
{
  ample = w;
  alt = h;
}

void MyGLWidget::keyPressEvent(QKeyEvent* event) 
{
  makeCurrent();
  switch (event->key()) {
    case Qt::Key_S: { // escalar a més gran
      scale += 0.05;
      break;
    }
    case Qt::Key_D: { // escalar a més petit
      scale -= 0.05;
      break;
    }

    /*Añadimos la funcionalidad de rotar dandole a la R*/
    case Qt::Key_R: { 
      rotate += float(M_PI)/4.0f;
      break;
    }
    default: event->ignore(); break;
  }
  update();
}

void MyGLWidget::creaBuffers () 
{

  //Cargamos el modelo
  HomerProves.load("Model/HomerProves.obj");

  //Creamos el VAO para pintar
  glGenVertexArrays(1, &VAO_HomerProves);
  glBindVertexArray(VAO_HomerProves);

  GLuint VBO_HomerProvesPos;
  glGenBuffers(1, &VBO_HomerProvesPos);
  
  glBindBuffer(GL_ARRAY_BUFFER, VBO_HomerProvesPos);

  /*sizeof(GLfloat) * m.faces ().size () * 3 * 3 --> nombre de bytes dels buffers del color
    VBO_vertices() --> es el VBO de posicion
    En la 149-150 estamos pasando los datos del buffer de posiciones a la GPU*/
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*HomerProves.faces().size()*3*3, 
              HomerProves.VBO_vertices(), GL_STATIC_DRAW);
 
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);
  
  GLuint VBO_HomerProvesCol;
  glGenBuffers(1, &VBO_HomerProvesCol);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_HomerProvesCol);

  /*VBO_matdiff() --> es el VBO Del color*/
  glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*HomerProves.faces().size()*3*3, 
        HomerProves.VBO_matdiff(), GL_STATIC_DRAW);

  // Activem l'atribut colorLoc
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);  

  glBindVertexArray (0);
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
  // Obtenim identificador per a l'atribut “color” del vertex shader
  colorLoc = glGetAttribLocation (program->programId(), "color");
  // Uniform locations
  transLoc = glGetUniformLocation(program->programId(), "TG");

  projLoc = glGetUniformLocation (program->programId(), "proj");

  viewLoc = glGetUniformLocation (program->programId(), "view");
}


