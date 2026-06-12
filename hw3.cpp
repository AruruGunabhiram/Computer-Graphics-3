/*
 * Homework 3
 * Gunabhiram Aruru
 *
 * Fixed-function OpenGL/GLUT windmill farm scene.
 * Lighting and textures will be added in later stages.
 */

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>

#define Cos(x) std::cos((x) * 3.1415927 / 180.0)
#define Sin(x) std::sin((x) * 3.1415927 / 180.0)

struct Instance
{
   double x;
   double y;
   double z;
   double sx;
   double sy;
   double sz;
   double rotation;
   float r;
   float g;
   float b;
   double bladeAngle;
};

struct FenceSection
{
   double x;
   double z;
   double length;
   double rotation;
};

int axes = 1;          // Display axes
int mode = 0;          // Projection mode
int rotateBlades = 1;  // Animate windmill blades
int lighting = 1;      // Placeholder for lighting on/off
int textures = 1;      // Placeholder for textures on/off
int moveLight = 1;     // Placeholder for moving light pause/run
double lightAngle = 90;
double lightHeight = 5;
int th = 35;           // Overhead azimuth
int ph = 25;           // Overhead elevation
int fov = 60;          // Perspective field of view
double asp = 1;        // Window aspect ratio
double dim = 9;        // Size of the overhead world
double bladeAngle = 0; // Shared blade rotation
double fpX = 0;        // First-person X position
double fpY = 1;        // First-person eye height
double fpZ = 12;       // First-person Z position
int fpYaw = 0;         // First-person heading
int windowWidth = 800;
int windowHeight = 600;
unsigned int textureGrass = 0;
unsigned int textureWood = 0;
unsigned int textureRoof = 0;
unsigned int texturePath = 0;
unsigned int textureMetal = 0;

void Reverse(void* value, int bytes)
{
   char* data = static_cast<char*>(value);
   for (int i = 0; i < bytes / 2; ++i)
   {
      const char temp = data[i];
      data[i] = data[bytes - 1 - i];
      data[bytes - 1 - i] = temp;
   }
}

void TextureError(const char* message, const char* file)
{
   std::fprintf(stderr, "Texture error: %s: %s\n", message, file);
   std::exit(1);
}

// Course-style loader for uncompressed 24-bit BMP texture files.
unsigned int LoadTexBMP(const char* file)
{
   FILE* input = std::fopen(file, "rb");
   if (!input)
      TextureError("cannot open file", file);

   unsigned short magic;
   if (std::fread(&magic, 2, 1, input) != 1)
      TextureError("cannot read BMP magic", file);
   if (magic != 0x4D42 && magic != 0x424D)
      TextureError("file is not a BMP", file);

   unsigned int offset;
   unsigned int width;
   int height;
   unsigned short planes;
   unsigned short bitsPerPixel;
   unsigned int compression;
   if (std::fseek(input, 8, SEEK_CUR) ||
       std::fread(&offset, 4, 1, input) != 1 ||
       std::fseek(input, 4, SEEK_CUR) ||
       std::fread(&width, 4, 1, input) != 1 ||
       std::fread(&height, 4, 1, input) != 1 ||
       std::fread(&planes, 2, 1, input) != 1 ||
       std::fread(&bitsPerPixel, 2, 1, input) != 1 ||
       std::fread(&compression, 4, 1, input) != 1)
      TextureError("cannot read BMP header", file);

   if (magic == 0x424D)
   {
      Reverse(&offset, 4);
      Reverse(&width, 4);
      Reverse(&height, 4);
      Reverse(&planes, 2);
      Reverse(&bitsPerPixel, 2);
      Reverse(&compression, 4);
   }

   const unsigned int imageHeight = height < 0 ? -height : height;
   int maxTextureSize;
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
   if (width < 1 || width > static_cast<unsigned int>(maxTextureSize) ||
       imageHeight < 1 || imageHeight > static_cast<unsigned int>(maxTextureSize))
      TextureError("texture dimensions are out of range", file);
   if ((width & (width - 1)) || (imageHeight & (imageHeight - 1)))
      TextureError("texture dimensions must be powers of two", file);
   if (planes != 1 || bitsPerPixel != 24 || compression != 0)
      TextureError("BMP must be uncompressed 24-bit RGB", file);

   const unsigned int size = 3 * width * imageHeight;
   unsigned char* image = static_cast<unsigned char*>(std::malloc(size));
   if (!image)
      TextureError("cannot allocate image memory", file);
   if (std::fseek(input, offset, SEEK_SET) || std::fread(image, size, 1, input) != 1)
      TextureError("cannot read BMP pixels", file);
   std::fclose(input);

   for (unsigned int i = 0; i < size; i += 3)
   {
      const unsigned char temp = image[i];
      image[i] = image[i + 2];
      image[i + 2] = temp;
   }

   unsigned int texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, imageHeight, 0,
                GL_RGB, GL_UNSIGNED_BYTE, image);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   std::free(image);
   return texture;
}

const char* ModeName()
{
   switch (mode)
   {
      case 0: return "Oblique overhead orthogonal";
      case 1: return "Oblique overhead perspective";
      default: return "First person perspective";
   }
}

void DrawText(int x, int y, const char* text)
{
   glRasterPos2i(x, y);
   for (const char* ch = text; *ch; ++ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *ch);
}

void Project()
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   // Mode 0 uses an orthogonal volume; modes 1 and 2 share perspective.
   if (mode == 0)
   {
      glOrtho(-asp * dim, asp * dim, -dim, dim, -4 * dim, 4 * dim);
   }
   else
   {
      gluPerspective(fov, asp, 0.1, 100);
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

void DrawAxes()
{
   const double len = 2.0;

   glColor3f(1, 1, 1);
   glBegin(GL_LINES);
   glVertex3d(0, 0, 0);
   glVertex3d(len, 0, 0);
   glVertex3d(0, 0, 0);
   glVertex3d(0, len, 0);
   glVertex3d(0, 0, 0);
   glVertex3d(0, 0, len);
   glEnd();

   glRasterPos3d(len, 0, 0);
   glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'X');
   glRasterPos3d(0, len, 0);
   glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'Y');
   glRasterPos3d(0, 0, len);
   glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'Z');
}

void drawBoxUnit(double repeatX = 1, double repeatY = 1, double repeatZ = 1)
{
   glBegin(GL_QUADS);

   glNormal3f(0, 0, 1);
   glTexCoord2f(0, 0); glVertex3d(-0.5, -0.5,  0.5);
   glTexCoord2d(repeatX, 0); glVertex3d( 0.5, -0.5,  0.5);
   glTexCoord2d(repeatX, repeatY); glVertex3d( 0.5,  0.5,  0.5);
   glTexCoord2d(0, repeatY); glVertex3d(-0.5,  0.5,  0.5);

   glNormal3f(0, 0, -1);
   glTexCoord2f(0, 0); glVertex3d( 0.5, -0.5, -0.5);
   glTexCoord2d(repeatX, 0); glVertex3d(-0.5, -0.5, -0.5);
   glTexCoord2d(repeatX, repeatY); glVertex3d(-0.5,  0.5, -0.5);
   glTexCoord2d(0, repeatY); glVertex3d( 0.5,  0.5, -0.5);

   glNormal3f(-1, 0, 0);
   glTexCoord2f(0, 0); glVertex3d(-0.5, -0.5, -0.5);
   glTexCoord2d(repeatZ, 0); glVertex3d(-0.5, -0.5,  0.5);
   glTexCoord2d(repeatZ, repeatY); glVertex3d(-0.5,  0.5,  0.5);
   glTexCoord2d(0, repeatY); glVertex3d(-0.5,  0.5, -0.5);

   glNormal3f(1, 0, 0);
   glTexCoord2f(0, 0); glVertex3d(0.5, -0.5,  0.5);
   glTexCoord2d(repeatZ, 0); glVertex3d(0.5, -0.5, -0.5);
   glTexCoord2d(repeatZ, repeatY); glVertex3d(0.5,  0.5, -0.5);
   glTexCoord2d(0, repeatY); glVertex3d(0.5,  0.5,  0.5);

   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0); glVertex3d(-0.5, 0.5,  0.5);
   glTexCoord2d(repeatX, 0); glVertex3d( 0.5, 0.5,  0.5);
   glTexCoord2d(repeatX, repeatZ); glVertex3d( 0.5, 0.5, -0.5);
   glTexCoord2d(0, repeatZ); glVertex3d(-0.5, 0.5, -0.5);

   glNormal3f(0, -1, 0);
   glTexCoord2f(0, 0); glVertex3d(-0.5, -0.5, -0.5);
   glTexCoord2d(repeatX, 0); glVertex3d( 0.5, -0.5, -0.5);
   glTexCoord2d(repeatX, repeatZ); glVertex3d( 0.5, -0.5,  0.5);
   glTexCoord2d(0, repeatZ); glVertex3d(-0.5, -0.5,  0.5);
   glEnd();
}

/*
AI assistance note:
This function was drafted with AI assistance and then reviewed/modified.
The important course concept used here is manual normal-vector assignment
for fixed-pipeline OpenGL lighting. No GLU/GLUT solid objects or imported
models are used.
*/
void drawBladeUnit()
{
   const double x[5] = {-0.08, 0.17, 0.30, 0.09, -0.16};
   const double y[5] = { 0.20, 0.43, 1.55, 1.78,  0.62};
   const double front = 0.07;
   const double back = -0.07;

   glBegin(GL_POLYGON);
   glNormal3f(0, 0, 1);
   for (int i = 0; i < 5; ++i)
   {
      glTexCoord2d(x[i] + 0.16, y[i] / 1.78);
      glVertex3d(x[i], y[i], front);
   }
   glEnd();

   glBegin(GL_POLYGON);
   glNormal3f(0, 0, -1);
   for (int i = 4; i >= 0; --i)
   {
      glTexCoord2d(x[i] + 0.16, y[i] / 1.78);
      glVertex3d(x[i], y[i], back);
   }
   glEnd();

   glBegin(GL_QUADS);
   for (int i = 0; i < 5; ++i)
   {
      const int next = (i + 1) % 5;
      const double dx = x[next] - x[i];
      const double dy = y[next] - y[i];
      const double length = std::sqrt(dx * dx + dy * dy);
      glNormal3d(dy / length, -dx / length, 0);
      glTexCoord2f(0, 0); glVertex3d(x[i], y[i], front);
      glTexCoord2f(1, 0); glVertex3d(x[next], y[next], front);
      glTexCoord2f(1, 1); glVertex3d(x[next], y[next], back);
      glTexCoord2f(0, 1); glVertex3d(x[i], y[i], back);
   }
   glEnd();
}

/*
AI assistance note:
This function was drafted with AI assistance and then reviewed/modified.
The important course concept used here is manual normal-vector assignment
for fixed-pipeline OpenGL lighting. No GLU/GLUT solid objects or imported
models are used.
*/
void drawHubUnit()
{
   const int sides = 12;
   const double radius = 0.22;
   const double front = 0.15;
   const double back = -0.15;

   glBegin(GL_QUAD_STRIP);
   for (int i = 0; i <= sides; ++i)
   {
      const double angle = 360.0 * i / sides;
      glNormal3d(Cos(angle), Sin(angle), 0);
      glTexCoord2d(static_cast<double>(i) / sides, 1);
      glVertex3d(radius * Cos(angle), radius * Sin(angle), front);
      glTexCoord2d(static_cast<double>(i) / sides, 0);
      glVertex3d(radius * Cos(angle), radius * Sin(angle), back);
   }
   glEnd();

   glBegin(GL_POLYGON);
   glNormal3f(0, 0, 1);
   for (int i = 0; i < sides; ++i)
   {
      const double angle = 360.0 * i / sides;
      glTexCoord2d(0.5 + 0.5 * Cos(angle), 0.5 + 0.5 * Sin(angle));
      glVertex3d(radius * Cos(angle), radius * Sin(angle), front);
   }
   glEnd();

   glBegin(GL_POLYGON);
   glNormal3f(0, 0, -1);
   for (int i = sides - 1; i >= 0; --i)
   {
      const double angle = 360.0 * i / sides;
      glTexCoord2d(0.5 + 0.5 * Cos(angle), 0.5 + 0.5 * Sin(angle));
      glVertex3d(radius * Cos(angle), radius * Sin(angle), back);
   }
   glEnd();
}

/*
AI assistance note:
This function was drafted with AI assistance and then reviewed/modified.
The important course concept used here is manual normal-vector assignment
for fixed-pipeline OpenGL lighting. No GLU/GLUT solid objects or imported
models are used.
*/
void drawWindmillBaseUnit()
{
   const double taper = 0.21 / 2.61;
   const double normalLength = std::sqrt(1.0 + taper * taper);
   const double normalSide = 1.0 / normalLength;
   const double normalUp = taper / normalLength;

   glPushMatrix();
   glTranslated(0, 0.12, 0);
   glScaled(0.9, 0.24, 0.9);
   drawBoxUnit(2, 1, 2);
   glPopMatrix();

   glBegin(GL_QUADS);
   glNormal3d(0, normalUp, normalSide);
   glTexCoord2f(0, 0); glVertex3d(-0.35, 0.24,  0.35);
   glTexCoord2f(1, 0); glVertex3d( 0.35, 0.24,  0.35);
   glTexCoord2f(1, 3); glVertex3d( 0.14, 2.85,  0.14);
   glTexCoord2f(0, 3); glVertex3d(-0.14, 2.85,  0.14);

   glNormal3d(0, normalUp, -normalSide);
   glTexCoord2f(0, 0); glVertex3d( 0.35, 0.24, -0.35);
   glTexCoord2f(1, 0); glVertex3d(-0.35, 0.24, -0.35);
   glTexCoord2f(1, 3); glVertex3d(-0.14, 2.85, -0.14);
   glTexCoord2f(0, 3); glVertex3d( 0.14, 2.85, -0.14);

   glNormal3d(-normalSide, normalUp, 0);
   glTexCoord2f(0, 0); glVertex3d(-0.35, 0.24, -0.35);
   glTexCoord2f(1, 0); glVertex3d(-0.35, 0.24,  0.35);
   glTexCoord2f(1, 3); glVertex3d(-0.14, 2.85,  0.14);
   glTexCoord2f(0, 3); glVertex3d(-0.14, 2.85, -0.14);

   glNormal3d(normalSide, normalUp, 0);
   glTexCoord2f(0, 0); glVertex3d(0.35, 0.24,  0.35);
   glTexCoord2f(1, 0); glVertex3d(0.35, 0.24, -0.35);
   glTexCoord2f(1, 3); glVertex3d(0.14, 2.85, -0.14);
   glTexCoord2f(0, 3); glVertex3d(0.14, 2.85,  0.14);
   glEnd();

   glPushMatrix();
   glTranslated(0, 2.95, 0.08);
   glScaled(0.65, 0.45, 0.85);
   drawBoxUnit(2, 1, 2);
   glPopMatrix();
}

void drawWindmillUnit(double bladeOffset)
{
   // The generic windmill stays at the origin so one model can be instanced.
   if (textures)
   {
      glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, textureMetal);
      glColor3f(0.86f, 0.88f, 0.90f);
   }
   drawWindmillBaseUnit();

   glPushMatrix();
   glTranslated(0, 3.0, 0.58);
   glRotated(bladeAngle + bladeOffset, 0, 0, 1);
   if (textures)
      glColor3f(0.95f, 0.90f, 0.78f);
   else
      glColor3f(0.92f, 0.84f, 0.58f);
   if (textures)
      glBindTexture(GL_TEXTURE_2D, textureWood);
   for (int blade = 0; blade < 4; ++blade)
   {
      glPushMatrix();
      glRotated(90 * blade, 0, 0, 1);
      drawBladeUnit();
      glPopMatrix();
   }
   if (textures)
      glColor3f(0.85f, 0.85f, 0.82f);
   else
      glColor3f(0.32f, 0.25f, 0.18f);
   if (textures)
      glBindTexture(GL_TEXTURE_2D, textureMetal);
   drawHubUnit();
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

void drawWindmillInstance(const Instance& instance)
{
   // Place, orient, and size the origin-centered model for this farm location.
   glPushMatrix();
   glTranslated(instance.x, instance.y, instance.z);
   glRotated(instance.rotation, 0, 1, 0);
   glScaled(instance.sx, instance.sy, instance.sz);
   glColor3f(instance.r, instance.g, instance.b);
   drawWindmillUnit(instance.bladeAngle);
   glPopMatrix();
}

void drawGround()
{
   glPushMatrix();
   glTranslated(0, -0.12, 0);
   glScaled(18, 0.2, 14);
   glColor3f(0.24f, 0.46f, 0.20f);
   drawBoxUnit();
   glPopMatrix();

   if (textures)
   {
      glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, textureGrass);
      glColor3f(1, 1, 1);
      glBegin(GL_QUADS);
      glNormal3f(0, 1, 0);
      glTexCoord2f(0, 0); glVertex3d(-9, -0.019, -7);
      glTexCoord2f(9, 0); glVertex3d( 9, -0.019, -7);
      glTexCoord2f(9, 7); glVertex3d( 9, -0.019,  7);
      glTexCoord2f(0, 7); glVertex3d(-9, -0.019,  7);
      glEnd();
      glDisable(GL_TEXTURE_2D);
   }

   glColor3f(0.34f, 0.55f, 0.25f);
   glBegin(GL_LINES);
   glNormal3f(0, 1, 0);
   for (int i = -8; i <= 8; ++i)
   {
      glVertex3d(i, 0, -7);
      glVertex3d(i, 0, 7);
   }
   for (int i = -7; i <= 7; ++i)
   {
      glVertex3d(-9, 0, i);
      glVertex3d(9, 0, i);
   }
   glEnd();
}

void drawPath()
{
   const double sections[][8] =
   {
      {-1.4,  7.0,  1.4,  7.0,  1.1,  3.5, -1.1,  3.5},
      {-1.1,  3.5,  1.1,  3.5,  2.8,  1.2,  0.8,  1.2},
      { 0.8,  1.2,  2.8,  1.2,  5.2,  2.8,  3.3,  3.5}
   };

   if (textures)
   {
      glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, texturePath);
      glColor3f(1, 1, 1);
   }
   else
      glColor3f(0.58f, 0.48f, 0.32f);

   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   for (int i = 0; i < 3; ++i)
   {
      glTexCoord2f(0, 0); glVertex3d(sections[i][0], 0.015, sections[i][1]);
      glTexCoord2f(1, 0); glVertex3d(sections[i][2], 0.015, sections[i][3]);
      glTexCoord2f(1, 2); glVertex3d(sections[i][4], 0.015, sections[i][5]);
      glTexCoord2f(0, 2); glVertex3d(sections[i][6], 0.015, sections[i][7]);
   }
   glEnd();
   glDisable(GL_TEXTURE_2D);
}

void drawFenceSection(double x, double z, double length, double rotation)
{
   glPushMatrix();
   glTranslated(x, 0, z);
   glRotated(rotation, 0, 1, 0);
   if (textures)
   {
      glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, textureWood);
      glColor3f(0.92f, 0.80f, 0.62f);
   }
   else
      glColor3f(0.55f, 0.34f, 0.16f);

   for (double post = -length / 2; post <= length / 2 + 0.01; post += 2.0)
   {
      glPushMatrix();
      glTranslated(post, 0.55, 0);
      glScaled(0.14, 1.1, 0.14);
      drawBoxUnit(1, 2, 1);
      glPopMatrix();
   }

   for (int rail = 0; rail < 2; ++rail)
   {
      glPushMatrix();
      glTranslated(0, 0.38 + 0.42 * rail, 0);
      glScaled(length, 0.12, 0.10);
      drawBoxUnit(length, 1, 1);
      glPopMatrix();
   }
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

void drawFence()
{
   const FenceSection sections[] =
   {
      { 0, -6.2, 16,  0},
      {-8,  0.0, 12, 90},
      { 8,  0.0, 12, 90},
      {-4,  6.2,  8,  0}
   };

   const int count = sizeof(sections) / sizeof(sections[0]);
   for (int i = 0; i < count; ++i)
   {
      drawFenceSection(sections[i].x, sections[i].z,
                       sections[i].length, sections[i].rotation);
   }
}

/*
AI assistance note:
This function was drafted with AI assistance and then reviewed/modified.
The important course concept used here is manual normal-vector assignment
for fixed-pipeline OpenGL lighting. No GLU/GLUT solid objects or imported
models are used.
*/
void drawGableRoofUnit()
{
   const double run = 0.5;
   const double rise = 0.6;
   const double slopeLength = std::sqrt(run * run + rise * rise);
   const double normalY = run / slopeLength;
   const double normalZ = rise / slopeLength;

   glBegin(GL_QUADS);
   glNormal3d(0, normalY, -normalZ);
   glTexCoord2f(0, 0); glVertex3d(-0.5, 0.0, -0.5);
   glTexCoord2f(2, 0); glVertex3d( 0.5, 0.0, -0.5);
   glTexCoord2f(2, 1); glVertex3d( 0.5, 0.6,  0.0);
   glTexCoord2f(0, 1); glVertex3d(-0.5, 0.6,  0.0);

   glNormal3d(0, normalY, normalZ);
   glTexCoord2f(0, 1); glVertex3d(-0.5, 0.6, 0.0);
   glTexCoord2f(2, 1); glVertex3d( 0.5, 0.6, 0.0);
   glTexCoord2f(2, 0); glVertex3d( 0.5, 0.0, 0.5);
   glTexCoord2f(0, 0); glVertex3d(-0.5, 0.0, 0.5);

   glNormal3f(0, -1, 0);
   glTexCoord2f(0, 0); glVertex3d(-0.5, 0.0,  0.5);
   glTexCoord2f(1, 0); glVertex3d( 0.5, 0.0,  0.5);
   glTexCoord2f(1, 1); glVertex3d( 0.5, 0.0, -0.5);
   glTexCoord2f(0, 1); glVertex3d(-0.5, 0.0, -0.5);
   glEnd();

   glBegin(GL_TRIANGLES);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0, 0);   glVertex3d(-0.5, 0.0, -0.5);
   glTexCoord2f(0.5, 1); glVertex3d(-0.5, 0.6,  0.0);
   glTexCoord2f(1, 0);   glVertex3d(-0.5, 0.0,  0.5);

   glNormal3f(1, 0, 0);
   glTexCoord2f(0, 0);   glVertex3d(0.5, 0.0,  0.5);
   glTexCoord2f(0.5, 1); glVertex3d(0.5, 0.6,  0.0);
   glTexCoord2f(1, 0);   glVertex3d(0.5, 0.0, -0.5);
   glEnd();
}

void drawBarnOrShed()
{
   glPushMatrix();
   glTranslated(5.7, 0, 3.4);
   glRotated(-18, 0, 1, 0);

   if (textures)
   {
      glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, textureWood);
      glColor3f(0.90f, 0.62f, 0.56f);
   }
   else
      glColor3f(0.55f, 0.16f, 0.12f);
   glPushMatrix();
   glTranslated(0, 0.9, 0);
   glScaled(2.4, 1.8, 2.0);
   drawBoxUnit(3, 2, 2);
   glPopMatrix();

   if (textures)
   {
      glBindTexture(GL_TEXTURE_2D, textureRoof);
      glColor3f(0.90f, 0.90f, 0.88f);
   }
   else
      glColor3f(0.30f, 0.12f, 0.09f);
   glPushMatrix();
   glTranslated(0, 1.75, 0);
   glScaled(2.7, 1.15, 2.3);
   drawGableRoofUnit();
   glPopMatrix();

   if (textures)
   {
      glBindTexture(GL_TEXTURE_2D, textureWood);
      glColor3f(0.96f, 0.90f, 0.74f);
   }
   else
      glColor3f(0.80f, 0.72f, 0.52f);
   glPushMatrix();
   glTranslated(-1.21, 0.75, 0);
   glScaled(0.05, 1.15, 0.75);
   drawBoxUnit(1, 2, 1);
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

void drawScene()
{
   const Instance windmills[] =
   {
      {-1.0, 0.0,  0.5, 1.35, 1.35, 1.35,  10, 0.72f, 0.72f, 0.68f,  12},
      {-5.1, 0.0, -2.3, 0.82, 0.95, 0.82, -28, 0.48f, 0.62f, 0.70f,  48},
      { 4.2, 0.0, -3.8, 0.62, 0.72, 0.62,  37, 0.72f, 0.58f, 0.42f, -25}
   };

   drawGround();
   drawPath();
   drawFence();
   drawBarnOrShed();

   const int count = sizeof(windmills) / sizeof(windmills[0]);
   for (int i = 0; i < count; ++i)
      drawWindmillInstance(windmills[i]);
}

void drawLightMarker(double x, double y, double z)
{
   glDisable(GL_LIGHTING);
   glDisable(GL_TEXTURE_2D);
   glColor3f(1.0f, 0.85f, 0.20f);
   glPushMatrix();
   glTranslated(x, y, z);
   glScaled(0.3, 0.3, 0.3);
   drawBoxUnit();
   glPopMatrix();
}

void ConfigureLighting(const float position[4])
{
   const float ambientLight[] = {0.15f, 0.15f, 0.15f, 1.0f};
   const float diffuseLight[] = {0.75f, 0.72f, 0.65f, 1.0f};
   const float specularLight[] = {0.35f, 0.35f, 0.30f, 1.0f};
   const float materialAmbient[] = {0.25f, 0.25f, 0.25f, 1.0f};
   const float materialDiffuse[] = {0.80f, 0.80f, 0.80f, 1.0f};
   const float materialSpecular[] = {0.25f, 0.25f, 0.25f, 1.0f};

   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
   glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
   glLightfv(GL_LIGHT0, GL_POSITION, position);

   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 24.0f);

   glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);
}

void display()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);

   Project();

   if (mode == 0)
   {
      glRotated(ph, 1, 0, 0);
      glRotated(th, 0, 1, 0);
   }
   else if (mode == 1)
   {
      const double ex = -2 * dim * Sin(th) * Cos(ph);
      const double ey =  2 * dim * Sin(ph);
      const double ez =  2 * dim * Cos(th) * Cos(ph);
      gluLookAt(ex, ey, ez, 0, 0, 0, 0, Cos(ph), 0);
   }
   else
   {
      // Look one unit ahead in the current first-person heading.
      const double lookX = fpX + Sin(fpYaw);
      const double lookZ = fpZ - Cos(fpYaw);
      gluLookAt(fpX, fpY, fpZ, lookX, fpY, lookZ, 0, 1, 0);
   }

   const float lightPosition[] =
   {
      static_cast<float>(7 * Cos(lightAngle)),
      static_cast<float>(lightHeight),
      static_cast<float>(7 * Sin(lightAngle)),
      1.0f
   };
   drawLightMarker(lightPosition[0], lightPosition[1], lightPosition[2]);
   if (lighting)
      ConfigureLighting(lightPosition);
   else
      glDisable(GL_LIGHTING);

   drawScene();
   glDisable(GL_LIGHTING);
   if (axes)
      DrawAxes();

   glDisable(GL_DEPTH_TEST);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   glColor3f(1, 1, 1);
   char viewText[80];
   if (mode == 2)
      std::snprintf(viewText, sizeof(viewText), "View angle: yaw=%d", fpYaw);
   else
      std::snprintf(viewText, sizeof(viewText), "View angle: th=%d ph=%d", th, ph);

   char lightText[100];
   std::snprintf(lightText, sizeof(lightText),
                 "Light: angle=%.0f height=%.1f movement=%s lighting=%s",
                 lightAngle, lightHeight, moveLight ? "running" : "paused",
                 lighting ? "on" : "off");

   DrawText(10, 110, lightText);
   DrawText(10, 90, viewText);
   DrawText(10, 70, ModeName());
   DrawText(10, 50, "arrows: navigate  l: lighting  t: textures  SPACE: pause light");
   DrawText(10, 30, "r: rotate blades  ,/.: light angle  [ / ]: light height");
   DrawText(10, 10,
            "m: mode  +/- or PgUp/PgDn: zoom/FOV  a: axes  0: reset  q/ESC: exit");

   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);

   glutSwapBuffers();
}

void reshape(int width, int height)
{
   windowWidth = width;
   windowHeight = height > 0 ? height : 1;
   asp = static_cast<double>(windowWidth) / windowHeight;
   glViewport(0, 0, windowWidth, windowHeight);
   Project();
}

void ResetCamera()
{
   th = 35;
   ph = 25;
   fov = 60;
   dim = 9;
   fpX = 0;
   fpY = 1;
   fpZ = 12;
   fpYaw = 0;
   lightAngle = 90;
   lightHeight = 5;
}

void AdjustZoom(int direction)
{
   if (mode == 0)
   {
      dim -= 0.5 * direction;
      if (dim < 3)
         dim = 3;
      if (dim > 20)
         dim = 20;
   }
   else
   {
      fov -= 5 * direction;
      if (fov < 20)
         fov = 20;
      if (fov > 100)
         fov = 100;
   }
}

void key(unsigned char ch, int, int)
{
   if (ch == 27 || ch == 'q' || ch == 'Q')
   {
      std::exit(0);
   }
   else if (ch == 'm' || ch == 'M')
   {
      mode = (mode + 1) % 3;
   }
   else if (ch == 'a' || ch == 'A')
   {
      axes = 1 - axes;
   }
   else if (ch == 'r' || ch == 'R')
   {
      rotateBlades = 1 - rotateBlades;
   }
   else if (ch == 't' || ch == 'T')
   {
      textures = 1 - textures;
   }
   else if (ch == 'l' || ch == 'L')
   {
      lighting = 1 - lighting;
   }
   else if (ch == ' ')
   {
      moveLight = 1 - moveLight;
   }
   else if (ch == ',')
   {
      lightAngle = std::fmod(lightAngle - 5 + 360, 360.0);
   }
   else if (ch == '.')
   {
      lightAngle = std::fmod(lightAngle + 5, 360.0);
   }
   else if (ch == '[')
   {
      lightHeight -= 0.25;
      if (lightHeight < 0.5)
         lightHeight = 0.5;
   }
   else if (ch == ']')
   {
      lightHeight += 0.25;
      if (lightHeight > 12)
         lightHeight = 12;
   }
   else if (ch == '0')
   {
      ResetCamera();
   }
   else if (ch == '+' || ch == '=')
   {
      AdjustZoom(1);
   }
   else if (ch == '-' || ch == '_')
   {
      AdjustZoom(-1);
   }

   Project();
   glutPostRedisplay();
}

void special(int key, int, int)
{
   if (key == GLUT_KEY_PAGE_UP)
   {
      AdjustZoom(1);
      Project();
      glutPostRedisplay();
      return;
   }
   if (key == GLUT_KEY_PAGE_DOWN)
   {
      AdjustZoom(-1);
      Project();
      glutPostRedisplay();
      return;
   }

   if (mode == 2)
   {
      if (key == GLUT_KEY_LEFT)
         fpYaw = (fpYaw - 5) % 360;
      else if (key == GLUT_KEY_RIGHT)
         fpYaw = (fpYaw + 5) % 360;
      else if (key == GLUT_KEY_UP)
      {
         fpX += 0.2 * Sin(fpYaw);
         fpZ -= 0.2 * Cos(fpYaw);
      }
      else if (key == GLUT_KEY_DOWN)
      {
         fpX -= 0.2 * Sin(fpYaw);
         fpZ += 0.2 * Cos(fpYaw);
      }

   }
   else
   {
      if (key == GLUT_KEY_LEFT)
         th -= 5;
      else if (key == GLUT_KEY_RIGHT)
         th += 5;
      else if (key == GLUT_KEY_UP)
         ph += 5;
      else if (key == GLUT_KEY_DOWN)
         ph -= 5;

      th %= 360;
      if (ph > 85)
         ph = 85;
      if (ph < -85)
         ph = -85;
   }

   glutPostRedisplay();
}

void idle()
{
   static int previousTime = glutGet(GLUT_ELAPSED_TIME);
   const int currentTime = glutGet(GLUT_ELAPSED_TIME);
   const int elapsed = currentTime - previousTime;
   previousTime = currentTime;

   // Use elapsed time so blade speed does not depend on frame rate.
   if (rotateBlades)
      bladeAngle = std::fmod(bladeAngle + 0.045 * elapsed, 360.0);
   if (moveLight)
      lightAngle = std::fmod(lightAngle + 0.025 * elapsed, 360.0);

   glutPostRedisplay();
}

int main(int argc, char* argv[])
{
   glutInit(&argc, argv);
   // GLUT_DEPTH allocates the depth buffer used for hidden-surface removal.
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutCreateWindow("Gunabhiram Aruru - Homework 3 - Lighting and Textures");

   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(key);
   glutSpecialFunc(special);
   glutIdleFunc(idle);

   glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
   // The depth test makes nearer solid surfaces hide farther surfaces.
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);

   textureGrass = LoadTexBMP("textures/grass.bmp");
   textureWood = LoadTexBMP("textures/wood.bmp");
   textureRoof = LoadTexBMP("textures/roof.bmp");
   texturePath = LoadTexBMP("textures/path.bmp");
   textureMetal = LoadTexBMP("textures/metal.bmp");

   glutMainLoop();
   return 0;
}
