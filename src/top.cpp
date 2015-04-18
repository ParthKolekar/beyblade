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

class Top{

	public:

		void drawTop(int topAngle){
			glPushMatrix();
			glScalef(2,2,2);
			glRotatef(topAngle,0,0,1);
			glTranslatef(0,0,-0.3);
			glColor4f(113.0/255.0 + topAngle/8.0, 112.0/255.0 - topAngle/8.0, 12.0/255.0 + topAngle/8.0, 0.7);
			glutWireTorus( 0.2,0.3, 100,100);
			glColor4f(113.0/255.0 + topAngle/8.0, 112.0/255.0 - topAngle/8.0, 12.0/255.0 + topAngle/8.0, 0.7);
			glutWireTorus( 0.2,0.3, 100,100);
			glTranslatef(0,0,0.3);

			glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(0,0,1);
			glEnd();

			glPushMatrix();
				glTranslatef(0,0,-0.3);
				glRotatef(180,0,1,0);
				glutWireCone(0.5, 0.8, 10, 2);
			glPopMatrix();

			glColor4f(113.0/255.0 + topAngle/8.0, 112.0/255.0 + topAngle/8.0, 12.0/255.0 - topAngle/8.0, 0.7);
			glutWireTorus( 0.2,0.5, 25, 20);
			glColor4f(113.0/255.0 - topAngle/8.0, 112.0/255.0 + topAngle/8.0, 12.0/255.0 - topAngle/8.0, 0.7);
			glPopMatrix();
		}
		
		//display power bar
		void drawBar(float velocity){
			glScalef(2,2,2);
			glTranslatef(32,0,25);
			glRotatef(90,0,0,0);
			for ( float i = 0; i < velocity; i++)
			{
				glColor4f(113.0/255.0 + velocity / 8, 112.0/255.0 - velocity/8, 12.0/255.0 + velocity/8, 0.9);
				glTranslatef(0, 0, -0.5);
				drawCylinder(0.7);
			}

		}

		void drawCylinder(float rad){

			for (int i = 0; i < 20; ++i)
			{
				glTranslatef(0,0,-0.05);
			   	glutSolidSphere(rad,3,3);
			}
		}

		int checkBoundary(float x, float z){

			if (x > 2 && x < 60 && z > 2 && z < 60)
				return 1;
			else
				return 0;
		}

		void drawArrow(int angle){
			glPushMatrix();
			glColor4f(113.0/255.0 - angle/360.0, 112.0/255.0 - angle/360.0, 12.0/255.0 + angle/360.0, 0.9);
			glTranslatef(-8,0,40);
			glRotatef(180, 1, 0 , 0);
			glRotatef(angle, 0, 1, 0);
			glutSolidCone(1.5, 8, 10, 2);
			glPopMatrix();
		}

		int getTarget(){

			int a = rand()%45 + 5;
			return a;

		}

		void drawTarget(int x, int y, int z){

			glPushMatrix();
			glRotatef(90, 1, 0, 0);
			glTranslatef(x, z, (-1*y)-2);
			glColor3f(7.0/244, 36.0/244, 87.0/244);
			glutSolidTorus( 0.3,2, 100,10);
			glPopMatrix();
		}

		int checkTarget(float x, float y, float a, float b){
			if((x-a)*(x-a) + (y-b)*(y-b) <= 1)
				return 1;
			else 
				return 0;
		}
};