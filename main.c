#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>

static void on_keyboard(unsigned char key, int x, int y);
static void on_display(void);
static void on_reshape(int width, int height);

float theta;
float phi;

typedef struct kockica {
	int otvorena; // fleg da li je otvorena kockica, 0 za nije, 1 za jeste
	int bomba; // fleg da li je kockica bomba, 0 za nije, 1 za jeste
} KOCKICA;

KOCKICA kocka[5][5][5];

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Minesweeper 3D");

	glutDisplayFunc(on_display);
	glutKeyboardFunc(on_keyboard);
	glutReshapeFunc(on_reshape);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.8, 0.8, 0.8, 1);


    phi = M_PI/4;
    theta = M_PI/4;

    int i, j, k;
	for(i = 0; i < 5; i++) {
		for(j = 0; j < 5; j++) {
			for(k = 0; k < 5; k++) {
				kocka[i][j][k].otvorena = 0;
				kocka[i][j][k].bomba = 0;
			}
		}
	}
	kocka[4][4][4].otvorena = 1;

	glutMainLoop();

	return 0;
}

static void on_reshape(int width, int height){
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float) width/height, 1, 1000);
}

static void on_keyboard(unsigned char key, int x, int y) {
	switch(key) {
		case 27:
			exit(0);
			break;
        case 's':
            phi = phi + M_PI/90;
            if(phi > M_PI - M_PI/90)
                phi = M_PI - M_PI/90;
            glutPostRedisplay();
            break;
        case 'w':
            phi = phi - M_PI/90;
            if(phi < 0 + M_PI/90)
                phi = 0 + M_PI/90;
            glutPostRedisplay();
            break;
        case 'd':
            theta = theta + M_PI/90;
            if(theta > 2*M_PI)
                theta -= 2*M_PI;
            glutPostRedisplay();
            break;
        case 'a':
            theta = theta - M_PI/90;
            if(theta < -2 * M_PI)
                theta += 2 * M_PI;
            glutPostRedisplay();
            break;
	}
}

static void on_display(void) {


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

	glLineWidth(3);

    GLfloat light_ambient[] = { 0.1, 0.1, 0.1, 1 };

    /* Difuzna boja svetla. */
    GLfloat light_diffuse[] = { 0.7, 0.7, 0.7, 1 };

    /* Spekularna boja svetla. */
    GLfloat light_specular[] = { 0.9, 0.9, 0.9, 1 };

    /* Koeficijenti ambijentalne refleksije materijala. */
    GLfloat ambient_coeffs[] = { 0.3, 0.7, 0.3, 1 };

    /* Koeficijenti difuzne refleksije materijala. */
    GLfloat diffuse_coeffs[] = { 0.2, 1, 0.2, 1 };

    /* Koeficijenti spekularne refleksije materijala. */
    GLfloat specular_coeffs[] = { 1, 1, 1, 1 };

    /* Koeficijent glatkosti materijala. */
    GLfloat shininess = 30;


	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(15 * sin(theta) * sin(phi), 
              15 * cos(phi), 
              15 * cos(theta) * sin(phi), 
              0, 0, 0,
              0, 1, 0);

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, (float[3]){15 * sin(theta) * sin(phi), 
              15 * cos(phi), 
              15 * cos(theta) * sin(phi)});

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_coeffs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_coeffs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_coeffs);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glShadeModel(GL_SMOOTH);

    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
	glBegin(GL_LINES); // Axis z, x, y
		glColor3f(1, 0, 0);
		glVertex3f(5, 0, 0);
		glVertex3f(0, 0, 0);

		glColor3f(0, 1, 0);
		glVertex3f(0, 5, 0);
		glVertex3f(0, 0, 0);

		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 5);
		glVertex3f(0, 0, 0);
	glEnd();
	    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glPopMatrix();

	int i, j, k;
	for(i = 0; i < 5; i++) {
		for(j = 0; j < 5; j++) {
			for(k = 0; k < 5; k++) {
				glPushMatrix();
				glTranslatef(i, j, k);
				if(kocka[i][j][k].otvorena == 0) {
					glutSolidCube(0.9);
				}
				if(kocka[i][j][k].bomba == 1) {
					exit(0);
				}
				glPopMatrix();
			}
		}
	}

	glutSwapBuffers();
}