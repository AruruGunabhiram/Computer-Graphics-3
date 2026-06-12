/*
 * Draw the wall for Example 21
 */
#include "CSCIx229.h"
#include "wall.h"

/*
 *  Draw vertex in polar coordinates with normal
 */
static void Vertex(double th,double ph)
{
   glVertex3d(Sin(th)*Cos(ph),Cos(th)*Cos(ph),Sin(ph));
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
static void ball(double x,double y,double z,double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  Bands of latitude
   glColor3f(1,1,0);
   for (int ph=-90;ph<90;ph+=10)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th=0;th<=360;th+=20)
      {
         Vertex(th,ph);
         Vertex(th,ph+10);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}

/*
 *  Draw the wall
 *    mode      Projection mode (1=perspective,0=orthogonal)
 *    side      Two sided lighting (true/false)
 *    ntex      Enable textures (true/false)
 *    axes      Draw axes (true/false)
 *    light     Enable lighting (true/false)
 *    th        View azimuth (degrees)
 *    ph        View elevation (degrees)
 *    dim       Dimension of scene
 *    Th        Spotlight azimuth (degrees)
 *    Ph        Spotlight elevation (degrees)
 *    X         Spotlight X
 *    Y         Spotlight Y
 *    Z         Spotlight Z
 *    emission  Emission light intensity (0-100)
 *    ambient   Ambient  light intensity (0-100)
 *    diffuse   Diffuse  light intensity (0-100)
 *    specular  Specular light intensity (0-100)
 *    shininess Shininess (power of two)
 *    sco       Spotlight Cut-off Angle
 *    Exp       Spotlight exponent
 *    at0       Light Constant  Attenuation (0-100)
 *    at1       Light Linear    Attenuation (0-100)
 *    at2       Light Quadratic Attenuation (0-100)
 *    num       Number of quads on wall
 *    inf       Light at infinity (0 or 1)
 *    smooth    Gouraud interpolation (true/false)
 *    local     Local light mode (true/false)
 */
void Wall(int mode,int side,int ntex,int axes,int light,
           int th,int ph,double dim,int Th,int Ph,float X,float Y,float Z,
           int emission,int ambient,int diffuse,int specular,int shininess,
           float sco,float Exp,int at0,int at1,int at2,
           int num,int inf,int smooth,int local)
{
   //  Light position
   float Position[] = {X+Cos(Th),Y+Sin(Th),Z,1-inf};
   //  Translate shininess power to value (-1 => 0)
   float shiny = shininess<0 ? 0 : pow(2.0,shininess);
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective - set eye position
   if (mode)
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }
   //  Orthogonal - set world orientation
   else
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }
   glShadeModel(smooth?GL_SMOOTH:GL_FLAT);
   //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
      float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
      //  Spotlight color and direction
      float yellow[] = {1.0,1.0,0.0,1.0};
      float Direction[] = {Cos(Th)*Sin(Ph),Sin(Th)*Sin(Ph),-Cos(Ph),0};
      //  Draw light position as ball (still no lighting here)
      ball(Position[0],Position[1],Position[2] , 0.1);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  Location of viewer for specular calculations
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
      //  Two sided mode
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,side);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Set specular colors
      glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
      glMaterialf(GL_FRONT,GL_SHININESS,shiny);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
      //  Set spotlight parameters
      glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,Direction);
      glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,sco);
      glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,Exp);
      //  Set attenuation
      glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION ,at0/100.0);
      glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION   ,at1/100.0);
      glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,at2/100.0);
   }
   else
      glDisable(GL_LIGHTING);
   //  Enable textures
   if (ntex)
      glEnable(GL_TEXTURE_2D);
   else
      glDisable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

   //  Draw the wall size -5 to +5
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(1,1);
   double mul = 2.0/num;
   glPushMatrix();
   glTranslated(-5,-5,0);
   glScaled(5*mul,5*mul,1);
   glColor3f(1.0,1.0,1.0);
   glBegin(GL_QUADS);
   glNormal3f(0,0,1); 
   for (int i=0;i<num;i++)
      for (int j=0;j<num;j++)
      {
         glTexCoord2d(mul*(i+0),mul*(j+0)); glVertex2d( i , j );
         glTexCoord2d(mul*(i+1),mul*(j+0)); glVertex2d(i+1, j );
         glTexCoord2d(mul*(i+1),mul*(j+1)); glVertex2d(i+1,j+1);
         glTexCoord2d(mul*(i+0),mul*(j+1)); glVertex2d( i ,j+1);
      }
   glEnd();
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_POLYGON_OFFSET_FILL);
   glPopMatrix();

   //  Draw axes - no lighting from here on
   glDisable(GL_LIGHTING);
   glColor3f(1,1,1);
   if (axes)
   {
      const double len=2.0;  //  Length of axes
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
      //  Show quads
      glPushMatrix();
      glTranslated(-5,-5,0);
      glScaled(5*mul,5*mul,1);
      glBegin(GL_LINES);
      for (int i=0;i<=num;i++)
      {
         glVertex2d(i, 0 );
         glVertex2d(i,num);
      }
      for (int j=0;j<=num;j++)
      {
         glVertex2d( 0 ,j);
         glVertex2d(num,j);
      }
      glEnd();
      glPopMatrix();
   }
   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f Projection=%s Light=%s",
     th,ph,dim,mode?"Perpective":"Orthogonal",light?"On":"Off");
   if (light)
   {
      glWindowPos2i(5,65);
      Print("Cutoff=%.0f Exponent=%.1f Direction=%d,%d Attenuation=%.2f,%.2f,%.2f",sco,Exp,Th,Ph,at0/100.0,at1/100.0,at2/100.0);
      glWindowPos2i(5,45);
      Print("Model=%s LocalViewer=%s TwoSided=%s Position=%.1f,%.1f,%.1f,%.1f Num=%d",smooth?"Smooth":"Flat",local?"On":"Off",side?"On":"Off",Position[0],Position[1],Position[2],Position[3],num);
      glWindowPos2i(5,25);
      Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d Shininess=%.0f",ambient,diffuse,specular,emission,shiny);
   }
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
}

