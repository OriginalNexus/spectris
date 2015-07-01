#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

#define VECTOR_IMPLEMENTATION
#include "../include/vector.h"
#define MOTION_IMPLEMENTATION
#include "../include/motion.h"
#include "../include/utility.h"
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
int skipToDrop = false;
int dropPos = 95;

// This will draw a 100 by 100 plane, the camera being always in the middle of it. Gives the impression of an infinite world
void drawGround() {
	int centerX = (int) mot_GetEyePos().x, centerY = (int) mot_GetEyePos().z;
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
	// FFT size (2 ^ n)
	int n = 10;
	// Number of bands
	int bandNum = 32;

	// Get FFT
	int N = (int)pow(2, n);
	float fft[N];
	if (BASS_ChannelGetData(soundStream, &fft, BASS_DATA_FFT2048) == -1) return;

	// Spits the buffer in bandNum bands and computes the magnitude for each one
	int i, j;
	for (i = 0; i < bandNum; i++) {
		float peak = 0;
		int p1 = (int)pow(2, i * n / (float)(bandNum - 1));
		if (p1 > N - 1) p1 = N - 1;
		if (p1 <= 0) p1 = 1;

		int p0 = (int)pow(2, (i - 1) * n / (float)(bandNum - 1));
		if (p0 > N - 1) p0 = N - 1;
		if (p0 < 0) p0 = 0;

		for (j = p0; j < p1; j++)
		{
		    if (peak < fft[j + 1]) peak = fft[j + 1];
		}
	
		// Draw band
		float scale = sqrt(peak) * 30;
		float width = 0.5;
		float spacing = 0.5;

		glPushMatrix();
			glTranslatef(0, scale * width / 2, i * (width + spacing));
			glScalef(1, scale, 1);
			glColor3f(0, 0.2, 1);
			glutSolidCube(width);
		glPopMatrix();
	}


	// // Failed attempt 2
	// for (i = 0; i < bandNum; i++) {
	// 	float peak = 0;
	// 	int p = (int)pow(2, i * 10.0 / (bandNum - 1));
	// 	if (p > 1023) p = 1023;
	// 	if (p <= 0) p = 1;
	// 	for (j = 0; j < p; j++)
	// 	{
	// 	    if (peak < fft[j + 1]) peak = fft[j + 1];
	// 	}
	// 	float y = sqrt(peak);
	
	// 	// Draw band
	// 	float scale = y * 30;
	// 	glPushMatrix();
	// 		glTranslatef(0, scale * 0.25, i);
	// 		glScalef(1, scale, 1);
	// 		glColor3f(0, 0.2, 1);
	// 		glutSolidCube(0.5);
	// 	glPopMatrix();
	// }


	// // Failed attempt 1
	// GLdouble mag;
	// for (i = 0; i < bandNum; i++) {
	// 	mag = 0;
	// 	for (j = 0; j < N / bandNum; j++) {
	// 			mag += fft[i * (N / bandNum) + j + 1];
	// 	}
	// 	// Average
	// 	mag /= N / bandNum;

	// 	// Draw band

	// 	float scaleCoef = 0.05 * (-20) * log10(mag);
	// 	glPushMatrix();
	// 		glTranslatef(0, scaleCoef * 0.25, i);
	// 		glScalef(1, scaleCoef, 1);
	// 		glColor3f(0, 0.2, 1);
	// 		glutSolidCube(0.5);
	// 	glPopMatrix();
	// }

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
	mot_MoveCamera();
	mot_SetCamera();
	// Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	// Draws ground
	// drawGround();

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
	if (skipToDrop) {
		BASS_ChannelSetPosition(soundStream, BASS_ChannelSeconds2Bytes(soundStream, dropPos), BASS_POS_BYTE);	
	}
}


int main(int argc, char *argv[])
{
	printf("===================================================\n");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Spectris");
	glewInit();

	// Event listeners
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	
	initialize();
	mot_Init(1.0 / 80);
	mot_TeleportCamera(-20, mot_GetConstant(MOT_EYE_HEIGHT), 15);
	bass();

	// Starts main timer
	glutTimerFunc(10, tick, 0);

	glutMainLoop();
	return 0;
}
