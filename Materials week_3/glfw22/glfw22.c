/*
 *  More Lighting
 *
 *  Demonstrates spotlights, lighting near large objects, two sided lighting
 *  and similar advanced lighting techniques using GLFW.
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
 *  []         Decrease/increase spot directon
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
 *  GLFW calls this routine to display the scene
 */
void display(GLFWwindow* window)
{
   Wall(mode,side,ntex,axes,light,
     th,ph,dim,Th,Ph,X,Y,Z,
     emission,ambient,diffuse,specular,shininess,
     sco,Exp,at0,at1,at2,
     num,inf,smooth,local);
   glfwSwapBuffers(window);
}

/*
 *  Call this routine when a key is pressed
 */
void key(GLFWwindow* window,int key,int scancode,int action,int mods)
{
   //  Discard key releases (keeps PRESS and REPEAT)
   if (action==GLFW_RELEASE) return;

   //  Check for shift
   int shift = (mods & GLFW_MOD_SHIFT);

   //  Exit on ESC
   if (key == GLFW_KEY_ESCAPE)
     glfwSetWindowShouldClose(window,1);
   //  Reset view angle
   else if (key==GLFW_KEY_0)
      X = Y = th = ph = 0;
   //  Toggle axes
   else if (key==GLFW_KEY_B)
      axes = 1-axes;
   //  Toggle textures
   else if (key==GLFW_KEY_T)
      ntex = 1-ntex;
   //  Toggle lighting
   else if (key==GLFW_KEY_L)
      light = 1-light;
   //  Toggle infinity
   else if (key==GLFW_KEY_I)
      inf = 1-inf;
   //  Switch projection mode
   else if (key==GLFW_KEY_P)
      mode = 1-mode;
   //  Change number of quadrangles
   else if ((key==GLFW_KEY_KP_SUBTRACT || key==GLFW_KEY_MINUS) && num>1)
      num--;
   else if ((key==GLFW_KEY_KP_ADD || key==GLFW_KEY_EQUAL) && num<100)
      num++;
   //  Increase/decrease spot azimuth
   else if (key==GLFW_KEY_LEFT_BRACKET && !shift)
      Ph -= 5;
   else if (key==GLFW_KEY_RIGHT_BRACKET && !shift)
      Ph += 5;
   //  Increase/decrease spot cutoff angle
   else if (key==GLFW_KEY_LEFT_BRACKET && shift && sco>5)
      sco = sco==180 ? 90 : sco-5;
   else if (key==GLFW_KEY_RIGHT_BRACKET && shift && sco<180)
      sco = sco<90 ? sco+5 : 180;
   //  Change spot exponent
   else if (key==GLFW_KEY_COMMA)
   {
      Exp -= 0.1;
      if (Exp<0) Exp=0;
   }
   else if (key==GLFW_KEY_PERIOD)
      Exp += 0.1;
   //  Change constant attenuation
   else if (key==GLFW_KEY_1 && !shift && at0>0)
      at0--;
   else if (key==GLFW_KEY_1 && shift)
      at0++;
   //  Change linear attenuation
   else if (key==GLFW_KEY_2 && !shift && at1>0)
      at1--;
   else if (key==GLFW_KEY_2 && shift)
      at1++;
   //  Change quadratic attenuation
   else if (key==GLFW_KEY_3 && !shift && at2>0)
      at2--;
   else if (key==GLFW_KEY_3 && shift)
      at2++;
   //  Light position
   else if (key==GLFW_KEY_X && !shift)
      X -= 0.1;
   else if (key==GLFW_KEY_X && shift)
      X += 0.1;
   else if (key==GLFW_KEY_Y && !shift)
      Y -= 0.1;
   else if (key==GLFW_KEY_Y && shift)
      Y += 0.1;
   else if (key==GLFW_KEY_Z && !shift)
      Z -= 0.1;
   else if (key==GLFW_KEY_Z && shift)
      Z += 0.1;
   //  Ambient level
   else if (key==GLFW_KEY_A && !shift && ambient>0)
      ambient -= 5;
   else if (key==GLFW_KEY_A && shift && ambient<100)
      ambient += 5;
   //  Diffuse level
   else if (key==GLFW_KEY_D && !shift && diffuse>0)
      diffuse -= 5;
   else if (key==GLFW_KEY_D && shift && diffuse<100)
      diffuse += 5;
   //  Specular level
   else if (key==GLFW_KEY_S && !shift && specular>0)
      specular -= 5;
   else if (key==GLFW_KEY_S && shift && specular<100)
      specular += 5;
   //  Emission level
   else if (key==GLFW_KEY_E && !shift && emission>0)
      emission -= 5;
   else if (key==GLFW_KEY_E && shift && emission<100)
      emission += 5;
   //  Shininess level
   else if (key==GLFW_KEY_N && !shift && shininess>-1)
      shininess -= 1;
   else if (key==GLFW_KEY_N && shift && shininess<7)
      shininess += 1;
   //  Increase/decrease asimuth
   else if (key==GLFW_KEY_RIGHT)
      th += 5;
   else if (key==GLFW_KEY_LEFT)
      th -= 5;
   //  Increase/decrease elevation
   else if (key==GLFW_KEY_UP)
      ph += 5;
   else if (key==GLFW_KEY_DOWN)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key==GLFW_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key==GLFW_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Smooth color model
   else if (key==GLFW_KEY_F1)
      smooth = 1-smooth;
   //  Local Viewer
   else if (key==GLFW_KEY_F2)
      local = 1-local;
   //  Two sided mode
   else if (key==GLFW_KEY_F3)
      side = 1-side;
   //  Wrap angles
   Th %= 360;
   Ph %= 360;
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(mode?fov:0,asp,dim);
}

/*
 *  Call this routine when the window is resized
 */
void reshape(GLFWwindow* window,int width,int height)
{
   //  Get framebuffer dimensions (makes Apple work right)
   glfwGetFramebufferSize(window,&width,&height);
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(mode?fov:0,asp,dim);
}

/*
 * GLFW errors
 */
void error(int error, const char* text)
{
    fprintf(stderr,"Error: %s\n",text);
}

/*
 *  Start up GLFW and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLFW
   if (!glfwInit()) Fatal("Cannot initialize glfw\n");
   //  Error callback
   glfwSetErrorCallback(error);
   //  Set window properties
   glfwWindowHint(GLFW_RESIZABLE,1);
   glfwWindowHint(GLFW_DOUBLEBUFFER,1);
   //  Create window and make current
   GLFWwindow* window = glfwCreateWindow(600, 600, "More Lighting - GLFW", NULL, NULL);
   if (!window) Fatal("Cannot create GLFW window\n");
   glfwMakeContextCurrent(window);
#ifdef USEGLEW
   //  Initialize GLEW (if compiled in)
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Enable VSYNC
   glfwSwapInterval(1);
   //  Set callback for window resize
   glfwSetWindowSizeCallback(window,reshape);
   //  Set initial window size
   int width,height;
   glfwGetWindowSize(window,&width,&height);
   reshape(window,width,height);
   //  Set callback for keyboard input
   glfwSetKeyCallback(window,key);

   //  Load textures
   LoadTexBMP("brick.bmp");

   //  Event loop
   ErrCheck("init");
   while(!glfwWindowShouldClose(window))
   {
      //  Elapsed time in seconds
      double t = glfwGetTime();
      Th = fmod(90*t,360.0);
      //  Display
      display(window);
      //  Process any events
      glfwPollEvents();
   }
   //  Shut down GLFW
   glfwDestroyWindow(window);
   glfwTerminate();
   return 0;
}
