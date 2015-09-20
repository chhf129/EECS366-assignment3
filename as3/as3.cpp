// EECS366 Assignment 2 
//James Du, Haifeng Chen

#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <GL\glut.h>
#include <math.h>

#define ON 1
#define OFF 0


// Global variables
int window_width, window_height;    // Window dimensions
int PERSPECTIVE = ON;
int OBJECT_ON = ON;
int AXIS_ON = ON;
int degrees = 0;
float theta = 1.570796;
float phi = 0;
float rho = 10;
float eyeX, eyeY, eyeZ;
int oldX, oldY;
int rotate, zoom;
// Vertex and Face data structure sued in the mesh reader
// Feel free to change them
typedef struct _point {
	float x, y, z;
} point;

typedef struct _faceStruct {
	int v1, v2, v3;
	int n1, n2, n3;
} faceStruct;

int verts, faces, norms;    // Number of vertices, faces and normals in the system
point *vertList, *normList; // Vertex and Normal Lists
faceStruct *faceList;	    // Face List

							// The mesh reader itself
							// It can read *very* simple obj files
void meshReader(char *filename, int sign)
{
	float x, y, z, len;
	int i;
	char letter;
	point v1, v2, crossP;
	int ix, iy, iz;
	int *normCount;
	FILE *fp;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Cannot open %s\n!", filename);
		return;
	}

	// Count the number of vertices and faces
	while (!feof(fp))
	{
		fscanf(fp, "%c %f %f %f\n", &letter, &x, &y, &z);
		if (letter == 'v')
		{
			verts++;
		}
		else
		{
			faces++;
		}
	}

	fclose(fp);

	printf("verts : %d\n", verts);
	printf("faces : %d\n", faces);

	// Dynamic allocation of vertex and face lists
	faceList = (faceStruct *)malloc(sizeof(faceStruct)*faces);
	vertList = (point *)malloc(sizeof(point)*verts);
	normList = (point *)malloc(sizeof(point)*verts);

	fp = fopen(filename, "r");

	// Read the veritces
	for (i = 0; i < verts; i++)
	{
		fscanf(fp, "%c %f %f %f\n", &letter, &x, &y, &z);
		vertList[i].x = x;
		vertList[i].y = y;
		vertList[i].z = z;
	}

	// Read the faces
	for (i = 0; i < faces; i++)
	{
		fscanf(fp, "%c %d %d %d\n", &letter, &ix, &iy, &iz);
		faceList[i].v1 = ix - 1;
		faceList[i].v2 = iy - 1;
		faceList[i].v3 = iz - 1;
	}
	fclose(fp);


	// The part below calculates the normals of each vertex
	normCount = (int *)malloc(sizeof(int)*verts);
	for (i = 0; i < verts; i++)
	{
		normList[i].x = normList[i].y = normList[i].z = 0.0;
		normCount[i] = 0;
	}

	for (i = 0; i < faces; i++)
	{
		v1.x = vertList[faceList[i].v2].x - vertList[faceList[i].v1].x;
		v1.y = vertList[faceList[i].v2].y - vertList[faceList[i].v1].y;
		v1.z = vertList[faceList[i].v2].z - vertList[faceList[i].v1].z;
		v2.x = vertList[faceList[i].v3].x - vertList[faceList[i].v2].x;
		v2.y = vertList[faceList[i].v3].y - vertList[faceList[i].v2].y;
		v2.z = vertList[faceList[i].v3].z - vertList[faceList[i].v2].z;

		crossP.x = v1.y*v2.z - v1.z*v2.y;
		crossP.y = v1.z*v2.x - v1.x*v2.z;
		crossP.z = v1.x*v2.y - v1.y*v2.x;

		len = sqrt(crossP.x*crossP.x + crossP.y*crossP.y + crossP.z*crossP.z);

		crossP.x = -crossP.x / len;
		crossP.y = -crossP.y / len;
		crossP.z = -crossP.z / len;

		normList[faceList[i].v1].x = normList[faceList[i].v1].x + crossP.x;
		normList[faceList[i].v1].y = normList[faceList[i].v1].y + crossP.y;
		normList[faceList[i].v1].z = normList[faceList[i].v1].z + crossP.z;
		normList[faceList[i].v2].x = normList[faceList[i].v2].x + crossP.x;
		normList[faceList[i].v2].y = normList[faceList[i].v2].y + crossP.y;
		normList[faceList[i].v2].z = normList[faceList[i].v2].z + crossP.z;
		normList[faceList[i].v3].x = normList[faceList[i].v3].x + crossP.x;
		normList[faceList[i].v3].y = normList[faceList[i].v3].y + crossP.y;
		normList[faceList[i].v3].z = normList[faceList[i].v3].z + crossP.z;
		normCount[faceList[i].v1]++;
		normCount[faceList[i].v2]++;
		normCount[faceList[i].v3]++;
	}
	for (i = 0; i < verts; i++)
	{
		normList[i].x = (float)sign*normList[i].x / (float)normCount[i];
		normList[i].y = (float)sign*normList[i].y / (float)normCount[i];
		normList[i].z = (float)sign*normList[i].z / (float)normCount[i];
	}

}



// The display function. It is called whenever the window needs
// redrawing (ie: overlapping window moves, resize, maximize)
// You should redraw your polygons here
void	display(void)
{
	// Clear the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (PERSPECTIVE) {
		// Perpective Projection 
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, (GLdouble)window_width / window_height, 0.01, 10000);
		glutSetWindowTitle("Assignment 2 Template (perspective)");
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// Set the camera position, orientation and target
		eyeX = rho * cos(theta)*sin(phi);
		eyeY = rho * sin(theta)*sin(phi);
		eyeZ = rho * cos(phi);
		gluLookAt(eyeX, eyeY, eyeZ, 0, 0, 0, 0, 1, 0);
	}
	else {
		// Orthogonal Projection 
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-0.25*rho, 0.25*rho, -0.25*rho, 0.25*rho, -10000, 10000);
		glutSetWindowTitle("Assignment 2 Template (orthogonal)");
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	glPushMatrix();
	glRotatef(degrees, 0.0, 0.0, 1);
	if (OBJECT_ON) {
		glColor3f(1, 0, 0);
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < faces; i++) {
			glNormal3f(normList[(faceList[i].v1)].x, normList[(faceList[i].v1)].y, normList[(faceList[i].v1)].z); // normals
			glVertex3f(vertList[(faceList[i].v1)].x, vertList[(faceList[i].v1)].y, vertList[(faceList[i].v1)].z);

			glNormal3f(normList[(faceList[i].v2)].x, normList[(faceList[i].v2)].y, normList[(faceList[i].v2)].z); // normals
			glVertex3f(vertList[(faceList[i].v2)].x, vertList[(faceList[i].v2)].y, vertList[(faceList[i].v2)].z);

			glNormal3f(normList[(faceList[i].v3)].x, normList[(faceList[i].v3)].y, normList[(faceList[i].v3)].z); // normals
			glVertex3f(vertList[(faceList[i].v3)].x, vertList[(faceList[i].v3)].y, vertList[(faceList[i].v3)].z);
		}
		glEnd();
	}


	//   //draw a red rectangle
	//   glColor3f(1,0,0);
	//glBegin(GL_POLYGON);
	//	glVertex3f(0.8,0.8,-0.8);
	//	glVertex3f(0.8,-0.8,-0.8);
	//	glVertex3f(-0.8,-0.8,-0.0);
	//	glVertex3f(-0.8,0.8,-0.0);
	//   glEnd();

	// Draw a blue tetraheadron
	/*
	glColor3f(0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex3f(0.0, 1.6, 0.0);
	glVertex3f(0.8, -0.4, 0.8);
	glVertex3f(-0.8, -0.4, 0.8);

	glVertex3f(0.0, 1.6, 0.0);
	glVertex3f(0.8, -0.4, 0.8);
	glVertex3f(0.0, -0.4, -0.8);

	glVertex3f(0.0, 1.6, 0.0);
	glVertex3f(0.0, -0.4, -0.8);
	glVertex3f(-0.8, -0.4, 0.8);

	glVertex3f(-0.8, -0.4, 0.8);
	glVertex3f(0.8, -0.4, 0.8);
	glVertex3f(0.0, -0.4, -0.8);
	glEnd();
	*/
	if (AXIS_ON) {
		// Draw a green line
		glColor3f(0, 1, 0);
		glBegin(GL_LINES);

		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(4, 0, 0);
		glEnd();

		// Draw a green line
		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0, 4, 0);
		glEnd();

		// Draw a green line
		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0, 0, 4);
		glEnd();
	}
	glPopMatrix();

	// (Note that the origin is lower left corner)
	// (Note also that the window spans (0,1) )
	// Finish drawing, update the frame buffer, and swap buffers
	glutSwapBuffers();
}


// This function is called whenever the window is resized. 
// Parameters are the new dimentions of the window
void	resize(int x, int y)
{
	glViewport(0, 0, x, y);
	window_width = x;
	window_height = y;
	if (PERSPECTIVE) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, (GLdouble)window_width / window_height, 0.01, 10000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	printf("Resized to %d %d\n", x, y);
}


//This function is called whenever the mouse is moved with a mouse button held down.
// x and y are the location of the mouse (in window-relative coordinates)
void OnMouseMove(int x, int y) {
	if (rotate) {
		//prevent the camera from going upside down (limit how far up it can move)
		if ((phi + (y - oldY) * 0.01f  > 1.570796 && y > oldY) || (phi + (y - oldY) * 0.01f < -1.570796 && y < oldY)) {
			//do nothing
		}
		else {
			phi += (y - oldY)*0.01f;
		}
		degrees = degrees + x - oldX;
		glutPostRedisplay();
	}
	else if (zoom) {
		rho += (y - oldY) * 0.01f;
		glutPostRedisplay();
	}
	oldX = x;
	oldY = y;

}

//determine which mouse button is pressed
void OnMouseDown(int button, int state, int x, int y) {
	rotate = 0;
	zoom = 1;
	if (button == GLUT_LEFT_BUTTON) {
		oldX = x;
		oldY = y;
		rotate = 1;
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		zoom = 1;
		oldY = y;
	}
}
// This function is called whenever there is a keyboard input
// key is the ASCII value of the key pressed
// x and y are the location of the mouse
void	keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a':
		AXIS_ON = !AXIS_ON;
		break;
	case 'q':                           /* Quit */
		exit(1);
		break;
	case 'p':
	case 'P':
		// Toggle Projection Type (orthogonal, perspective)
		if (PERSPECTIVE) {
			// switch from perspective to orthogonal
			PERSPECTIVE = OFF;
		}
		else {
			// switch from orthogonal to perspective
			PERSPECTIVE = ON;
		}
		break;
	case 's':
		OBJECT_ON = !OBJECT_ON;

		break;
	default:
		printf("wtf");
		break;
	}

	// Schedule a new display event
	glutPostRedisplay();
}


// Here's the main
int main(int argc, char* argv[])
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Assignment 2 Template (orthogonal)");
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutKeyboardFunc(keyboard);

	// Initialize GL
	//	glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//	glOrtho(-2.5, 2.5, -2.5, 2.5, -10000, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	meshReader("sphere.obj", 1);
	// Switch to main loop
	glutMainLoop();
	return 0;
}