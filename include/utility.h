#include <GL/glut.h>

#define PI 3.14159265359
#define DEG_TO_RAD PI / 180
#define RAD_TO_DEG 180 / PI

typedef struct {
	GLdouble x, y, z;
} vector;

void normalizev(vector *v);
void multiplyv(vector *v, GLdouble factor);
vector addv(vector v1, vector v2);
vector substractv(vector v1, vector v2);
vector rotatev(vector v, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
vector createv(GLdouble x, GLdouble y, GLdouble z);
GLdouble vlength(vector v);
void printv(vector v);

// Loads two Shaders from files
void loadShaders(char * path1, GLenum type1, char * path2, GLenum type2);