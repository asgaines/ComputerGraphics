/*
 *  Coordinates of Lorenz Attractor Plotted to Window
 *
 *  Display 2, 3 and 4 dimensional coordinates of the Lorenz Attractor in 3D.
 *
 *  Key bindings:
 *  p      Increase number of points displayed
 *  P      Decrease number of points displayed
 *  t      Decrease amount of "time" between points being drawn
 *  T      Increase amount of "time" between points being drawn
 *  c      Toggle between points being connected by lines or not
 *  mousedrag      Change position of the camera
 *  mousewheel     Zoom in and out
 *  arrows Change view angle
 *  0      Reset view angle
 *
 *  ESC    Exit
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//  Globals
int th=30;       // Azimuth of view angle, set for optimizing view of Lorenz Attractor
int ph=30;       // Elevation of view angle
int mode=1;     // Dimension (1-4)
double z=0;     // Z variable
double w=1;     // W variable
double dim=50;   // Dimension of orthogonal box
char* text[] = {"","2D","3D constant Z","3D","4D"};  // Dimension display text
// globals for defining Lorenz attractor
double s = 10.0; // Prandtl variable
double b = 8.0 / 3.0; // Beta variable
double r = 28.0; // Rayleigh variable
float dt = 0.01; // Change in time between points
int numPoints = 5000; // Number of points of Lorenz Attractor to plot
bool connectDots = true; // Connect dots with lines if requested
// Globals for dragging scene with mouse
bool mouseDown = false;
int mouseX;
int mouseY;

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  // Maximum length of text string
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

/*
 * Helper function to calculate distance between two points
 */
double distance(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
}

/*
 *  Display the scene
 */
void display()
{
    //  Clear the image
    glClear(GL_COLOR_BUFFER_BIT);
    //  Reset previous transforms
    glLoadIdentity();
    //  Set view angle
    glRotated(ph,1,0,0);
    glRotated(th,0,1,0);
    //  Draw 10 pixel yellow points
    glColor3f(0,0,1);
    glPointSize(1);
   
    // Draw the Lorenz Attractor
    double x = 1.0;
    double y = 1.0;
    double z = 1.0;
    double dx = 0;
    double dy = 0;
    double dz = 0;

    if (connectDots) {
        glBegin(GL_LINE_STRIP);
    } else {
        glBegin(GL_POINTS);
    }

    double tempX = x;
    double tempY = y;
    double tempZ = z;
    double dist = 0; // Measure distance between points
    double maxDistance = 0; // Stores greatest distance between points
    double points[numPoints][3]; // Create array to hold points
    // Calculate position of points stored in array and greatest distance between points
    for (int point = 0; point < numPoints; point++) {
        // Store point in array
        points[point][0] = x;
        points[point][1] = y;
        points[point][2] = z;
        // Calculate change in position for next point, dictated by Lorenz equation
        dx = s * (y - x);
        dy = x * (r - z) - y;
        dz = x * y - b * z;
        // Load change to current point
        tempX += dx * dt;
        tempY += dy * dt;
        tempZ += dz * dt;
        // Distance between point just displayed and the next one
        dist = distance(x, y, z, tempX, tempY, tempZ);
        // Store the greatest distance for heat map
        maxDistance = dist > maxDistance ? dist : maxDistance;
        x = tempX;
        y = tempY;
        z = tempZ;
    }

    // Commit points to the display
    for (int point = 0; point < numPoints - 1; point++) {
        x = points[point][0];
        y = points[point][1];
        z = points[point][2];
        double previousX = points[point + 1][0];
        double previousY = points[point + 1][1];
        double previousZ = points[point + 1][2];
        // Display color of points as heat map; farther apart the point, more red, else blue
        glColor3f(distance(x, y, z, previousX, previousY, previousZ) / maxDistance, 0, 0.5 - distance(x, y, z, previousX, previousY, previousZ) / maxDistance);
        glVertex4d(x, y, z, w);
    }
    glEnd();
    //  Draw axes in white
    int lenAxes = 25;
    glColor3f(1,1,1);
    glBegin(GL_LINES);
    glVertex3d(0,0,0);
    glVertex3d(lenAxes,0,0);
    glVertex3d(0,0,0);
    glVertex3d(0,lenAxes,0);
    glVertex3d(0,0,0);
    glVertex3d(0,0,lenAxes);
    glEnd();
    //  Label axes
    glRasterPos3d(lenAxes,0,0);
    Print("X");
    glRasterPos3d(0,lenAxes,0);
    Print("Y");
    glRasterPos3d(0,0,lenAxes);
    Print("Z");
    //  Display parameters
    glWindowPos2i(5,5);
    Print("View Angle=%d,%d  %s",th,ph,text[mode]);
    if (mode==2)
      Print("  z=%.1f",z);
    else if (mode==4)
      Print("  w=%.1f",w);
    //  Flush and swap
    glFlush();
    glutSwapBuffers();
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
        th = ph = 30;
    // Toggle whether dots should be connected by lines
    else if (ch == 'c')
    {
        connectDots = !connectDots;
    }
    // Increase number of points
    else if (ch == 'p')
    {
        numPoints += 1000;
    }
    // Decrease number of points
    else if (ch == 'P')
    {
        if (numPoints > 0)
            numPoints -= 1000;
    }
    // Decrease time between point display
    else if (ch == 't')
    {
        dt *= 0.9;
    }
    // Increase time between point display
    else if (ch == 'T')
    {
        dt *= 1.1;
    }

    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
    //  Right arrow key - increase azimuth by 5 degrees
    if (key == GLUT_KEY_RIGHT)
        th += 5;
    //  Left arrow key - decrease azimuth by 5 degrees
    else if (key == GLUT_KEY_LEFT)
        th -= 5;
    //  Up arrow key - increase elevation by 5 degrees
    else if (key == GLUT_KEY_UP)
        ph += 5;
    //  Down arrow key - decrease elevation by 5 degrees
    else if (key == GLUT_KEY_DOWN)
        ph -= 5;
    //  Keep angles to +/-360 degrees
    th %= 360;
    ph %= 360;
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
    } else if (button == 3) {
        w -= 0.1;
        glutPostRedisplay();
    } else if (button == 4) {
        w += 0.1;
        glutPostRedisplay();
    }

}

/*
 * GLUT calls this routine when mouse click event takes place
 */
void mouseMove(int x, int y)
{
    if (mouseDown) {
        th = (th + (x - mouseX) / 2) % 360;
        ph = (ph + (y - mouseY) / 2) % 360;
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
    double w2h = (height>0) ? (double)width/height : 1;
    //  Set the viewport to the entire window
    glViewport(0,0, width,height);
    //  Tell OpenGL we want to manipulate the projection matrix
    glMatrixMode(GL_PROJECTION);
    //  Undo previous transformations
    glLoadIdentity();
    //  Orthogonal projection box adjusted for the
    //  aspect ratio of the window
    glOrtho(-dim*w2h,+dim*w2h, -dim,+dim, -dim,+dim);
    //  Switch to manipulating the model matrix
    glMatrixMode(GL_MODELVIEW);
    //  Undo previous transformations
    glLoadIdentity();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
    //  Initialize GLUT and process user parameters
    glutInit(&argc,argv);
    //  Request double buffered, true color window 
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    //  Request 500 x 500 pixel window
    glutInitWindowSize(800,600);
    //  Create the window
    glutCreateWindow("Assignment 1: Andrew Gaines");
    //  Tell GLUT to call "display" when the scene should be drawn
    glutDisplayFunc(display);
    //  Tell GLUT to call "reshape" when the window is resized
    glutReshapeFunc(reshape);
    //  Tell GLUT to call "special" when an arrow key is pressed
    glutSpecialFunc(special);
    //  Tell GLUT to call "key" when a key is pressed
    glutKeyboardFunc(key);
    // Tell GLUT to call "mouseButton" when mouse click event occurs
    glutMouseFunc(mouseButton);
    // Tell GLUT to call "mouse" when mouse move event occurs
    glutMotionFunc(mouseMove);
    //  Pass control to GLUT so it can interact with the user
    glutMainLoop();
    //  Return code
    return 0;
}
