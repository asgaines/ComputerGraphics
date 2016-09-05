#include <stdio.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

enable Z-buffer
    glutInitDisplayMode(GLUT_DEPTH);
    glEnable(GL_DEPTH_TEST);

Z-buffering happens in hardware
face-culling happens in software


