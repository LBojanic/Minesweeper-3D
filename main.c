#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define CUBE_SIZE 5
#define NUMBER_OF_MINES 5

static void on_keyboard(unsigned char key, int x, int y);
static void on_display(void);
static void on_reshape(int width, int height);
static void on_mouse(int button, int state, int x, int y);
static void on_timer(int value);
static void initializeCube(void);  
static void minesweeper(int a, int b, int c);
static void drawMine(float size);
static void displayTimeElapsed();
static void textFunc(const char* text, double x, double y);

float theta; 
float phi; /*theta i phi are spherical coordinates*/

int gameover; /*gameover flag: 1 if its gameover, 0 if it is not*/
int victory; /*victory flag: 1 if its victory, 0 if it is not*/

int cellsToGo;/*if cellsToGo is 0 then its victory*/

float scale; /*used for scroll wheel*/

int currAnim; /*indicator for which cells need to be animated when redrawn*/ 

double animationParameter; /*used in timerFunc to scale cubes when animated*/
int timer1_ongoing; /*timer1 indicator*/
int timer2_ongoing; /*timer2 indicator*/

int widthW;
int heightW;/*viewport width and height*/

/*Screen timer flags*/
int resetTimer;
int timeAtReset;
int stopTimer;
char timeElapsed[100];

typedef struct cube {
	int isOpen; /*flag that indicates if the cube is opened or not*/
	int isBomb; /*flag that indicates if the cube is a bomb*/
	int numberOfBombsAround; /*number of bombs around single cube*/
	int bombFlag; /*flag for changing color on right click*/
	int animate; /*flag that indicates if the cube should be animated when redrawn*/
} CUBE;

CUBE cube[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE];

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
    cellsToGo = CUBE_SIZE * CUBE_SIZE * CUBE_SIZE - NUMBER_OF_MINES;
    victory = 0;
    scale = 10;
    currAnim = 1;
    animationParameter = 1;
    initializeCube();
    timer1_ongoing = 0;
    timer2_ongoing = 0;
    resetTimer = 0;
    timeAtReset = 0;
    stopTimer = 0;

	glutMainLoop();

	return 0;	
}

static void on_reshape(int width, int height){
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float) width/height, 1, 1000);
	widthW = width;
	heightW = height;

}
static void on_timer(int value) {
    if(value != 1)
        return;
    
    animationParameter -= 0.03;
	
    glutPostRedisplay();
    
    if(animationParameter > 0 && timer1_ongoing == 1)
        glutTimerFunc(1, on_timer, 1);
    else
        timer1_ongoing = 0;
}

static void on_timer2(int value) {
    if(value != 2)
        return;
    
    glutPostRedisplay();
    if(timer2_ongoing == 1 && stopTimer == 0)
    	glutTimerFunc(50, on_timer2, 2);
}
static void on_keyboard(unsigned char key, int x, int y) {
	switch(key) {
		case 27:
			exit(0);
			break;
		case 'S':
        case 's':
            phi = phi + M_PI/90;
            if(phi > M_PI - M_PI/90)
                phi = M_PI - M_PI/90;
            glutPostRedisplay();
            break;
        case 'W':
        case 'w':
            phi = phi - M_PI/90;
            if(phi < 0 + M_PI/90)
                phi = 0 + M_PI/90;
            glutPostRedisplay();
            break;
        case 'D':
        case 'd':
            theta = theta + M_PI/90;
            if(theta > 2*M_PI)
                theta -= 2*M_PI;
            glutPostRedisplay();
            break;
        case 'A':
        case 'a':
            theta = theta - M_PI/90;
            if(theta < -2 * M_PI)
                theta += 2 * M_PI;
            glutPostRedisplay();
            break;
        case 'R':
        case 'r':
        	phi = M_PI/4;
		    theta = M_PI/4; 
		    gameover = 0;
		    victory = 0;
		    cellsToGo = CUBE_SIZE * CUBE_SIZE * CUBE_SIZE - NUMBER_OF_MINES;
		    scale = 10;
		    timer1_ongoing = 0;
		    timer2_ongoing = 0;
		    stopTimer = 0;
		    timeAtReset = glutGet(GLUT_ELAPSED_TIME);
		    initializeCube();
		    glutPostRedisplay();
		    break;
	}
}

static void on_mouse(int button, int state, int x, int y) {
    
	switch(button) {
		case GLUT_LEFT_BUTTON:
			if(state == GLUT_DOWN) { 
				GLdouble x1, y1, z1; /*changing 2D coordinates of mouse to 3D scene coordinates*/
				GLdouble model[16], projection[16];
				GLint viewport[4];
				GLfloat z;

				glGetDoublev(GL_PROJECTION_MATRIX, projection);
				glGetDoublev(GL_MODELVIEW_MATRIX, model);
				glGetIntegerv(GL_VIEWPORT, viewport);

				y = (float)viewport[3] - y;
				glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
				gluUnProject(x, y, z, model, projection, viewport, &x1, &y1, &z1);

				if((int)(x1 + 0.5) <= CUBE_SIZE-1 && (int)(x1 + 0.5) >= 0 &&
				   (int)(y1 + 0.5) <= CUBE_SIZE-1 && (int)(y1 + 0.5) >= 0 &&
				   (int)(z1 + 0.5) <= CUBE_SIZE-1 && (int)(z1 + 0.5) >= 0) {
				   			currAnim ++;
				   			animationParameter = 1;
							minesweeper((int)(x1 + 0.5), (int)(y1 + 0.5), (int)(z1 + 0.5));
							glutPostRedisplay();
						}	
				}		
			break;
		case GLUT_RIGHT_BUTTON:
			if(state == GLUT_DOWN) { /*changing 2D coordinates of mouse to 3D scene coordinates*/
				GLdouble x1, y1, z1; 
				GLdouble model[16], projection[16];
				GLint viewport[4];
				GLfloat z;

				glGetDoublev(GL_PROJECTION_MATRIX, projection);
				glGetDoublev(GL_MODELVIEW_MATRIX, model);
				glGetIntegerv(GL_VIEWPORT, viewport);

				y = (float)viewport[3] - y;
				glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
				gluUnProject(x, y, z, model, projection, viewport, &x1, &y1, &z1);

				if((int)(x1 + 0.5) <= CUBE_SIZE-1 && (int)(x1 + 0.5) >= 0 && 
				   (int)(y1 + 0.5) <= CUBE_SIZE-1 && (int)(y1 + 0.5) >= 0 &&
				   (int)(z1 + 0.5) <= CUBE_SIZE-1 && (int)(z1 + 0.5) >= 0) {
							cube[(int)(x1 + 0.5)][(int)(y1 + 0.5)][(int)(z1 + 0.5)].bombFlag = !cube[(int)(x1 + 0.5)][(int)(y1 + 0.5)][(int)(z1 + 0.5)].bombFlag;
							glutPostRedisplay();
						}	
				}	
			break;
		case 3:
			scale -= 0.1;
			glutPostRedisplay();
			break;
		case 4:
			scale += 0.1;
			glutPostRedisplay();
			break;	
	}
}

static void on_display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);



	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(scale * sin(theta) * sin(phi), 
              scale * cos(phi), 
              scale * cos(theta) * sin(phi), 
              0, 0, 0,
              0, 1, 0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, (float[4]){ 0.2f, 0.2f, 0.2f, 1.0f });
    glLightfv(GL_LIGHT0, GL_DIFFUSE, (float[4]){ 1.0f, 1.0f, 1.0f, 1.0f });
    glLightfv(GL_LIGHT0, GL_SPECULAR, (float[4]){ 1.0f, 1.0f, 1.0f, 1.0f });

    glLightfv(GL_LIGHT0, GL_POSITION, (float[4]){scale * sin(theta) * sin(phi), 
              							scale * cos(phi), 
              							scale * cos(theta) * sin(phi), 1});
    

    glShadeModel(GL_SMOOTH);

	displayTimeElapsed();
	
    if(!gameover && !victory) { 
		glTranslatef(-CUBE_SIZE/2, -CUBE_SIZE/2, -CUBE_SIZE/2);
		int i, j, k;
		for(i = 0; i < CUBE_SIZE; i++) {
			for(j = 0; j < CUBE_SIZE; j++) {
				for(k = 0; k < CUBE_SIZE; k++) {
					glPushMatrix();
						glPushMatrix();

						glDisable(GL_LIGHT0);
						glDisable(GL_LIGHTING);

						glColor3f(0,0,0); 
						glTranslatef(i, j, k);
						glScalef(0.003,0.003,0.003);
						glRotatef(theta * 180 / M_PI, 0, 1, 0);
						glRotatef(phi * 180 / M_PI - 50, 1, 0, 0);
						char str[8];
						sprintf(str, "%d", cube[i][j][k].numberOfBombsAround);
						char *c = str;
						
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				        glEnable(GL_BLEND);
				        glEnable(GL_LINE_SMOOTH);
				        glLineWidth(2.0);	

						if(*c != '0') {	
							for (c=str; *c; c++) {
							    glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
							}
						}
						glEnable(GL_LIGHT0);
						glEnable(GL_LIGHTING);

					glPopMatrix();
                    if(cube[i][j][k].isOpen == 1 && cube[i][j][k].bombFlag == 0 && cube[i][j][k].animate == currAnim) {
                        if(animationParameter < 0)
                        {
                            cube[i][j][k].isOpen++;
                        }
						glPushMatrix();
						glMaterialfv(GL_FRONT, GL_AMBIENT, (float[4]){ 0.24725f, 0.1995f, 0.0745f, 1.0f });
    					glMaterialfv(GL_FRONT, GL_DIFFUSE, (float[4]){0.75164f, 0.60648f, 0.22648f, 1.0f });
    					glMaterialfv(GL_FRONT, GL_SPECULAR, (float[4]){0.628281f, 0.555802f, 0.366065f, 1.0f });
    					glMaterialf(GL_FRONT, GL_SHININESS, 51.2f );
						glTranslatef(i, j, k);
                        glScalef(animationParameter, animationParameter, animationParameter);
						glutSolidCube(0.9);
						if(timer1_ongoing == 0) {
                            timer1_ongoing = 1;
							glutTimerFunc(50, on_timer, 1);
                        }
						glPopMatrix();
					} else if(cube[i][j][k].isOpen == 0 && cube[i][j][k].bombFlag == 0 && cube[i][j][k].animate != currAnim) {
						glPushMatrix();
						glMaterialfv(GL_FRONT, GL_AMBIENT, (float[4]){ 0.24725f, 0.1995f, 0.0745f, 1.0f });
    					glMaterialfv(GL_FRONT, GL_DIFFUSE, (float[4]){0.75164f, 0.60648f, 0.22648f, 1.0f });
    					glMaterialfv(GL_FRONT, GL_SPECULAR, (float[4]){0.628281f, 0.555802f, 0.366065f, 1.0f });
    					glMaterialf(GL_FRONT, GL_SHININESS, 51.2f );
						glTranslatef(i, j, k);
						glutSolidCube(0.9);
						glPopMatrix();
					} else if(cube[i][j][k].isOpen == 0) {
						glPushMatrix();
						glMaterialfv(GL_FRONT, GL_AMBIENT, (float[4]){ 0.105882f, 0.058824f, 0.113725f, 1.0f });
    					glMaterialfv(GL_FRONT, GL_DIFFUSE, (float[4]){0.427451f, 0.470588f, 0.541176f, 1.0f });
    					glMaterialfv(GL_FRONT, GL_SPECULAR, (float[4]){0.333333f, 0.333333f, 0.521569f, 1.0f });
    					glMaterialf(GL_FRONT, GL_SHININESS, 9.84615f);

						glTranslatef(i, j, k);
						glutSolidCube(0.9);
						glPopMatrix();
					}
					glPopMatrix();
				}
			}
		}
	} else if(gameover && !victory){
		textFunc("YOU LOST!", widthW/2 - 30, heightW - 50);
		stopTimer = 1;
		glTranslatef(-CUBE_SIZE/2, -CUBE_SIZE/2, -CUBE_SIZE/2);
		int i, j, k;
		for(i = 0; i < CUBE_SIZE; i++) {
			for(j = 0; j < CUBE_SIZE; j++) {
				for(k = 0; k < CUBE_SIZE; k++) {
					glPushMatrix();
					if(cube[i][j][k].isBomb == 1) {
						glTranslatef(i, j, k);
						glMaterialfv(GL_FRONT, GL_AMBIENT, (float[4]){1, 0, 0, 1});
    					glMaterialfv(GL_FRONT, GL_DIFFUSE, (float[4]){1, 0, 0, 1});
						drawMine(0.5);
					}
					glPopMatrix();
				}
			}
		}
	} else if(victory) {
		stopTimer = 1;
		char victoryString[51];
		sprintf(victoryString, "YOU WON!!! Your score is: %s\n", timeElapsed);
		textFunc(victoryString, widthW/2 - 150, heightW - 50);
		glutSolidTeapot(3);
	}

	glutSwapBuffers();
}

static void initializeCube(void) {
	int i, j, k;
	for(i = 0; i < CUBE_SIZE; i++) {
		for(j = 0; j < CUBE_SIZE; j++) {
			for(k = 0; k < CUBE_SIZE; k++) {
				cube[i][j][k].isOpen = 0;
				cube[i][j][k].isBomb = 0;
				cube[i][j][k].numberOfBombsAround = 0;
				cube[i][j][k].bombFlag = 0;
				cube[i][j][k].animate = 0;
			}
		}
	}

	srand(time(NULL));
	
	for(i = 0; i < NUMBER_OF_MINES; i++) {
		int a, b, c;
		a = rand() % CUBE_SIZE;
		b = rand() % CUBE_SIZE;
		c = rand() % CUBE_SIZE;
		while(cube[a][b][c].isBomb == 1){
			a = rand() % CUBE_SIZE;
			b = rand() % CUBE_SIZE;
			c = rand() % CUBE_SIZE;
		}
		cube[a][b][c].isBomb = 1; 

		if(b + 1 < CUBE_SIZE)
			cube[a][b+1][c].numberOfBombsAround += 1;
		if(b + 1 < CUBE_SIZE && c + 1 < CUBE_SIZE)
			cube[a][b+1][c+1].numberOfBombsAround += 1;
		if(b + 1 < CUBE_SIZE && c - 1 >= 0)
			cube[a][b+1][c-1].numberOfBombsAround += 1;
		if(c - 1 >= 0)
			cube[a][b][c-1].numberOfBombsAround += 1;
		if(c + 1 < CUBE_SIZE)
			cube[a][b][c+1].numberOfBombsAround += 1;
		if(b - 1 >= 0)
			cube[a][b-1][c].numberOfBombsAround += 1;
		if(c + 1 < CUBE_SIZE && b - 1 >= 0)
			cube[a][b-1][c+1].numberOfBombsAround += 1;
		if(c - 1 >= 0 && b - 1 >= 0)
			cube[a][b-1][c-1].numberOfBombsAround += 1;


		if(a + 1 < CUBE_SIZE)
			cube[a+1][b][c].numberOfBombsAround += 1;
		if(a + 1 < CUBE_SIZE && b + 1 < CUBE_SIZE)
			cube[a+1][b+1][c].numberOfBombsAround += 1;
		if(a + 1 < CUBE_SIZE && b + 1 < CUBE_SIZE && c - 1 >= 0)
			cube[a+1][b+1][c-1].numberOfBombsAround += 1;
		if(a + 1 < CUBE_SIZE && b + 1 < CUBE_SIZE && c + 1 < CUBE_SIZE)
			cube[a+1][b+1][c+1].numberOfBombsAround += 1;
		if(a + 1 < CUBE_SIZE && c - 1 >= 0)
			cube[a+1][b][c-1].numberOfBombsAround += 1;
		if(a + 1 < CUBE_SIZE && c + 1 < CUBE_SIZE)
			cube[a+1][b][c+1].numberOfBombsAround += 1;
		if(a + 1 < CUBE_SIZE && b - 1 >= 0)
			cube[a+1][b-1][c].numberOfBombsAround += 1;
		if(a + 1 < CUBE_SIZE && b - 1 >= 0 && c - 1 >= 0)
			cube[a+1][b-1][c-1].numberOfBombsAround += 1;
		if(a + 1 < CUBE_SIZE && b - 1 >= 0 && c + 1 < CUBE_SIZE)
			cube[a+1][b-1][c+1].numberOfBombsAround += 1;


		if(a - 1 >= 0)
			cube[a-1][b][c].numberOfBombsAround += 1;
		if(a - 1 >= 0 && c - 1 >= 0)
			cube[a-1][b][c-1].numberOfBombsAround += 1;
		if(a - 1 >= 0 && c + 1 < CUBE_SIZE)
			cube[a-1][b][c+1].numberOfBombsAround += 1;
		if(a - 1 >= 0 && b + 1 < CUBE_SIZE)
			cube[a-1][b+1][c].numberOfBombsAround += 1;
		if(a - 1 >= 0 && b + 1 < CUBE_SIZE && c - 1 >= 0)
			cube[a-1][b+1][c-1].numberOfBombsAround += 1;
		if(a - 1 >= 0 && b + 1 < CUBE_SIZE && c + 1 < CUBE_SIZE)
			cube[a-1][b+1][c+1].numberOfBombsAround += 1;
		if(a - 1 >= 0 && b - 1 >= 0)
			cube[a-1][b-1][c].numberOfBombsAround += 1;
		if(a - 1 >= 0 && b - 1 >= 0 && c - 1 >= 0)
			cube[a-1][b-1][c-1].numberOfBombsAround += 1;
		if(a - 1 >= 0 && b - 1 >= 0 && c + 1 < CUBE_SIZE)
			cube[a-1][b-1][c+1].numberOfBombsAround += 1;
	}
}

static void minesweeper(int a, int b, int c){
	if(cube[a][b][c].isOpen == 1) {
		return ; 
	} else {
		if(cube[a][b][c].isBomb == 1) {
			gameover = 1;
			return ;
		} else {
			cube[a][b][c].animate = currAnim;
			if(cube[a][b][c].numberOfBombsAround > 0) {
				cube[a][b][c].isOpen = 1;
				cellsToGo -= 1;
				if(cellsToGo <= 0){
					victory = 1; 
				}
				return ;
			} else {
				cube[a][b][c].isOpen = 1;
				cellsToGo -= 1;
				if(b + 1 < CUBE_SIZE)
					minesweeper(a, b+1, c);
				if(b + 1 < CUBE_SIZE && c + 1 < CUBE_SIZE)
					minesweeper(a, b+1, c+1);
				if(b + 1 < CUBE_SIZE && c - 1 >= 0)
					minesweeper(a, b+1, c-1);
				if(c - 1 >= 0)
					minesweeper(a, b, c-1);
				if(c + 1 < CUBE_SIZE)
					minesweeper(a, b, c+1);
				if(b - 1 >= 0)
					minesweeper(a, b-1, c);
				if(c + 1 < CUBE_SIZE && b - 1 >= 0)
					minesweeper(a, b-1, c+1);
				if(c - 1 >= 0 && b - 1 >= 0)
					minesweeper(a, b-1, c-1);


				if(a + 1 < CUBE_SIZE)
					minesweeper(a+1, b, c);
				if(a + 1 < CUBE_SIZE && b + 1 < CUBE_SIZE)
					minesweeper(a+1, b+1, c);
				if(a + 1 < CUBE_SIZE && b + 1 < CUBE_SIZE && c - 1 >= 0)
					minesweeper(a+1, b+1, c-1);
				if(a + 1 < CUBE_SIZE && b + 1 < CUBE_SIZE && c + 1 < CUBE_SIZE)
					minesweeper(a+1, b+1, c+1);
				if(a + 1 < CUBE_SIZE && c - 1 >= 0)
					minesweeper(a+1, b, c-1);
				if(a + 1 < CUBE_SIZE && c + 1 < CUBE_SIZE)
					minesweeper(a+1, b, c+1);
				if(a + 1 < CUBE_SIZE && b - 1 >= 0)
					minesweeper(a+1, b-1, c);
				if(a + 1 < CUBE_SIZE && b - 1 >= 0 && c - 1 >= 0)
					minesweeper(a+1, b-1, c-1);
				if(a + 1 < CUBE_SIZE && b - 1 >= 0 && c + 1 < CUBE_SIZE)
					minesweeper(a+1, b-1, c+1);


				if(a - 1 >= 0)
					minesweeper(a-1, b, c);
				if(a - 1 >= 0 && c - 1 >= 0)
					minesweeper(a-1, b, c-1);
				if(a - 1 >= 0 && c + 1 < CUBE_SIZE)
					minesweeper(a-1, b, c+1);
				if(a - 1 >= 0 && b + 1 < CUBE_SIZE)
					minesweeper(a-1, b+1, c);
				if(a - 1 >= 0 && b + 1 < CUBE_SIZE && c - 1 >= 0)
					minesweeper(a-1, b+1, c-1);
				if(a - 1 >= 0 && b + 1 < CUBE_SIZE && c + 1 < CUBE_SIZE)
					minesweeper(a-1, b+1, c+1);
				if(a - 1 >= 0 && b - 1 >= 0)
					minesweeper(a-1, b-1, c);
				if(a - 1 >= 0 && b - 1 >= 0 && c - 1 >= 0)
					minesweeper(a-1, b-1, c-1);
				if(a - 1 >= 0 && b - 1 >= 0 && c + 1 < CUBE_SIZE)
					minesweeper(a-1, b-1, c+1);
			}
		}
	}
}
static void drawMine(float size) {
    
    glMatrixMode(GL_MODELVIEW);
    
    glPushMatrix();
    glTranslatef(-size, 0, 0);
    glutSolidCube(0.2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(size, 0, 0);
    glutSolidCube(0.2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0, -size);
    glutSolidCube(0.2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0, size);
    glutSolidCube(0.2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -(size), 0);
    glutSolidCube(0.2);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0, size, 0);
    glutSolidCube(0.2);
    glPopMatrix();

    glutSolidSphere(size, 20, 20);
}

static void displayTimeElapsed() {
	if(stopTimer != 1) {
		struct tm *info;
		time_t pom = (glutGet(GLUT_ELAPSED_TIME) - timeAtReset)/1000;
		info = localtime(&pom);	
		strftime(timeElapsed, 50, "%n %M : %S", info);
	    textFunc(timeElapsed, widthW/2 - 36, heightW - 50);
	    if(timer2_ongoing == 0) {
	    	timer2_ongoing = 1;
	    	glutTimerFunc(50, on_timer2, 2);
	    } 
	}	     
}
void textFunc(const char* text, double x, double y){
    glPushMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0, 0, 0);

    glMatrixMode(GL_PROJECTION);
    double matrix[16];
    glGetDoublev(GL_PROJECTION_MATRIX, matrix);
    glLoadIdentity();
    glOrtho(0, widthW, 0, heightW, -5, 5);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRasterPos2f(x,y);
    
    for(int i = 0; text[i]; i++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)text[i]);
    }
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(matrix);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);

    glPopMatrix();
}