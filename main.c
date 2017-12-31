#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define VELICINA_KOCKE 5
#define BROJ_MINA 5

static void on_keyboard(unsigned char key, int x, int y);
static void on_display(void);
static void on_reshape(int width, int height);
static void on_mouse(int button, int state, int x, int y);
static void initializeCube(void);  
static void minesweeper(int a, int b, int c);

float theta; //sferne koordinate
float phi;
int gameover; //fleg da li je doslo do kraja igre
int cellsToGo;
int victory;

typedef struct kockica {
	int otvorena; // fleg da li je otvorena kockica, 0 za nije, 1 za jeste
	int bomba; // fleg da li je kockica bomba, 0 za nije, 1 za jeste
	int brojBombiUOkolini;
	int zastavica;   
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

    phi = M_PI/4; 
    theta = M_PI/4; 
    gameover = 0;
    cellsToGo = VELICINA_KOCKE * VELICINA_KOCKE * VELICINA_KOCKE - BROJ_MINA;
    victory = 0;

    initializeCube();

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
        case 'r':
        	phi = M_PI/4;
		    theta = M_PI/4; 
		    gameover = 0;
		    victory = 0;
		    cellsToGo = VELICINA_KOCKE * VELICINA_KOCKE * VELICINA_KOCKE - BROJ_MINA;
		    initializeCube();
		    glutPostRedisplay();
		    break;

	}
}

static void on_mouse(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_LEFT_BUTTON:
			if(state == GLUT_DOWN) { // kada se pritisne levi klik na misu od 2d koordinata dobijamo 3
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
							minesweeper((int)(x1 + 0.5), (int)(y1 + 0.5), (int)(z1 + 0.5));
							glutPostRedisplay();
						}	
				}		
			break;
		case GLUT_RIGHT_BUTTON:
			if(state == GLUT_DOWN) { // kada se pritisne levi klik na misu od 2d koordinata dobijamo 3
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
							kocka[(int)(x1 + 0.5)][(int)(y1 + 0.5)][(int)(z1 + 0.5)].zastavica = !kocka[(int)(x1 + 0.5)][(int)(y1 + 0.5)][(int)(z1 + 0.5)].zastavica;
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

    if(!gameover && !victory) { // Ako nije kliknuta mina onda iscrtavamo kocke
		glTranslatef(-VELICINA_KOCKE/2, -VELICINA_KOCKE/2, -VELICINA_KOCKE/2);
		int i, j, k;
		for(i = 0; i < VELICINA_KOCKE; i++) {
			for(j = 0; j < VELICINA_KOCKE; j++) {
				for(k = 0; k < VELICINA_KOCKE; k++) {
					glPushMatrix();
					if(kocka[i][j][k].otvorena == 1) {

						glDisable(GL_LIGHT0);
						glDisable(GL_LIGHTING);

						glColor3f(0,0,0); 
						glTranslatef(i, j, k);
						glScalef(0.003,0.003,0.003);
						char str[8];
						sprintf(str, "%d", kocka[i][j][k].brojBombiUOkolini);
						char *c;
						for (c=str; *c != '\0'; c++) {
						    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *c);
						}
						glEnable(GL_LIGHT0);
						glEnable(GL_LIGHTING);
					}
					
					if(kocka[i][j][k].otvorena == 0 && kocka[i][j][k].zastavica == 0) {
						glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (float[4]){ 0.3, 0.7, 0.3, 1 });
    					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (float[4]){ 0.2, 1, 0.2, 1 });
						glTranslatef(i, j, k);
						glutSolidCube(0.9);
					} else if(kocka[i][j][k].otvorena == 0) {
						glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (float[4]){0, 1, 1, 1});
    					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (float[4]){0, 1, 1, 1});
						glTranslatef(i, j, k);
						glutSolidCube(0.9);
					}
					glPopMatrix();
				}
			}
		}
	} else if(gameover && !victory){ // inace iscrtavamo mine
		glTranslatef(-VELICINA_KOCKE/2, -VELICINA_KOCKE/2, -VELICINA_KOCKE/2);
		int i, j, k;
		for(i = 0; i < VELICINA_KOCKE; i++) {
			for(j = 0; j < VELICINA_KOCKE; j++) {
				for(k = 0; k < VELICINA_KOCKE; k++) {
					glPushMatrix();
					if(kocka[i][j][k].bomba == 1) {
						glTranslatef(i, j, k);
						glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (float[4]){1, 0, 0, 1});
    					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (float[4]){1, 0, 0, 1});
						glutSolidCube(0.5);
					}
					glPopMatrix();
				}
			}
		}
	} else if(victory) {
		glutSolidTeapot(1);
	}

	glutSwapBuffers();
}

static void initializeCube(void) {
	int i, j, k;
	for(i = 0; i < VELICINA_KOCKE; i++) {
		for(j = 0; j < VELICINA_KOCKE; j++) {
			for(k = 0; k < VELICINA_KOCKE; k++) {
				kocka[i][j][k].otvorena = 0;
				kocka[i][j][k].bomba = 0;
				kocka[i][j][k].brojBombiUOkolini = 0;
				kocka[i][j][k].zastavica = 0;
			}
		}
	}

	srand(time(NULL));
	
	for(i = 0; i < BROJ_MINA; i++) {
		int a, b, c;
		a = rand() % VELICINA_KOCKE;
		b = rand() % VELICINA_KOCKE;
		c = rand() % VELICINA_KOCKE;
		while(kocka[a][b][c].bomba == 1){
			a = rand() % VELICINA_KOCKE;
			b = rand() % VELICINA_KOCKE;
			c = rand() % VELICINA_KOCKE;
		}
		kocka[a][b][c].bomba = 1; // postavljena bomba

		/*Sada treba svakom susedu uvecati brojac bombi u okolini. 
		Ispitujemo za svako polje oko bombe da li postoji i ako postoji uvecavamo brojac bombi u okolini*/

		if(b + 1 < VELICINA_KOCKE)
			kocka[a][b+1][c].brojBombiUOkolini += 1;
		if(b + 1 < VELICINA_KOCKE && c + 1 < VELICINA_KOCKE)
			kocka[a][b+1][c+1].brojBombiUOkolini += 1;
		if(b + 1 < VELICINA_KOCKE && c - 1 >= 0)
			kocka[a][b+1][c-1].brojBombiUOkolini += 1;
		if(c - 1 >= 0)
			kocka[a][b][c-1].brojBombiUOkolini += 1;
		if(c + 1 < VELICINA_KOCKE)
			kocka[a][b][c+1].brojBombiUOkolini += 1;
		if(b - 1 >= 0)
			kocka[a][b-1][c].brojBombiUOkolini += 1;
		if(c + 1 < VELICINA_KOCKE && b - 1 >= 0)
			kocka[a][b-1][c+1].brojBombiUOkolini += 1;
		if(c - 1 >= 0 && b - 1 >= 0)
			kocka[a][b-1][c-1].brojBombiUOkolini += 1;


		if(a + 1 < VELICINA_KOCKE)
			kocka[a+1][b][c].brojBombiUOkolini += 1;
		if(a + 1 < VELICINA_KOCKE && b + 1 < VELICINA_KOCKE)
			kocka[a+1][b+1][c].brojBombiUOkolini += 1;
		if(a + 1 < VELICINA_KOCKE && b + 1 < VELICINA_KOCKE && c - 1 >= 0)
			kocka[a+1][b+1][c-1].brojBombiUOkolini += 1;
		if(a + 1 < VELICINA_KOCKE && b + 1 < VELICINA_KOCKE && c + 1 < VELICINA_KOCKE)
			kocka[a+1][b+1][c+1].brojBombiUOkolini += 1;
		if(a + 1 < VELICINA_KOCKE && c - 1 >= 0)
			kocka[a+1][b][c-1].brojBombiUOkolini += 1;
		if(a + 1 < VELICINA_KOCKE && c + 1 < VELICINA_KOCKE)
			kocka[a+1][b][c+1].brojBombiUOkolini += 1;
		if(a + 1 < VELICINA_KOCKE && b - 1 >= 0)
			kocka[a+1][b-1][c].brojBombiUOkolini += 1;
		if(a + 1 < VELICINA_KOCKE && b - 1 >= 0 && c - 1 >= 0)
			kocka[a+1][b-1][c-1].brojBombiUOkolini += 1;
		if(a + 1 < VELICINA_KOCKE && b - 1 >= 0 && c + 1 < VELICINA_KOCKE)
			kocka[a+1][b-1][c+1].brojBombiUOkolini += 1;


		if(a - 1 >= 0)
			kocka[a-1][b][c].brojBombiUOkolini += 1;
		if(a - 1 >= 0 && c - 1 >= 0)
			kocka[a-1][b][c-1].brojBombiUOkolini += 1;
		if(a - 1 >= 0 && c + 1 < VELICINA_KOCKE)
			kocka[a-1][b][c+1].brojBombiUOkolini += 1;
		if(a - 1 >= 0 && b + 1 < VELICINA_KOCKE)
			kocka[a-1][b+1][c].brojBombiUOkolini += 1;
		if(a - 1 >= 0 && b + 1 < VELICINA_KOCKE && c - 1 >= 0)
			kocka[a-1][b+1][c-1].brojBombiUOkolini += 1;
		if(a - 1 >= 0 && b + 1 < VELICINA_KOCKE && c + 1 < VELICINA_KOCKE)
			kocka[a-1][b+1][c+1].brojBombiUOkolini += 1;
		if(a - 1 >= 0 && b - 1 >= 0)
			kocka[a-1][b-1][c].brojBombiUOkolini += 1;
		if(a - 1 >= 0 && b - 1 >= 0 && c - 1 >= 0)
			kocka[a-1][b-1][c-1].brojBombiUOkolini += 1;
		if(a - 1 >= 0 && b - 1 >= 0 && c + 1 < VELICINA_KOCKE)
			kocka[a-1][b-1][c+1].brojBombiUOkolini += 1;
	}
}

static void minesweeper(int a, int b, int c){
	if(kocka[a][b][c].otvorena == 1) {
		return ; 
	} else {
		if(kocka[a][b][c].bomba == 1) {
			gameover = 1;
			return ;
		} else {
			if(kocka[a][b][c].brojBombiUOkolini > 0) {
				kocka[a][b][c].otvorena = 1;
				if(--cellsToGo == 0){
					victory = 1;
				}
				return ;
			} else {
				kocka[a][b][c].otvorena = 1;
				if(b + 1 < VELICINA_KOCKE)
					minesweeper(a, b+1, c);
				if(b + 1 < VELICINA_KOCKE && c + 1 < VELICINA_KOCKE)
					minesweeper(a, b+1, c+1);
				if(b + 1 < VELICINA_KOCKE && c - 1 >= 0)
					minesweeper(a, b+1, c-1);
				if(c - 1 >= 0)
					minesweeper(a, b, c-1);
				if(c + 1 < VELICINA_KOCKE)
					minesweeper(a, b, c+1);
				if(b - 1 >= 0)
					minesweeper(a, b-1, c);
				if(c + 1 < VELICINA_KOCKE && b - 1 >= 0)
					minesweeper(a, b-1, c+1);
				if(c - 1 >= 0 && b - 1 >= 0)
					minesweeper(a, b-1, c-1);


				if(a + 1 < VELICINA_KOCKE)
					minesweeper(a+1, b, c);
				if(a + 1 < VELICINA_KOCKE && b + 1 < VELICINA_KOCKE)
					minesweeper(a+1, b+1, c);
				if(a + 1 < VELICINA_KOCKE && b + 1 < VELICINA_KOCKE && c - 1 >= 0)
					minesweeper(a+1, b+1, c-1);
				if(a + 1 < VELICINA_KOCKE && b + 1 < VELICINA_KOCKE && c + 1 < VELICINA_KOCKE)
					minesweeper(a+1, b+1, c+1);
				if(a + 1 < VELICINA_KOCKE && c - 1 >= 0)
					minesweeper(a+1, b, c-1);
				if(a + 1 < VELICINA_KOCKE && c + 1 < VELICINA_KOCKE)
					minesweeper(a+1, b, c+1);
				if(a + 1 < VELICINA_KOCKE && b - 1 >= 0)
					minesweeper(a+1, b-1, c);
				if(a + 1 < VELICINA_KOCKE && b - 1 >= 0 && c - 1 >= 0)
					minesweeper(a+1, b-1, c-1);
				if(a + 1 < VELICINA_KOCKE && b - 1 >= 0 && c + 1 < VELICINA_KOCKE)
					minesweeper(a+1, b-1, c+1);


				if(a - 1 >= 0)
					minesweeper(a-1, b, c);
				if(a - 1 >= 0 && c - 1 >= 0)
					minesweeper(a-1, b, c-1);
				if(a - 1 >= 0 && c + 1 < VELICINA_KOCKE)
					minesweeper(a-1, b, c+1);
				if(a - 1 >= 0 && b + 1 < VELICINA_KOCKE)
					minesweeper(a-1, b+1, c);
				if(a - 1 >= 0 && b + 1 < VELICINA_KOCKE && c - 1 >= 0)
					minesweeper(a-1, b+1, c-1);
				if(a - 1 >= 0 && b + 1 < VELICINA_KOCKE && c + 1 < VELICINA_KOCKE)
					minesweeper(a-1, b+1, c+1);
				if(a - 1 >= 0 && b - 1 >= 0)
					minesweeper(a-1, b-1, c);
				if(a - 1 >= 0 && b - 1 >= 0 && c - 1 >= 0)
					minesweeper(a-1, b-1, c-1);
				if(a - 1 >= 0 && b - 1 >= 0 && c + 1 < VELICINA_KOCKE)
					minesweeper(a-1, b-1, c+1);
			}
		}
	}
}