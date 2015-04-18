#include <stdlib.h> 
#include <math.h> 
#include <stdio.h> 
#include <cstdlib>
#include <cmath>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define ESC 27
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#include <string>
#include <iostream>

class Scoreboard{

	public:

		void getScore(int value){

			glPushMatrix();
			glRotatef(90, 1, 0, 0);
			glTranslatef(-5, 25, 0);

			glPushMatrix();
			glColor3f(21.0/244, 191.0/244, 230.0/244);
	
			for (int i = 0; i < value; i++)
			{
				glTranslatef(0,0,-2);
				drawScore((float)value);
			}

			glPopMatrix();

		}

		void drawScore(float value){

			glPushMatrix();
			glColor4f(13.0/255.0 + value/100.0, 112.0/255.0 + value/100.0, 81.0/255.0, 0.9);
			for (int i = 0; i < 20; ++i)
			{
				glTranslatef(0,0,-0.05);
			   	glutSolidSphere( 0.3,100, 100);
			}
			glPopMatrix();

		}
	
};