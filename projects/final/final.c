/*
 *  Final Project
 *
 *  Create simple game in cave scene
 *
 *  For controls, see README
 */
#include "CSCIx229.h"
int mode = 2;       // Start in first person view
int th = 0;         //  Azimuth of view angle
int ph = 0;         //  Elevation of view angle
int fov = 45;       //  Field of view (for perspective)
double asp = 1;     //  Aspect ratio
double dim = 9.0;   //  Size of world
double w = 1;       // W variable
// Globals determining camera position/orientation
#define PLAYER_HEIGHT 1.75
double camX = 0.0;
double camZ = 20.0;
double camY = PLAYER_HEIGHT; // Height of eyes above the ground (in meters). Changes later
float crawlHeight = 0.5; // Used for later calculations
float lx = 0.0f, lz = -1.0f; // actual vector representing the camera's direction
float dt = 0.125f; // Change in time between scenes
// Initialize stalagtite (roof) /stalagmite (ground) variables
// Repeating pattern based on number used
#define NUM_OFFSETS 81
double offsets[NUM_OFFSETS][3]; // x, y offsets, z offset of tip
float offsetMagnitude = 0.625; // Maximum offset possible
unsigned int textures[5]; // Texture names (IDs)
float chestDim = 0.6; // height, length, 1/2 width of chest
int chestPos; // Randomized position of chest, 4 possible locations
int keyPos; // Randomized position of key, any other location except where chest is placed
// Speed of walking
#define WALK_SPEED 5.0 / 1000
float currentSpeed = WALK_SPEED;
bool lightAcquired = false; // Trigger controlling whether light is rotating around player
bool keyAcquired = false; // Determines whether chest can be opened or not
bool chestUnlocked = false; // Determines when chest opens, just before treasure acquired
bool treasureAcquired = false; // treasureAcquired triggered when treasure taken
bool hint = false;
bool victory = false;
bool gameOver = false;
float collapseStart = 0.0; // Timer set when collapse begins
float collapseTime = 45*1000; // # seconds (x1000 for msec) cave takes to crush down to collapseHeight
float collapseHeight = 1.0; // Height to which cave collapses. Game over by crushing if reached
float collapsePercent = 0.0; // 0-1 for amount cave crushed (ceiling coming down, stops at collapseHeight before final crumbling
bool collapseImminent = false; // Set when near to total collapse after reaching collapse height. Used to control shaking
float rumblePercent = 0.82; // Cotrols when rumbling of cave begins
float crushStopPercent = 0.75; // Controls when the ceiling stops moving downwards
float rumbleX = 0.0; // How much the X axis is rumbled when shaking
float rumbleZ = 0.0; // How much Z axis is rumbled when shaking
// Light values
bool light = true;
int at0=100;      //  Constant  attenuation %
int at1=0;        //  Linear    attenuation %
int at2=5;        //  Quadratic attenuation %
int side = 0; // Two sided mode
int local = 0;
int emission  =   50;  // Emission intensity (%)
int ambient   =  0;  // Ambient intensity (%)
int diffuse   = 100;  // Diffuse intensity (%)
int specular  =   35;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shinyvec[1];    // Shininess (value)
int zh        =  90;  // Light azimuth
float lightY  =   1.2;  // Elevation of light
float lightX = 35;
float lightZ = -30;
bool lightMove = true; // Determines if light should move or be stationary
// Represents where the player cannot walk
int numRailings = 0;
double railings[79][5];
float railingOverhang = 0.5;
bool railingsLoaded = false;

double max(double x1, double x2)
{
   return (x1 > x2) ? x1 : x2;
}

double min(double x1, double x2)
{
   return (x1 < x2) ? x1 : x2;
}

/*
 * Add a railing to the array to be checked for collision avoidance purposes
 * Ensure array only built once, flag set in display() after first build
 */
void addRailing(double x1, double z1, double x2, double z2, double y)
{
   if (!railingsLoaded) {
      railings[numRailings][0] = x1;
      railings[numRailings][1] = z1;
      railings[numRailings][2] = x2;
      railings[numRailings][3] = z2;
      railings[numRailings][4] = y;
      numRailings++; // Increment number of railings in prep for next addition
   }
}

/*
 * Calculates the distance between the player and the railing specified
 * If outside of the bounds, return 99
 */
double distRailing(int id)
{
   double x1 = railings[id][0];
   double z1 = railings[id][1];
   double x2 = railings[id][2];
   double z2 = railings[id][3];

   if (x1 == x2) {
      // Wall stretching from front to back
      if (max(z1, z2) + railingOverhang < camZ || min(z1, z2) - railingOverhang > camZ) {
         // Outside the bounds of the line segment
         return 99;
      }
   } else {
      // Wall stretching from left to right
      if (max(x1, x2) + railingOverhang < camX || min(x1, x2) - railingOverhang > camX) {
         // Outside the bounds of the line segment
         return 99;
      }
   }

   double num = abs((z2 - z1)*camX - (x2 - x1)*camZ + x2*z1 - z2*x1);
   double den = sqrt((z2 - z1)*(z2 - z1) + (x2 - x1)*(x2 - x1));

   return num / den;
}

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
   glDisable(GL_CULL_FACE);
   //  Offset
   glTranslated(x + rumbleX,y,z + rumbleZ);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   // Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, textures[0]);
   //  Begin drawing chest
   //  Front
   glColor3f(br2R, br2G, br2B);
   glNormal3f(0, 0, 1);
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(1, 1);
   glBegin(GL_QUADS);
   glTexCoord2f(0, 0); glVertex4f(-1,-1, 1, w);
   glTexCoord2f(1, 0); glVertex4f(+1,-1, 1, w);
   glTexCoord2f(1, 1); glVertex4f(+1,+1, 1, w);
   glTexCoord2f(0, 1); glVertex4f(-1,+1, 1, w);
   glEnd();
   glDisable(GL_POLYGON_OFFSET_FILL);
   // Keyhole on front
   glBegin(GL_POLYGON);
   glColor3f(0, 0, 0);
   // Hexagon for the top circle part of keyhole
   glVertex4f(0, 0.25, 1, w); // Bottom middle of hexagon
   glVertex4f(-0.433/6, 0.3125, 1, w); // Moving counter clockwise
   glVertex4f(-0.433/6, 0.4375, 1, w);
   glVertex4f(0, 0.5, 1, w);
   glVertex4f(0.433/6, 0.4375, 1, w);
   glVertex4f(0.433/6, 0.3125, 1, w);
   glEnd();
   // Triangle part of keyhole extending to bottom
   glBegin(GL_TRIANGLES);
   glVertex4f(0, 0.375, 1, w);
   glVertex4f(-0.433/7, 0.1, 1, w);
   glVertex4f(0.433/7, 0.1, 1, w);
   glEnd();
   //  Back
   glBegin(GL_QUADS);
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
   //  Bottom (required for when lid open)
   glColor3f(br1R, br1G, br1B);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0); glVertex4f(-1, -0.9, 1, w);
   glTexCoord2f(1, 0); glVertex4f(1, -0.9, 1, w);
   glTexCoord2f(1, 1); glVertex4f(1, -0.9, -1, w);
   glTexCoord2f(0, 1); glVertex4f(-1, -0.9, -1, w);
   glEnd();

   if (!chestUnlocked) { // Lid is removed when unlocked
      // Round cylindrical part of lid
      int angleStep = 15;
      glBegin(GL_QUADS);
      for (int angle = 0; angle < 180; angle += angleStep) {
         if (angle % (2*angleStep) != 0) { // Toggle between 2 browns
            glColor3f(br2R, br2G, br2B);
         } else {
            glColor3f(br1R, br1G, br1B);
         }
         glNormal3f(0, Sin(angle), Cos(angle));
         glTexCoord2f(1, 0); glVertex3d(-1, 1 + Sin(angle), Cos(angle));
         glTexCoord2f(0, 0); glVertex3d(1, 1 + Sin(angle), Cos(angle));
         glNormal3f(0, Sin(angle + angleStep), Cos(angle + angleStep));
         glTexCoord2f(0, 1); glVertex3d(1, 1 + Sin(angle + angleStep), Cos(angle + angleStep));
         glTexCoord2f(1, 1); glVertex3d(-1, 1 + Sin(angle + angleStep), Cos(angle + angleStep));
      }
      glEnd();
      // Right side of lid
      glColor3f(br2R, br2G, br2B);
      glNormal3f(1, 0, 0);
      glBegin(GL_TRIANGLE_FAN);
      glTexCoord2f(0.5, 0); glVertex3d(1, 1, 0); // Starting point, center of fan
      for (int angle = 180; angle >= 0; angle -= angleStep) {
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
      glEnd();
   }

   // Show treasure in chest until it is both unlocked and acquired
   if (!treasureAcquired) {
      glBindTexture(GL_TEXTURE_2D, textures[3]);
      glBegin(GL_QUADS);
      // Front canvas of the painting
      glNormal3f(0, 0, 1);
      glTexCoord2f(0.03, 0); glVertex4f(-0.8, 0.6, 0, w);
      glTexCoord2f(1, 0); glVertex4f(0.8, 0.6, 0, w);
      glTexCoord2f(1, 1); glVertex4f(0.8, 1.8, 0, w);
      glTexCoord2f(0.03, 1); glVertex4f(-0.8, 1.8, 0, w);
      glEnd();

      // Wooden frame of painting
      glBindTexture(GL_TEXTURE_2D, textures[0]);
      glBegin(GL_QUADS);
      // Left edge
      glNormal3f(-1, 0, 1);
      glTexCoord2f(0, 0); glVertex4f(-0.8, 0.6, 0, w);
      glTexCoord2f(1, 0); glVertex4f(-0.8, 1.8, 0, w);
      glTexCoord2f(1, 1); glVertex4f(-0.9, 1.9, -0.1, w);
      glTexCoord2f(0, 1); glVertex4f(-0.9, 0.5, -0.1, w);
      // Bottom edge
      glNormal3f(0, -1, 1);
      glTexCoord2f(0, 0); glVertex4f(-0.9, 0.5, -0.1, w);
      glTexCoord2f(1, 0); glVertex4f(0.9, 0.5, -0.1, w);
      glTexCoord2f(1, 1); glVertex4f(0.8, 0.6, 0, w);
      glTexCoord2f(0, 1); glVertex4f(-0.8, 0.6, 0, w);
      // Top edge
      glNormal3f(0, 1, 1);
      glTexCoord2f(0, 0); glVertex4f(-0.8, 1.8, 0, w);
      glTexCoord2f(1, 0); glVertex4f(0.8, 1.8, 0, w);
      glTexCoord2f(1, 1); glVertex4f(0.9, 1.9, -0.1, w);
      glTexCoord2f(0, 1); glVertex4f(-0.9, 1.9, -0.1, w);
      // Right edge
      glNormal3f(1, 0, 1);
      glTexCoord2f(0, 0); glVertex4f(0.8, 1.8, 0, w);
      glTexCoord2f(1, 0); glVertex4f(0.8, 0.6, 0, w);
      glTexCoord2f(1, 1); glVertex4f(0.9, 0.5, -0.1, w);
      glTexCoord2f(0, 1); glVertex4f(0.9, 1.9, -0.1, w);
      // Back of frame
      glNormal3f(0, 0, -1);
      glTexCoord2f(0, 0); glVertex4f(0.9, 0.5, -0.1, w);
      glTexCoord2f(1, 0); glVertex4f(-0.9, 0.5, -0.1, w);
      glTexCoord2f(1, 1); glVertex4f(-0.9, 1.9, -0.1, w);
      glTexCoord2f(0, 1); glVertex4f(0.9, 1.9, -0.1, w);
      glEnd();
   }
   //  Undo transformations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
   glEnable(GL_CULL_FACE);
}

/*
 * Key used to unlock the treasure chest
 */
void key(double x, double y, double z, int th, int ph)
{
   glPushMatrix();
   glTranslated(x + rumbleX, y, z + rumbleZ);
   glRotated(th, 0, 1, 0);
   glRotated(ph, 1, 0, 0);
   glScaled(0.2, 0.2, 0.2);
   glColor3f(1, 1, 0);
   glBegin(GL_QUADS);
   // Front of the key
   glNormal3f(0, 0, 1);
   // Top handle of key
   // Left part
   glVertex4f(-0.5, 2, 0.05, w);
   glVertex4f(-0.4, 2, 0.05, w);
   glVertex4f(-0.4, 3, 0.05, w);
   glVertex4f(-0.5, 3, 0.05, w);
   // Top part
   glVertex4f(-0.4, 2.9, 0.05, w);
   glVertex4f(0.4, 2.9, 0.05, w);
   glVertex4f(0.4, 3, 0.05, w);
   glVertex4f(-0.4, 3, 0.05, w);
   // Right part
   glVertex4f(0.4, 2, 0.05, w);
   glVertex4f(0.5, 2, 0.05, w);
   glVertex4f(0.5, 3, 0.05, w);
   glVertex4f(0.4, 3, 0.05, w);
   // Bottom part
   glVertex4f(-0.4, 2, 0.05, w);
   glVertex4f(0.4, 2, 0.05, w);
   glVertex4f(0.4, 2.1, 0.05, w);
   glVertex4f(-0.4, 2.1, 0.05, w);
   // Long rod of key
   glVertex4f(-0.1, 0, 0.05, w);
   glVertex4f(0.1, 0, 0.05, w);
   glVertex4f(0.1, 2, 0.05, w);
   glVertex4f(-0.1, 2, 0.05, w);
   // Lowest tine
   glVertex4f(0.1, 0.5, 0.05, w);
   glVertex4f(0.5, 0.5, 0.05, w);
   glVertex4f(0.5, 0.6, 0.05, w);
   glVertex4f(0.1, 0.6, 0.05, w);
   // Higher tine
   glVertex4f(0.1, 0.9, 0.05, w);
   glVertex4f(0.3, 0.9, 0.05, w);
   glVertex4f(0.3, 1, 0.05, w);
   glVertex4f(0.1, 1, 0.05, w);

   // Back side of key
   glNormal3f(0, 0, -1);
   // Top handle of key
   // Right part (from front)
   glVertex4f(0.5, 2, -0.05, w);
   glVertex4f(0.4, 2, -0.05, w);
   glVertex4f(0.4, 3, -0.05, w);
   glVertex4f(0.5, 3, -0.05, w);
   // Top part
   glVertex4f(0.4, 2.9, -0.05, w);
   glVertex4f(-0.4, 2.9, -0.05, w);
   glVertex4f(-0.4, 3, -0.05, w);
   glVertex4f(0.4, 3, -0.05, w);
   // Left part (from front)
   glVertex4f(-0.4, 2, -0.05, w);
   glVertex4f(-0.5, 2, -0.05, w);
   glVertex4f(-0.5, 3, -0.05, w);
   glVertex4f(-0.4, 3, -0.05, w);
   // Bottom part
   glVertex4f(0.4, 2, -0.05, w);
   glVertex4f(-0.4, 2, -0.05, w);
   glVertex4f(-0.4, 2.1, -0.05, w);
   glVertex4f(0.4, 2.1, -0.05, w);
   // Long rod part of key
   glVertex4f(0.1, 0, -0.05, w);
   glVertex4f(-0.1, 0, -0.05, w);
   glVertex4f(-0.1, 2, -0.05, w);
   glVertex4f(0.1, 2, -0.05, w);
   // Lowest tine
   glVertex4f(0.5, 0.5, -0.05, w);
   glVertex4f(0.1, 0.5, -0.05, w);
   glVertex4f(0.1, 0.6, -0.05, w);
   glVertex4f(0.5, 0.6, -0.05, w);
   // Higher tine
   glVertex4f(0.3, 0.9, -0.05, w);
   glVertex4f(0.1, 0.9, -0.05, w);
   glVertex4f(0.1, 1, -0.05, w);
   glVertex4f(0.3, 1, -0.05, w);

   // Left faces of key
   glNormal3f(-1, 0, 0);
   // Inside of handle
   glVertex4f(0.4, 2.1, -0.05, w);
   glVertex4f(0.4, 2.1, 0.05, w);
   glVertex4f(0.4, 2.9, 0.05, w);
   glVertex4f(0.4, 2.9, -0.05, w);
   // Outside of handle
   glVertex4f(-0.5, 2, -0.05, w);
   glVertex4f(-0.5, 2, 0.05, w);
   glVertex4f(-0.5, 3, 0.05, w);
   glVertex4f(-0.5, 3, -0.05, w);
   // Long rod
   glVertex4f(-0.1, 0, -0.05, w);
   glVertex4f(-0.1, 0, 0.05, w);
   glVertex4f(-0.1, 2, 0.05, w);
   glVertex4f(-0.1, 2, -0.05, w);

   // Right faces of key
   glNormal3f(1, 0, 0);
   // Inside of handle
   glVertex4f(-0.4, 2.1, 0.05, w);
   glVertex4f(-0.4, 2.1, -0.05, w);
   glVertex4f(-0.4, 2.9, -0.05, w);
   glVertex4f(-0.4, 2.9, 0.05, w);
   // Outside of handle
   glVertex4f(0.5, 2, 0.05, w);
   glVertex4f(0.5, 2, -0.05, w);
   glVertex4f(0.5, 3, -0.05, w);
   glVertex4f(0.5, 3, 0.05, w);
   // Long rod
   glVertex4f(0.1, 0, 0.05, w);
   glVertex4f(0.1, 0, -0.05, w);
   glVertex4f(0.1, 2, -0.05, w);
   glVertex4f(0.1, 2, 0.05, w);
   // Tip of lowest tine
   glVertex4f(0.5, 0.5, 0.05, w);
   glVertex4f(0.5, 0.5, -0.05, w);
   glVertex4f(0.5, 0.6, -0.05, w);
   glVertex4f(0.5, 0.6, 0.05, w);
   // Top of higher tine
   glVertex4f(0.3, 0.9, 0.05, w);
   glVertex4f(0.3, 0.9, -0.05, w);
   glVertex4f(0.3, 1, -0.05, w);
   glVertex4f(0.3, 1, 0.05, w);

   // Bottom faces of key
   glNormal3f(0, -1, 0);
   // Tip of end of key (very bottom)
   glVertex4f(-0.1, 0, 0.05, w);
   glVertex4f(-0.1, 0, -0.05, w);
   glVertex4f(0.1, 0, -0.05, w);
   glVertex4f(0.1, 0, 0.05, w);
   // Inside of key
   glVertex4f(-0.4, 2.9, 0.05, w);
   glVertex4f(-0.4, 2.9, -0.05, w);
   glVertex4f(0.4, 2.9, -0.05, w);
   glVertex4f(0.4, 2.9, 0.05, w);
   // Bottom of key handle
   glVertex4f(-0.5, 2, 0.05, w);
   glVertex4f(-0.5, 2, -0.05, w);
   glVertex4f(0.5, 2, -0.05, w);
   glVertex4f(0.5, 2, 0.05, w);
   // Bottom of lowest tine
   glVertex4f(0.1, 0.5, 0.05, w);
   glVertex4f(0.1, 0.5, -0.05, w);
   glVertex4f(0.5, 0.5, -0.05, w);
   glVertex4f(0.5, 0.5, 0.05, w);
   // Bottom of higher tine
   glVertex4f(0.1, 0.9, 0.05, w);
   glVertex4f(0.1, 0.9, -0.05, w);
   glVertex4f(0.3, 0.9, -0.05, w);
   glVertex4f(0.3, 0.9, 0.05, w);
   
   // Top faces of key
   glNormal3f(0, 1, 0);
   // Inside of key
   glVertex4f(-0.4, 2.1, -0.05, w);
   glVertex4f(-0.4, 2.1, 0.05, w);
   glVertex4f(0.4, 2.1, 0.05, w);
   glVertex4f(0.4, 2.1, -0.05, w);
   // Top of key
   glVertex4f(-0.5, 3, -0.05, w);
   glVertex4f(-0.5, 3, 0.05, w);
   glVertex4f(0.5, 3, 0.05, w);
   glVertex4f(0.5, 3, -0.05, w);
   // Bottom of lowest tine
   glVertex4f(0.1, 0.6, -0.05, w);
   glVertex4f(0.1, 0.6, 0.05, w);
   glVertex4f(0.5, 0.6, 0.05, w);
   glVertex4f(0.5, 0.6, -0.05, w);
   // Bottom of higher tine
   glVertex4f(0.1, 1, -0.05, w);
   glVertex4f(0.1, 1, 0.05, w);
   glVertex4f(0.3, 1, 0.05, w);
   glVertex4f(0.3, 1, -0.05, w);
   
   glEnd();
   glPopMatrix();
}

/*
 * Single crystal stalk, aggregated for a larger arrangement by crystalArrangement
 */
void crystal(double x, double y, double z, double dx, double dy, double dz, int rotX, int rotY, int rotZ)
{
   glPushMatrix();
   glTranslated(x, y, z);
   glRotated(rotX, 1, 0, 0);
   glRotated(rotY, 0, 1, 0);
   glRotated(rotZ, 0, 0, 1);
   glScaled(dx, dy, dz);

   float root3Over2 = 0.866; // In case compiler doesn't replace calculations automatically

   glBegin(GL_QUADS);
   // Base of crystal
   // Front face
   glNormal3f(0, 0, 1);
   glVertex4f(-0.5, 0, root3Over2, w);
   glVertex4f(0.5, 0, root3Over2, w);
   glVertex4f(0.5, 2, root3Over2, w);
   glVertex4f(-0.5, 2, root3Over2, w);
   // Right front face
   glNormal3f(1, 0, 1);
   glVertex4f(0.5, 0, root3Over2, w);
   glVertex4f(1, 0, 0, w);
   glVertex4f(1, 2, 0, w);
   glVertex4f(0.5, 2, root3Over2, w);
   // Right back face
   glNormal3f(1, 0, -1);
   glVertex4f(1, 0, 0, w);
   glVertex4f(0.5, 0, -root3Over2, w);
   glVertex4f(0.5, 2, -root3Over2, w);
   glVertex4f(1, 2, 0, w);
   // Back face
   glNormal3f(0, 0, -1);
   glVertex4f(0.5, 0, -root3Over2, w);
   glVertex4f(-0.5, 0, -root3Over2, w);
   glVertex4f(-0.5, 2, -root3Over2, w);
   glVertex4f(0.5, 2, -root3Over2, w);
   // Left back face
   glNormal3f(-1, 0, -1);
   glVertex4f(-0.5, 0, -root3Over2, w);
   glVertex4f(-1, 0, 0, w);
   glVertex4f(-1, 2, 0, w);
   glVertex4f(-0.5, 2, -root3Over2, w);
   // Left front face
   glNormal3f(-1, 0, 1);
   glVertex4f(-1, 0, 0, w);
   glVertex4f(-0.5, 0, root3Over2, w);
   glVertex4f(-0.5, 2, root3Over2, w);
   glVertex4f(-1, 2, 0, w);

   glEnd(); // End of crystal base

   glBegin(GL_TRIANGLES);
   // Tip of crystal
   // Front face
   glNormal3f(0, 0, 1);
   glVertex4f(-0.5, 2, root3Over2, w);
   glVertex4f(0.5, 2, root3Over2, w);
   glVertex4f(0, 3, 0, w);
   // Right front face
   glNormal3f(1, 0, 1);
   glVertex4f(0.5, 2, root3Over2, w);
   glVertex4f(1, 2, 0, w);
   glVertex4f(0, 3, 0, w);
   // Right back face
   glNormal3f(1, 0, -1);
   glVertex4f(1, 2, 0, w);
   glVertex4f(0.5, 2, -root3Over2, w);
   glVertex4f(0, 3, 0, w);
   // Back face
   glNormal3f(0, 0, -1);
   glVertex4f(0.5, 2, -root3Over2, w);
   glVertex4f(-0.5, 2, -root3Over2, w);
   glVertex4f(0, 3, 0, w);
   // Left back face
   glNormal3f(-1, 0, -1);
   glVertex4f(-0.5, 2, -root3Over2, w);
   glVertex4f(-1, 2, 0, w);
   glVertex4f(0, 3, 0, w);
   // Left front face
   glNormal3f(-1, 0, 1);
   glVertex4f(-1, 2, 0, w);
   glVertex4f(-0.5, 2, root3Over2, w);
   glVertex4f(0, 3, 0, w);

   glEnd(); // End of tip

   glPopMatrix();
}

/*
 * Small growth of a few crystals
 * Gives a cohesive look
 */
void crystalArrangement(double x, double y, double z, double rotX, double rotY, double rotZ, char color)
{
   glPushMatrix();
   
   glTranslated(x + rumbleX, y, z + rumbleZ);
   glRotated(rotX, 1, 0, 0);
   glRotated(rotY, 0, 1, 0);
   glRotated(rotZ, 0, 0, 1);
   glScaled(0.2, 0.2, 0.2);

   switch (color) {
      case 'b':
         glColor3f(0, 0, 0.7);
         break;
      case 'g':
         glColor3f(0, 0.7, 0);
         break;
      case 'r':
         glColor3f(0.7, 0, 0);
         break;
   }
   
   // Draw the configuration
   crystal(0, 0, 0, 0.5, 1.0, 0.5, 0, 0, -10); // Main one, tall and very slightly to left
   crystal(0, -0.3, 0, 0.4, 0.9, 0.4, -20, 0, -35); // Sticking to right
   crystal(0, -0.3, 0, 0.35, 0.8, 0.35, 30, 0, 30);
   crystal(0, -0.3, 0, 0.25, 0.75, 0.25, 30, 0, -30);
   crystal(0, -0.3, 0, 0.3, 0.7, 0.3, -40, 0, 0);
   crystal(0, -0.3, 0, 0.2, 0.7, 0.2, 50, 0, 30);
   crystal(0, -0.3, 0, 0.15, 0.6, 0.15, -50, 0, 30);

   glPopMatrix();
}

/* 
 * Stalactite or stalagmite for the cave feel
 * Detail decreases with distance for performance purposes
 */
void cone(bool down, double x, double y, double z, double dy)
{
   glPushMatrix();
   glTranslated(x + rumbleX, y, z + rumbleZ);
   glRotated(0, 0, 0, 0);
   // Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, textures[1]);
   // Stalagtites (hanging down on roof) will be larger than stalagmites on ground
   float radius = (down) ? 0.2 : 0.1;
   float height = (down) ? 1.5 : 0.75;
   glScaled(radius, height + dy, radius);
   glBegin(GL_TRIANGLES);
   glColor3f(0.25, 0.25, 0.25);
   int dist = sqrt((camX - x)*(camX - x) + (camZ - z)*(camZ - z));
   int degreeStep;
   // Optimize number of cone faces based on distance from user
   if (dist >= 30)
      degreeStep = 120;
   else if (dist >= 20)
      degreeStep = 90;
   else if (dist >= 10)
      degreeStep = 72;
   else if (dist >= 8)
      degreeStep = 60;
   else if (dist >= 5)
      degreeStep = 45;
   else
      degreeStep = 30;
   for (int degree = 0; degree <= 360; degree += degreeStep) {
      // Tip of cone
      glNormal3f(Cos(degree + degreeStep/2.0), 1, Sin(degree + degreeStep/2.0));
      glTexCoord2f(0, 0); glVertex3d(0, (down) ? -1 : 1, 0);
      // For face culling, need to order vertices based on orientation
      if (down) {
         glNormal3f(Cos(degree), 1, Sin(degree));
         glTexCoord2f(1, 0); glVertex3d(Cos(degree), 0, Sin(degree));
      }
      glNormal3f(Cos(degree + degreeStep), 1, Sin(degree + degreeStep));
      glTexCoord2f(Cos(degreeStep), Sin(degreeStep)); glVertex3d(Cos(degree + degreeStep), 0, Sin(degree + degreeStep));
      if (!down) {
         glNormal3f(Cos(degree), 1, Sin(degree));
         glTexCoord2f(1, 0); glVertex3d(Cos(degree), 0, Sin(degree));
      }
   }
   glEnd();
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

/*
 * Return how much a wall should be moved down based on starting height
 * Allows cave to reach minimum level at same time
 */
double collapseAmount(double startHeight)
{
   if (treasureAcquired)
   {
      collapsePercent = (glutGet(GLUT_ELAPSED_TIME) - collapseStart) / (collapseTime*crushStopPercent); // Roof fully lowered at crushStopPercent
      if (collapsePercent >= 1) {
         return startHeight - collapseHeight;
      }

      return collapsePercent * (startHeight - collapseHeight);
   }
   return 0;
}

/*
 * Draw a cave room, specifying whether to include walls and stalactites/stalagmites
 */
void room(double x, double y, double z, double dx, double dy, double dz, bool front, bool back, bool left, bool right, int divs, bool cones)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x + rumbleX, y, z + rumbleZ);
   // glRotated(0, 0, 0, 0);
   glScaled(dx, dy - collapseAmount(dy), dz);
   // Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, textures[1]);

   glColor3f(0.25, 0.25, 0.25);
   double divLen = 2.0 / divs; // Length of each division
   glBegin(GL_QUADS);
   
   
   // Draw the floor of the cave room
   glNormal3f(0, 1, 0);
   for (int row = 0; row < divs; row++) {
      for (int col = 0; col < divs; col++) {
         glTexCoord2f(0, 1); glVertex4f(divLen*col - 1, 0, divLen*row + divLen - 1, w);
         glTexCoord2f(1, 1); glVertex4f(divLen*col + divLen - 1, 0, divLen*row + divLen - 1, w);
         glTexCoord2f(1, 0); glVertex4f(divLen*col + divLen - 1, 0, divLen*row - 1, w);
         glTexCoord2f(0, 0); glVertex4f(divLen*col - 1, 0, divLen*row - 1, w);
      }
   }
   glEnd();
   // Draw the roof/ceiling of the cave room
   glBegin(GL_QUADS);
   glNormal3f(0, -1, 0);
   for (int row = 0; row < divs; row++) {
      for (int col = 0; col < divs; col++) {
         glTexCoord2f(0, 0); glVertex4f(divLen*col - 1, 1, divLen*row - 1, w);
         glTexCoord2f(1, 0); glVertex4f(divLen*col + divLen - 1, 1, divLen*row - 1, w);
         glTexCoord2f(1, 1); glVertex4f(divLen*col + divLen - 1, 1, divLen*row + divLen - 1, w);
         glTexCoord2f(0, 1); glVertex4f(divLen*col - 1, 1, divLen*row + divLen - 1, w);
      }
   }
   if (front) {
      // Add a railing to wall
      addRailing(x - dx, z - dz, x + dx, z - dz, PLAYER_HEIGHT);
      // Draw the front wall of the cave room
      glNormal3f(0, 0, 1);
      for (int row = 0; row < divs / 2; row++) { // Cave is originally twice as wide/long as tall, to keep floor at y=0
         for (int col = 0; col < divs; col++) {
            glTexCoord2f(0, 0); glVertex4f(divLen*col - 1, divLen*row, -1, w);
            glTexCoord2f(1, 0); glVertex4f(divLen*col + divLen - 1, divLen*row, -1, w);
            glTexCoord2f(1, 1); glVertex4f(divLen*col + divLen - 1, divLen*row + divLen, -1, w);
            glTexCoord2f(0, 1); glVertex4f(divLen*col - 1, divLen*row + divLen, -1, w);
         }
      }
   }
   if (back) {
      // Add a railing to wall
      addRailing(x - dx, z + dz, x + dx, z + dz, PLAYER_HEIGHT);
      // Draw the back wall of the cave room
      glNormal3f(0, 0, -1);
      for (int row = 0; row < divs / 2; row++) {
         for (int col = 0; col < divs; col++) {
            glTexCoord2f(0, 1); glVertex4f(divLen*col - 1, divLen*row + divLen, 1, w);
            glTexCoord2f(1, 1); glVertex4f(divLen*col + divLen - 1, divLen*row + divLen, 1, w);
            glTexCoord2f(1, 0); glVertex4f(divLen*col + divLen - 1, divLen*row, 1, w);
            glTexCoord2f(0, 0); glVertex4f(divLen*col - 1, divLen*row, 1, w);
         }
      }
   }
   if (left) {
      // Add a railing to wall
      addRailing(x - dx, z + dz, x - dx, z - dz, PLAYER_HEIGHT);
      // Draw the left wall of the cave room
      glNormal3f(1, 0, 0);
      for (int row = 0; row < divs / 2; row++) {
         for (int col = 0; col < divs; col++) {
            glTexCoord2f(0, 1); glVertex4f(-1, divLen*row + divLen, divLen*col - 1, w);
            glTexCoord2f(1, 1); glVertex4f(-1, divLen*row + divLen, divLen*col + divLen - 1, w);
            glTexCoord2f(1, 0); glVertex4f(-1, divLen*row, divLen*col + divLen - 1, w);
            glTexCoord2f(0, 0); glVertex4f(-1, divLen*row, divLen*col - 1, w);
         }
      }
   }
   if (right) {
      // Add a railing to wall
      addRailing(x + dx, z - dz, x + dx, z + dz, PLAYER_HEIGHT);
      // Draw the right wall of the cave room
      glNormal3f(-1, 0, 0);
      for (int row = 0; row < divs / 2; row++) {
         for (int col = 0; col < divs; col++) {
            glTexCoord2f(0, 0); glVertex4f(1, divLen*row, divLen*col - 1, w);
            glTexCoord2f(1, 0); glVertex4f(1, divLen*row, divLen*col + divLen - 1, w);
            glTexCoord2f(1, 1); glVertex4f(1, divLen*row + divLen, divLen*col + divLen - 1, w);
            glTexCoord2f(0, 1); glVertex4f(1, divLen*row + divLen, divLen*col - 1, w);
         }
      }
   }
   glEnd();
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);

   // Put crystal arrangements in room
   if (sqrt((camX - x)*(camX - x) + (camZ - z)*(camZ - z)) < 75) {
      crystalArrangement(x + (offsets[abs((int)x) % NUM_OFFSETS][0]/offsetMagnitude * (dx - 0.5)), y, z + (offsets[abs((int)z) % NUM_OFFSETS][2]/offsetMagnitude * (dz - 0.5)), 0, 0, 0, 'b'); // On floor
      crystalArrangement(x + (offsets[abs((int)x) + 1 % NUM_OFFSETS][0]/offsetMagnitude * (dx - 0.5)), y + dy, z + (offsets[abs((int)z) + 1 % NUM_OFFSETS][2]/offsetMagnitude * (dz - 0.5)), 180, 0, 0, 'b'); // On roof/ceiling
      if (front)
         crystalArrangement(x + (offsets[abs((int)x) + 2 % NUM_OFFSETS][0]/offsetMagnitude * (dx - 0.5)), y + dy/2 + (offsets[abs((int)z) + 2 % NUM_OFFSETS][1]/offsetMagnitude * (dy/2 - 0.5)), z - dz, 90, 0, 0, 'g');
      if (back && x != 0) // Ugly way of protecting against crystal in cave entrance
         crystalArrangement(x + (offsets[abs((int)x) + 3 % NUM_OFFSETS][0]/offsetMagnitude * (dx - 0.5)), y + dy/2 + (offsets[abs((int)z) + 3 % NUM_OFFSETS][1]/offsetMagnitude * (dy/2 - 0.5)), z + dz, -90, 0, 0, 'g');
      if (left)
         crystalArrangement(x - dx, y + dy/2 + (offsets[abs((int)z) + 4 % NUM_OFFSETS][1]/offsetMagnitude * (dy/2 - 0.5)), z + (offsets[abs((int)z) + 4 % NUM_OFFSETS][2]/offsetMagnitude * (dz - 0.5)), 0, 0, -90, 'r');
      if (right)
         crystalArrangement(x + dx, y + dy/2 + (offsets[abs((int)z) + 5 % NUM_OFFSETS][1]/offsetMagnitude * (dy/2 - 0.5)), z + (offsets[abs((int)z) + 5 % NUM_OFFSETS][2]/offsetMagnitude * (dz - 0.5)), 0, 0, 90, 'r');
   }

   if (cones) {
      // Draw the stalactites/stalagmites
      float collapseFactor = collapseAmount(dy);


      int counter = 0; // Used for cycling through randomly generated offsets
      for (float row = x - dx + 1; row < x + dx; row += 1.5) { // " + 1" to protect against insertion in walls
         for (float col = z - dz + 1; col < z + dz; col += 1.5) {
            // Stalactites on roof: down = true
            cone(true, row + offsets[counter % NUM_OFFSETS][0], dy - collapseFactor, col + offsets[counter % NUM_OFFSETS][1], offsets[counter % NUM_OFFSETS][2]);
            // Stalagmites on ground: down = true
            cone(false, row + offsets[counter % NUM_OFFSETS][0], 0, col + offsets[counter % NUM_OFFSETS][1], offsets[counter % NUM_OFFSETS][2]);
            counter++;
         }
      }
   }
}

/*
 * Draw a single wall
 */
void wall(double x1, double z1, double x2, double z2, double height, double elevation)
{
   glPushMatrix();

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, textures[1]);

   addRailing(x1, z1, x2, z2, elevation + PLAYER_HEIGHT);
   double dx = abs(x1 - x2);
   double dz = abs(z1 - z2);

   float dElevation = elevation > 0 ? collapseAmount(elevation) : 0;
   glTranslated((x1 + x2) / 2.0 + rumbleX, elevation - dElevation, (z1 + z2) / 2.0 + rumbleZ);
   glScaled(dx / 2.0, height - collapseAmount(height), dz / 2.0);

   glColor3f(0.25, 0.25, 0.25);
   glBegin(GL_QUADS);

   double length = dx > dz ? dx : dz;
   double divLength = 2.0 / length;
   double divHeight = 1.0 / height;
   
   for (int row = 0; row < height; row++) {
      for (int col = 0; col < length; col++) {
         if (dx) {
            glTexCoord2f(0, 0); glVertex4f(divLength*col - 1, divHeight*row, 0, w);
            if (x1 < x2) {
               glTexCoord2f(1, 0); glVertex4f(divLength*col + divLength - 1, divHeight*row, 0, w);
               glTexCoord2f(1, 1); glVertex4f(divLength*col + divLength - 1, divHeight*row + divHeight, 0, w);
               glTexCoord2f(0, 1); glVertex4f(divLength*col - 1, divHeight*row + divHeight, 0, w);
            } else {
               glTexCoord2f(0, 1); glVertex4f(divLength*col - 1, divHeight*row + divHeight, 0, w);
               glTexCoord2f(1, 1); glVertex4f(divLength*col + divLength - 1, divHeight*row + divHeight, 0, w);
               glTexCoord2f(1, 0); glVertex4f(divLength*col + divLength - 1, divHeight*row, 0, w);
            }
         } else {
            glTexCoord2f(0, 0); glVertex4f(0, divHeight*row, divLength*col - 1, w);
            if (z1 < z2) {
               glTexCoord2f(1, 0); glVertex4f(0, divHeight*row, divLength*col + divLength - 1, w);
               glTexCoord2f(1, 1); glVertex4f(0, divHeight*row + divHeight, divLength*col + divLength - 1, w);
               glTexCoord2f(0, 1); glVertex4f(0, divHeight*row + divHeight, divLength*col - 1, w);
            } else {
               glTexCoord2f(0, 1); glVertex4f(0, divHeight*row + divHeight, divLength*col - 1, w);
               glTexCoord2f(1, 1); glVertex4f(0, divHeight*row + divHeight, divLength*col + divLength - 1, w);
               glTexCoord2f(1, 0); glVertex4f(0, divHeight*row, divLength*col + divLength - 1, w);
            }
         }
      }
   }
   glEnd();
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

/*
 * Draw the entrance to the cave
 */
void caveEntrance()
{
   glPushMatrix();

   glTranslated(0 + rumbleX, 0, 24 + rumbleZ);
   glScaled(1, 3, 0);

   glColor3f(1, 1, 1);
   glNormal3f(0, 0, 1);

   glBegin(GL_QUADS);
   glVertex4f(1, 0, 0, w);
   glVertex4f(-1, 0, 0, w);
   glVertex4f(-1, 1, 0, w);
   glVertex4f(1, 1, 0, w);

   glEnd();

   // Place some wooden beams around entrance
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, textures[0]);
   float darken = 0.2;
   float br1R = 0.7*darken;
   float br1G = 0.4*darken;
   float br1B = 0.133*darken;
   glColor3f(br1R, br1G, br1B);
   glBegin(GL_QUADS);
   // Top beam
   glTexCoord2f(0, 0); glVertex4f(1.4, 1, 0, w);
   glTexCoord2f(1, 0); glVertex4f(-1.4, 1, 0, w);
   glTexCoord2f(1, 1); glVertex4f(-1.4, 1.2, 0, w);
   glTexCoord2f(0, 1); glVertex4f(1.4, 1.2, 0, w);
   // Beam on left of entrance
   glTexCoord2f(0, 0); glVertex4f(1.4, 0, 0, w);
   glTexCoord2f(1, 0); glVertex4f(1, 0, 0, w);
   glTexCoord2f(1, 1); glVertex4f(1, 1, 0, w);
   glTexCoord2f(0, 1); glVertex4f(1.4, 1, 0, w);
   // Beam on right of entrance
   glTexCoord2f(0, 0); glVertex4f(-1, 0, 0, w);
   glTexCoord2f(1, 0); glVertex4f(-1.4, 0, 0, w);
   glTexCoord2f(1, 1); glVertex4f(-1.4, 1, 0, w);
   glTexCoord2f(0, 1); glVertex4f(-1, 1, 0, w);
   glEnd();
   
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

void lightSource(double x, double y, double z, double r, int th)
{
   glPushMatrix();
   glTranslated(x, y, z);
   glRotated(th, 0, 1, 0);
   glScaled(r, 1.5*r, r);
   // Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glColor3f(1, 1, 1);
   glBindTexture(GL_TEXTURE_2D, textures[2]);
   // Begin drawing: Front
   glColor3f(1, 0, treasureAcquired ? 0 : 1);
   glBegin(GL_QUADS);
   glNormal3f(0, 0, 1);
   glTexCoord2f(0, 0); glVertex3f(-1,-1, 1);
   glTexCoord2f(1, 0); glVertex3f(+1,-1, 1);
   glTexCoord2f(1, 1); glVertex3f(+1,+1, 1);
   glTexCoord2f(0, 1); glVertex3f(-1,+1, 1);
   glEnd();
   //  Back
   glBegin(GL_QUADS);
   glNormal3f(0, 0, -1);
   glTexCoord2f(0, 0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1, 0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1, 1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0, 1); glVertex3f(+1,+1,-1);
   glEnd();
   //  Right
   glBegin(GL_QUADS);
   glNormal3f(1, 0, 0);
   glTexCoord2f(0, 0); glVertex3f(+1,-1,+1);
   glTexCoord2f(1, 0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1, 1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0, 1); glVertex3f(+1,+1,+1);
   glEnd();
   //  Left
   glBegin(GL_QUADS);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0, 0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1, 0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1, 1); glVertex3f(-1,+1,+1);
   glTexCoord2f(0, 1); glVertex3f(-1,+1,-1);
   glEnd();
   //  Top (Pyramid shape)
   glBindTexture(GL_TEXTURE_2D, textures[0]);
   float darken = 0.2;
   glColor3f(0.7*darken,0.4*darken,0.133*darken);
   glBegin(GL_TRIANGLES);
   // Front face
   glNormal3f(0, 1, 1);
   glTexCoord2f(0.5, 0.5); glVertex3f(0, 2, 0);
   glTexCoord2f(0, 0); glVertex3f(-1,+1,1);
   glTexCoord2f(1, 0); glVertex3f(1,+1,1);
   // Right face
   glNormal3f(1, 1, 0);
   glTexCoord2f(0.5, 0.5); glVertex3f(0, 2, 0);
   glTexCoord2f(0, 0); glVertex3f(+1,+1,+1);
   glTexCoord2f(1, 0); glVertex3f(+1,+1,-1);
   // Left face
   glNormal3f(-1, 1, 0);
   glTexCoord2f(0.5, 0.5); glVertex3f(0, 2, 0);
   glTexCoord2f(0, 0); glVertex3f(-1,+1,-1);
   glTexCoord2f(1, 0); glVertex3f(-1,+1,+1);
   // Back face
   glNormal3f(0, 1, -1);
   glTexCoord2f(0.5, 0.5); glVertex3f(0, 2, 0);
   glTexCoord2f(0, 0); glVertex3f(1, 1, -1);
   glTexCoord2f(1, 0); glVertex3f(-1, 1, -1);
   //  Bottom (pyramid shape)
   // Front face
   glNormal3f(0, -1, 1);
   glTexCoord2f(0.5, 0.5); glVertex3f(0, -2, 0);
   glTexCoord2f(0, 0); glVertex3f(1,-1,1);
   glTexCoord2f(1, 0); glVertex3f(-1,-1,1);
   // Right face
   glNormal3f(1, -1, 0);
   glTexCoord2f(0.5, 0.5); glVertex3f(0, -2, 0);
   glTexCoord2f(0, 0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1, 0); glVertex3f(+1,-1,+1);
   // Left face
   glNormal3f(-1, -1, 0);
   glTexCoord2f(0.5, 0.5); glVertex3f(0, -2, 0);
   glTexCoord2f(0, 0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1, 0); glVertex3f(-1,-1,-1);
   // Back face
   glNormal3f(0, -1, -1);
   glTexCoord2f(0.5, 0.5); glVertex3f(0, -2, 0);
   glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
   glTexCoord2f(1, 0); glVertex3f(1, -1, -1);
   glEnd();

   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   if (gameOver || victory){
      return;
   }
   if (treasureAcquired && sqrt(camX*camX + (camZ - 23)*(camZ - 23)) < 5) // Have the treasure and at the entrance/exit
      victory = true;
   if (treasureAcquired && !victory) {
      // Calculate offsets to make cave appear to rumble. Used for ceiling/roof and stalactites
      if (collapseImminent) {
         rumbleX = Cos(360*(float)rand()/(float)RAND_MAX)*0.1;
         rumbleZ = Cos(360*(float)rand()/(float)RAND_MAX)*0.1;
      }
      if ((glutGet(GLUT_ELAPSED_TIME) - collapseStart) > collapseTime*rumblePercent)
         collapseImminent = true;
      if ((glutGet(GLUT_ELAPSED_TIME) - collapseStart) > collapseTime)
         gameOver = true;
   }
   float startLoop = glutGet(GLUT_ELAPSED_TIME);
   shinyvec[0] = shininess<0 ? 0 : pow(2.0,shininess);
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST); //  Enable Z-buffering
   glEnable(GL_CULL_FACE); // Enable face culling
   //  Undo previous transformations
   glLoadIdentity();
   if (mode == 2)
   {
      //  First person navigation - set eye position
      lx = Sin(th) * Cos(ph);
      lz = -Cos(th)*Cos(ph);
      
      // Check if cave is collapsing
      if (treasureAcquired && !gameOver) {
         // Lower head position as cave collapses
         camY = max(crawlHeight, PLAYER_HEIGHT - (PLAYER_HEIGHT - crawlHeight) * collapsePercent);
         // Become slower as player crouches and crawls
         if (collapsePercent > 0.5) {
            // Crouching
            currentSpeed = WALK_SPEED * 0.75;
         } else if (collapsePercent >= 1) {
            // Crawling
            currentSpeed = WALK_SPEED * 0.5;
         }
      }

      gluLookAt(camX,camY,camZ , camX + lx,camY + Sin(ph),camZ + lz , gameOver ? 1 : 0, gameOver ? 0 : 1, 0); // Flipped on side if game over
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

   // If light hasn't been acquired yet, have it circling in the second room
   float lightOffsetX = lightAcquired ? camX : lightX;
   float lightOffsetZ = lightAcquired ? camZ : lightZ;
   float lightRadius = 2.0;
   float Position[] = {lightRadius*Cos(zh) + lightOffsetX, lightY + Cos(4*zh)/6, lightRadius*Sin(zh) + lightOffsetZ, 1};
   if (light)
   {//  Translate intensity to color vectors 
      if (treasureAcquired)
         specular = 50 + 50 * Cos(glutGet(GLUT_ELAPSED_TIME));// Cos(zh); // Create siren-like behavior, cycling through intensity of light
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0}; 
      float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0}; 
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0}; 
      //  Light color and direction 
      float lightColor[] = {1.0, victory ? 1.0 : 0.0, treasureAcquired ? 0.0 : 1.0, 1.0}; // Red if treasureAcquired going off, otherwise purple

      // float Direction[] = {Cos(Th)*Sin(Ph),Sin(Th)*Sin(Ph),-Cos(Ph),0}; 
      lightSource(Position[0],Position[1],Position[2] , 0.125, zh); 
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
      glMaterialfv(GL_FRONT,GL_SPECULAR,lightColor); 
      glMaterialfv(GL_FRONT,GL_SHININESS,shinyvec); 
      //  Enable light 0 (floating lantern)
      glEnable(GL_LIGHT0); 
      //  Set ambient, diffuse, specular components and position of light 0 
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient); 
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse); 
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular); 
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
      //  Set attenuation
      glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION ,at0/100.0);
      glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION   ,at1/100.0);
      glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,at2/100.0);

      // Enable the lighting coming from cave entrance
      glEnable(GL_LIGHT1);
      float Position[] = {0, 2.0, 23, 1};
      glLightfv(GL_LIGHT1,GL_AMBIENT, Ambient);
      glLightfv(GL_LIGHT1,GL_DIFFUSE ,Diffuse); 
      glLightfv(GL_LIGHT1,GL_SPECULAR,Specular); 
      glLightfv(GL_LIGHT1,GL_POSITION,Position);
      glLightf(GL_LIGHT1,GL_CONSTANT_ATTENUATION ,at0/100.0);
      glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION   ,at1/100.0);
      glLightf(GL_LIGHT1,GL_QUADRATIC_ATTENUATION,at2/2/100.0);
   }
   else
      glDisable(GL_LIGHTING);

   //crystalArrangement(-6, 3, 0, 0, 0, -90);
   //crystalArrangement(1, 0, 5, 0, 0, 0);

   // Starting cave room
   glNormal3f(-1, 0, 0); wall(42, -32, 42, -28, 4, 0); // Patch right wall
   glNormal3f(0, 0, -1);
   // Push the room back so cave entrance appears flush
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(1, 1);
   room(0, 0, 0, 6, 5, 24, false, true, true, true, 8, true);
   glDisable(GL_POLYGON_OFFSET_FILL);
   caveEntrance();

   // Second room, or room veering to right
   room(18, 0, -30, 24, 5, 6, true, false, true, false, 8, true);
   glNormal3f(0, 0, -1); wall(42, -24, 6, -24, 5, 0); // Patch back wall
   glNormal3f(-1, 0, 0); wall(42, -32, 42, -28, 4, 0); // Patch right wall
   wall(42, -36, 42, -24, 1, 4); // Continued
   
   
   // Rooms splitting off to left of second room:
   room(44, 0, -54, 2, 4, 22, true, true, false, true, 8, true); // Long connecting hallway
   glNormal3f(1, 0, 0); wall(42, -36, 42, -72, 4, 0); // Patch left wall
   
   room(30, 0, -90, 12, 14, 18, true, true, false, false, 8, true); // Atrium, ceiling higher
   glNormal3f(1, 0, 0); wall(18, -72, 18, -104, 14, 0); // Patch left wall (big chunk)
   wall(18, -104, 18, -108, 10, 4); // Continued (small piece above hallway)
   glNormal3f(-1, 0, 0); wall(42, -104, 42, -76, 14, 0); // Patch right wall (big chunk)
   wall(42, -108, 42, -104, 10, 4); // Continued (small piece above hallway leading to treasure room)
   wall(42, -76, 42, -72, 10, 4); // Continued (small piece above hallway leading back to entrance)e
   room(0, 0, 0, 6, 5, 24, false, false, true, true, 8, true);
   // Second room, or room veering to right
   room(18, 0, -30, 24, 5, 6, true, false, true, false, 8, true);
   glNormal3f(0, 0, -1); wall(42, -24, 6, -24, 5, 0); // Patch back wall
   glNormal3f(-1, 0, 0); wall(42, -32, 42, -28, 4, 0); // Patch right wall
   wall(42, -36, 42, -24, 1, 4); // Continued
   
   
   // Rooms splitting off to left of second room:
   room(44, 0, -54, 2, 4, 22, true, true, false, true, 8, true); // Long connecting hallway
   glNormal3f(1, 0, 0); wall(42, -36, 42, -72, 4, 0); // Patch left wall
   
   room(30, 0, -90, 12, 14, 18, true, true, false, false, 8, true); // Atrium, ceiling higher
   glNormal3f(1, 0, 0); wall(18, -72, 18, -104, 14, 0); // Patch left wall (big chunk)
   wall(18, -104, 18, -108, 10, 4); // Continued (small piece above hallway)
   glNormal3f(-1, 0, 0); wall(42, -104, 42, -76, 14, 0); // Patch right wall (big chunk)
   wall(42, -108, 42, -104, 10, 4); // Continued (small piece above hallway leading to treasure room)
   wall(42, -76, 42, -72, 10, 4); // Continued (small piece above hallway leading back to entrance)

   room(-8, 0, -106, 26, 4, 2, true, true, false, false, 8, true); // Hallway leading to treasure from atrium to left
   glNormal3f(1, 0, 0); wall(-34, -104, -34, -108, 1, 3); // Patch small chunk above treasure room entrance
   room(-36, 0, -110, 2, 3, 6, true, true, true, false, 8, false); // Treasure room at end of hall
   glNormal3f(-1, 0, 0); wall(-34, -116, -34, -108, 3, 0); // Patch right wall of treasure room

   room(68, 0, -106, 26, 4, 2, true, true, false, false, 8, true); // Hallway leading to treasure from atrium to right
   glNormal3f(-1, 0, 0); wall(94, -108, 94, -104, 1, 3); // Patch small chunk above treasure room entrance
   room(96, 0, -110, 2, 3, 6, true, true, false, true, 8, false); // Treasure room at end of hall
   glNormal3f(1, 0, 0); wall(94, -108, 94, -116, 3, 0); // Patch left wall of treasure room

   // Rooms splitting off to right of second room
   room(44, 0, -6, 2, 4, 22, true, true, false, false, 8, true); // Long connecting hallway
   glNormal3f(1, 0, 0); wall(42, 16, 42, -24, 4, 0); // Patch left wall
   glNormal3f(-1, 0, 0); wall(46, -28, 46, 12, 4, 0); // Patch right wall

   room(58, 0, 30, 12, 14, 18, true, true, false, false, 8, true); // Atrium, ceiling higher
   glNormal3f(-1, 0, 0); wall(70, 12, 70, 44, 14, 0); // Patch right wall (big chunk)
   wall(70, 44, 70, 48, 10, 4); // Continued (small piece above hallway leading to treasure chest) 
   glNormal3f(1, 0, 0); wall(46, 44, 46, 16, 14, 0); // Patch left wall (big chunk)
   wall(46, 48, 46, 44, 10, 4); // Continued (small piece above hallway leading to treasure chest)
   wall(46, 16, 46, 12, 10, 4); // Continued (small piece above hallway leading back to entrance)

   room(96, 0, 46, 26, 4, 2, true, true, false, false, 8, true); // Hallway leading to treasure from atrium to left (from point of view)
   glNormal3f(-1, 0, 0); wall(122, 44, 122, 48, 1, 3); // Patch small chunk above treasure room entrance
   room(124, 0, 50, 2, 3, 6, true, true, false, true, 8, false); // Treasure room at end of hall
   glNormal3f(1, 0, 0); wall(122, 56, 122, 48, 3, 0); // Patch left wall of treasure room

   room(20, 0, 46, 26, 4, 2, true, true, false, false, 8, true); // Hallway leading to treasure from atrium to right (from point of view)
   glNormal3f(1, 0, 0); wall(-6, 48, -6, 44, 1, 3); // Patch small chunk above treasure room entrance
   room(-8, 0, 50, 2, 3, 6, true, true, true, false, 8, false); // Treasure room at end of hall
   glNormal3f(-1, 0, 0); wall(-6, 48, -6, 56, 3, 0); // Patch right wall of treasure room

   // Place the chest
   switch (chestPos) {
      case 0:  
         // Top left
         chest(-36, chestDim, -115, 1.5*chestDim, chestDim, chestDim, 0);
         addRailing(-38, -114, -34, -114, PLAYER_HEIGHT);
         break;
      case 1:  
         // Top right
         chest(96, chestDim, -115, 1.5*chestDim, chestDim, chestDim, 0);
         addRailing(94, -114, 98, -114, PLAYER_HEIGHT);
         break;
      case 2:  
         // Bottom left
         chest(-8, chestDim, 55, 1.5*chestDim, chestDim, chestDim, 180);
         addRailing(-10, 54, -6, 54, PLAYER_HEIGHT);
         break;
      case 3:  
         // Bottom right
         chest(124, chestDim, 55, 1.5*chestDim, chestDim, chestDim, 180);
         addRailing(122, 54, 126, 54, PLAYER_HEIGHT);
         break;
   }

   // Place the key
   if (!keyAcquired) {
      switch (keyPos) {
         case 0:  
            // Top left
            key(-36, 1, -115, zh, 0);
            addRailing(-38, -115, -34, -115, PLAYER_HEIGHT);
            break;
         case 1:  
            // Top right
            key(96, 1, -115, zh, 0);
            addRailing(94, -115, 98, -115, PLAYER_HEIGHT);
            break;
         case 2:  
            // Bottom left
            key(-8, 1, 55, zh, 0);
            addRailing(-10, 55, -6, 55, PLAYER_HEIGHT);
            break;
         case 3:  
            // Bottom right
            key(124, 1, 55, zh, 0);
            addRailing(122, 55, 126, 55, PLAYER_HEIGHT);
            break;
      }
   }
   
   // All railings have now been loaded. Significant on first pass only
   railingsLoaded = true;

   // No need for lighting from here on
   glDisable(GL_LIGHTING);
   //  Display parameters
   glWindowPos2i(5,5);
   if (victory) {
      glWindowPos2i(glutGet(GLUT_WINDOW_WIDTH) / 2,glutGet(GLUT_WINDOW_HEIGHT) / 2);
      Print("Victory!");
   } else if (gameOver) {
      Print("The cave collapsed with you inside... You've been crushed");
   } else if (treasureAcquired) {
      if (collapseImminent)
         Print("This cave will collapse any second!");
      else if (collapsePercent < 0.5)
         Print("The treasure was rigged and now the cave is collapsing! Escape before you're crushed!");
      else if (collapsePercent < 1.0)
         Print("You're having to crouch!");
      else
         Print("You're having to crawl!");
   } else if (chestUnlocked)
      Print("The treasure! Take it back!");
   else if (keyAcquired)
      Print("There must be a use for this key...");
   else if (hint)
      Print("There's a keyhole in the chest...");
   else if (!lightAcquired && sqrt((camX - lightX)*(camX - lightX) + (camZ - lightZ)*(camZ - lightZ)) < 25) // If near the light
      Print("That lantern looks useful. Move near it and click.");
   else {
      Print("Your long quest to recover your stolen treasure has led you into this cave...");
      if (lightAcquired)
         Print(" This lantern will help light the way");
   }
   //  Render the scene and make it visible
   glFlush();
   glutSwapBuffers();
   dt = glutGet(GLUT_ELAPSED_TIME) - startLoop;
   ErrCheck("display");
   // Reset cursor back to center of screen for next movement
   glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
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
   Project(fov, asp, dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

void checkCollision()
{
   for (int railing = 0; railing < numRailings; railing++) {
      if (railings[railing][4] < PLAYER_HEIGHT + 0.5) { // Ignore railings higher up
         double d = distRailing(railing);
         if (d < 0.7) {
            // Too close to wall, push back
            double x1 = railings[railing][0];
            double z1 = railings[railing][1];
            double x2 = railings[railing][2];
            if (x1 == x2) {
               if (camX < x1) // Player hitting right wall
                  camX -= 0.7 - d;
               else // Player hitting left wall
                  camX += 0.7 - d;
            } else {
               if (camZ < z1) // Player is bouncing into back wall
                  camZ -= 0.7 - d;
               else // Player is bounding into front wall
                  camZ += 0.7 - d;
            }
         }
      }
   }
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void keyboardKey(unsigned char ch,int x,int y)
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
   else if (ch == 'd') {
      //  Strafe right
      camX += (currentSpeed * 0.75) * Sin(th + 90) * dt;
      camZ -= (currentSpeed * 0.75) * Cos(th + 90) * dt;
      checkCollision();
   } else if (ch == 'a') {
      //  Strafe left
      camX -= (currentSpeed * 0.75) * Sin(th + 90) * dt;
      camZ += (currentSpeed * 0.75) * Cos(th + 90) * dt;
      checkCollision();
   } else if (ch == 'w') {
      //  Move forward
      camX += currentSpeed * lx * dt;
      camZ += currentSpeed * lz * dt;
      checkCollision();
   //  Move backwards
   } else if (ch == 's') {
      camX -= (currentSpeed * 0.75) * lx * dt;
      camZ -= (currentSpeed * 0.75) * lz * dt;
      checkCollision();
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
   //  Reproject
   Project(fov, asp, dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 * GLUT calls this routine when mouse click event takes place
 */
void mouseButton(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        if (state != GLUT_UP) {
           if (!treasureAcquired) {
               // Check if player is clicking on the treasure chest
               switch (chestPos) {
                  case 0:
                     if (camX < -34 && camZ < -110) {
                        if (keyAcquired) {
                           if (chestUnlocked && !treasureAcquired) {
                              treasureAcquired = true;
                              collapseStart = glutGet(GLUT_ELAPSED_TIME);
                           }
                           chestUnlocked = true;
                        } else {
                           hint = true;
                        }
                     }
                     break;
                  case 1:
                     if (camX > 94 && camZ < -110) {
                        if (keyAcquired) {
                           if (chestUnlocked && !treasureAcquired) {
                              treasureAcquired = true;
                              collapseStart = glutGet(GLUT_ELAPSED_TIME);
                           }
                           chestUnlocked = true;
                        } else {
                           hint = true;
                        }
                     }
                     break;
                  case 2:
                     if (camX < -6 && camZ > 50) {
                        if (keyAcquired) {
                           if (chestUnlocked && !treasureAcquired) {
                              treasureAcquired = true;
                              collapseStart = glutGet(GLUT_ELAPSED_TIME);
                           }
                           chestUnlocked = true;
                        } else {
                           hint = true;
                        }
                     }
                     break;
                  case 3:
                     if (camX > 122 && camZ > 50) {
                        if (keyAcquired) {
                           if (chestUnlocked && !treasureAcquired) {
                              treasureAcquired = true;
                              collapseStart = glutGet(GLUT_ELAPSED_TIME);
                           }
                           chestUnlocked = true;
                        } else {
                           hint = true;
                        }
                     }
                     break;
               }
            }
            if (!keyAcquired) {
               // Check if user is clicking on the key
               switch (keyPos) {
                  case 0:
                     if (camX < -34 && camZ < -110) {
                        keyAcquired = true;
                     }
                     break;
                  case 1:
                     if (camX > 94 && camZ < -110) {
                        keyAcquired = true;
                     }
                     break;
                  case 2:
                     if (camX < -6 && camZ > 50) {
                        keyAcquired = true;
                     }
                     break;
                  case 3:
                     if (camX > 122 && camZ > 50) {
                        keyAcquired = true;
                     }
                     break;
               }
            }
            if (!lightAcquired && sqrt((camX - lightX)*(camX - lightX) + (camZ - lightZ)*(camZ - lightZ)) < 3)
               lightAcquired = true;
            glutPostRedisplay();
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
 * GLUT calls this routine when mouse event (click or move) takes place
 */
void mouseMove(int x, int y)
{
   int sensitivity = 24; // Increasing value decreases sensitivity
   th = (th + (x - glutGet(GLUT_WINDOW_WIDTH) / 2) / sensitivity) % 360;
   ph = (ph - (y - glutGet(GLUT_WINDOW_HEIGHT) / 2) / (sensitivity)) % 360;
   if (ph > 90)
      ph = 90;
   if (ph < -90)
      ph = -90;
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
   Project(fov, asp, dim);
}

/*
 * Called only once in main function
 * Generate variables needed for display functions
 */
void setUp()
{
   // Seed the random number generator
   srand(time(NULL));
   // Define 25 position with offsets for cones
   for (int pos = 0; pos < NUM_OFFSETS; pos++) {
      // Offsets will be between -0.625 and 0.625 inclusive
      offsets[pos][0] = ((float)rand()/(float)RAND_MAX - 0.5) * 1.25; // x offset added to position
      offsets[pos][1] = ((float)rand()/(float)RAND_MAX - 0.5) * 1.25; // z offset added to position
      offsets[pos][2] = ((float)rand()/(float)RAND_MAX - 0.5) * 1.25; // Offset of cone tip
   }

   chestPos = rand() % 4; // Place the chest

   do { // Place the key
      keyPos = rand() % 4;
   } while (keyPos == chestPos); // Cannot be in same place as chest
}

void idle()
{
   if (lightMove) {
      // Elapsed time in seconds
      double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
      zh = fmod(90*t, 360.0);
      // Necessary to redisplay
      glutPostRedisplay();
   }
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   // Set up some values for the program
   setUp();
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(800,600);
   glutCreateWindow("Final Project: Andrew Gaines");
   glutFullScreen();
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(keyboardKey);
   glutIdleFunc(idle);
   // Tell GLUT to call "mouseButton" when mouse click/scroll event occurs
   glutMouseFunc(mouseButton);
   // Tell GLUT to call "mouseMove" when mouse move event occurs
   //glutMotionFunc(mouseMove); // When a key is pressed
   glutPassiveMotionFunc(mouseMove); // Regardless of whether key pressed
   glutSetCursor(GLUT_CURSOR_NONE); // Hide the mouse cursor
   // Load textures
   textures[0] = LoadTexBMP("textures/wood.bmp");
   textures[1] = LoadTexBMP("textures/roughrock.bmp");
   textures[2] = LoadTexBMP("textures/lantern.bmp");
   textures[3] = LoadTexBMP("textures/dali.bmp");
   textures[4] = LoadTexBMP("textures/entrance.bmp");
   ErrCheck("main"); // Sanity check
   collapseStart = glutGet(GLUT_ELAPSED_TIME);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
