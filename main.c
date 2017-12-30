#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define VELICINA_KOCKE 5

static void on_keyboard(unsigned char key, int x, int y);
static void on_display(void);
static void on_reshape(int width, int height);
static void on_mouse(int button, int state, int x, int y);

float theta; //sferne koordinate
float phi;

typedef struct kockica {
	int otvorena; // fleg da li je otvorena kockica, 0 za nije, 1 za jeste
	int bomba; // fleg da li je kockica bomba, 0 za nije, 1 za jeste
} KOCKICA;

KOCKICA kocka[VELICINA_KOCKE][VELICINA_KOCKE][VELICINA_KOCKE];

int main(int argc, char *argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Minesweeper 3D");

	glutDisplayFunc(on_display);
	glutKeyboardFunc(on_keyboard);
	glutReshapeFunc(on_reshape);
	glutMouseFunc(on_mouse);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.8, 0.8, 0.8, 1);

    phi = M_PI/4; // Inicijalni pogled 
    theta = M_PI/4; 

    int i, j, k;
	for(i = 0; i < VELICINA_KOCKE; i++) {
		for(j = 0; j < VELICINA_KOCKE; j++) {
			for(k = 0; k < VELICINA_KOCKE; k++) {
				kocka[i][j][k].otvorena = 0;
				kocka[i][j][k].bomba = 0;
			}
		}
	}

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

static void on_mouse(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_LEFT_BUTTON:
			if(state == GLUT_DOWN) { // kada se pritisne levi klik na misu od 2d koordinata dobijamo 3d

				GLdouble x1, y1, z1; //Svetske koordinate
				GLdouble model[16], projection[16];
				GLint viewport[4];
				GLfloat z;

				glGetDoublev(GL_PROJECTION_MATRIX, projection);
				glGetDoublev(GL_MODELVIEW_MATRIX, model);
				glGetIntegerv(GL_VIEWPORT, viewport);

				y = (float)viewport[3] - y;
				glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
				gluUnProject(x, y, z, model, projection, viewport, &x1, &y1, &z1);

				if((int)(x1 + 0.5) <= VELICINA_KOCKE-1 && (int)(x1 + 0.5) >= 0 && //samo ako su nam koordinate iz intervala [0, 4]
				   (int)(y1 + 0.5) <= VELICINA_KOCKE-1 && (int)(y1 + 0.5) >= 0 &&
				   (int)(z1 + 0.5) <= VELICINA_KOCKE-1 && (int)(z1 + 0.5) >= 0) {

						kocka[(int)(x1 + 0.5)][(int)(y1 + 0.5)][(int)(z1 + 0.5)].otvorena = 1;
						glutPostRedisplay();	
				}		
			}
			break;

	}
}

static void on_display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

	glLineWidth(3);

    GLfloat light_ambient[] = { 0, 0, 0, 1};
    GLfloat light_diffuse[] = { 0.7, 0.7, 0.7, 1 };
    GLfloat light_specular[] = { 0, 0, 0, 0 };


    GLfloat ambient_coeffs[] = { 0.3, 0.7, 0.3, 1 };
    GLfloat diffuse_coeffs[] = { 0.2, 1, 0.2, 1 };
    GLfloat specular_coeffs[] = { 0, 0, 0, 0 };
    GLfloat shininess = 30;


	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(10 * sin(theta) * sin(phi), 
              10 * cos(phi), 
              10 * cos(theta) * sin(phi), 
              0, 0, 0,
              0, 1, 0);

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, (float[4]){10 * sin(theta) * sin(phi), 
              							10 * cos(phi), 
              							10 * cos(theta) * sin(phi), 0});

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_coeffs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_coeffs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_coeffs);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    
    glShadeModel(GL_SMOOTH);

 //    glPushMatrix();
 //    glDisable(GL_LIGHTING);
 //    glDisable(GL_LIGHT0);
	// glBegin(GL_LINES); // Axis z, x, y
	// 	glColor3f(1, 0, 0);
	// 	glVertex3f(VELICINA_KOCKE, 0, 0);
	// 	glVertex3f(0, 0, 0);

	// 	glColor3f(0, 1, 0);
	// 	glVertex3f(0, VELICINA_KOCKE, 0);
	// 	glVertex3f(0, 0, 0);

	// 	glColor3f(0, 0, 1);
	// 	glVertex3f(0, 0, VELICINA_KOCKE);	
	// 	glVertex3f(0, 0, 0);
	// glEnd();
	//     glEnable(GL_LIGHTING);
 //    glEnable(GL_LIGHT0);
	// glPopMatrix();

	glTranslatef(-VELICINA_KOCKE/2, -VELICINA_KOCKE/2, -VELICINA_KOCKE/2);
	int i, j, k;
	for(i = 0; i < VELICINA_KOCKE; i++) {
		for(j = 0; j < VELICINA_KOCKE; j++) {
			for(k = 0; k < VELICINA_KOCKE; k++) {
				glPushMatrix();
				if(kocka[i][j][k].otvorena == 0) {
					glTranslatef(i, j, k);
					glutSolidCube(0.9);
				}
				glPopMatrix();
			}
		}
	}

	glutSwapBuffers();
}