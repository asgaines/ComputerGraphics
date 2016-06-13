/*
 *  Projections
 *
 *  Draw 27 cubes to demonstrate orthogonal & prespective projections
 *
 *  Key bindings:
  m          Toggle between perspective and orthogonal
  +/-        Changes field of view for perspective
  w					 Move forward
  a          Turn left
  s					 Move backwards
  d					 Turn right
  l/r arrows     Change view angle
  PgDn/PgUp  Zoom in and out
  0          Reset view angle
  ESC        Exit
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

int mode = 1;
int th = -45;         //  Azimuth of view angle
int ph = 0;         //  Elevation of view angle
int fov = 55;       //  Field of view (for perspective)
double asp = 1;     //  Aspect ratio
double dim = 6.0;   //  Size of world
double w = 1;       // W variable
double eyeHeight = 1.75; // Height of eyes above the ground (in meters)
bool axes = false;
// Globals for dragging scene with mouse
bool mouseDown = false;
int mouseX;
int mouseY;
// Globals determining camera position/orientation
double camX = 10.0;
double camZ = 10.0;
float lx = 0.0f, lz = -1.0f; // actual vector representing the camera's direction
float dt = 0.125f; // Change in time between scenes
// Initialize stalagtite (roof) /stalagmite (ground) variables
// Repeating pattern based on number used
#define NUM_CONES 81
double cones[NUM_CONES][2];
int rows = 9;
int cols = 9;


//  Macro for sin & cos in degrees
#define Cos(th) cos(3.1415927/180*(th))
#define Sin(th) sin(3.1415927/180*(th))

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  //  Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

void ErrCheck(const char* where)
{
   int err = glGetError();
   if (err) fprintf(stderr,"ERROR: %s [%s]\n",gluErrorString(err),where);
}

/*
 *  Set projection
 */
static void Project()
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective transformation
   if (mode) 
      gluPerspective(fov,asp,dim/4,4*dim);
   else
      glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

/*
 *  Draw a cube
 *     at (x,y,z)
 *     dimentions (dx,dy,dz)
 *     rotated th about the y axis
 */
static void chest(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //  Cube
   glBegin(GL_QUADS);
   float darken = 0.9;
   // Brown shade 1 (darker)
   float br1R = 0.7*darken;
   float br1G = 0.4*darken;
   float br1B = 0.133*darken;
   // Brown shade 2 (lighter)
   float br2R = 0.5*darken;
   float br2G = 0.25*darken;
   float br2B = 0.0;
   //  Front
   glColor3f(br2R, br2G, br2B);
   glVertex4f(-1,-1, 1, w);
   glVertex4f(+1,-1, 1, w);
   glVertex4f(+1,+1, 1, w);
   glVertex4f(-1,+1, 1, w);
   //  Back
   glColor3f(br2R, br2G, br2B);
   glVertex4f(+1,-1,-1, w);
   glVertex4f(-1,-1,-1, w);
   glVertex4f(-1,+1,-1, w);
   glVertex4f(+1,+1,-1, w);
   //  Right
   glColor3f(br1R, br1G, br1B);
   glVertex4f(+1,-1,+1, w);
   glVertex4f(+1,-1,-1, w);
   glVertex4f(+1,+1,-1, w);
   glVertex4f(+1,+1,+1, w);
   //  Left
   glColor3f(br1R, br1G, br1B);
   glVertex4f(-1,-1,-1, w);
   glVertex4f(-1,-1,+1, w);
   glVertex4f(-1,+1,+1, w);
   glVertex4f(-1,+1,-1, w);
   // Round cylindrical part of lid
   bool toggleBrown = false;
   int angleStep = 15;
   for (int angle = 0; angle < 180; angle += angleStep) {
      if (toggleBrown) {
         glColor3f(br2R, br2G, br2B);
      } else {
         glColor3f(br1R, br1G, br1B);
      }
      toggleBrown = !toggleBrown;
      glVertex3d(1, 1 + Sin(angle), Cos(angle));
      glVertex3d(-1, 1 + Sin(angle), Cos(angle));
      glVertex3d(-1, 1 + Sin(angle + angleStep), Cos(angle + angleStep));
      glVertex3d(1, 1 + Sin(angle + angleStep), Cos(angle + angleStep));
   }
   glEnd();
   glColor3f(br2R, br2G, br2B);
   // Right side of lid
   glBegin(GL_TRIANGLE_FAN);
   glVertex3d(1, 1, 0);
   for (int angle = 0; angle <= 180; angle += angleStep) {
      glVertex3d(1, 1 + Sin(angle), Cos(angle));
   }
   glEnd();
   // Left side of lid
   glBegin(GL_TRIANGLE_FAN);
   glVertex3d(-1, 1, 0);
   for (int angle = 0; angle <= 180; angle += angleStep) {
      glVertex3d(-1, 1 + Sin(angle), Cos(angle));
   }
   //  End
   glEnd();
   //  Undo transformations
   glPopMatrix();
}

void cone(bool down, double x, double y, double z)
{
   if (!down && x < 2 && x > -2 && z < 1 && z > -1) {
      // Guard against stalagmites under chest
      return;
   }
   glPushMatrix();
   glTranslated(x, y, z);
   glRotated(0, 0, 0, 0);
   // Stalagtites (hanging down on roof) will be larger than stalagmites on ground
   float radius = (down) ? 0.3 : 0.15;
   float height = (down) ? 1.5 : 0.75;
   glScaled(radius, height, radius);
   glBegin(GL_TRIANGLE_FAN);
   glColor3f(0.1, 0.1, 0.1);
   // Tip of cone
   glVertex3d(0, (down) ? -1 : 1, 0);
   for (int degree = 0; degree <= 360; degree += 15) {
      glVertex3d(Cos(degree), 0, Sin(degree));
   }
   glEnd();
   glPopMatrix();
}

void plane(double x, double y, double z, double sf)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x, y, z);
   glRotated(0, 0, 0, 0);
   glScaled(sf, sf, sf);
   // Ground plane
   glBegin(GL_QUADS);
   glColor3f(0.15, 0.15, 0.15);
   glVertex4f(1, 0, 1, w);
   glVertex4f(1, 0, -1, w);
   glVertex4f(-1, 0, -1, w);
   glVertex4f(-1, 0, 1, w);
   glEnd();
   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   const double len = 1.5;  //  Length of axes
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective - set eye position
   if (mode)
   {
      lx = Sin(th);
      lz = -Cos(th);
      gluLookAt(camX,eyeHeight,camZ , camX + lx,eyeHeight,camZ + lz , 0,1,0);
   }
   // Orthogonal - set world orientation
   else
   {
      glRotatef(ph, 1, 0, 0);
      glRotatef(th, 0, 1, 0);
   }
   // Draw the ground and roof planes
   plane(0, 0, 0, 10.0); // Ground
   plane(0, 4, 0, 10.0); // Roof
   //  Draw cubes
   float cubeDim = 0.6; // 1/2 width, height, length of cube
   chest(0, cubeDim, 0, 1.5*cubeDim, cubeDim, cubeDim, 0);
   for (int c = 0; c < NUM_CONES; c++) {
      // Stalagtites
      cone(true, cones[c][0], 4, cones[c][1]);
      // Stalagmites
      cone(false, cones[c][0], 0, cones[c][1]);
   }
   //  Draw axes
   if (axes) {
      glColor3f(1,1,1);
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
   Print("Angle=%d,%d  Dim=%.1f FOV=%d",th,ph,dim,fov);
   //  Render the scene and make it visible
   glFlush();
   glutSwapBuffers();
   ErrCheck("called in display()");
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
   else if (key == GLUT_KEY_UP) {
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   } else if (key == GLUT_KEY_DOWN) {
      ph -= 5;
   //  PageUp key - increase dim
   } else if (key == GLUT_KEY_PAGE_UP)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_DOWN && dim>1)
      dim -= 0.1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project();
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
   else if (ch == '0') {
      th = -45;
      ph = 0;
   //  Switch display mode
   } else if (ch == 'm')
      mode = !mode;
   //  Change field of view angle
   else if (ch == '-' && ch>1)
      fov--;
   else if (ch == '+' && ch<179)
      fov++;
   //  Directional movements
   //  Turn right
   else if (ch == 'd')
      th += 5;
   //  Turn left
   else if (ch == 'a')
      th -= 5;
   //  Move forward
   else if (ch == 'w') {
      camX += lx * dt;
      camZ += lz * dt;
   //  Move backwards
   } else if (ch == 's') {
      camX -= lx * dt;
      camZ -= lz * dt;
   }
   //  Reproject
   Project();
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 * GLUT calls this routine when mouse click event takes place
 */
void mouseButton(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_UP) {
            mouseDown = false;
        } else {
            mouseDown = true;
            mouseX = x;
            mouseY = y;
        }
    // Scrolling for zooming in and out
    } else if (button == 3) {
        dim -= 0.1;
        glutPostRedisplay();
    } else if (button == 4) {
        dim += 0.1;
        glutPostRedisplay();
    }

}

/*
 * GLUT calls this routine when mouse click event takes place
 */
void mouseMove(int x, int y)
{
    if (mouseDown) {
        th = (th + (x - mouseX)) % 360;
        ph = (ph + (y - mouseY)) % 360;
        mouseX = x;
        mouseY = y;
        glutPostRedisplay();
    }
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
   Project();
}

void setUp()
{
   // Seed the random number generator
   srand(time(NULL));
   // Define position with offsets for cones
   for (int row = 0; row < rows; row++) {
      for (int col = 0; col < cols; col++) {
         // Offsets will be between -0.5 and 0.5 inclusive
         cones[row * cols + col][0] = 2*(row - rows/2 + (float)rand()/(float)RAND_MAX - 0.5); // x offset added to position
         cones[row * cols + col][1] = 2*(col - cols/2 + (float)rand()/(float)RAND_MAX - 0.5); // z offset added to position
      }
   }
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   // Set up some values for the program to come
   setUp();
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow("Assignment 2: Andrew Gaines");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   // Tell GLUT to call "mouseButton" when mouse click/scroll event occurs
   glutMouseFunc(mouseButton);
   // Tell GLUT to call "mouseMove" when mouse move event occurs
   glutMotionFunc(mouseMove);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
