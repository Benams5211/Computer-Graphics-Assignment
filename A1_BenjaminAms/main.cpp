#include <GL/freeglut.h>
#include <windows.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <list>
#include <deque>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <iostream>

//the sound engine we used in game engine concepts
//included for the BONUS!!!!!!!
//anywhere you see "SoundEngine" is where this is used
#include <IrrKlang/irrKlang.h>

using namespace std;
using namespace irrklang;

//various booleans for toggling stuff
bool showAxes = true, clearScreen = false;
bool wire = false, vertex = false, music = false;
bool blackAndWhite = false, outline = true, dance = false;
//added for part 2
bool mainOrCorner = true, birdEye = true, rearEye = true;

//zoom and rotation for the scene
float zoom = 1.0f, rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;

//variables needed for dancing
float rotateTwo = 0.0f, rotateOne = 0.0f, armsY = -0.45f, headY = 0.0f;
float fullRot = 0.0f, superSpin = 0.0f;

//for changing colors of robot parts
float headColorR = 0.133f, headColorG = 0.545f, headColorB = 0.133f;
float armsColorR = 0.133f, armsColorG = 0.545f, armsColorB = 0.133f;
float legsColorR = 0.4f, legsColorG = 0.4f, legsColorB = 0.4f;
float bodyColorR = 0.408f, bodyColorG = 0.341f, bodyColorB = 0.094f;

//code from lectures
float lx = 0.0f, lz = -1.0f, ly = 0.0f;
float x = 0.0f, z = 0.0f, y = 1.0f;
float testX = 0.0f, testZ = 0.0f;
float angle = 0.0f, pitch = 0.0f;

//new for assignment 3
int windowW = 0, windowH = 0, windowPosx = 50, windowPosy = 50;
bool fullScreenMode = false;

bool showCollider = false;
float jumpForce = 0.0f;
int moveTimer = 0;

int seconds = 30;

#define PI 3.141592
GLint rightMouseButton = GLUT_UP, leftMouseButton = GLUT_UP;
float cameraTheta, cameraPhi, cameraRadius=1.0f;

int mouseX = 0, mouseY = 0;

list<float> speeds = { 0.01f, 0.035f, 0.1f };
char speedDisplay[17];
int speed;

char timeDisplay[50];
auto it = speeds.begin();

bool gameDone = false;

//from irrklang library, for sound
ISoundEngine* SoundEngine = createIrrKlangDevice();

class Bullet {
public:
	Bullet(float bx=x, float bz=z): bx_(bx), bz_(bz), by_(y-0.55f+(jumpForce*1.5f) ) {}
	void DrawBullet();
	float getDist();
	float getBX() const { return bx_; }
	float getBZ() const { return bz_; }
	float getBY() const { return by_; }
	void destroy() { active = false; }
private:
	float bx_, bz_, by_;
	float bulletDistZ = 0.0f;
	float bulletDistX = 0.0f;
	float bulletSpeed = *it;
	bool active = true;
};

void Bullet::DrawBullet() {
	if (!active) return;
	bx_ += lx * bulletSpeed;
	bz_ += lz * bulletSpeed;
	by_ += ly * bulletSpeed;

	glPushMatrix();
	glTranslatef(bx_, by_, bz_);
	glColor3f(0, 0, 0);
	glutSolidSphere(0.08f, 10, 10);
	glPopMatrix();
}

float Bullet::getDist() {
	return sqrt(((bx_ - x) * (bx_ - x)) +
		((bz_ - z) * (bz_ - z)));
}

deque<Bullet*> bullets;

#define WINDOW_W 640
#define WINDOW_H 480

//to easily store robot placements
struct Vector2 {
	float x;
	float z;
};
Vector2 randPos[10];

bool killed[10];

//function to generate random positions
void setRandPos() {
	for (int i = 0; i < 10; ++i) {
		Vector2 tmp;
		tmp.x = rand() % 12 - 6;
		tmp.z = rand() % 12 - 6;
		for (auto e : randPos) {
			if (tmp.x == e.x && tmp.z == e.z) {//if position is already in use...
				setRandPos();//try again...
				return;//and leave
			}
		}
		randPos[i] = tmp;
	}
}

bool checkWin() {
	if (seconds == 0) {
		gameDone = true;
		return false;
	}
	else {
		for (int i = 0; i < 10; ++i) {
			if (killed[i] == false) return false;
		}
	}
	gameDone = true;
	return true;
}

void DrawAxes() {
	glScalef(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glBegin(GL_LINES);
	glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(10, 0, 0);
	glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 10, 0);
	glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 10);
	glEnd();
	glPopMatrix();
}

void recomputeOrientation()
{
	z = (cameraRadius * sinf(cameraTheta) * sinf(cameraPhi));
	x = (cameraRadius * cosf(cameraPhi) * sinf(cameraTheta));
	y = cameraRadius * cosf(cameraTheta);
	if (y <= 1) y = 1.001f;
	glutPostRedisplay();
}

void DrawCamera(float x, float y, float z, float width, float height, float depth) {
	glPushMatrix();
	glTranslatef(x, y+jumpForce, z);
	glScalef(width, height, depth);
	glRotatef(-angle * 57.5f, 0, 1, 0);//57.5 just because it works

	//main camera body
	glColor3f(1.0f, 1.0f, 1.0f);
	glutSolidCube(1.0f);

	//front sphere to denote front direction
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -0.2f);
	glutSolidSphere(0.4f, 20, 20);
	glPopMatrix();

	//outline for easy viewing
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireCube(1.01f);
	glTranslatef(0.0f, 0.0f, -0.2f);
	glutWireSphere(0.41f, 20, 20);

	glPopMatrix();
}

void DrawGun(float x, float y, float z, float width, float height, float depth) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glClear(GL_DEPTH_BUFFER_BIT);//so viewports dont overlap, from https://stackoverflow.com/questions/13710791/multiple-viewports-interfering
	gluPerspective(60.0, (double)(8 * windowW) / (windowH * 7), 0.1, 100.0);//from slides
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0, -1, -2);
	glScalef(width, height, depth);

	//the gun
	glColor3f(0.5f, 0.5f, 0.5f);
	glutSolidCube(1.0f);

	//outline for easy viewing
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireCube(1.01f);

	glColor3f(1, 0, 0); //red dot sight
	glTranslatef(0, 2.5f, -2);
	glutSolidSphere(0.25f, 10, 10);

	glPopMatrix();
}

void DrawBox(bool wire, bool vertexOnly, float width, float height, float depth, float x, float y, float z) {
	glPushMatrix();

	//move to the x y z parameters
	glTranslatef(x, y, z);

	//scale according to parameters
	glScalef(width, height, depth);

	if (vertexOnly) {
		//draw vertices on a cube of size 1.0, placement will
		//be taken care of because of the previous glScalef
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_POINTS);
		glVertex3f(-0.5f, -0.5f, -0.5f);
		glVertex3f(-0.5f, -0.5f, 0.5f);
		glVertex3f(-0.5f, 0.5f, -0.5f);
		glVertex3f(-0.5f, 0.5f, 0.5f);
		glVertex3f(0.5f, -0.5f, -0.5f);
		glVertex3f(0.5f, -0.5f, 0.5f);
		glVertex3f(0.5f, 0.5f, -0.5f);
		glVertex3f(0.5f, 0.5f, 0.5f);
		glEnd();
	}

	else if (!wire) {
		//just do size 1.0, glScalef resizes it
		glutSolidCube(1.0f);
		if (outline) {//draws black outline
			glColor3f(0.0f, 0.0f, 0.0f);
			glutWireCube(1.01f);
		}
	}
	else {
		glColor3f(1.0f, 1.0f, 1.0f);
		glutWireCube(1.0f);
	}

	glPopMatrix();
}


bool dontShow[10];
void init(void) {  
    setRandPos(); //generate random positions once at the start  
    for (int i = 0; i < 10; ++i) {  
        killed[i] = false;  
		dontShow[i] = false;
    }  
	SoundEngine->setSoundVolume(0.5f); // Set volume to 20% (adjust as needed)  
	SoundEngine->play2D("song.wav", true); // Added SND_LOOP to make the song loop 


}

int killTime = 0;

//handles drawing the 6 pieces of robot
void DrawRobot(int i) {
	if (killed[i] && killTime == 0) dontShow[i]=true;
	//int i was added for individual dancing
	if (!dontShow[i]) {
		glScalef(zoom, zoom, zoom);//handles zooming

		glTranslated(0, 1.5f, 0);//just moves it up so the robot is more centered
		
		//legs
		glPushMatrix();
		//if (dance) glTranslatef(headY, 0.0f, 0.0f);
		if (killed[i]) glColor3f(1, 0, 0);
		else glColor3f(legsColorR, legsColorG, legsColorB);
		DrawBox(wire, vertex, 0.18f, 0.65f, 0.3f, -0.1f, -0.975f, 0.0f);
		if (killed[i]) glColor3f(1, 0, 0);
		else glColor3f(legsColorR, legsColorG, legsColorB);
		DrawBox(wire, vertex, 0.18f, 0.65f, 0.3f, 0.1f, -0.975f, 0.0f);
		glPopMatrix();//end both legs

		//head
		glPushMatrix();

		//head uses same rotation variable as legs for dancing
		if (dance) {
			glTranslatef(0, headY * 1.3, 0);
			glRotatef(rotateTwo, 0, 1, 0);
		}

		if (killed[i]) glColor3f(1, 0, 0);
		else glColor3f(headColorR, headColorG, headColorB);
		//drawing the head, with dynamically changing headY for dance purposes
		DrawBox(wire, vertex, 0.3f, 0.3f, 0.3f, 0.0f, 0.0f, 0.0f);

		//arms

		if (dance) armsY = 0.10f;
		else armsY = -0.45f;
		if (killed[i]) glColor3f(1, 0, 0);
		else glColor3f(headColorR, headColorG, headColorB);
		DrawBox(wire, vertex, 0.15f, 0.6f, 0.3f, 0.275f, armsY, 0.0f);
		if (killed[i]) glColor3f(1, 0, 0);
		else glColor3f(headColorR, headColorG, headColorB);
		DrawBox(wire, vertex, 0.15f, 0.6f, 0.3f, -0.275f, armsY, 0.0f);

		//body
		if (killed[i]) glColor3f(1, 0, 0);
		else glColor3f(bodyColorR, bodyColorG, bodyColorB);
		DrawBox(wire, vertex, 0.40f, 0.50f, 0.3f, 0.0f, -0.40f, 0.0f);

		//making object invisible from:
		//https://stackoverflow.com/questions/25281611/how-to-set-an-invisible-occluder-in-opengl-2
		if (showCollider) {
			//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glColor3f(0.678f, 0.847f, 0.902f);//hitbox
			glTranslatef(0, -0.6f + (headY * 0.1f), 0);
			glutWireSphere(0.4f, 7, 7);
		}
		glPopMatrix();

	}
	
}


int score = 0; // update this score in real time
int robotsKilled = 0; // update this number in real time
//collision logic from example at:
//https://www.swiftless.com/tutorials/opengl/collision.html
float d;
void collision(void) {
	for (auto bullet : bullets) {
		for (int i = 0; i < 10; ++i) {
			d = sqrt(((bullet->getBX() - randPos[i].x * 1.5f) * (bullet->getBX() - randPos[i].x * 1.5f)) +
				((bullet->getBY() - (0.9f + (i/5)+(headY)) ) * (bullet->getBY() - (0.9f + (i/5)+(headY)))) +
				((bullet->getBZ() - randPos[i].z * 1.5f) * (bullet->getBZ() - randPos[i].z * 1.5f )));
			if (d <= 0.08f + 0.4f && !killed[i]) {
				killTime = 1;
				killed[i] = true;
				score += 10;
				robotsKilled++;
				bullet->destroy();
				if (i % 2 == 0) SoundEngine->play2D("kill1.wav", false);
				else SoundEngine->play2D("kill2.wav", false);
			}
		}
	}
}

//setting up for main viewport
void LoadMainView() {
	glViewport(0, (windowH / 8), windowW, 7 * (windowH / 8));

}

//setting up viewport for top right corner
void LoadCornerView() {
	glViewport(windowW - ((windowW / 4)), windowH - (windowH / 4), windowW / 4, windowH / 4);
}

// Function to draw text at a specific position
void drawString(float x, float y, void* font, const char* string) {
	glRasterPos2f(x, y); // Set the raster position for drawing text
	for (char* c = (char*)string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c); // Draw each character
	}
}

// Display text with the 2D viewport (window height * 1/8)
char displayText[100]; // for a text to be displayed
char winText[35];
char loseText[35];
char toggleText[250];
char controlsText[105];
bool showControls = false;

void DrawUI() {
	// First viewport of height h/8
	glViewport(0, 0, windowW, windowW / 8);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-2.0, 2.0, -2.0, 2.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(0.3f, 0.3f, 0.3f);
	glRectf(-5.0, 5.0, 5.0, -5.0);

	if (*it == 0.01f) speed = 1;
	else if (*it == 0.035f) speed = 2;
	else if (*it == 0.1f) speed = 3;

	snprintf(displayText, sizeof(displayText), "Score:%d      Demons Killed : % d / 10", score, robotsKilled);
	snprintf(speedDisplay, sizeof(speedDisplay), "Bullet Speed: %d", speed);
	snprintf(timeDisplay, sizeof(timeDisplay), "Seconds Left: %d", seconds);
	snprintf(winText, sizeof(winText), "Mission Complete!");
	snprintf(loseText, sizeof(loseText), "Mission Fail!");
	snprintf(toggleText, sizeof(toggleText), "toggles: w-wireframe | s-solid | c-collider | a-axes | b-bullet speed | m-motion | F1-fullscreen | F2-FPV/ESV");
	snprintf(controlsText, sizeof(controlsText), "controls: up-forwards | down-backwards | left-rotate left | right-rotate right | space-shoot | ESC-exit");
	glColor3f(1.0f, 0.0f, 0.0f); // text color
	if (!gameDone) {
		if (showControls) {
			drawString(-2.0, -1.0, GLUT_BITMAP_HELVETICA_10, toggleText);
			drawString(-2.0, -1.5, GLUT_BITMAP_HELVETICA_10, controlsText);
			drawString(-1.0, 1.25, GLUT_BITMAP_HELVETICA_18, displayText);
			drawString(-0.5, 0.45, GLUT_BITMAP_HELVETICA_18, speedDisplay);
			drawString(-0.5, -0.30, GLUT_BITMAP_HELVETICA_18, timeDisplay);
		}
		else {
			drawString(-1.0, 1.0, GLUT_BITMAP_HELVETICA_18, displayText);
			drawString(-0.5, 0.0, GLUT_BITMAP_HELVETICA_18, speedDisplay);
			drawString(-0.5, -1.0, GLUT_BITMAP_HELVETICA_18, timeDisplay);
		}
	}
	else {
		if(checkWin()) drawString(-0.5, -0.5, GLUT_BITMAP_HELVETICA_18, winText);
		else drawString(-0.5, -0.5, GLUT_BITMAP_HELVETICA_18, loseText);
	}
}

int tmp;

//for my simple game AI
//for BONUS!!!!!!!!!!!!!!!!!!!!!!!!!!
void movement(int i) {
	if (gameDone) return;
		//basically, just adding values for the robot posiitons based on
		//random number tmp and the robots' index
		if (moveTimer < 25 && (tmp + i == 0 || tmp - i == 0 || (tmp - 1) - i == 0 || (tmp + 1) - i == 0)) {
			{ randPos[i].x += 0.0008f; }
		}
		if (moveTimer < 25 && (tmp + i == 1 || tmp - i == 1 || (tmp - 1) - i == 1 || (tmp + 1) - i == 1)) {
			{ randPos[i].x -= 0.0008f; }
		}
		if (moveTimer < 25 && (tmp + i == 2 || tmp - i == 2 || (tmp - 1) - i == 2 || (tmp + 1) - i == 2)) {
			{ randPos[i].z += 0.0008f; }
		}
		if (moveTimer < 25 && (tmp + i == 3 || tmp - i == 3 || (tmp - 1) - i == 3 || (tmp + 1) - i == 3)) {
			{ randPos[i].z -= 0.0008f; }
		}

		if (moveTimer > 25 && (tmp + i == 0 || tmp - i == 0 || (tmp - 1) - i == 0 || (tmp + 1) - i == 0)) {
			{ randPos[i].x += 0.0008f; }
		}
		if (moveTimer > 25 && (tmp + i == 1 || tmp - i == 1 || (tmp - 1) - i == 1 || (tmp + 1) - i == 1)) {
			{ randPos[i].x -= 0.0008f; }
		}
		if (moveTimer > 25 && (tmp + i == 2 || tmp - i == 2 || (tmp - 1) - i == 2 || (tmp + 1) - i == 2)) {
			{ randPos[i].z += 0.0008f; }
		}
		if (moveTimer > 25 && (tmp + i == 3 || tmp - i == 3 || (tmp - 1) - i == 3 || (tmp + 1) - i == 3)) {
			{ randPos[i].z -= 0.0008f; }
		}
}

void DrawNormal() {
	//top part here is from slides
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glClear(GL_DEPTH_BUFFER_BIT);//so viewports dont overlap, from https://stackoverflow.com/questions/13710791/multiple-viewports-interfering
	gluPerspective(60.0, (double)(8*windowW) / (windowH * 7), 0.1, 100.0);//from slides
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0, -jumpForce * 1.5f, 0);
	gluLookAt(x, y, z, x+lx, y, z+lz, 0.0f, 1.0f, 0.0f);

	glColor3f(0.396f, 0.261f, 0.0f);
	glBegin(GL_QUADS);
	glVertex3f(-100.0f, 0.0f, -100.0f);
	glVertex3f(-100.0f, 0.0f, 100.0f);
	glVertex3f(100.0f, 0.0f, 100.0f);
	glVertex3f(100.0f, 0.0f, -100.0f);
	glEnd();

	for (int i = 0; i < 10; i++) {
		glPushMatrix();
		glTranslatef(randPos[i].x * 1.5f, (i / 5)-0.25f, randPos[i].z * 1.5f);//idea from slides
		glColor3f(0.2f, 0.2f, 0.2f);
		if (i / 5 == 0) movement(i);
		if (i / 5 == 1)DrawBox(false, false, 1.0f, 1.0f, 1.0f, 0.0f, -0.25f, 0.0f);
		DrawRobot(i);
		glPopMatrix();
	}


	for (auto bullet : bullets) {
		bullet->DrawBullet();
		if (bullet->getDist() > 15.0f || bullets.size() > 10) { bullets.pop_front(); }
	}

	collision();

	if (showAxes) {
		DrawAxes();
	}
}

//for camera 3
void DrawBirdEye() {
	//glViewport(windowW - ((windowW / 4)), windowH - (windowH / 4), windowW / 4, windowH / 4);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glClear(GL_DEPTH_BUFFER_BIT);
	gluPerspective(60.0, (double)(windowW / 5) / (windowH / 5), 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(10.0f, 5.0f, 10.0f, 0, 0, 0, 0, 1, 0); //a good view of the scene

	glColor3f(0.396f, 0.261f, 0.0f);
	glBegin(GL_QUADS);
	glVertex3f(-100.0f, 0.0f, -100.0f);
	glVertex3f(-100.0f, 0.0f, 100.0f);
	glVertex3f(100.0f, 0.0f, 100.0f);
	glVertex3f(100.0f, 0.0f, -100.0f);
	glEnd();

	glColor3f(255.0f, 255.0f, 255.0f);
	DrawCamera(x, y, z, 1.0f, 1.0f, 1.0f);

	for (int i = 0; i < 10; i++) {
		glPushMatrix();
		glTranslatef(randPos[i].x * 1.5f, (i / 5) - 0.25f, randPos[i].z * 1.5f);//idea from slides
		glColor3f(0.2f, 0.2f, 0.2f);
		if (i / 5 == 0) movement(i);
		if (i / 5 == 1)DrawBox(false, false, 1.0f, 1.0f, 1.0f, 0.0f, -0.25f, 0.0f);
		if (!killed[i]) {
			DrawRobot(i);
		}
		glPopMatrix();
	}

	for (auto bullet : bullets) {
		bullet->DrawBullet();
		if (bullet->getDist() > 15.0f || bullets.size() > 10) { bullets.pop_front(); }
	}
}

void MyDisplay(void) {
	if (!clearScreen) glClearColor(0.678f, 0.063f, 0.184f, 0.0f); //a nice sky blue
	else glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //or a black if cleared
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	if (mainOrCorner) {
		LoadMainView();//make main viewport
		DrawNormal();  //and draw FPV
		DrawGun(x, 0.5, z, 0.3f, 0.3f, 3.0f);
		DrawUI();


		LoadCornerView();//make corner viewport
		DrawBirdEye();   //and draw ESV

	}
	else {
		LoadCornerView();
		DrawNormal();
		DrawGun(x, 0.5, z, 0.3f, 0.3f, 3.0f);


		LoadMainView();
		DrawBirdEye();
		DrawUI();

	}

	checkWin();

	glutSwapBuffers();

	glFlush();
}
bool jump = false;

void KeyboardFunc(unsigned char key, int x, int y) {
	if (gameDone) return;
	switch (key) {
	case 'a': //axes
		showAxes = !showAxes;
		break;
	case 'w': //wire
		vertex = false;
		wire = true;
		break;
	case 'o'://outline
		outline = !outline;
		break;
	case 's': //solid
		vertex = false;
		wire = false;
		break;
	case 'p': //vertex
		vertex = true;
		break;
	case 'c': //clear screen
		showCollider = !showCollider;
		break;
	case 'm': //music
		dance = !dance;
		break;
	case 'z': //jump
		jump = true;
		break;
	case 'd': //show controls
		showControls= !showControls;
		break;
	case 'b': //bullet speeds
		if (*it == 0.1f) {
			it = speeds.begin();
		}
		else { ++it; }
		break;
	case 27:
		exit(0);
		break;
	case ' ': //shooting
		Bullet* bul = new Bullet;
		bullets.push_back(bul);
		SoundEngine->play2D("shoot.wav", false);
		break;
	}
	glutPostRedisplay();
}



//sourced from GEC projects
void MouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		leftMouseButton = state;
	}
	if (button == GLUT_RIGHT_BUTTON) {
		rightMouseButton = state;
	}
	mouseX = x;
	mouseY = y;
}

void mouseMotion(int x, int y) {
	if (!mainOrCorner) return;
	if (leftMouseButton == GLUT_DOWN) {
		cameraPhi += (mouseX - x) * 0.005;
		cameraTheta += (mouseY - y) * 0.005;
		recomputeOrientation();
	}
	if (rightMouseButton == GLUT_DOWN) {
		double totalChangeSq = (x - mouseX) + (y - mouseY);
		cameraRadius += totalChangeSq * 0.1;
		if (cameraRadius < 0.0) cameraRadius = 0.0;
		if (cameraRadius > 30.0) cameraRadius = 30.0;
		recomputeOrientation();
	}
	mouseX = x;
	mouseY = y;
}

//from the slides
void changeSize(GLsizei width, GLsizei height)
{
	windowW = width;
	windowH = height;

	glViewport(0, 0, GLsizei(width), GLsizei(height));

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0, (double)(8*windowW) / (windowH * 7), 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void processSpecialKeys(int key, int xx, int yy) {
	if (gameDone) return;
	//this part is from the slides
	float fraction = 0.1f;
	switch (key) {
	case GLUT_KEY_LEFT:
		angle -= 0.05f;
		lx = sin(angle);
		lz = -cos(angle);
		break;
	case GLUT_KEY_RIGHT:
		angle += 0.05f;
		lx = sin(angle);
		lz = -cos(angle);
		break;
	case GLUT_KEY_UP:
		x += (lx * fraction) *3;
		z += (lz * fraction) *3;
		break;
	case GLUT_KEY_DOWN:
		x -= (lx * fraction) * 3;
		z -= (lz * fraction) * 3;
		break;

		//key names from https://stackoverflow.com/questions/15435715/opengl-glut-buttons-and-keys
	case GLUT_KEY_F2:
		mainOrCorner = !mainOrCorner;
		break;
	case GLUT_KEY_F1:
		fullScreenMode = !fullScreenMode;
		if (fullScreenMode) {
			windowPosx = glutGet(GLUT_WINDOW_X);
			windowPosy = glutGet(GLUT_WINDOW_Y);
			windowW = glutGet(GLUT_WINDOW_WIDTH);
			windowH = glutGet(GLUT_WINDOW_HEIGHT);
			glutFullScreen();
		}
		else {
			glutReshapeWindow(WINDOW_W, WINDOW_H);
			glutPositionWindow(windowPosx, windowPosy);
		}
		break;
	}
}

//needed for timer/dancing
int danceFrame = 0;
int timeFrame = 0;
bool once = false;
bool bulletOnce = false;
bool jumpDone = false;
bool moveOnce = false;
bool moveTwice = true;
ISoundEngine* SoundEngine2 = createIrrKlangDevice();

void Timer(int v) {
	++danceFrame;
	++moveTimer;
	++timeFrame;
	++killTime;
	if (danceFrame >= 10) danceFrame = 0;//loop through 10 frames
	if (timeFrame >= 10) timeFrame = 0;//loop through 10 frames
	if (moveTimer >= 50) moveTimer = 0;
	if (killTime >= 10) killTime = 0;
	if (timeFrame == 9 && seconds != 0 && !gameDone) { seconds--; }

	rotateTwo = danceFrame * 36.0f;//each frame, change bodySpin and
	superSpin = danceFrame * 108.0f; //faster spin speed
	rotateOne = danceFrame * -36.0f;//legsSpin so it rotates

	if (jump && !jumpDone) {
		jumpForce += 0.3f;
	}
	else if (jump && jumpDone) {
		jumpForce -= 0.3f;
	}
	if (jumpForce >= 2.0f) {
		jumpDone = true;
	}
	if (jumpForce == 0.0f) {
		jump = false;
		jumpDone = false;
	}
	
	if (moveTimer < 25 && !moveOnce) {
		tmp = rand() % 4;
		moveTwice = false;
		moveOnce = true;
	}
	if (moveTimer > 25 && !moveTwice) {
		tmp = rand() % 4;
		moveOnce = false;
		moveTwice = true;
	}
	

	if (!once) danceFrame = 0;//to make sure dancing variables begin at the right place, 
	//regardless of current frame
	if (dance) {
		if (danceFrame < 5) headY += 0.075f; //make head rise/fall
		else headY -= 0.075f;
		once = true;
		if (danceFrame < 5) fullRot += 1.0f;
		else fullRot -= 1.0f;
	}
	else { headY = 0.0f; fullRot = -2.5f; once = false; }//to make sure these start at the right place

	if (danceFrame == 0 && rand()%150==0 && !gameDone) { //play demon sound at random
		SoundEngine2->setSoundVolume(5.0f);
		SoundEngine2->play2D("idle.wav", false);
	}

	glutPostRedisplay();
	glutTimerFunc(100, Timer, v);
}

void menuFunc(int i) {
	if (i == 1) { exit(0); }
	else {
		for (int i = 0; i < 10; ++i) {
			killed[i] = false;
			score = 0; robotsKilled = 0;
			seconds = 30;
			gameDone = false;
		}
	}
}

//setup from your example
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	srand(time(nullptr));
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE); // RGB mode
	glutInitWindowSize(WINDOW_W, WINDOW_H); // window size
	glutInitWindowPosition(windowPosx, windowPosy);
	glutCreateWindow("Assignment 3 - Ben Ams 811254818");

	cameraRadius = 10.0f;
	cameraTheta = 1.471f;
	cameraPhi = 1.197f;

	recomputeOrientation();

	init();

	cout << "=================================" << endl;
	cout << "Computer Graphics Assignment 3" << endl;
	cout << "Ben Ams" << endl;
	cout << "=================================" << endl << endl;
	cout << "Press [d] to show commands" << endl << endl;

	glClearColor(0.0, 0.0, 0.0, 1.0); // clear the window screen

	//from best solution example
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 1, 100);

	//display callbacks
	glutDisplayFunc(MyDisplay);
	glutIdleFunc(MyDisplay);

	//input callbacks
	glutKeyboardFunc(KeyboardFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(mouseMotion);
	glutSpecialFunc(processSpecialKeys);

	//window reshape callback from slides
	glutReshapeFunc(changeSize);
	glutTimerFunc(0, Timer, 0);

	auto MainMenu = glutCreateMenu(menuFunc);
	glutAddMenuEntry("RESUME", 0);
	glutAddMenuEntry("EXIT", 1);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);

	glutMainLoop();
	return 0;

}
