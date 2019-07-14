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

  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  carregaShaders();
  creaBuffers();

  /*ACTIVAR EL Z-BUFFER, NOMES UN COP! En paintGL() molt incorrecte, 
    esto es necesario para la carga del Homer no se pq pero lo es */
  glEnable(GL_DEPTH_TEST); 

  //projectTransform(); //SUPER IMPORTANTE AÑADIR ESTO SI NO HAY INI CAMARA

  /*Lo de arriba no hace falta en este caso pq hemos creado una funcion que ya inicializa
    la project matrix y la view matrix que se llama iniCamera */

  calculaCapsaModel();

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
  //glViewport (0, 0, ample, alt);
  
  // Esborrar el buffer de profunditats a la vegada que el frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //ESTO ES IMPORTANTE va ligado a lo de la linea 34

  // Carreguem la transformació de model
  

  // Activem el VAO per a pintar el Homer
  glBindVertexArray (VAO_Patricio);
  modelTransform ();

  // pintem l'objecte
  glDrawArrays(GL_TRIANGLES, 0, Patricio.faces().size()*3);

  glBindVertexArray (VAO_Terra);
  modelTransformTerra();

  // pintem l'objecte
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindVertexArray (0);
}

/*Inicializamos todos los parametros de la camara */
void MyGLWidget::iniCamera () 
{
	escenaMaxima.x = 2.5;
	escenaMaxima.z = 2.5;
	escenaMinima = glm::vec3(-2.5,0.0,-2.5);
	
	radiEscena = distance(escenaMinima,escenaMaxima)/2.0;
	std::cout << radiEscena <<std::endl;
	centreEscena = (escenaMaxima + escenaMinima);
  centreEscena = centreEscena / glm::vec3(2.0);

	znear = radiEscena;
	zfar = 3*radiEscena;

	FOVini = 2.0*asin(radiEscena/(2*radiEscena));
	FOV = FOVini;
	//FOV = float(M_PI)/3;
	raw = 1.0f;
  angleX = 0.0;
  angleY = 0.0;

	projectTransform();

	OBS = glm::vec3(0.0,2.0,(2*radiEscena));
	VRP= glm::vec3(centreEscena);
	UP = glm::vec3(0,1,0);

	viewTransform();

}

void MyGLWidget::modelTransform () 
{
  glm::mat4 transform (1.0f);
  transform = glm::rotate(transform, rotate, glm::vec3(0.0, 1.0, 0.0));
  transform = glm::scale(transform, glm::vec3(scale));
  transform = glm::scale(transform, glm::vec3(escalaPat));
  transform = glm::translate(transform, -centreBasePat);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}

void MyGLWidget::modelTransformTerra () 
{
  // Matriu de transformació de model
  glm::mat4 transform (1.0f);
  transform = glm::scale(transform, glm::vec3(scale));
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}

/*Metodo para definir la project matrix*/ 

void MyGLWidget::projectTransform () 
{
  //glm::perspective (FOV en radians, ra window, Znear, Zfar);
  glm::mat4 Proj = glm::perspective (FOV, raw, znear, zfar);
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::viewTransform () 
{
  
  /*Camara a partir de angulos de Euler, aqui lo unico que faltaria seria el angulo z
    que va justo encima de la x*/


  glm::mat4 View(1.0);
  View = glm::translate(View,glm::vec3(0.,0.,-(radiEscena*2)));
  //Aqui iria la rotacion del -angleZ, si el - es importante MUY IMPORTANTE
  View = glm::rotate(View,angleX, glm::vec3(1.,0.,0.));
  View = glm::rotate(View,-angleY, glm::vec3(0.,1.,0.));//OJO CON EL MENOS QUE HAY QUE PONERLO CRACKS
  View = glm::translate(View,-VRP);
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &View[0][0]);
}

void MyGLWidget::resizeGL (int w, int h) 
{
 	float rav = w/h;
	raw = rav;
	if (rav < 1.0) FOV = 2.0*atan(tan(FOVini/2.0)/rav);
	projectTransform ();
	glViewport(0, 0, w, h);
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
    case Qt::Key_R: {
    rotate += float(M_PI)/4.0f;
    break;
  }
    default: event->ignore(); break;
  }
  update();
}

void MyGLWidget::calculaCapsaModel ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = Patricio.vertices()[0];
  miny = maxy = Patricio.vertices()[1];
  minz = maxz = Patricio.vertices()[2];
  for (unsigned int i = 3; i < Patricio.vertices().size(); i+=3)
  {
    if (Patricio.vertices()[i+0] < minx)
      minx = Patricio.vertices()[i+0];
    if (Patricio.vertices()[i+0] > maxx)
      maxx = Patricio.vertices()[i+0];
    if (Patricio.vertices()[i+1] < miny)
      miny = Patricio.vertices()[i+1];
    if (Patricio.vertices()[i+1] > maxy)
      maxy = Patricio.vertices()[i+1];
    if (Patricio.vertices()[i+2] < minz)
      minz = Patricio.vertices()[i+2];
    if (Patricio.vertices()[i+2] > maxz)
      maxz = Patricio.vertices()[i+2];
  }

  escalaPat = 4.0/(maxy-miny);
  centreBasePat= glm::vec3((minx+maxx)/2, miny, (minz+maxz)/2);
  escenaMaxima.y = 4.0;
}



void MyGLWidget::creaBuffers () 
{
  // Patricio
  Patricio.load("Model/Patricio.obj");
  
  // Creació del Vertex Array Object per pintar
  glGenVertexArrays(1, &VAO_Patricio);
  glBindVertexArray(VAO_Patricio);
  
  GLuint VBO_PatricioPos;
  glGenBuffers(1, &VBO_PatricioPos);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatricioPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*Patricio.faces().size()*3*3, 
        Patricio.VBO_vertices(), GL_STATIC_DRAW);
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);
  
  GLuint VBO_PatricioCol;
  glGenBuffers(1, &VBO_PatricioCol);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatricioCol);
  glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*Patricio.faces().size()*3*3, 
        Patricio.VBO_matdiff(), GL_STATIC_DRAW);

  // Activem l'atribut colorLoc
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);        
  
  
  //Terra
  glm::vec3 posicio[6] = {
  glm::vec3(2.5, 0, 2.5),
  glm::vec3(2.5, 0, -2.5),
  glm::vec3(-2.5, 0, -2.5),
  
  glm::vec3(2.5, 0, 2.5),
  glm::vec3(-2.5, 0, -2.5),
  glm::vec3(-2.5, 0, 2.5)
  }; 
   glm::vec3 color[6] = {
  glm::vec3(1,0,0),
  glm::vec3(0,0,1),
  glm::vec3(0,1,0),
  
  glm::vec3(1,0,0),
  glm::vec3(0,1,0),
  glm::vec3(0,0,1),
  };
  
  glGenVertexArrays(1, &VAO_Terra);
  glBindVertexArray(VAO_Terra);
  
  GLuint VBO_TerraPos;
  glGenBuffers(1, &VBO_TerraPos);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(posicio), posicio, GL_STATIC_DRAW);
  
  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);
  
  GLuint VBO_TerraCol;
  glGenBuffers(1, &VBO_TerraCol);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraCol);
  glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);

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
  projLoc = glGetUniformLocation(program->programId(), "Proj");
  viewLoc = glGetUniformLocation(program->programId(), "View");
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *e) {
  makeCurrent();
  if(e->x() > x_ant) angleY -= 0.03;
  else if(e->x() < x_ant) angleY += 0.03;
  
  if(e->y() > y_ant) angleX -= 0.03;
  else if(e->y() < y_ant) angleX += 0.03;
  
  x_ant = e->x();
  y_ant = e->y();
  viewTransform();
  update();
}