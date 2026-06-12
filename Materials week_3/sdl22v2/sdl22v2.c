/*
 *  More Lighting
 *
 *  Demonstrates spotlights, lighting near large objects, two sided lighting
 *  and similar advanced lighting techniques using SDL2.
 *
 *  Key bindings:
 *  space      Play boing sound
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
 *  SDL calls this routine to display the scene
 */
void display(SDL_Window* window)
{
   Wall(mode,side,ntex,axes,light,
     th,ph,dim,Th,Ph,X,Y,Z,
     emission,ambient,diffuse,specular,shininess,
     sco,Exp,at0,at1,at2,
     num,inf,smooth,local);
   SDL_GL_SwapWindow(window);
}

/*
 *  Call this routine when a key is pressed
 *     Returns 1 to continue, 0 to exit
 */
int key(Mix_Chunk* boing)
{
   const Uint8* keys = SDL_GetKeyboardState(NULL);
   int shift = SDL_GetModState()&KMOD_SHIFT;

   //  Exit on ESC
   if (keys[SDL_SCANCODE_ESCAPE])
      return 0;
   //  Reset view angle
   else if (keys[SDL_SCANCODE_0])
      X = Y = th = ph = 0;
   //  Toggle axes
   else if (keys[SDL_SCANCODE_B])
      axes = 1-axes;
   //  Toggle textures
   else if (keys[SDL_SCANCODE_T])
      ntex = 1-ntex;
   //  Toggle lighting
   else if (keys[SDL_SCANCODE_L])
      light = 1-light;
   //  Toggle infinity
   else if (keys[SDL_SCANCODE_I])
      inf = 1-inf;
   //  Switch projection mode
   else if (keys[SDL_SCANCODE_P])
      mode = 1-mode;
   //  Change number of quadrangles
   else if ((keys[SDL_SCANCODE_KP_MINUS] || keys[SDL_SCANCODE_MINUS]) && num>1)
      num--;
   else if ((keys[SDL_SCANCODE_KP_PLUS] || keys[SDL_SCANCODE_EQUALS]) && num<100)
      num++;
   //  Increase/decrease spot azimuth
   else if (keys[SDL_SCANCODE_LEFTBRACKET] && !shift)
      Ph -= 5;
   else if (keys[SDL_SCANCODE_RIGHTBRACKET] && !shift)
      Ph += 5;
   //  Increase/decrease spot cutoff angle
   else if (keys[SDL_SCANCODE_LEFTBRACKET] && shift && sco>5)
      sco = sco==180 ? 90 : sco-5;
   else if (keys[SDL_SCANCODE_RIGHTBRACKET] && shift && sco<180)
      sco = sco<90 ? sco+5 : 180;
   //  Change spot exponent
   else if (keys[SDL_SCANCODE_COMMA])
   {
      Exp -= 0.1;
      if (Exp<0) Exp=0;
   }
   else if (keys[SDL_SCANCODE_PERIOD])
      Exp += 0.1;
   //  Change constant attenuation
   else if (keys[SDL_SCANCODE_1] && !shift && at0>0)
      at0--;
   else if (keys[SDL_SCANCODE_1] && shift)
      at0++;
   //  Change linear attenuation
   else if (keys[SDL_SCANCODE_2] && !shift && at1>0)
      at1--;
   else if (keys[SDL_SCANCODE_2] && shift)
      at1++;
   //  Change quadratic attenuation
   else if (keys[SDL_SCANCODE_3] && !shift && at2>0)
      at2--;
   else if (keys[SDL_SCANCODE_3] && shift)
      at2++;
   //  Light position
   else if (keys[SDL_SCANCODE_X] && !shift)
      X -= 0.1;
   else if (keys[SDL_SCANCODE_X] && shift)
      X += 0.1;
   else if (keys[SDL_SCANCODE_Y] && !shift)
      Y -= 0.1;
   else if (keys[SDL_SCANCODE_Y] && shift)
      Y += 0.1;
   else if (keys[SDL_SCANCODE_Z] && !shift)
      Z -= 0.1;
   else if (keys[SDL_SCANCODE_Z] && shift)
      Z += 0.1;
   //  Ambient level
   else if (keys[SDL_SCANCODE_A] && !shift && ambient>0)
      ambient -= 5;
   else if (keys[SDL_SCANCODE_A] && shift && ambient<100)
      ambient += 5;
   //  Diffuse level
   else if (keys[SDL_SCANCODE_D] && !shift && diffuse>0)
      diffuse -= 5;
   else if (keys[SDL_SCANCODE_D] && shift && diffuse<100)
      diffuse += 5;
   //  Specular level
   else if (keys[SDL_SCANCODE_S] && !shift && specular>0)
      specular -= 5;
   else if (keys[SDL_SCANCODE_S] && shift && specular<100)
      specular += 5;
   //  Emission level
   else if (keys[SDL_SCANCODE_E] && !shift && emission>0)
      emission -= 5;
   else if (keys[SDL_SCANCODE_E] && shift && emission<100)
      emission += 5;
   //  Shininess level
   else if (keys[SDL_SCANCODE_N] && !shift && shininess>-1)
      shininess -= 1;
   else if (keys[SDL_SCANCODE_N] && shift && shininess<7)
      shininess += 1;
   //  Increase/decrease asimuth
   else if (keys[SDL_SCANCODE_RIGHT])
      th += 5;
   else if (keys[SDL_SCANCODE_LEFT])
      th -= 5;
   //  Increase/decrease elevation
   else if (keys[SDL_SCANCODE_UP])
      ph += 5;
   else if (keys[SDL_SCANCODE_DOWN])
      ph -= 5;
   //  PageUp key - increase dim
   else if (keys[SDL_SCANCODE_PAGEDOWN])
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (keys[SDL_SCANCODE_PAGEUP] && dim>1)
      dim -= 0.1;
   //  Smooth color model
   else if (keys[SDL_SCANCODE_F1])
      smooth = 1-smooth;
   //  Local Viewer
   else if (keys[SDL_SCANCODE_F2])
      local = 1-local;
   //  Two sided mode
   else if (keys[SDL_SCANCODE_F3])
      side = 1-side;
   //  Play boing once
   else if (keys[SDL_SCANCODE_SPACE])
      Mix_PlayChannel(-1,boing,0);

   //  Wrap angles
   Th %= 360;
   Ph %= 360;
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(mode?fov:0,asp,dim);
   //  Return 1 to keep running
   return 1;
}

/*
 *  Call this routine when the window is resized
 */
void reshape(SDL_Window* window)
{
   int width,height;
   SDL_GetWindowSize(window,&width,&height);
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(mode?fov:0,asp,dim);
}

/*
 *  Start up SDL and tell it what to do
 */
int main(int argc,char* argv[])
{
   int run=1;
   double t0=0;

   //  Initialize SDL
   SDL_Init(SDL_INIT_VIDEO);
   //  Set size, resizable and double buffering
   SDL_Window* window = SDL_CreateWindow("More Lighting - SDL2",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,600,600,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
   if (!window) Fatal("Cannot create window\n");
   SDL_GL_CreateContext(window);
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Set screen size
   reshape(window);

   //  Load textures
   LoadTexBMP("brick.bmp");

   //  Initialize audio
   if (Mix_OpenAudio(44100,AUDIO_S16SYS,2,4096)) Fatal("Cannot initialize audio\n");
   //  Load "The Wall"
   Mix_Music* music = Mix_LoadMUS("thewall.ogg");
   if (!music) Fatal("Cannot load thewall.ogg\n");
   //  Load boing
   Mix_Chunk* boing = Mix_LoadWAV("boing.wav");
   if (!boing) Fatal("Cannot load boing.wav\n");
   //  Play (looping)
   Mix_PlayMusic(music,-1);

   //  SDL event loop
   ErrCheck("init");
   while (run)
   {
      //  Elapsed time in seconds
      double t = SDL_GetTicks()/1000.0;
      //  Process all pending events
      SDL_Event event;
      while (SDL_PollEvent(&event))
         switch (event.type)
         {
            case SDL_WINDOWEVENT:
               if (event.window.event==SDL_WINDOWEVENT_RESIZED)
               {
                  SDL_SetWindowSize(window,event.window.data1,event.window.data2);
                  reshape(window);
               }
               break;
            case SDL_QUIT:
               run = 0;
               break;
            case SDL_KEYDOWN:
               run = key(boing);
               t0 = t+0.5;  // Wait 1/2 s before repeating
               break;
            default:
               //  Do nothing
               break;
         }
      //  Repeat key every 50 ms
      if (t-t0>0.05)
      {
         run = key(boing);
         t0 = t;
      }
      //  Display
      Th = fmod(90*t,360.0);
      display(window);
      //  Slow down display rate to about 100 fps by sleeping 5ms
      SDL_Delay(5);
   }
   //  Shut down SDL
   Mix_CloseAudio();
   SDL_Quit();
   return 0;
}
