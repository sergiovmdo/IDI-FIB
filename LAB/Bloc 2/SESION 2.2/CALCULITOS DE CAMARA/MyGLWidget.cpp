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

  //projectTransform(); 

  /*Lo de arriba no hace falta en este caso pq hemos creado una funcion que ya inicializa
    la project matrix y la view matrix que se llama iniCamera */

  calculaCapsaModel(); /*Antes de ini camara ya que necesitamos datos que se calculan aqui*/

  iniCamera(); /* Para iniciar la camara*/

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

  glBindVertexArray (VAO_Terra);

  // pintem l'objecte
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindVertexArray (0);
}

/*Inicializamos todos los parametros de la camara */
void MyGLWidget::iniCamera () 
{
	escenaMaxima.x = 1.0; // Se mira a ojo
	escenaMaxima.z = 1.0;// Se mira a ojo
	escenaMinima = glm::vec3(-1,-1,-1); // a ojo


	/*Radi escena lo necesitamos para el znear,zfar y el fov, es importante calcularlo bien y sin errores
    para poder ver una escena sin recortes */

	radiEscena = distance(escenaMinima,escenaMaxima)/2.0; 

  /*El centre escena deberia ser el VRP pero aveces va un poco troleao asi que mejor calcular el VRP a mano*/
	centreEscena = (escenaMaxima + escenaMinima)/ glm::vec3(2.0);


  /*Tanto znear como zfar se calculan asi siempre, asi logramos ver la escena completa*/
	znear = radiEscena;
	zfar = 3*radiEscena;

  /*Esto es una formula , los parentesis son super importantes
    en caso de que nos especifiquen el FOV pues lo ponemos manual*/
	FOV = 2.0*asin(radiEscena/(2*radiEscena));
	FOVini = FOV;
	//FOV = float(M_PI)/3;
	raw = 1.0f;

	projectTransform();

  /*Si no nos especifican OBS Hay que ponerlo asi*/
	OBS = glm::vec3(0,0,2*radiEscena);
	VRP= glm::vec3(0,0,0);
	UP = glm::vec3(0,1,0);

	viewTransform();

}

void MyGLWidget::modelTransform () 
{
  // Matriu de transformació de model
  glm::mat4 transform (1.0f);
  
  transform = glm::scale(transform, glm::vec3(scale));
  transform = glm::rotate(transform,rotate,glm::vec3(0.0,1.0,0.0));
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

    /*Añadimos la funcionalidad de rotar dandole a la R*/
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
  minx = maxx = HomerProves.vertices()[0];
  miny = maxy = HomerProves.vertices()[1];
  minz = maxz = HomerProves.vertices()[2];

  for (unsigned int i = 3; i < HomerProves.vertices().size(); i+=3)
  {
    if (HomerProves.vertices()[i+0] < minx)
      minx = HomerProves.vertices()[i+0];
    if (HomerProves.vertices()[i+0] > maxx)
      maxx = HomerProves.vertices()[i+0];
    if (HomerProves.vertices()[i+1] < miny)
      miny = HomerProves.vertices()[i+1];
    if (HomerProves.vertices()[i+1] > maxy)
      maxy = HomerProves.vertices()[i+1];
    if (HomerProves.vertices()[i+2] < minz)
      minz = HomerProves.vertices()[i+2];
    if (HomerProves.vertices()[i+2] > maxz)
      maxz = HomerProves.vertices()[i+2];
  }
  
  /*En este caso no necesitamos calcular nada mas aquí*/
  escenaMaxima.y = maxy;
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

   //Terra
  glm::vec3 posicio[6] = {
	glm::vec3(1.0, -1.0, 1.0),
	glm::vec3(1.0, -1.0, -1.0),
	glm::vec3(-1.0, -1.0, -1.0),
	
	glm::vec3(1.0, -1.0, 1.0),
	glm::vec3(-1.0, -1.0, -1.0),
	glm::vec3(-1.0, -1.0, 1.0)
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

  projLoc = glGetUniformLocation (program->programId(), "Proj");

  viewLoc = glGetUniformLocation (program->programId(), "View");
}


