/*
 *  Lighting and Textures
 *
 *  Draw cave scene to demonstrate lighting and textures
 *
 *  Key bindings:
  m          Toggle modes between orthogonal (0), perspective (1), first-person navigation (2)
  +/-        Changes field of view for perspective
  w					 Move forward (only in mode 2)
  a          Turn left
  s					 Move backwards (only in mode 2)
  d					 Turn right
  l/r arrows     Change view angle
  PgDn/PgUp  Zoom in and out (mode 0)
  z          Stop/start light source moving
  b/B        Change ambient light levels
  e/E        Change emission amount
  i/I        Change diffuse lighting amount
  s/S        Change specular amount
  n/N        Change shininess
  0          Reset view angle
  ESC        Exit
 */
#include "CSCIx229.h"
int mode = 2;       // Start in first person view
int th = -45;         //  Azimuth of view angle
int ph = 5;         //  Elevation of view angle
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
#define NUM_CONES 121
double cones[NUM_CONES][2];
int rows = 11;
int cols = 11;
unsigned int textures[2]; // Texture names (IDs)
// Light values
bool light = true;
int emission  =   0;  // Emission intensity (%)
int ambient   =  10;  // Ambient intensity (%)
int diffuse   = 100;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shinyvec[1];    // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   1.0;  // Elevation of light
float radiusLight = 9.0; // Radius of path which light source follows
bool radiusIncrease = false; // Determines whether light source moves farther/closer to center
bool lightMove = true; // Determines if light should move or be stationary

/*
 *  Draw a chest
 *     at (x,y,z)
 *     dimentions (dx,dy,dz)
 *     rotated th about the y axis
 */
static void chest(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th)
{
   //  Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.0*emission,1.0};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   float darken = 0.9;
   // Brown shade 1 (darker)
   float br1R = 0.7*darken;
   float br1G = 0.4*darken;
   float br1B = 0.133*darken;
   // Brown shade 2 (lighter)
   float br2R = 0.5*darken;
   float br2G = 0.25*darken;
   float br2B = 0.0;
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   // Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, textures[0]);
   //  Begin drawing chest
   glBegin(GL_QUADS);
   //  Front
   glColor3f(br2R, br2G, br2B);
   glNormal3f(0, 0, 1);
   glTexCoord2f(0, 0); glVertex4f(-1,-1, 1, w);
   glTexCoord2f(1, 0); glVertex4f(+1,-1, 1, w);
   glTexCoord2f(1, 1); glVertex4f(+1,+1, 1, w);
   glTexCoord2f(0, 1); glVertex4f(-1,+1, 1, w);
   //  Back
   glColor3f(br2R, br2G, br2B);
   glNormal3f(0, 0, -1);
   glTexCoord2f(0, 0);glVertex4f(+1,-1,-1, w);
   glTexCoord2f(1, 0);glVertex4f(-1,-1,-1, w);
   glTexCoord2f(1, 1);glVertex4f(-1,+1,-1, w);
   glTexCoord2f(0, 1);glVertex4f(+1,+1,-1, w);
   //  Right
   glColor3f(br1R, br1G, br1B);
   glNormal3f(1, 0, 0);
   glTexCoord2f(0, 0); glVertex4f(+1,-1,+1, w);
   glTexCoord2f(1, 0); glVertex4f(+1,-1,-1, w);
   glTexCoord2f(1, 1); glVertex4f(+1,+1,-1, w);
   glTexCoord2f(0, 1); glVertex4f(+1,+1,+1, w);
   //  Left
   glColor3f(br1R, br1G, br1B);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0, 0); glVertex4f(-1,-1,-1, w);
   glTexCoord2f(1, 0); glVertex4f(-1,-1,+1, w);
   glTexCoord2f(1, 1); glVertex4f(-1,+1,+1, w);
   glTexCoord2f(0, 1); glVertex4f(-1,+1,-1, w);
   // Round cylindrical part of lid
   int angleStep = 15;
   for (int angle = 0; angle < 180; angle += angleStep) {
      if (angle % (2*angleStep) != 0) { // Toggle between 2 browns
         glColor3f(br2R, br2G, br2B);
      } else {
         glColor3f(br1R, br1G, br1B);
      }
      glNormal3f(0, Sin(angle), Cos(angle));
      glTexCoord2f(0, 0); glVertex3d(1, 1 + Sin(angle), Cos(angle));
      glTexCoord2f(1, 0); glVertex3d(-1, 1 + Sin(angle), Cos(angle));
      glNormal3f(0, Sin(angle + angleStep), Cos(angle + angleStep));
      glTexCoord2f(1, 1); glVertex3d(-1, 1 + Sin(angle + angleStep), Cos(angle + angleStep));
      glTexCoord2f(0, 1); glVertex3d(1, 1 + Sin(angle + angleStep), Cos(angle + angleStep));
   }
   glEnd();
   // Right side of lid
   glColor3f(br2R, br2G, br2B);
   glNormal3f(1, 0, 0);
   glBegin(GL_TRIANGLE_FAN);
   glTexCoord2f(0.5, 0); glVertex3d(1, 1, 0); // Starting point, center of fan
   for (int angle = 0; angle <= 180; angle += angleStep) {
      glTexCoord2f(Sin(angle), Cos(angle)); glVertex3d(1, 1 + Sin(angle), Cos(angle));
   }
   glEnd();
   // Left side of lid
   glNormal3f(-1, 0, 0);
   glBegin(GL_TRIANGLE_FAN);
   glTexCoord2f(0.5, 0); glVertex3d(-1, 1, 0);
   for (int angle = 0; angle <= 180; angle += angleStep) {
      glTexCoord2f(Sin(angle), Cos(angle)); glVertex3d(-1, 1 + Sin(angle), Cos(angle));
   }
   //  End
   glEnd();
   //  Undo transformations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

void cone(bool down, double x, double y, double z)
{
   if (!down && x < 2 && x > -2 && z < 1 && z > -1) {
      // Guard against stalagmites under chest
      return;
   }
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   glPushMatrix();
   glTranslated(x, y, z);
   glRotated(0, 0, 0, 0);
   // Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, textures[1]);
   // Stalagtites (hanging down on roof) will be larger than stalagmites on ground
   float radius = (down) ? 0.2 : 0.1;
   float height = (down) ? 1.5 : 0.75;
   glScaled(radius, height, radius);
   glBegin(GL_TRIANGLES);
   glColor3f(0.2, 0.2, 0.2);
   int degreeStep = 15;
   for (int degree = 0; degree <= 360; degree += degreeStep) {
      glNormal3f(Cos(degree + degreeStep/2.0), 1, Sin(degree + degreeStep/2.0));
      // Tip of cone
      glTexCoord2f(0, 0); glVertex3d(0, (down) ? -1 : 1, 0);
      glNormal3f(Cos(degree), 1, Sin(degree));
      glTexCoord2f(1, 0); glVertex3d(Cos(degree), 0, Sin(degree));
      glNormal3f(Cos(degree + degreeStep), 1, Sin(degree + degreeStep));
      glTexCoord2f(Cos(degreeStep), Sin(degreeStep)); glVertex3d(Cos(degree + degreeStep), 0, Sin(degree + degreeStep));
   }
   glEnd();
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

void plane(double x, double y, double z, double stretchFactor, int divs)
{
   //  Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x, y, z);
   glRotated(0, 0, 0, 0);
   glScaled(stretchFactor, stretchFactor, stretchFactor);
   // Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, textures[1]);

   glBegin(GL_QUADS);
   double divLen = 2.0 / divs;
   glColor3f(0.15, 0.15, 0.15);
   for (int row = 0; row < divs; row++) {
      for (int col = 0; col < divs; col++) {
         glTexCoord2f(0, 0); glVertex4f(divLen*col - 1, 0, divLen*row - 1, w);
         glTexCoord2f(1, 0); glVertex4f(divLen*col + divLen - 1, 0, divLen*row - 1, w);
         glTexCoord2f(1, 1); glVertex4f(divLen*col + divLen - 1, 0, divLen*row + divLen - 1, w);
         glTexCoord2f(0, 1); glVertex4f(divLen*col - 1, 0, divLen*row + divLen - 1, w);
      }
   }
   glEnd();
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

void lightSource(double x, double y, double z, double r)
{
   glPushMatrix();
   glTranslated(x, y, z);
   glScaled(r, r, r);
   // Begin drawing: Front
   glColor3f(1, 1, 1);
   glBegin(GL_QUADS);
   glNormal3f(0, 0, 1);
   glVertex3f(-1,-1, 1);
   glVertex3f(+1,-1, 1);
   glVertex3f(+1,+1, 1);
   glVertex3f(-1,+1, 1);
   glEnd();
   //  Back
   glColor3f(1,1,1);
   glBegin(GL_QUADS);
   glNormal3f(0, 0, -1);
   glVertex3f(+1,-1,-1);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,+1,-1);
   glVertex3f(+1,+1,-1);
   glEnd();
   //  Right
   glColor3f(1,1,1);
   glBegin(GL_QUADS);
   glNormal3f(1, 0, 0);
   glVertex3f(+1,-1,+1);
   glVertex3f(+1,-1,-1);
   glVertex3f(+1,+1,-1);
   glVertex3f(+1,+1,+1);
   glEnd();
   //  Left
   glColor3f(1,1,1);
   glBegin(GL_QUADS);
   glNormal3f(-1, 0, 0);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,-1,+1);
   glVertex3f(-1,+1,+1);
   glVertex3f(-1,+1,-1);
   glEnd();
   //  Top (Pyramid shape)
   glColor3f(1,1,0);
   glBegin(GL_TRIANGLES);
   // Front face
   glNormal3f(0, 1, 1);
   glVertex3f(0, 2, 0);
   glVertex3f(-1,+1,1);
   glVertex3f(1,+1,1);
   // Right face
   glNormal3f(1, 1, 0);
   glVertex3f(0, 2, 0);
   glVertex3f(+1,+1,+1);
   glVertex3f(+1,+1,-1);
   // Left face
   glNormal3f(-1, 1, 0);
   glVertex3f(0, 2, 0);
   glVertex3f(-1,+1,-1);
   glVertex3f(-1,+1,+1);
   // Back face
   glNormal3f(0, 1, -1);
   glVertex3f(0, 2, 0);
   glVertex3f(1, 1, -1);
   glVertex3f(-1, 1, -1);
   glEnd();
   //  Bottom (pyramid shape)
   glColor3f(1,1,0);
   glBegin(GL_TRIANGLES);
   // Front face
   glNormal3f(0, -1, 1);
   glVertex3f(0, -2, 0);
   glVertex3f(-1,-1,1);
   glVertex3f(1,-1,1);
   // Right face
   glNormal3f(1, -1, 0);
   glVertex3f(0, -2, 0);
   glVertex3f(+1,-1,+1);
   glVertex3f(+1,-1,-1);
   // Left face
   glNormal3f(-1, -1, 0);
   glVertex3f(0, -2, 0);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,-1,+1);
   // Back face
   glNormal3f(0, -1, -1);
   glVertex3f(0, -2, 0);
   glVertex3f(-1, -1, 1);
   glVertex3f(-1, -1, -1);
   glEnd();
   // glutSolidSphere(1.0, 16, 16);
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
   if (mode == 2)
   {
      //  First person navigation - set eye position
      lx = Sin(th);
      lz = -Cos(th);
      gluLookAt(camX,eyeHeight,camZ , camX + lx,eyeHeight + Sin(ph),camZ + lz , 0,1,0);
   } else if (mode == 1) {
      // Perspective mode without first person view
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }
   // Orthogonal - set world orientation
   else
   {
      glRotatef(ph, 1, 0, 0);
      glRotatef(th, 0, 1, 0);
   }


   float Position[] = {radiusLight*Cos(zh), ylight, radiusLight*Sin(zh), 1};
   if (light)
   {
      // Translate intensity to color vectors
      float Ambient[] = {0.01*ambient, 0.01*ambient, 0.01*ambient, 1.0};
      float Diffuse[] = {0.01*diffuse, 0.01*diffuse, 0.01*diffuse, 1.0};
      float Specular[] = {0.01*specular, 0.01*specular, 0.01*specular, 1.0};
      // Draw light position
      glColor3f(1, 1, 1);
      lightSource(Position[0], Position[1], Position[2], 0.1);
      // OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      // Enable lighting!
      glEnable(GL_LIGHTING);
      // glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      // Enable light 0
      glEnable(GL_LIGHT0);
      // Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse);
      glLightfv(GL_LIGHT0, GL_SPECULAR, Specular);
      glLightfv(GL_LIGHT0, GL_POSITION, Position);
   }
   else
      glDisable(GL_LIGHTING);

   // Draw the ground and roof planes
   glNormal3f(0, 1, 0);
   plane(0, 0, 0, rows + 0.5, 8); // Ground
   glNormal3f(0, -1, 0);
   plane(0, 4, 0, rows + 0.5, 8); // Roof
   //  Draw cubes
   float cubeDim = 0.6; // 1/2 width, height, length of cube
   chest(0, cubeDim, 0, 1.5*cubeDim, cubeDim, cubeDim, 0);
   for (int c = 0; c < NUM_CONES; c++) {
      // Stalagtites
      cone(true, cones[c][0], 4, cones[c][1]);
      // Stalagmites
      cone(false, cones[c][0], 0, cones[c][1]);
   }

   // No need for lighting from here on
   glDisable(GL_LIGHTING);
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
   Print("Angle=%d,%d  Dim=%.1f FOV=%d mode=%d",th,ph,dim,fov,mode);
   //  Render the scene and make it visible
   glFlush();
   glutSwapBuffers();
   ErrCheck("display()");
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
   Project(45, asp, dim);
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
   } else if (ch == 'm') {
      mode += 1;
      mode %= 3;
   //  Change field of view angle
   } else if (ch == '-' && ch>1)
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
   else if (ch=='b' && ambient>0)
      ambient -= 5;
   else if (ch=='B' && ambient<100)
      ambient += 5;
   //  Diffuse level
   else if (ch=='i' && diffuse>0)
      diffuse -= 5;
   else if (ch=='I' && diffuse<100)
      diffuse += 5;
   //  Specular level
   else if (ch=='p' && specular>0)
      specular -= 5;
   else if (ch=='P' && specular<100)
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
   else if (ch == 'z')
      lightMove = !lightMove;
   shinyvec[0] = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project(45, asp, dim);
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
   Project(45, asp, dim);
}

/*
 * Called only once in main function
 * Generate variables needed for display functions
 */
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

void idle()
{
   if (lightMove) {
      // Elapsed time in seconds
      double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
      zh = fmod(90*t, 360.0);
      // Necessary to redisplay
      glutPostRedisplay();

      if (radiusLight >= 9.0)
         radiusIncrease = false;
      else if (radiusLight <= 3.0)
         radiusIncrease = true;

      if (radiusIncrease)
         radiusLight += 0.01;
      else
         radiusLight -= 0.01;
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
   glutInitWindowSize(800,600);
   glutCreateWindow("Assignment 3: Andrew Gaines");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   // Tell GLUT to call "mouseButton" when mouse click/scroll event occurs
   glutMouseFunc(mouseButton);
   // Tell GLUT to call "mouseMove" when mouse move event occurs
   glutMotionFunc(mouseMove);
   // Load textures
   textures[0] = LoadTexBMP("textures/wood.bmp");
   textures[1] = LoadTexBMP("textures/roughrock.bmp");
   ErrCheck("main"); // Sanity check
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
