/*
 *  Draped Textures
 *
 *  Combines a large texture with a quadrangles to make a simple terrain viewer.
 *
 *  Note that when we exaggerate the Z direction, the normals are not changed.
 *  With Z exaggeration the normals are no longer perpendicular to the surface, 
 *  but the goal is to do lighting on the "true" Z values, and just exaggerate
 *  the Z scale for visual effect without updating the lighting calculations.
 *
 *  Key bindings:
 *  m           Cycle display modes
 *  l           Toggle lighting
 *  +/-         Increase/decrease vertical exaggeration
 *  a           Toggle axes
 *  arrows      Change view angle
 *  PgDn/PgUp   Zoom in and out
 *  LeftMouse   Pan horizontally
 *  RightMouse  Pan vertically
 *  0           Reset view angle
 *  ESC         Exit
 */
#include "CSCIx229.h"
//  State variables
int mode=0;            //  Display mode
int light=0;           //  Lighting
int axes=1;            //  Display axes
int th=0;              //  Azimuth of view angle
int ph=90;             //  Elevation of view angle
float zh=0;            //  Light azimuth
double asp=1;          //  Aspect ratio
int dim=500;           //  Size of world
double Ox=0,Oy=0,Oz=0; //  LookAt Location
int move=0;            //  Move mode
int X=0,Y=0;           //  Last mouse location
int d=16;              //  Grid spacing
int D=512;             //  Grid size (half)
float z[65][65];       //  DEM data
float zmin=+1e8;       //  DEM lowest location
float zmax=-1e8;       //  DEM highest location
float zmag=1;          //  DEM magnification
float N[65][65][3];    //  DEM normals

/*
 *  Draw scene
 */
void DEM()
{
   double z0 = (zmin+zmax)/2;
   //  Apply texture to one large quad
   if (mode==0)
   {
      glColor3f(1,1,1);
      glEnable(GL_TEXTURE_2D);
      glBegin(GL_QUADS);
      glTexCoord2f(0,0); glVertex2d(-D,-D);
      glTexCoord2f(1,0); glVertex2d(+D,-D);
      glTexCoord2f(1,1); glVertex2d(+D,+D);
      glTexCoord2f(0,1); glVertex2d(-D,+D);
      glEnd();
      glDisable(GL_TEXTURE_2D);
   }
   //  Show DEM wire frame
   else if (mode==1)
   {
      glColor3f(1,1,0);
      for (int i=0;i<64;i++)
         for (int j=0;j<64;j++)
         {
            float x=d*i-D;
            float y=d*j-D;
            glBegin(GL_LINE_LOOP);
            glVertex3d(x+0,y+0,zmag*(z[i+0][j+0]-z0));
            glVertex3d(x+d,y+0,zmag*(z[i+1][j+0]-z0));
            glVertex3d(x+d,y+d,zmag*(z[i+1][j+1]-z0));
            glVertex3d(x+0,y+d,zmag*(z[i+0][j+1]-z0));
            glEnd();
         }
      if (light)
      {
         glColor3f(1,0,0);
         glBegin(GL_LINES);
         for (int i=0;i<=64;i++)
            for (int j=0;j<=64;j++)
            {
               float x=d*i-D;
               float y=d*j-D;
               glVertex3d(x,y,zmag*(z[i][j]-z0));
               glVertex3d(x+N[i][j][0],y+N[i][j][1],zmag*(z[i][j]-z0)+N[i][j][2]);
            }
         glEnd();
      }
   }
   //  Apply texture to DEM wireframe
   else
   {
      if (light)
      {
         float black[4] = {0.0,0.0,0.0,1.0};
         float white[4] = {1.0,1.0,1.0,1.0};
         float dark[4]  = {0.2,0.2,0.2,1.0};
         float light[4] = {0.8,0.8,0.8,1.0};
         float lpos[4]  = {Cos(zh),-1,Sin(zh),0};
         glEnable(GL_LIGHTING);
         glEnable(GL_LIGHT0);
         glEnable(GL_NORMALIZE);
         glLightfv(GL_LIGHT0,GL_AMBIENT ,dark);
         glLightfv(GL_LIGHT0,GL_DIFFUSE ,light);
         glLightfv(GL_LIGHT0,GL_SPECULAR,black);
         glLightfv(GL_LIGHT0,GL_POSITION,lpos);
         glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,white);;
         glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,black);;
      }
      else
         glColor3f(1,1,1);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);
      for (int i=0;i<64;i++)
         for (int j=0;j<64;j++)
         {
            float x=d*i-D;
            float y=d*j-D;
            glBegin(GL_QUADS);
            glNormal3fv(N[i+0][j+0]); glTexCoord2f((i+0)/64.,(j+0)/64.); glVertex3d(x+0,y+0,zmag*(z[i+0][j+0]-z0));
            glNormal3fv(N[i+1][j+0]); glTexCoord2f((i+1)/64.,(j+0)/64.); glVertex3d(x+d,y+0,zmag*(z[i+1][j+0]-z0));
            glNormal3fv(N[i+1][j+1]); glTexCoord2f((i+1)/64.,(j+1)/64.); glVertex3d(x+d,y+d,zmag*(z[i+1][j+1]-z0));
            glNormal3fv(N[i+0][j+1]); glTexCoord2f((i+0)/64.,(j+1)/64.); glVertex3d(x+0,y+d,zmag*(z[i+0][j+1]-z0));
            glEnd();
         }
      glDisable(GL_CULL_FACE);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_LIGHTING);
   }
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   //  Eye position
   double Ex = Ox-2*dim*Sin(th)*Cos(ph);
   double Ey = Oy+2*dim        *Sin(ph);
   double Ez = Oz+2*dim*Cos(th)*Cos(ph);
   //  Erase the window and the depth buffer
   glClearColor(0.3,0.5,1.0,1);
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Set perspective
   glLoadIdentity();
   gluLookAt(Ex,Ey,Ez , Ox,Oy,Oz , 0,1,0);
   //  Draw scene
   glPushMatrix();
   glRotated(-90,1,0,0);
   DEM();
   glPopMatrix();
   //  Draw axes - no lighting from here on
   glColor3f(1,1,1);
   if (axes)
   {
      const double len=100;
      glBegin(GL_LINES);
      glVertex3d(Ox,Oy,Oz);
      glVertex3d(Ox+len,Oy,Oz);
      glVertex3d(Ox,Oy,Oz);
      glVertex3d(Ox,Oy+len,Oz);
      glVertex3d(Ox,Oy,Oz);
      glVertex3d(Ox,Oy,Oz+len);
      glEnd();
      //  Label axes
      glRasterPos3d(Ox+len,Oy,Oz);
      Print("X");
      glRasterPos3d(Ox,Oy+len,Oz);
      Print("Y");
      glRasterPos3d(Ox,Oy,Oz+len);
      Print("Z");
   }
   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%d  Vertical Magnification=%.1f  Mouse=%d,%d",th,ph,dim,zmag,X,Y);
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
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
   else if (key == GLUT_KEY_UP && ph<90)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN && ph>0)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 10;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>10)
      dim -= 10;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(60,asp,dim);
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
   {
      th =  0;
      ph = 90;
      Ox = Oy = Oz = 0;
   }
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = !light;
   //  Toggle mode
   else if (ch == 'm')
      mode = (mode+1)%3;
   else if (ch == 'M')
      mode = (mode+2)%3;
   //  Toggle axes
   else if (ch == 'a' || ch == 'A')
      axes = 1-axes;
   //  Vertical magnification
   else if (ch == '+')
      zmag += 0.1;
   else if (ch == '-' && zmag>1)
      zmag -= 0.1;
   //  Reproject
   Project(60,asp,dim);
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
   Project(60,asp,dim);
}

/*
 *  GLUT calls this routine when a mouse is moved
 */
void motion(int x,int y)
{
   //  Do only when move is set
   //  WARNING:  this only works because by coincidence 1m = 1pixel
   if (move)
   {
      //  Up/down movement
      if (move<0)
         Oy -= Y-y;
      //  Pan movement
      else
      {
         Ox += Cos(th)*(X-x) - Sin(th)*(Y-y);
         Oz += Sin(th)*(X-x) + Cos(th)*(Y-y);
      }
      //  Remember location
      X = x;
      Y = y;
      glutPostRedisplay();
   }
}

/*
 *  GLUT calls this routine when a mouse button is pressed or released
 */
void mouse(int key,int status,int x,int y)
{
   //  On button down, set 'move' and remember location
   if (status==GLUT_DOWN)
   {
      move = (key==GLUT_LEFT_BUTTON) ? 1 : -1;
      X = x;
      Y = y;
      glutPostRedisplay();
   }
   //  On button up, unset move
   else if (status==GLUT_UP)
      move = 0;
}

/*
 *  Read DEM from file
 */
void ReadDEM(char* file)
{
   int i,j;
   FILE* f = fopen(file,"r");
   if (!f) Fatal("Cannot open file %s\n",file);
   for (j=0;j<=64;j++)
      for (i=0;i<=64;i++)
      {
         if (fscanf(f,"%f",&z[i][j])!=1) Fatal("Error reading saddleback.dem\n");
         if (z[i][j] < zmin) zmin = z[i][j];
         if (z[i][j] > zmax) zmax = z[i][j];
      }
   fclose(f);
   // Compute normal
   for (j=0;j<=64;j++)
      for (i=0;i<=64;i++)
      {
         //  X component is half Zw-Ze
         if (i==0)
           N[i][j][0] = z[i][j]-z[i+1][j];
         else if (i==64)
           N[i][j][0] = z[i-1][j]-z[i][j];
         else
           N[i][j][0] = (z[i-1][j]-z[i+1][j])/2;
         //  Y component is half Zs-Zn
         if (j==0)
           N[i][j][1] = z[i][j]-z[i][j+1];
         else if (j==64)
           N[i][j][1] = z[i][j-1]-z[i][j];
         else
           N[i][j][1] = (z[i][j-1]-z[i][j+1])/2;
         //  Z component is grid size
         N[i][j][2] = 16;
      }
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(20*t,240.0)-30;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
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
   glutCreateWindow("Draped Textures");
   glutFullScreen();
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutIdleFunc(idle);
   glutKeyboardFunc(key);
   glutMouseFunc(mouse);
   glutMotionFunc(motion);
   //  Load texture
   LoadTexBMP("saddleback.bmp");
   //  Load DEM
   ReadDEM("saddleback.dem");
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
