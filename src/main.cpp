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

#include "imageloader.h"
#include "vec3f.h"
#include "top.cpp"
#include "score.cpp" 

#define ESC 27
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#include <string>
#include <iostream>

float topAngle;
float posX, posY,  posZ ;
float angla;
float vel, velX, velZ;

int flags, flagw;
int windowWidth, windowHeight;
Top topObject;
int launchTop;
int planeNormal[3];
int xAngle;
int cameraFlag;
int directionAngle;
int targetX, targetY, targetZ;
int totalScore;
float previousPosition;

Scoreboard score;

using namespace std;

void gravityForce();
void reset();

class Terrain {
	private:
		int w; //Width
		int l; //Length
		float** hs; //Heights
		Vec3f** normals;
		bool computedNormals; //Whether normals is up-to-date
	public:
		Terrain(int w2, int l2) {
			w = w2 ;
			l = l2 ; 
			
			hs = new float*[l];
			for(int i = 0; i < l; i++) {
				hs[i] = new float[w];
			}
			
			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}
			
			computedNormals = false;
		}
		
		~Terrain() {
			for(int i = 0; i < l; i++) {
				delete[] hs[i];
			}
			delete[] hs;
			
			for(int i = 0; i < l; i++) {
				delete[] normals[i];
			}
			delete[] normals;
		}
		
		int width() {
			return w;
		}
		
		int length() {
			return l;
		}
		
		//Sets the height at (x, z) to y
		void setHeight(int x, int z, float y) {
			hs[z][x] = y;
			computedNormals = false;
		}
		
		float getHeight(int x, int z) {
			return hs[z][x];
		}
		
		void computeNormals() {
			if (computedNormals) {
				return;
			}
			
			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}
			
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);
					
					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
					}
					
					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}
					
					normals2[z][x] = sum;
				}
			}
			
			const float FALLOUT_RATIO = 1.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];
					
					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}
					
					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}
					normals[z][x] = sum;
				}
			}
			
			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;
			
			computedNormals = true;
		}
		
		//Returns the normal at (x, z)
		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};

Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for(int y = 0; y < image->height; y++) {
		for(int x = 0; x < image->width; x++) {
			unsigned char color =
				(unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}
	
	delete image;
	t->computeNormals();
	return t;
}

Terrain* _terrain;

void cleanup() {
	delete _terrain;
}

void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
}

void handleResize(int w, int h) 
{
	float ratio =  ((float) w) / ((float) h); // window aspect ratio
	glMatrixMode(GL_PROJECTION); // projection matrix is active
	glLoadIdentity(); // reset the projection
	gluPerspective(45.0, ratio, 0.1, 100.0); // perspective transformation
	glMatrixMode(GL_MODELVIEW); // return to modelview mode
	glViewport(0, 0, w, h); // set viewport (drawing area) to entire window
}


void setVelocity(){

	velZ = vel*cos(DEG2RAD(-1*directionAngle));
	velX = vel*sin(DEG2RAD(-1*directionAngle));
	//printf("%f %f\n",velX, velZ );
}

void frictionForce(float a, float b){
	if(a < b){
		velX = velX*0.8;
		velZ = velZ*0.8;
		
	}
	if(abs(velX) <= 0.1)
		velX = 0;

	if(abs(velZ) <= 0.1)
		velZ = 0;

	if(velX == 0 && velZ == 0){
		vel = 0;
	}
}


void handleKeypress1(unsigned char key, int x, int y) {
	switch (key) {
		case 27: //Escape key
			exit(0);
			break;

		case 'd':
			if(directionAngle < 90)
				directionAngle += 3;
			break;

		case 'a':

			if(directionAngle > -90)
				directionAngle -= 3;
			break;

		case 'w':
			if(vel < 8)
				vel = vel + 1;
			break;

		case 's':
			if(vel > 1)
				vel = vel -1;
			break;				
		case 32: //SPACE key
			if(launchTop == 0){
				setVelocity();
				launchTop = 1;
			}
			else{
				reset();
			}
			break;

		case 13:
			cameraFlag+=1;
			switch(cameraFlag){
				case 1:
					//player view
					flagw=0;
					flags=1;
					break;
				case 2:
					//top view
					flagw=1;
					flags=0;
					break;
				case 3:
					//follow view
					break;
				case 4:

					break;
				case 5:
					cameraFlag=0;
					break;
			}
		break;			
	}
}


void handleKeypress2(int key, int x, int y) {
	if(launchTop == 0){
		gravityForce();
	    if (key == GLUT_KEY_LEFT && posX > 3)
	        posX = posX - 0.3;
	    if (key == GLUT_KEY_RIGHT && posX < 57) 
	        posX = posX + 0.3;
	    if (key == GLUT_KEY_UP && posZ > 3)
	        posZ = posZ - 0.3;
	    if (key == GLUT_KEY_DOWN && posZ < 57) 
	        posZ = posZ + 0.3;
	       
	    posY = _terrain->getHeight(posX, posZ)+2;
		previousPosition = posY;
	}
}

void drawTop()
{
	glPushMatrix();	
	
	GLfloat cyan[] = {0.f, .8f, .8f, 1.f};
	glMaterialfv(GL_FRONT, GL_DIFFUSE, cyan);
	
	planeNormal[0] = _terrain->getNormal(posX, posZ)[0];
	planeNormal[1] = _terrain->getNormal(posX, posZ)[1];
	planeNormal[2] = _terrain->getNormal(posX, posZ)[2];

	int xAngle = asin(planeNormal[0]/(sqrt(planeNormal[0]*planeNormal[0] + planeNormal[1]*planeNormal[1] +planeNormal[2]*planeNormal[2])))*(180.0/PI);

	previousPosition =  posY;
	posY = _terrain->getHeight(posX, posZ)+2;
	glTranslatef(posX, posY, posZ );
	
	glRotatef(-90,1,0,0);	
	//glRotatef(topAngle/10,0,0,1);
	glRotatef(xAngle, 0, 1, 0);


	topObject.drawTop(topAngle);

	glPopMatrix();
}

void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	//Draw terrain
	glTranslatef(0.0f, 0.0f, -10.0f);
	glRotatef(30.0f, 1.0f, 0.0f, 0.0f);

	if(flagw==1){
		glRotatef(60,1,0,0);
	}
	
	GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
	
	GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	
	float scale = 8.0f / max(_terrain->width() - 1, _terrain->length() - 1);
	glScalef(scale, scale, scale);

	if(cameraFlag == 3){
		posY = _terrain->getHeight(posX, posZ);
		gluLookAt(
			posX, posY, posZ,
			0, 0, 0,
			0, 1, 0);
	}
	else if(cameraFlag == 4){
		posY = _terrain->getHeight(posX, posZ);
		gluLookAt(
			posX, posY, posZ,
			0, 0, 0,
			0, 1, 0);
	}
	else{
		glTranslatef(-(float)(_terrain->width() - 1) / 2,
				 0.0f,
				 -(float)(_terrain->length() - 1) / 2);

	}
	
	glColor4f(13.0/255.0, 112.0/255.0, 81.0/255.0, 0.9);
	for(int z = 0; z < _terrain->length() - 1; z++) {
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < _terrain->width(); x++) {
			Vec3f normal = _terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _terrain->getHeight(x, z), z);
			normal = _terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}

	if(launchTop == 0){
		topObject.drawArrow(directionAngle);
		setVelocity();
		glPushMatrix();
		topObject.drawBar(vel);
		glPopMatrix();
	}

	topObject.drawTarget(targetX, targetY, targetZ);

	if(topObject.checkBoundary(posX, posZ) == 1) {
	//Draw top
	glPushMatrix();
	drawTop();
	glPopMatrix();
	}

	score.getScore(totalScore);

	glutSwapBuffers();
}



void gravityForce(){

	planeNormal[0] = _terrain->getNormal(posX, posZ)[0];
	planeNormal[1] = _terrain->getNormal(posX, posZ)[1];
	planeNormal[2] = _terrain->getNormal(posX, posZ)[2];

	float gravityX = planeNormal[0]/(sqrt(planeNormal[0]*planeNormal[0] + planeNormal[1]*planeNormal[1] +planeNormal[2]*planeNormal[2]));
	float gravityY = planeNormal[1]/(sqrt(planeNormal[0]*planeNormal[0] + planeNormal[1]*planeNormal[1] +planeNormal[2]*planeNormal[2]));
	float gravityZ = planeNormal[2]/(sqrt(planeNormal[0]*planeNormal[0] + planeNormal[1]*planeNormal[1] +planeNormal[2]*planeNormal[2]));
	
	if(launchTop == 1){
		// vel = sqrt(velX*velX + velZ*velZ);
		velX = velX - 1*gravityX;
		velZ = velZ + 1*gravityZ;
		vel = sqrt(velX*velX + velZ*velZ);
	}
}


void reset(){
	vel = 4;
	angla = 0.0;
	launchTop = 0;
	cameraFlag = 1;
	topAngle = 0.0;
	directionAngle = 0;
	targetX = topObject.getTarget();
	targetZ = topObject.getTarget();
	targetY = _terrain->getHeight(targetX, targetZ);
	flags=0;
	flagw=0;
	posX = 10;
	posY = 100;
	posZ = 50;
	previousPosition = 1.8;
}

void motionTop(){

	if(launchTop == 1 && topObject.checkTarget(posX, posZ, (float)targetX, (float)targetZ) == 0){
		
		gravityForce();
		if(posZ > 2 && posZ < 59){
			posZ = posZ - velZ*0.05;
		}
		else if(posZ <= 2){
			posZ = 2.1;
			velZ = velZ*(-0.01);
		}
		else if(posZ >= 59){
			posZ = 58.9;
			velZ = velZ*(-0.01);
		}

		
		if(posX > 2 && posX < 59){
			posX = posX - velX*0.05;
		}
		else if(posX <=2 ){
			posX = 2.1;
			velX = velX*(-0.01);
		}
		else if(posX >= 59){
			posX = 58.9;
			velX = velX*(-0.01);
		}
	}
	else if(launchTop == 1 && topObject.checkTarget(posX, posZ, (float)targetX, (float)targetZ) == 1){
		totalScore++;
		reset();
	}
}


void update(int value) {

	topAngle = vel*0.5f;

	/*if(launchTop == 1){
		frictionForce();
	}*/

	if(launchTop ==1)
		frictionForce(previousPosition, posY);

	motionTop();
	
	glutPostRedisplay();
	glutTimerFunc(10, update, 0);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	windowWidth = glutGet(GLUT_SCREEN_WIDTH);
    windowHeight = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Beyblade");
	initRendering();
	_terrain = loadTerrain("heightmap.bmp", 20);
	reset();
	totalScore = 0;
	vel = 4;
	glutDisplayFunc(drawScene);
	glutReshapeFunc(handleResize);
	glutKeyboardFunc(handleKeypress1);
	glutSpecialFunc(handleKeypress2);
	glutTimerFunc(10, update, 0);
	glutMainLoop();
	return 0;
}
