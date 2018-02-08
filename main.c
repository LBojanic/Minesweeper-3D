#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define CUBE_SIZE 5
#define NUMBER_OF_MINES 5
#define TEXTUREFILENAME "wood.bmp"

typedef struct {
  unsigned short type;
  unsigned int size;
  unsigned short reserved1;
  unsigned short reserved2;
  unsigned int offsetbits;
} BITMAPFILEHEADER;

typedef struct {
  unsigned int size;
  unsigned int width;
  unsigned int height;
  unsigned short planes;
  unsigned short bitcount;
  unsigned int compression;
  unsigned int sizeimage;
  int xpelspermeter;
  int ypelspermeter;
  unsigned int colorsused;
  unsigned int colorsimportant;
} BITMAPINFOHEADER;

typedef struct Image {
  int width; /*Image width*/
  int height; /*Image height*/
  char *pixels; /*Array of data for each pixel*/
} Image;

typedef struct cube {
	int isOpen; /*flag that indicates if the cube is opened or not*/
	int isBomb; /*flag that indicates if the cube is a bomb*/
	int numberOfBombsAround; /*number of bombs around single cube*/
	int bombFlag; /*flag for changing color on right click*/
	int animate; /*flag that indicates if the cube should be animated when redrawn*/
} CUBE;

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
static void image_func(Image *image, char *fileName);
static void destroy_image(Image *image);
Image *image_initialization(int width, int height);

static GLuint textureID[1]; /*texture IDs*/

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
int timeAtReset;
int stopTimer;
char timeElapsed[100];

CUBE cube[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE];

int main(int argc, char *argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Minesweeper 3D"); /*Glut initializations*/

	glutDisplayFunc(on_display); /*Declaring callback functions*/
	glutKeyboardFunc(on_keyboard);
	glutReshapeFunc(on_reshape);
	glutMouseFunc(on_mouse);

	glEnable(GL_DEPTH_TEST); /*Enabling depth*/
	glClearColor(0.8, 0.8, 0.8, 1);

    phi = M_PI/4;  
    theta = M_PI/4; /*declaring initial spherical coordinates*/

    gameover = 0; /*declaring initial gameover flag*/
    cellsToGo = CUBE_SIZE * CUBE_SIZE * CUBE_SIZE - NUMBER_OF_MINES; /*declaring initial cells that are not bombs => 
    																cellsToGo = number of cells - number ofbombs*/
    victory = 0; /*declaring initial victory flag*/
    scale = 10; /*initial scale used for scroll wheel*/
    currAnim = 1; /*initial flag for animating cubes*/
    animationParameter = 1; /*initial flag for timer function*/
    initializeCube(); /*cube initializations*/
    timer1_ongoing = 0; /*timer1 flag initialization*/
    timer2_ongoing = 0; /*timer2 flag initialization*/
    timeAtReset = 0; /*reset timer flag - it will remember the time when pressed 'r' - initially 0*/
    stopTimer = 0; /*used for timer on screen, initially 0*/

	glutMainLoop();

	return 0;	
}

static void on_reshape(int width, int height){
	glViewport(0, 0, width, height); /*setting viewport on reshape*/
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float) width/height, 1, 1000); /*setting view volume*/
	widthW = width; /*remembering width and height on reshape in global variables*/
	heightW = height;

}
static void on_timer(int value) {
    if(value != 1) 
        return;
    
    animationParameter -= 0.03; /*decreasing animation parameter so we could scale cube that needs to be animated*/
	
    glutPostRedisplay();
    
    if(animationParameter > 0 && timer1_ongoing == 1)
        glutTimerFunc(1, on_timer, 1); /*cubes need to be scaled down to zero, so we need to call timer again if parameter > 0*/
    else
        timer1_ongoing = 0; /*if cubes are scaled down to zero we dont call timer again*/
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
		case 27: /*program stops on esc button*/
			exit(0);
			break;
		case 'S':
        case 's':
            phi = phi + M_PI/90; /*on 's' button, we increase phi by PI/90 radians*/
            if(phi > M_PI - M_PI/90) /*if larger than maximum value possible, we set phi to max value possible -- PI-PI/90 radians*/
                phi = M_PI - M_PI/90;
            glutPostRedisplay();
            break;
        case 'W':
        case 'w':
            phi = phi - M_PI/90; /*on 'w' button we decrease phi by PI/90 radians*/
            if(phi < 0 + M_PI/90) /*if smaller than minimum value possible, we set phi to min value possible -- PI/90 radians*/
                phi = 0 + M_PI/90;
            glutPostRedisplay();
            break;
        case 'D':
        case 'd':
            theta = theta + M_PI/90; /*on 'd' button we increase theta by PI/90 radians*/
            if(theta > 2*M_PI) /*if our increasing becomes larger than 2PI radians(full circle) we subtract 2PI radians*/
                theta -= 2*M_PI;
            glutPostRedisplay();
            break;
        case 'A':
        case 'a':
            theta = theta - M_PI/90; /*on 'a' button we decrease theta by PI/90 radians*/
            if(theta < -2 * M_PI) /*if our decreasing becomes smaller than 2PI radians(full circle) we add 2PI radians*/
                theta += 2 * M_PI;
            glutPostRedisplay();
            break;
        case 'R':
        case 'r': /*on 'r' button we reset every parametar on initial values*/
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
			if(state == GLUT_DOWN) { /*on pressing left mouse button*/
				GLdouble x1, y1, z1; /*changing 2D coordinates of mouse to 3D scene coordinates*/
				GLdouble model[16], projection[16];
				GLint viewport[4];
				GLfloat z;

				glGetDoublev(GL_PROJECTION_MATRIX, projection); /*we get projection matrix*/
				glGetDoublev(GL_MODELVIEW_MATRIX, model); /*we get modelview matrix*/
				glGetIntegerv(GL_VIEWPORT, viewport); /*we get viewport matrix*/

				y = (float)viewport[3] - y; /*set y to be in bottom left corner*/
				glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z); /*read z coordinate from depth buffer*/
				gluUnProject(x, y, z, model, projection, viewport, &x1, &y1, &z1); /*get x1 y1 z1 from x y and z and matrices that we got
																					x1 y1 z1 will be coordinates in 3d space*/

				if((int)(x1 + 0.5) <= CUBE_SIZE-1 && (int)(x1 + 0.5) >= 0 && /*if our 3d coordinates are in area of our cube*/
				   (int)(y1 + 0.5) <= CUBE_SIZE-1 && (int)(y1 + 0.5) >= 0 &&
				   (int)(z1 + 0.5) <= CUBE_SIZE-1 && (int)(z1 + 0.5) >= 0) {
				   			currAnim ++; /*we increase currAnim parameter to signalize that only this set of cubes should be animated now*/
				   			animationParameter = 1; /*initial value of scaling cubes, we will decrease this in timer function*/
							minesweeper((int)(x1 + 0.5), (int)(y1 + 0.5), (int)(z1 + 0.5)); /*call minesweeper algorithm for selected cube*/
							glutPostRedisplay(); /*redisplay cubes now*/
						}	
				}		
			break;
		case GLUT_RIGHT_BUTTON:
			if(state == GLUT_DOWN) { /*if right mouse button is pressed down*/
				GLdouble x1, y1, z1; 
				GLdouble model[16], projection[16];
				GLint viewport[4];
				GLfloat z;

				glGetDoublev(GL_PROJECTION_MATRIX, projection); /*we get projection matrix*/
				glGetDoublev(GL_MODELVIEW_MATRIX, model);/*we get modelview matrix*/
				glGetIntegerv(GL_VIEWPORT, viewport); /*we get viewport matrix*/

				y = (float)viewport[3] - y;/*set y to be in bottom left corner*/
				glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);/*read z coordinate from depth buffer*/
				gluUnProject(x, y, z, model, projection, viewport, &x1, &y1, &z1);/*get x1 y1 z1 from x y and z and matrices that we got
																					x1 y1 z1 will be coordinates in 3d space*/

				if((int)(x1 + 0.5) <= CUBE_SIZE-1 && (int)(x1 + 0.5) >= 0 && 
				   (int)(y1 + 0.5) <= CUBE_SIZE-1 && (int)(y1 + 0.5) >= 0 &&
				   (int)(z1 + 0.5) <= CUBE_SIZE-1 && (int)(z1 + 0.5) >= 0) {/*if our 3d coordinates are in area of our cube*/
							cube[(int)(x1 + 0.5)][(int)(y1 + 0.5)][(int)(z1 + 0.5)].bombFlag = !cube[(int)(x1 + 0.5)][(int)(y1 + 0.5)][(int)(z1 + 0.5)].bombFlag;
							/*we mark pressed cube to be colored differently, because player thinks that there is a bomb
							inverting parameter bombFlag will remove new color if pressed again*/
							glutPostRedisplay();
						}	
				}	
			break;
		case 3: /*on scroll wheel we want to get further from cube*/
			scale -= 0.1;
			glutPostRedisplay();
			break;
		case 4:/*on scroll wheel we want to get closer to cube*/
			scale += 0.1;
			glutPostRedisplay();
			break;	
	}
}

static void on_display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); /*clear previous color buffer and depth buffer*/

    glEnable(GL_LIGHTING); /*enable lighting*/
    glEnable(GL_LIGHT0);/*enable one light*/ 
	Image *image; /*Texture object read from a file*/
	glEnable(GL_TEXTURE_2D); /*Enabling 2d textures*/
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE); /*Setting texture environment parameters*/

	image = image_initialization(0, 0); /*Initializing texture object*/
	image_func(image, TEXTUREFILENAME); /*Creating first texture*/
	glGenTextures(1, textureID); /*Generating texture ID*/
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	glTexParameteri(GL_TEXTURE_2D, /*Setting texture wrap parameter for texture parameter s.*/
                    GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, /*Setting texture wrap parameter for texture parameter t*/
                    GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, /*Setting texture magnification function*/
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, /*Setting texture minifying function*/
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, /*Setting 2D texture image, second parameter is 0 for base image level, RGB color components*/
                 image->width, image->height, 0, /*Setting image width and height, without border*/
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels); /*RGB format of the pixel data, unsigned data type of the pixel data, image data in memory*/
	glBindTexture(GL_TEXTURE_2D, 0); /*Turning off current texture*/
	destroy_image(image);

	glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, widthW, 0 , heightW, -1, 1);

	glBindTexture(GL_TEXTURE_2D, textureID[0]); /*Drawing background*/
    glBegin(GL_QUADS);

        glTexCoord2f(0, 0);
        glVertex2i(0, 0);

        glTexCoord2f(1, 0);
        glVertex2i(widthW, 0);

        glTexCoord2f(1, 1);
        glVertex2i(widthW, heightW);

        glTexCoord2f(0, 1);
        glVertex2i(0, heightW);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0); /*Turning off current texture*/
    glEnable(GL_DEPTH_TEST); /*Re enabling depth testing*/
    glClear(GL_DEPTH_BUFFER_BIT); /*Clearing depth buffer*/
    glLoadIdentity();
	gluPerspective(60, (float) widthW/heightW, 1, 1000); /*Setting perspective again*/
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();


    gluLookAt(scale * sin(theta) * sin(phi), /*camera position, implemented using spherical coordinates*/ 
              scale * cos(phi), 
              scale * cos(theta) * sin(phi), 
              0, 0, 0,
              0, 1, 0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, (float[4]){ 0.2f, 0.2f, 0.2f, 1.0f }); /*setting light ambient parameters*/
    glLightfv(GL_LIGHT0, GL_DIFFUSE, (float[4]){ 1.0f, 1.0f, 1.0f, 1.0f }); /*setting light diffuse parameters*/
    glLightfv(GL_LIGHT0, GL_SPECULAR, (float[4]){ 1.0f, 1.0f, 1.0f, 1.0f }); /*setting light specular parameters*/

    glLightfv(GL_LIGHT0, GL_POSITION, (float[4]){scale * sin(theta) * sin(phi),  /*light position should be the same as camera position,
    																				as we move camera, light moves with us*/
              							scale * cos(phi), 
              							scale * cos(theta) * sin(phi), 1});
    

    glShadeModel(GL_SMOOTH); /*shading is smooth*/

	displayTimeElapsed(); /*displaying time on screen everytime*/
	
    if(!gameover && !victory) {  /*if it's not victory and it's not gameover, the game is still played*/
		glTranslatef(-CUBE_SIZE/2, -CUBE_SIZE/2, -CUBE_SIZE/2); /*we want our corner to be in (0, 0, 0) not center of our cube*/
		int i, j, k;
		for(i = 0; i < CUBE_SIZE; i++) {
			for(j = 0; j < CUBE_SIZE; j++) {
				for(k = 0; k < CUBE_SIZE; k++) {
					glPushMatrix();
						glPushMatrix();

						glDisable(GL_LIGHT0); /*everytime we disable light and lighting at first, and later we will enable it*/
						glDisable(GL_LIGHTING);

						/*here we will write numbers for mines around that field for every cube, but unopened ones will be on top of number*/
						glColor3f(0,0,0);  /*black color of text*/
						glTranslatef(i, j, k); /*translate text to the center of little cube that*/
						glScalef(0.003,0.003,0.003); /*scale text a bit*/
						glRotatef(theta * 180 / M_PI, 0, 1, 0); /*rotate text with the camera rotation by theta*/
						glRotatef(phi * 180 / M_PI - 50, 1, 0, 0); /*rotate text with the camera rotation by phi*/
						char str[8]; /*string for storing text that should be printed, its the number of bombs around that cube*/
						sprintf(str, "%d", cube[i][j][k].numberOfBombsAround);
						char *c = str;
						
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); /*making font pretty*/
				        glEnable(GL_BLEND); /*enabling blend and line to be smooth*/
				        glEnable(GL_LINE_SMOOTH);
				        glLineWidth(2.0);	/*setting line width of printed text*/

						if(*c != '0') {	
							for (c=str; *c; c++) {
							    glutStrokeCharacter(GLUT_STROKE_ROMAN, *c); /*printing character by character of the text string*/
							}
						}
						glEnable(GL_LIGHT0); /*enabling light and ligthing*/
						glEnable(GL_LIGHTING);

					glPopMatrix();

					/*if cube is opened and need to be animated*/
                    if(cube[i][j][k].isOpen == 1 && cube[i][j][k].bombFlag == 0 && cube[i][j][k].animate == currAnim) {
                        if(animationParameter < 0) /*we stop when animation parameter gets lower than 0, we increment isOpen so we dont get in this 'if' anymore*/
                        {
                            cube[i][j][k].isOpen++;
                        }

						glPushMatrix();
						
						glMaterialfv(GL_FRONT, GL_AMBIENT, (float[4]){ 0.24725f, 0.1995f, 0.0745f, 1.0f }); /*setting material ambient parameters*/
    					glMaterialfv(GL_FRONT, GL_DIFFUSE, (float[4]){0.75164f, 0.60648f, 0.22648f, 1.0f }); /*setting material diffuse parameters*/
    					glMaterialfv(GL_FRONT, GL_SPECULAR, (float[4]){0.628281f, 0.555802f, 0.366065f, 1.0f }); /*setting material specular paremeters*/
    					glMaterialf(GL_FRONT, GL_SHININESS, 51.2f ); /*setting material shininess*/
						glTranslatef(i, j, k); /*translating cube to its spot*/
                        glScalef(animationParameter, animationParameter, animationParameter); /*scaling it with animation parameter until we get to zero*/
						glutSolidCube(0.9); /*drawing cube*/
						if(timer1_ongoing == 0) { /*if timer is not going we must start it so we decrease animation parameter that will scale down our cube*/
                            timer1_ongoing = 1;
							glutTimerFunc(50, on_timer, 1);
                        }
						glPopMatrix();
						/*if cube isn't opened and doesnt need to be animated, and its not a bomb flag, we only draw that cube*/
					} else if(cube[i][j][k].isOpen == 0 && cube[i][j][k].bombFlag == 0 && cube[i][j][k].animate != currAnim) {
						glPushMatrix();
						glMaterialfv(GL_FRONT, GL_AMBIENT, (float[4]){ 0.24725f, 0.1995f, 0.0745f, 1.0f });/*setting material ambient parameters*/
    					glMaterialfv(GL_FRONT, GL_DIFFUSE, (float[4]){0.75164f, 0.60648f, 0.22648f, 1.0f });/*setting material diffuse parameters*/
    					glMaterialfv(GL_FRONT, GL_SPECULAR, (float[4]){0.628281f, 0.555802f, 0.366065f, 1.0f });/*setting material specular paremeters*/
    					glMaterialf(GL_FRONT, GL_SHININESS, 51.2f );/*setting material shininess*/
						glTranslatef(i, j, k); /*translating cube to its spot*/
						glutSolidCube(0.9); /*drawing cube*/
						glPopMatrix();

						/*here we draw the cubes that are marked as bombs*/
					} else if(cube[i][j][k].isOpen == 0) {
						glPushMatrix();
						glMaterialfv(GL_FRONT, GL_AMBIENT, (float[4]){ 0.105882f, 0.058824f, 0.113725f, 1.0f });/*setting material ambient parameters*/
    					glMaterialfv(GL_FRONT, GL_DIFFUSE, (float[4]){0.427451f, 0.470588f, 0.541176f, 1.0f });/*setting material diffuse parameters*/
    					glMaterialfv(GL_FRONT, GL_SPECULAR, (float[4]){0.333333f, 0.333333f, 0.521569f, 1.0f });/*setting material specular paremeters*/
    					glMaterialf(GL_FRONT, GL_SHININESS, 9.84615f);/*setting material shininess*/

						glTranslatef(i, j, k); /*translating cube to its spot*/
						glutSolidCube(0.9); /*drawing cube*/
						glPopMatrix();
					}
					glPopMatrix();
				}
			}
		}
	} else if(gameover && !victory){ /*this is 'if' when player lost */
		textFunc("YOU LOST!", widthW/2 - 30, heightW - 50); /*displaying this text */
		stopTimer = 1; /*stopping timer since the game is over*/
		glTranslatef(-CUBE_SIZE/2, -CUBE_SIZE/2, -CUBE_SIZE/2); /*drawing mines that are present in cube*/
		int i, j, k;
		for(i = 0; i < CUBE_SIZE; i++) {
			for(j = 0; j < CUBE_SIZE; j++) {
				for(k = 0; k < CUBE_SIZE; k++) {
					glPushMatrix(); 
					if(cube[i][j][k].isBomb == 1) { /*if cube is a bomb it should be drawn*/
						glTranslatef(i, j, k); /*translating cube to its spot*/
						glMaterialfv(GL_FRONT, GL_AMBIENT, (float[4]){1, 0, 0, 1}); /*setting ambient parameters for a bomb*/
    					glMaterialfv(GL_FRONT, GL_DIFFUSE, (float[4]){1, 0, 0, 1});/*setting diffuse parameters for a bomb*/
						drawMine(0.5); /*drawing a bomb*/
					}
					glPopMatrix();
				}
			}
		}
	} else if(victory) { /*this is 'if' when player won*/
		stopTimer = 1; /*stopping timer since the game is over*/
		char victoryString[51]; 
		sprintf(victoryString, "YOU WON!!! Your score is: %s\n", timeElapsed);/*printing text and player score that is time elapsed from beginning*/
		textFunc(victoryString, widthW/2 - 150, heightW - 50); /*printing text*/
		glutSolidTeapot(3); /*drawing teapot as a reward :)*/
	}

	glutSwapBuffers();
}

static void initializeCube(void) {/*cube initializations*/
	int i, j, k;
	for(i = 0; i < CUBE_SIZE; i++) {
		for(j = 0; j < CUBE_SIZE; j++) {
			for(k = 0; k < CUBE_SIZE; k++) {
				cube[i][j][k].isOpen = 0;/*initially all cubes are closed*/
				cube[i][j][k].isBomb = 0; /*initially all cubes are not bombs*/
				cube[i][j][k].numberOfBombsAround = 0;/*initially no bombs around any cube*/
				cube[i][j][k].bombFlag = 0;/*initially no cubes have different color*/
				cube[i][j][k].animate = 0;/*initially no cubes should be animated*/
			}
		}
	}



	srand(time(NULL)); /*setting seed for random generator so we don't get each set of bombs each time*/
	
	for(i = 0; i < NUMBER_OF_MINES; i++) {/*for every bomb, generate x, y, z randomly, and set cube with those coordinates to be a bomb*/
		int a, b, c;
		a = rand() % CUBE_SIZE;
		b = rand() % CUBE_SIZE;
		c = rand() % CUBE_SIZE;
		while(cube[a][b][c].isBomb == 1){ /*we want new set of coordinates each time, we don't want to mark cube as bomb twice*/
			a = rand() % CUBE_SIZE;
			b = rand() % CUBE_SIZE;
			c = rand() % CUBE_SIZE;
		}
		cube[a][b][c].isBomb = 1; /*set cube with these coordinates to be a bomb*/


		/*for every bomb neighbour we need to increment their parameter 'numberOfBombsAround'*/
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

static void minesweeper(int a, int b, int c){ /*minesweeper algorithm based on flood fill*/
	if(cube[a][b][c].isOpen == 1) { /*if cube is opened we return from recursion*/
		return ; 
	} else { /*if cube isn't opened*/
		if(cube[a][b][c].isBomb == 1) { /*if pressed field is a bomb, player lost, we set gameover flag to 1 and return from recursion*/
			gameover = 1;
			return ;
		} else {
			cube[a][b][c].animate = currAnim; /*we set currAnim of cubes that are currently pressed to mark that only them should be animated*/
			if(cube[a][b][c].numberOfBombsAround > 0) { /*if there are bombs around pressed cube, only that cube should be opened*/
				cube[a][b][c].isOpen = 1; /*we mark that cube to be opened and not be drawn in display function*/
				cellsToGo -= 1;  /*since we didn't press a bomb, number of cells to be opened should be decreased*/
				if(cellsToGo <= 0){
					victory = 1;  /*if there are no more cells, that are not bombs, to be opened a player has won*/
				}
				return ;
			} else { /*if there are no bombs around pressed cube, we must call algorithm recursively for every neighbour of that cell*/
				cube[a][b][c].isOpen = 1; /* mark cube as opened*/
				cellsToGo -= 1; /*decrease cells to go since we didn't press a bomb*/

				/*recursively call this algorithm for every neighbour of pressed cube*/
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
static void drawMine(float size) { /*this function draws object that looks like a mine
									It will be a sphere with cubes translated up, down, left, right, closer, further*/
    
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
	if(stopTimer != 1) { /*if timer is not stoped we must get time elapsed*/
		struct tm *info; /*this struct is necessary for strftime*/
		time_t pom = (glutGet(GLUT_ELAPSED_TIME) - timeAtReset)/1000; /*current time elapsed is time from first glutInit minus time at last reset*/
		info = localtime(&pom);	 /*setting struct tm with these parameters*/
		strftime(timeElapsed, 50, "%n %M : %S", info); /*printing time in timeElapsed variable*/
	    textFunc(timeElapsed, widthW/2 - 36, heightW - 50); /*printing timeElapsed variable with text function*/
	    if(timer2_ongoing == 0) { /*if timer is 0 we must call it, this timer must be used so we could se timer changing on the screen*/
	    	timer2_ongoing = 1;
	    	glutTimerFunc(50, on_timer2, 2); /*call timer func*/
	    } 
	}	     
}
void textFunc(const char* text, double x, double y){
    glPushMatrix(); /*function that displays text on viewport*/ 

    glDisable(GL_LIGHTING); /*disable lighting */
    glColor3f(0, 0, 0); /* set color of letters to be black*/

    glMatrixMode(GL_PROJECTION); 
    double matrix[16];
    glGetDoublev(GL_PROJECTION_MATRIX, matrix);/*get current projection matrix*/
    glLoadIdentity();
    glOrtho(0, widthW, 0, heightW, -5, 5);/*set ortho*/
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRasterPos2f(x,y); /*set raster position to be x and y that are parameters of function*/
    
    for(int i = 0; text[i]; i++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)text[i]); /*print string that is parameter of function*/
    }
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(matrix); /*load matrix*/
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING); /*enable lighting*/

    glPopMatrix();
}
Image *image_initialization(int width, int height) {

  Image *image;

  if(width < 0 || height < 0) { /*Image sizes can't be lesser than zero*/
  	fprintf(stderr, "Error: width and height of image are invalid.\n");
  	exit(EXIT_FAILURE);
  }

  image = (Image*)malloc(sizeof(Image)); /*Memory allocation for image structure*/
  if(image == NULL) {
  	fprintf(stderr, "Error: cannot allocate memory for image structure.\n");
  	exit(EXIT_FAILURE);
  }

  image->width = width; /*Initializing image width*/
  image->height = height; /*Initializing image height*/
  if (width == 0 || height == 0) 
    image->pixels = NULL; 
  else {
    image->pixels = (char *)malloc(3 * width * height * sizeof(char)); /*for every pixel R, G, B value*/
    if(image->pixels == NULL) {
    	fprintf(stderr, "Error: cannot allocate memory for pixels array in image structure.\n");
    	exit(EXIT_FAILURE);
    }
  }
  return image;
}

static void destroy_image(Image *image) {
	free(image->pixels); /*this will free array inside structure*/
	free(image); /*this will free structure*/
}
void image_func(Image *image, char *fileName) {

  FILE *file;
  BITMAPFILEHEADER bfh;
  BITMAPINFOHEADER bih;
  unsigned int i;
  unsigned char r, g, b, a;

  if((file = fopen(fileName, "rb")) == NULL) { /*Opening file in binary mode*/
  	fprintf(stderr, "Error: cannot open image file.\n");
  	exit(EXIT_FAILURE);
  }

  /*Reading data for first header*/
  fread(&bfh.type, 2, 1, file);
  fread(&bfh.size, 4, 1, file);
  fread(&bfh.reserved1, 2, 1, file);
  fread(&bfh.reserved2, 2, 1, file);
  fread(&bfh.offsetbits, 4, 1, file);

  /*Reading data for second header. Here we will only use width and height of image*/
  fread(&bih.size, 4, 1, file);
  fread(&bih.width, 4, 1, file);
  fread(&bih.height, 4, 1, file);
  fread(&bih.planes, 2, 1, file);
  fread(&bih.bitcount, 2, 1, file);
  fread(&bih.compression, 4, 1, file);
  fread(&bih.sizeimage, 4, 1, file);
  fread(&bih.xpelspermeter, 4, 1, file);
  fread(&bih.ypelspermeter, 4, 1, file);
  fread(&bih.colorsused, 4, 1, file);
  fread(&bih.colorsimportant, 4, 1, file);

  image->width = bih.width;
  image->height = bih.height;

  /*If we have R, G, B components we allocate 3 chars for each pixel, if we have R, G, B, A we allocate 4 chars for each pixel*/
  if (bih.bitcount == 24)
    image->pixels = (char *)malloc(3 * bih.width * bih.height * sizeof(char));
  else if (bih.bitcount == 32)
    image->pixels = (char *)malloc(4 * bih.width * bih.height * sizeof(char));
  else {
    fprintf(stderr, "Error: you can only read 24/32bit per pixel.\n");
    exit(EXIT_FAILURE);
  }
  if(image->pixels == NULL) {
  	fprintf(stderr, "Error: image pixels allocation failed.\n");
  	exit(EXIT_FAILURE);
  }

  /*Here we will read data for each pixel*/
  if (bih.bitcount == 24)

    for (i = 0; i < bih.width * bih.height; i++) {
      /*We read data in B, G, R order (reverse R, G, B)*/
      fread(&b, sizeof(char), 1, file);
      fread(&g, sizeof(char), 1, file);
      fread(&r, sizeof(char), 1, file);
      image->pixels[3 * i] = r;
      image->pixels[3 * i + 1] = g;
      image->pixels[3 * i + 2] = b;
    }
  else if (bih.bitcount == 32)
  	/*We read data in B, G, R, A order*/
    for (i = 0; i < bih.width * bih.height; i++) {
      fread(&b, sizeof(char), 1, file);
      fread(&g, sizeof(char), 1, file);
      fread(&r, sizeof(char), 1, file);
      fread(&a, sizeof(char), 1, file);

      image->pixels[4 * i] = r;
      image->pixels[4 * i + 1] = g;
      image->pixels[4 * i + 2] = b;
      image->pixels[4 * i + 3] = a;
    }

  fclose(file); /*Close file*/
}