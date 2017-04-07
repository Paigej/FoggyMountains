/* Paige Johnson
 * Final Project
 */
#include <stdio.h>

//include custom library
#include "customLib.h"

#include <vector>

#include <stdlib.h>
#include <stdarg.h>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//  Globals

int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio

double dim= 3.0;  //  Size of world

double Ex = 1;   //  Eye
double Ey = -200;   //  Eye
double Ez = 1;   //  Eye

double Fx = 1;   //  Fireball
double Fy = 1;   //  Fireball
double Fz = 1;   //  Fireball
double Fspin = 0.0;
double Ftilt = 0.0;


int    sky[2];   //  Sky textures
int    map[2]; // Texture for Map

stbi_uc gHeightMap[1081][1081];//  DEM data

double Tilt = 0.0;
double Spin = 0.0;
double Twist = 0.0;

bool leftMouseDown = false;
int mouseX;
int mouseY;

double near = 0.001;
double far = 50000;

int    move=0;   //  Light movement
int    light=1;  //  Lighting
bool firebool=false;
//float Position[]  = {(float)Fx, (float)Fy, (float)Fz};


typedef struct Vertex
{
  float x, y, z;

  float uvX, uvY;
} Vertex;

float Norm(Vertex *aV)
{
  return (aV->y + 20.f)/20.f;
}

void DrawV(Vertex *aV)
{
  glTexCoord2f(aV->uvX, aV->uvY);
  glVertex3d(aV->x, aV->y, aV->z);
}


typedef struct Triangle
{
  Vertex a, b, c;

  float color;
}Triangle;

void DrawT(Triangle *aT)
{
  glBegin(GL_TRIANGLES);
  DrawV(&aT->a);
  DrawV(&aT->b);
  DrawV(&aT->c);
  glEnd();
}

typedef struct Quad
{
  Triangle a, b;
  //Vertex a, b, c, d;
} Quad;

Quad MakeQuad(Vertex *aA, Vertex *aB, Vertex *aC, Vertex *aD)
{
  Quad toReturn;
  toReturn.a.a = *aA;
  toReturn.a.b = *aB;
  toReturn.a.c = *aC;
  toReturn.a.color = (Norm(aA)  + Norm(aB) + Norm(aC)) / 3.0f;

  toReturn.b.a = *aA;
  toReturn.b.b = *aC;
  toReturn.b.c = *aD;
  toReturn.b.color = (Norm(aA)  + Norm(aC) + Norm(aD)) / 3.0f;

  return toReturn;
}

void DrawQ(Quad *aQ)
{
  //glBindTexture(GL_TEXTURE_2D,map[0]);
  glBindTexture(GL_TEXTURE_2D,map[1]);
  glEnable(GL_TEXTURE_2D);
  DrawT(&aQ->a);
  DrawT(&aQ->b);
  glDisable(GL_TEXTURE_2D);
}

std::vector<Quad> gQuads;


void mouseButton(int button, int state, int x, int y)
{
  fflush(stdout);
  mouseX = x;
  mouseY = y;

  // Ignore high order bits, some GLUT implementations set these.
  button = button % 8;

  if (button == GLUT_LEFT_BUTTON)
  {
    leftMouseDown = (state == GLUT_DOWN);
  }
}

void mouseMotion(int x, int y)
{
  int dx = x - mouseX;
  int dy = y - mouseY;

  if (true == leftMouseDown)
  {
    Tilt += dy * 0.5;
    Spin += dx * 0.5;
  }

  mouseX = x;
  mouseY = y;

  glutPostRedisplay();
}

double degreesToRadians(double aDegrees)
{
  //return aDegrees * M_PI / 180;
  //fix 90 deg issues?
  return aDegrees * 3.14159 / 180;
}

static void RotateAboutX(double aAngle)
{

   glRotated(aAngle, 1.0, 0.0, 0.0);
}

static void RotateAboutY(double aAngle)
{
   glRotated(aAngle, 0.0, 1.0, 0.0);
}

static void RotateAboutZ(double aAngle)
{
  float x = sin(degreesToRadians(-Spin));
  float y = tan(degreesToRadians(Tilt));
  float z = cos(degreesToRadians(-Spin));

  float mag = sqrt((x*x) + (y*y) + (z*z));

  x /= mag;
  y /= mag;
  z /= mag;

  glRotated(aAngle, x, y, z);
}

static void Rotate(double aX, double aY, double aZ)
{
   RotateAboutX(aX);
   RotateAboutY(aY);
   RotateAboutZ(aZ);
}

/* 
 *  Draw sky box: Taken from Example 25
 */
static void Sky(double D)
{
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);

   //  Sides
   glBindTexture(GL_TEXTURE_2D,sky[0]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.00,0); glVertex3f(-D,-D,-D);
   glTexCoord2f(0.25,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.00,1); glVertex3f(-D,+D,-D);

   glTexCoord2f(0.25,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.50,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);

   glTexCoord2f(0.50,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.75,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);

   glTexCoord2f(0.75,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(1.00,0); glVertex3f(-D,-D,-D);
   glTexCoord2f(1.00,1); glVertex3f(-D,+D,-D);
   glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
   glEnd();

   //  Top and bottom
   glBindTexture(GL_TEXTURE_2D,sky[1]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.0,0); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.5,0); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.5,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.0,1); glVertex3f(-D,+D,-D);

   glTexCoord2f(1.0,1); glVertex3f(-D,-D,+D);
   glTexCoord2f(0.5,1); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.5,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(1.0,0); glVertex3f(-D,-D,-D);
   glEnd();

   glDisable(GL_TEXTURE_2D);
}

static float GreyScaleToHeight(stbi_uc aGreyScale)
{
  //multipy by scalar
  return (aGreyScale / 255.0f) * 300.0f;
}

void SetUpTriangles()
{
  std::vector<std::vector<Vertex>> points2d;

  const int inc = 10;


  int widthSize  = 1081 / inc;
  int heightSize = 1081 / inc;

  points2d.resize(heightSize);

  for (auto &points : points2d)
  {
    points.resize(widthSize);
  }

  int correctSize = widthSize * heightSize;

  float scalar = 3.0f;

  for (int i = inc; i < 1081; i += inc)
  {
    float x = (i - (1081 / 2))*scalar; //multipy by mesh scalar

    for (int j = inc; j < 1081; j += inc)
    {
      float z = (j - (1081 / 2))*scalar;


      Vertex v; 
      v.x = x;
      //subtract from y axis to move map down
      v.y = GreyScaleToHeight(gHeightMap[i][j]) - 300.0f;
      v.z = z;

      v.uvX  = i / 1081.0f;
      v.uvY  = j / 1081.0f;

      points2d[(i / inc) - 1][(j / inc) - 1] = v;
      --correctSize;
    }
  }

  for (int i = 1; i < heightSize; i += 1)
  {
    for (int j = 1; j < widthSize; j += 1)
    {
      Quad a = MakeQuad(&points2d[i-1][j+0], &points2d[i+0][j+0],
                        &points2d[i+0][j-1], &points2d[i-1][j-1]);

      gQuads.push_back(a);
    }
  }

  if (correctSize != 0)
  {
    printf("Wrong size, left: %d\n", correctSize);
  }
}

void DEM()
{
  //printf("Drawing DEM\n");

  //increment variable for ittering through hight map data
  //const int inc = 10;

  //color added to map
  //glColor3f(.5,.5,.5);

  //Draw all points in the Mesh.  Used for testing
  /*
  glBegin(GL_QUADS);
  glBegin(GL_LINE_LOOP);
  
  for (int i = inc; i < 1081; i += inc)
  {
    float x = (i - (1081 / 2)) / 10.0f;

    for (int j = inc; j < 1081; j += inc)
    {
      float z = (j - (1081 / 2)) / 10.0f;

      glBegin(GL_POINTS);
      glVertex3d(x, GreyScaleToHeight(gHeightMap[i][j]) - 20.0f, z);
      glEnd();



      //glBegin(GL_QUADS);
      //glVertex3d(x - inc, GreyScaleToHeight(gHeightMap[i - inc][j - inc]), z - inc); 
      //glVertex3d(x +   0, GreyScaleToHeight(gHeightMap[i +   0][j - inc]), z - inc);
      //glVertex3d(x +   0, GreyScaleToHeight(gHeightMap[i +   0][j +   0]), z +   0);
      //glVertex3d(x - inc, GreyScaleToHeight(gHeightMap[i - inc][j +   0]), z +   0);
      //glEnd();
    }
  }
  */

  //glBegin(GL_QUADS);
  for (auto& quad : gQuads)
  {
    DrawQ(&quad);
  }
  //glEnd();
}


void Fog()
{
//fog color
GLfloat fogcolor[] ={0.5f, 0.5f, 0.5f, 1.0f};
 // Enables GL_FOG
glEnable(GL_FOG);                  
//GL_EXP, GL_EXP2, GL_LINEAR }      // Fog Mode
glFogi(GL_FOG_MODE, GL_EXP);  
//0.5f, 0.5f, 0.5f, 1.0f};         // Set Fog Color
glFogfv(GL_FOG_COLOR, fogcolor);   
// How Dense Will The Fog Be
glFogf(GL_FOG_DENSITY, 0.0015f);              
// Fog Hint Value
glHint(GL_FOG_HINT, GL_DONT_CARE);          
// Fog Start Depth
glFogf(GL_FOG_START, 1.0f);             
// Fog End Depth
glFogf(GL_FOG_END, 5.0f);               
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius r
 */
static void fireball()
{


  // float yellow[] = {1.0,1.0,0.0,1.0};
    float Emission[]  = {0.1,0.1,0,1.0,0.1};

   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glScaled(1,1,1);
   glTranslated(Fx,Fy,Fz);

   //  White ball
   glColor3f(1.0f,1.0f,1.0f);

   // glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   //glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);



   glutSolidSphere(10.0,16,16);

   //printf("fireball called at x:%f y:%f z:%f\n Ex: %f Ey:%f, Ez: %f\n", Fx,Fy,Fz,Ex,Ey,Ez);
   // printf("fireball called at x:%f y:%f z:%f\n Ex: %f Ey:%f, Ez: %f\n", x,y,z,Ex,Ey,Ez);
   //  Undo transofrmations

   glPopMatrix();
}





void display()
{
   //  Erase the window and the depth buffer
   glClearColor(0,0.3,0.7,0);
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   //  Undo previous transformations
   glLoadIdentity();
   
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();

   Rotate(Tilt, Spin, Twist);
   glTranslated(Ex,Ey,Ez);



   //  Light switch
   if (firebool)
   {

      //  Translate intensity to color vectors
      //float Ambient[]   = {0.3f*F,0.3f*F,0.3f*F,1.0f};
      float F=2;
      float Diffuse[]   = {0.5f*F,0.5f*F,0.5f*F,1.0f}; //high diffuse
      float Specular[]  = {1.0f*F,1.0f*F,0.5f*F,1}; //high specular
      float white[]     = {1,1,1,1};
      //  Light direction
      
      //  Draw light position as ball (still no lighting here)
      //fireball(Position[0],Position[1],Position[2] , 0.1);
      //  Enable lighting with normalization
      float positionf[4]  = {(float)Fx, (float)Fy, (float)Fz,1.0f};
      glEnable(GL_LIGHTING);
      glEnable(GL_NORMALIZE);
      //  glColor sets ambient and diffuse color materials
      //glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      //glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
    
      // glLightfv(GL_LIGHT1,GL_AMBIENT ,Ambient); No ambiant light
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,positionf);
      //glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,0.1f); //no shinyness
      glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);

      glEnable(GL_COLOR_MATERIAL);

      //smooth?

      //  Set attenuation
      // glLightf(GL_LIGHT1,GL_CONSTANT_ATTENUATION ,100/100.0);
      // glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION   ,100/100.0);
      glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,.0001);//0.01/100.0);
      glEnable(GL_LIGHT0);
      //firebool = false;
      
   }
   else{
      glDisable(GL_LIGHTING);
      glDisable(GL_LIGHT0);
      glEnable(GL_LIGHTING);
      glEnable(GL_NORMALIZE);
      //turn off ambient light
      GLfloat global_ambient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
   }

      

   //build landscape
   DEM();
   //Add fog
   Fog();
   //Create sky box, increase size by 500
   Sky(1000.0*dim);

   //shootFire(Ex,Ey,Ez,Spin,Tilt);
   //Remove parameters 
   if(firebool){fireball();}

   //  Display parameters
   glWindowPos2i(5,5);

   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}


//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
void loadTerrain(const char *filename) 
{
  int width,height,channels;
  stbi_uc *image = stbi_load(filename, &width, &height, &channels, STBI_grey);

  if (!image) Fatal("Cannot load image %s\n",filename);

  if ((width * height) != (1081 * 1081))
  {
    printf("Incorrect width and height");
  }

  //process data if not NULL
  //x = width, y = height, n = # 8-bit components per pixel 
  //replace '0' with '1'..'4' to force that many components per pixel
  //but 'n' will always be the number that it would have been if you said 0

  memcpy(gHeightMap, image, (1081 * 1081));

  stbi_image_free(image);
  return;
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
  //  Exit on ESC
  if (ch == 27)
  {
    exit(0);
  }
  //  Reset view angle
  else if (ch == '0')
       Ex = Ey = Ez = 1;   //  Eye

  //fireball
  else if (ch == ' ')
  {
       //glTranslated(Ex,Ey,Ez);
       Fspin = Spin;
       Ftilt = Tilt;
       Fx=-Ex-sin(degreesToRadians(-Fspin))*50;
       Fz=-Ez-cos(degreesToRadians(-Fspin))*50;
       Fy=-Ey-tan(degreesToRadians(Ftilt))*50;
       firebool = true;

       //printf("Fx: %f\n vs Ex:%f\n Fy: %f\n vs Ey:%f\n Fz: %f\n vs Ez:%f\n", Fx,Ex,Fy,Ey,Fz,Ez);
      
      // firebool=!firebool;
  }
  //  Forward
  else if (ch == 'w' || ch == 'W')
  {
    Ex += sin(degreesToRadians(-Spin))*5;
    Ez += cos(degreesToRadians(-Spin))*5;
    Ey += tan(degreesToRadians(Tilt))*5;
    //Ey -= 1.5;
  }

  // Backward
  else if (ch == 's' || ch == 'S')
  {
    Ex -= sin(degreesToRadians(-Spin));
    Ez -= cos(degreesToRadians(-Spin));
    Ey -= tan(degreesToRadians(Tilt));
    //Ey += 1.5;
  }
  // Strafe Left
  else if (ch == 'a' || ch == 'A')
  {
    Ex +=  cos(degreesToRadians(-Spin));
    Ez += -sin(degreesToRadians(-Spin));

    //Ex += 1.5;
  }
  // Strafe Right
  else if (ch == 'd' || ch == 'D')
  {
    Ex -=  cos(degreesToRadians(-Spin));
    Ez -= -sin(degreesToRadians(-Spin));

    //Ex -= 1.5;
  }
  else if (ch == 'q' || ch == 'Q')
  {
    Twist -= 1.5;
    //Spin -= 1.5;
  }
  else if (ch == 'e' || ch == 'E')
  {
    Twist += 1.5;
    //Spin += 1.5;
  }
  // else if (ch == 'x' || ch == 'X')
  // {
  //   Ez -= 1.5;
  // }
  // else if (ch == 'z' || ch == 'Z')
  // {
  //   Ez += 1.5;
  // }
  else if (ch == 'x' || ch == 'X')
  {
    Tilt -= 1.5;
  }
  else if (ch == 'z' || ch == 'Z')
  {
    Tilt += 1.5;
  }

  else if (ch == '1')
  {
    Spin -= 1.5;
  }
  else if (ch == '2')
  {
    Spin += 1.5;
  }

  //  Reproject
  //Project(fov,asp, near, far);
  //glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */

void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(fov,asp, near, far);
}

void Timer(int aTime)
{
  Ex += sin(degreesToRadians(-Spin))  * 2;
  Ez += cos(degreesToRadians(-Spin))  * 2;
  Ey += tan(degreesToRadians(Tilt)) * 2;

  if(firebool){
    Fx -= sin(degreesToRadians(-Fspin)) * 10;
    Fz -= cos(degreesToRadians(-Fspin)) * 10;
    Fy -= tan(degreesToRadians(Ftilt)) * 10;

    //calculate the distance between the camera and the fireball 
    //delete the fireball after you don't see it.
    if ((sqrt(pow((-Fx-Ex),2)+pow((-Fy-Ey),2)+pow((-Fz-Ez),2))) > 2000){
      firebool=false;
    }
  } 
  //printf("firebool=%d\n", firebool);

  //  Reproject
  Project(fov, asp, near, far);
  glutPostRedisplay();
  glutTimerFunc(16, Timer, 0);
  
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(1000,600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("Final Project: Paige Johnson");
   
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   
   //  Tell GLUT to call "special" when an arrow key is pressed
   //glutSpecialFunc(special);
   
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   
   //  Pass control to GLUT so it can interact with the user
   glutMouseFunc(mouseButton);
   glutMotionFunc(mouseMotion);
  
   glutTimerFunc(16, Timer, 0);
   


  //Load Height Map
  loadTerrain("WulingyuanFar.png");
   
   
   SetUpTriangles();
   //Load Textures
   sky[0] = LoadTexBMP("sky0.bmp");
   sky[1] = LoadTexBMP("sky1.bmp");
   map[0] = LoadTexBMP("WulingyuanFar.bmp");

   map[1] = LoadTexBMP("snow.bmp");


   ErrCheck("init");




   glutMainLoop();

   return 0;
}