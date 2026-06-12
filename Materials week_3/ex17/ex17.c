/*
 *  Axi-symmetric object for lighting and textures.
 *
 *  Key bindings:
 *  l          Toggle lighting on/off
 *  m          Toggles textures on/off
 *  a/A        decrease/increase ambient light
 *  d/D        decrease/increase diffuse light
 *  s/S        decrease/increase specular light
 *  e/E        decrease/increase emitted light
 *  n/N        Decrease/increase shininess
 *  []         Lower/rise light
 *  x          Toggle axes
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include "CSCIx229.h"
int mode=1;       //  Texture mode
int axes=1;       //  Display axes
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int light=1;      //  Lighting
double asp=1;     //  Aspect ratio
double dim=3.0;   //  Size of world
// Light values
int emission  =   0;  // Emission intensity (%)
int ambient   =  30;  // Ambient intensity (%)
int diffuse   =  50;  // Diffuse intensity (%)
int specular  =  50;  // Specular intensity (%)
int shininess =   5;  // Shininess (power of two)
float shiny   =  32;    // Shininess (value)
int zh        =   0;  // Light azimuth
float ylight  =   1;  // Elevation of light

//  2D vector
typedef struct {float x,y;} vec2;
//  Pawn outline
#define Np 50
vec2 P[Np] = {
   {0.235,0.092},
   {0.248,0.108},
   {0.257,0.125},
   {0.261,0.144},
   {0.259,0.169},
   {0.248,0.195},
   {0.233,0.210},
   {0.213,0.224},
   {0.191,0.243},
   {0.180,0.258},
   {0.172,0.276},
   {0.166,0.294},
   {0.156,0.302},
   {0.146,0.307},
   {0.131,0.311},
   {0.125,0.329},
   {0.115,0.369},
   {0.105,0.411},
   {0.099,0.465},
   {0.096,0.526},
   {0.096,0.578},
   {0.099,0.599},
   {0.104,0.606},
   {0.139,0.613},
   {0.174,0.621},
   {0.190,0.628},
   {0.197,0.638},
   {0.202,0.652},
   {0.200,0.667},
   {0.192,0.676},
   {0.180,0.680},
   {0.162,0.685},
   {0.146,0.690},
   {0.134,0.693},
   {0.123,0.696},
   {0.120,0.701},
   {0.130,0.713},
   {0.142,0.731},
   {0.153,0.750},
   {0.164,0.778},
   {0.172,0.814},
   {0.171,0.846},
   {0.164,0.880},
   {0.148,0.918},
   {0.120,0.951},
   {0.085,0.976},
   {0.053,0.991},
   {0.023,0.998},
   {0.000,1.000},
  };

//  Make vec2 unit length
vec2 normalize(float x,float y)
{
   float len = sqrt(x*x+y*y);
   if (len>0)
   {
      x /= len;
      y /= len;
   }
   return (vec2){x,y};
}

//  Calculate normals for Pawn
vec2 N[Np];
void CalcNorm(void)
{
   //  Calculate normals for each facet
   //  y increases with point index so dy>0
   for (int i=0;i<Np-2;i++)
   {
      // Vector in the plane of the facet
      float dx = P[i+1].x - P[i].x;
      float dy = P[i+1].y - P[i].y;
      // Normal is perpendicular
      // dy>0 so normal faces out
      N[i] = normalize(dy,-dx);
   }
   //  The normal at the last point on the pawn is straight up
   //  This is NOT true in general
   N[Np-1] = (vec2){0,1};

   //  Average normals of adjacent facets
   //  First and last normal unchanged
   vec2 N2 = N[0];
   for (int i=1;i<Np-2;i++)
   {
      vec2 N1 = N2;
      N2 = N[i];
      N[i] = normalize(N1.x+N2.x , N1.y+N2.y);
   }
}

/*
 *  Draw the pawn
 *     at (x,y,z) size s
 */
static void Pawn(double x,double y,double z,  double s)
{
   int inc=15;
   //  Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glColor4fv(white);
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(s,s,s);
   //  Draw bottom
   glColor3f(0.0,0.5,0.0);
   glBegin(GL_TRIANGLE_FAN);
   glNormal3f(0,-1,0);
   glVertex3f(0,0,0);
   for (int th=0;th<=360;th+=inc)
      glVertex3f(Cos(th)*P[0].x , 0 , Sin(th)*P[0].x);
   glEnd();
   //  Draw cylindrical base
   glColor4fv(white);
   if (mode) glEnable(GL_TEXTURE_2D);
   glBegin(GL_QUAD_STRIP);
   for (int th=0;th<=360;th+=inc)
   {
      float c = Cos(th);
      float s = Sin(th);
      glNormal3f(c,0,s);
      glTexCoord2f(th/360.0 ,   0   ); glVertex3f(c*P[0].x ,   0    , s*P[0].x);
      glTexCoord2f(th/360.0 , P[0].y); glVertex3f(c*P[0].x , P[0].y , s*P[0].x);
   }
   glEnd();
   //  Draw the curved part of the pawn
   for (int i=0;i<Np-1;i++)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th=0;th<=360;th+=inc)
      {
         float c = Cos(th);
         float s = Sin(th);
         glTexCoord2f(th/360.0 ,P[i].y);
         glNormal3f(c*N[i].x,N[i].y,s*N[i].x);
         glVertex3f(c*P[i].x,P[i].y,s*P[i].x);

         glTexCoord2f(th/360.0   ,P[i+1].y);
         glNormal3f(c*N[i+1].x,N[i+1].y,s*N[i+1].x);
         glVertex3f(c*P[i+1].x,P[i+1].y,s*P[i+1].x);
      }
      glEnd();
   }
   if (mode) glDisable(GL_TEXTURE_2D);
   //  Undo transformations and textures
   glPopMatrix();
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius r
 */
static void ball(double x,double y,double z,double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glColor3f(1,1,1);
   glutSolidSphere(1.0,16,16);
   //  Undo transofrmations
   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   //  Eye position
   double Ex = -2*dim*Sin(th)*Cos(ph);
   double Ey = +2*dim        *Sin(ph);
   double Ez = +2*dim*Cos(th)*Cos(ph);
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Set perspective
   glLoadIdentity();
   gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
      float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
      //  Light direction
      float Position[]  = {5*Cos(zh),ylight,5*Sin(zh),1};
      //  Draw light position as ball (still no lighting here)
      glColor3f(1,1,1);
      ball(Position[0],Position[1],Position[2] , 0.1);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else
      glDisable(GL_LIGHTING);
   //  Draw scene
   Pawn(0,-0.5,0 , 2.0);
   
   //  Draw axes - no lighting from here on
   glDisable(GL_LIGHTING);
   glColor3f(1,1,1);
   if (axes)
   {
      const double len=2.0; //  Length of axes
      glBegin(GL_LINES);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(len,0.0,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,len,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,0.0,len);
      glEnd();
      //  Label axes
      glRasterPos3d(len,0.0,0.0);
      Print("X");
      glRasterPos3d(0.0,len,0.0);
      Print("Y");
      glRasterPos3d(0.0,0.0,len);
      Print("Z");
   }
   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f Light=%s Texture=%s",th,ph,dim,light?"On":"Off",mode?"On":"Off");
   if (light)
   {
      glWindowPos2i(5,25);
      Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d Shininess=%.0f",ambient,diffuse,specular,emission,shiny);
   }
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(45,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Toggle Gouraud interpolation
   else if (ch == 'm' || ch == 'M')
      mode = 1-mode;
   //  Toggle axes
   else if (ch == 'x' || ch == 'X')
      axes = 1-axes;
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
   //  Light elevation
   else if (ch=='[')
      ylight -= 0.1;
   else if (ch==']')
      ylight += 0.1;
   //  Ambient level
   else if (ch=='a' && ambient>0)
      ambient -= 5;
   else if (ch=='A' && ambient<100)
      ambient += 5;
   //  Diffuse level
   else if (ch=='d' && diffuse>0)
      diffuse -= 5;
   else if (ch=='D' && diffuse<100)
      diffuse += 5;
   //  Specular level
   else if (ch=='s' && specular>0)
      specular -= 5;
   else if (ch=='S' && specular<100)
      specular += 5;
   //  Emission level
   else if (ch=='e' && emission>0)
      emission -= 5;
   else if (ch=='E' && emission<100)
      emission += 5;
   //  Shininess level
   else if (ch=='n' && shininess>-1)
      shininess -= 1;
   else if (ch=='N' && shininess<7)
      shininess += 1;
   //  Translate shininess power to value (-1 => 0)
   shiny = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project(45,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
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
   Project(45,asp,dim);
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow("Axisymmetric Objects");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   //  Compute normals for pawn
   CalcNorm();
   //  Load texture
   LoadTexBMP("wood.bmp");
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
