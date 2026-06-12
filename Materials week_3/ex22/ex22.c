/*
 *  More Lighting
 *
 *  Demonstrates spotlights, lighting near large objects, two sided lighting
 *  and similar advanced lighting techniques.
 *
 *  Key bindings:
 *  l/L        Toggle lighting on/off
 *  t/T        Toggle textures on/off
 *  p/P        Toggle projection between orthogonal/perspective
 *  b/B        Toggle display of quads
 *  +/-        Increase/decrease number of quads
 *  F1         Toggle smooth/flat shading
 *  F2         Toggle local viewer mode on/off
 *  F3         Toggle two sided mode on/off
 *  'i'        Toggle light at infinity
 *  a/A        Decrease/increase ambient light
 *  d/D        Decrease/increase diffuse light
 *  s/S        Decrease/increase specular light
 *  e/E        Decrease/increase emitted light
 *  n/N        Decrease/increase shininess
 *  []         Decrease/increase spot direction
 *  {}         Decrease/increase spot cutoff
 *  1/!        Decrease/increase constant attenuation
 *  2/@        Decrease/increase linear attenuation
 *  3/#        Decrease/increase quadratic attenuation
 *  x/X        Decrease/increase light X-position
 *  y/Y        Decrease/increase light Y-position
 *  z/Z        Decrease/increase light Z-position
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include "CSCIx229.h"
#include "wall.h"
int axes=0;       //  Display axes
int mode=1;       //  Projection mode
int side=0;       //  Two sided mode
int ntex=1;       //  Texture mode
int th=0,ph=0;    //  View angles
int Th=0,Ph=30;   //  Light angles
float sco=180;    //  Spot cutoff angle
float Exp=0;      //  Spot exponent
int at0=100;      //  Constant  attenuation %
int at1=0;        //  Linear    attenuation %
int at2=0;        //  Quadratic attenuation %
int fov=53;       //  Field of view (for perspective)
int light=1;      //  Lighting
double asp=1;     //  Aspect ratio
double dim=8;     //  Size of world
// Light values
int num       =   1;  // Number of quads
int inf       =   0;  // Infinite distance light
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =   0;  // Ambient intensity (%)
int diffuse   = 100;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float X       = 0;    // Light X position
float Y       = 0;    // Light Y position
float Z       = 1;    // Light Z position

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   Wall(mode,side,ntex,axes,light,
        th,ph,dim,Th,Ph,X,Y,Z,
        emission,ambient,diffuse,specular,shininess,
        sco,Exp,at0,at1,at2,
        num,inf,smooth,local);
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   Th = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Increase/decrease asimuth
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Increase/decrease elevation
   else if (key == GLUT_KEY_UP)
      ph += 5;
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Smooth color model
   else if (key == GLUT_KEY_F1)
      smooth = 1-smooth;
   //  Local Viewer
   else if (key == GLUT_KEY_F2)
      local = 1-local;
   //  Two sided mode
   else if (key == GLUT_KEY_F3)
      side = 1-side;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(mode?fov:0,asp,dim);
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
      X = Y = th = ph = 0;
   //  Toggle axes
   else if (ch == 'b' || ch == 'B')
      axes = 1-axes;
   //  Toggle textures
   else if (ch == 't' || ch == 'T')
      ntex = 1-ntex;
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
   //  Toggle infinity
   else if (ch == 'i' || ch == 'I')
      inf = 1-inf;
   //  Switch projection mode
   else if (ch == 'p' || ch == 'P')
      mode = 1-mode;
   //  Change number of quadrangles
   else if (ch == '-' && num>1)
      num--;
   else if (ch == '+' && num<100)
      num++;
   //  Increase/decrease spot azimuth
   else if (ch == '[')
      Ph -= 5;
   else if (ch == ']')
      Ph += 5;
   //  Increase/decrease spot cutoff angle
   else if (ch == '{' && sco>5)
      sco = sco==180 ? 90 : sco-5;
   else if (ch == '}' && sco<180)
      sco = sco<90 ? sco+5 : 180;
   //  Change spot exponent
   else if (ch == '<')
   {
      Exp -= 0.1;
      if (Exp<0) Exp=0;
   }
   else if (ch == '>')
      Exp += 0.1;
   //  Change constant attenuation
   else if (ch == '1' && at0>0)
      at0--;
   else if (ch == '!')
      at0++;
   //  Change linear attenuation
   else if (ch == '2' && at1>0)
      at1--;
   else if (ch == '@')
      at1++;
   //  Change quadratic attenuation
   else if (ch == '3' && at2>0)
      at2--;
   else if (ch == '#')
      at2++;
   //  Light position
   else if (ch=='x')
      X -= 0.1;
   else if (ch=='X')
      X += 0.1;
   else if (ch=='y')
      Y -= 0.1;
   else if (ch=='Y')
      Y += 0.1;
   else if (ch=='z')
      Z -= 0.1;
   else if (ch=='Z')
      Z += 0.1;
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
   //  Wrap angles
   Th = Th%360;
   Ph = Ph%360;
   //  Reproject
   Project(mode?fov:0,asp,dim);
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
   Project(mode?fov:0,asp,dim);
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
   glutCreateWindow("More Lighting");
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
   LoadTexBMP("brick.bmp");
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
