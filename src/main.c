#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../include/utility.h"
#include "../include/motion.h"
#include "../include/bass.h"

// C does not support boolean
#define true 1
#define false 0

// Ligth parameters
GLfloat lightPos[] = {-1, 4.5, 16, 1};
GLfloat lightAmbient[] = {0.5, 0.5, 0.5, 1};
GLfloat lightDiffuse[] = {0.8, 0.8, 0.8, 1};
GLfloat lightSpecular[] = {0.4, 0.4, 0.4, 1};

int soundStream = 0;

// This will draw a 100 by 100 plane, the camera being always in the middle of it. Gives the impression of an infinite world
void drawGround() {
	int centerX = (int) motGetEyePos().x, centerY = (int) motGetEyePos().z;
	int i, j;
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
		glTranslatef(0, -0.01, 0);
		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);
			for (i = 0; i < 100; i++) {
				for (j = 0; j < 100; j++) {
					glTexCoord2f(1, 0);
					glVertex3f(centerX + i - 50, 0, centerY + j - 50);
					glTexCoord2f(1, 1);
					glVertex3f(centerX + i - 50 + 1, 0, centerY + j - 50);
					glTexCoord2f(0, 1);
					glVertex3f(centerX + i - 50 + 1, 0, centerY + j - 50 + 1);
					glTexCoord2f(0, 0);
					glVertex3f(centerX + i - 50, 0, centerY + j - 50 + 1);
				}
			}
		glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void drawSpectrum(void) {
	// Spectrum buffer
	GLfloat buff[1024];
	// Get the buffer
	BASS_ChannelGetData(soundStream, &buff, BASS_DATA_FFT2048 | BASS_DATA_FFT_REMOVEDC | BASS_DATA_FLOAT);

	// Spits the buffer in 32 bands and computes the average for each one
	int i, j;
	GLdouble med;
	for (i = 0; i < 32; i++) {
		med = 0;
		for (j = 0; j < 14; j++) {
				med += buff[64 + i * 14 + j]; // Ignores the firt 64 and the last 512 (they are either too loud or silent)
		}
		// Average
		med /= 14;

		// Draws a Line
		glPushMatrix();
			glTranslatef(0, sqrt(med) * 100 * 0.25, i);
			glScalef(1, sqrt(med) * 100, 1);
			glColor3f(0, 0.2, 1);
			glutSolidCube(0.5);
		glPopMatrix();
	}

	// Reset color
	glColor3f(0.5, 0.5, 0.5);
}

void display(void)
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	motMoveCamera();
	// Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	// Draws ground
	drawGround();

	// Draw spectrum
	drawSpectrum();

	// Swap buffers in GPU
	glutSwapBuffers();
}

void reshape(int width, int height)
{
	// Set the viewport to the full window size
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	// Load the projection matrix
	glMatrixMode(GL_PROJECTION);
	// Clear all the transformations on the projection matrix
	glLoadIdentity();
	// Set the perspective according to the window size
	gluPerspective(70, (GLdouble) width / (GLdouble) height, 0.1, 60000);
	// Load back the modelview matrix
	glMatrixMode(GL_MODELVIEW);
}

void initialize(void)
{
	// Set the background to light gray
	glClearColor(0.8, 0.8, 0.8, 1);
	// Enables depth test
	glEnable(GL_DEPTH_TEST);
	// Disable color material
	glEnable(GL_COLOR_MATERIAL);
	// Enable normalize 
	glEnable(GL_NORMALIZE);
	// Enable Blend and transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Sets the light
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	// Enables lighting
	glEnable(GL_LIGHTING);
	// Enables the light
	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);
}

void tick(int value)
{	
	// Calls the display() function
	glutPostRedisplay();

	// Waits 10 ms
	glutTimerFunc(10, tick, 0);
}

// Called when the song ends
void replayMusic(HSYNC handle, DWORD channel, DWORD data, void *user) {
	// Replays the song
	BASS_ChannelPlay(channel, TRUE);
}

void bass(void) {
	// Initializes the library
	BASS_Init(-1, 44100, 0, 0, NULL);
	// Creates a stream from a file (the stream is some sort of ID)
	soundStream = BASS_StreamCreateFile(FALSE, "data/sound/Tritonal  Paris Blohm ft Sterling Fox - Colors Culture Code Remix.mp3", 0, 0, 0);
	// Adds an "Event Listener" (it is called sync) to detect when the song ends
	BASS_ChannelSetSync(soundStream, BASS_SYNC_MIXTIME | BASS_SYNC_END, 0, replayMusic, 0);
	// Plays the stream
	BASS_ChannelPlay(soundStream, TRUE);
	//BASS_ChannelSetPosition(soundStream, BASS_ChannelSeconds2Bytes(soundStream, 95), BASS_POS_BYTE);
}


int main(int argc, char *argv[])
{
	printf("===================================================\n");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Spectris");
	glewInit();
	motionInit();

	// Event listeners
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	
	initialize();

	bass();

	// Starts main timer
	glutTimerFunc(10, tick, 0);

	glutMainLoop();
	return 0;
}
