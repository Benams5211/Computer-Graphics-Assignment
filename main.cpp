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
#include <string>
#pragma comment(lib, "winmm.lib")
#include <iostream>
#include <functional>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//the sound engine we used in game engine concepts
//included for the BONUS!!!!!!!
//anywhere you see "SoundEngine" is where this is used
#include <IrrKlang/irrKlang.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

int texWidth, texHeight, texChannels;

using namespace std;
using namespace irrklang;

//various booleans for toggling stuff
bool showAxes = true, wire = false, vertex = false;
bool outline = true, dance = false;
//added for part 2
bool mainOrCorner = true, birdEye = true, rearEye = true;

//zoom and rotation for the scene
float zoom = 1.0f, rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;

//variables needed for dancing
float rotateTwo = 0.0f, rotateOne = 0.0f, armsY = -0.45f, headY = 0.0f;
float fullRot = 0.0f, superSpin = 0.0f;

//code from lectures
float lx = 0.0f, lz = -1.0f;
float x = 0.0f, z = 0.0f, y = 1.0f;
float ax = 0.0f, az = 0.0f, ay = 1.0f;
float testX = 0.0f, testZ = 0.0f;
float angle = 0.0f, pitch = 0.0f;

//new for assignment 3
float windowW = 0, windowH = 0, windowPosx = 50, windowPosy = 50;
bool fullScreenMode = false;

float jumpForce = 0.0f;//for my jump functionality that i added!

list<float> speeds = { 0.01f, 0.035f, 0.1f };
int speed;//for text display
auto it = speeds.begin();

//from irrklang library, for sound
ISoundEngine* SoundEngine = createIrrKlangDevice();

//for assignment 4
GLuint floorTexID, skinTexID, shirtTexID, legsTexID, boxTexID, rockTexID, redTexID;
unsigned char* skyIMG = nullptr;
int imageWidth, imageHeight, imageComponents;

bool showMenu = false, bGouraud = false;
bool lightType = false, model = true, sound = true, paused = false;

void loadImage(const char* filename) {
	skyIMG = stbi_load(filename, &imageWidth, &imageHeight, &imageComponents, 0);
	if (!skyIMG) {
		fprintf(stderr, "Error loading image: %s\n", stbi_failure_reason());
	}
}

//this function is adapted from:
//https://github.com/assimp/assimp/blob/master/samples/SimpleOpenGL/Sample_SimpleOpenGL.c
//many changes were made because i didnt need it all
void recursive_render(const aiScene* sc, const aiNode* nd)
{
	if (!model) return;
	aiMatrix4x4 m = nd->mTransformation;
	m.Transpose();
	glPushMatrix();
	glMultMatrixf((float*)&m);

	for (int i = 0; i < nd->mNumMeshes; i++) {
		const aiMesh* mesh = sc->mMeshes[nd->mMeshes[i]];
		
		for (int j = 0; j < mesh->mNumFaces; j++) {//for each face in the mesh
			const aiFace* face = &mesh->mFaces[j];

			if (!wire) {
				glBegin(GL_TRIANGLES);
				for (int i = 0; i < 3; i++) {

					int index = face->mIndices[i];

					if (mesh->mNormals != NULL)
						glNormal3fv(&mesh->mNormals[index].x);

					if (mesh->mTextureCoords == NULL) {
						glTexCoord2f(
							mesh->mTextureCoords[0][index].x,
							mesh->mTextureCoords[0][index].y
						);
					}
					else {
						glTexCoord2f(
							mesh->mVertices[index].x * 0.5f,
							mesh->mVertices[index].z * 0.5f
						);
					}
					glVertex3fv(&mesh->mVertices[index].x);

				}
				glEnd();
			}

			else {
				//stores the vertices of each triangle
				const aiVector3D* v0 = &mesh->mVertices[face->mIndices[0]];
				const aiVector3D* v1 = &mesh->mVertices[face->mIndices[1]];
				const aiVector3D* v2 = &mesh->mVertices[face->mIndices[2]];

				//makes a wireframe out of them
				glBegin(GL_LINES);
				glVertex3f(v0->x, v0->y, v0->z);
				glVertex3f(v1->x, v1->y, v1->z);
				glVertex3f(v1->x, v1->y, v1->z);
				glVertex3f(v2->x, v2->y, v2->z);
				glVertex3f(v2->x, v2->y, v2->z);
				glVertex3f(v0->x, v0->y, v0->z);
				glEnd();
			}

		}
	}

	for (int i = 0; i < nd->mNumChildren; i++) {
		recursive_render(sc, nd->mChildren[i]);
	}

	glPopMatrix();
}

//the following texture loading functions are all based 
//on examples from slides
void drawBackground() {
	if (skyIMG) {
		// Save current OpenGL state
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();
		// Switch to orthographic projection for 2D drawing
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH),
			0, glutGet(GLUT_WINDOW_HEIGHT));
		// Switch to modelview matrix and reset it
		glMatrixMode(GL_MODELVIEW);

		glLoadIdentity();
		// Disable depth testing and lighting for background
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		// Set pixel storage mode for correct alignment
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		// Position the raster for drawing at the bottom-left corner
		glRasterPos2i(0, 0);
		// Determine the format for glDrawPixels based on components
		GLenum format = GL_RGB;
		if (imageComponents == 4) {
			format = GL_RGBA;
		}
		else if (imageComponents == 1) {
			format = GL_LUMINANCE;
		}
		// Draw the pixels
		glDrawPixels(imageWidth, imageHeight, format,
			GL_UNSIGNED_BYTE, skyIMG);
		// Restore previous OpenGL state
		glPopMatrix(); // Pop modelview matrix
		glMatrixMode(GL_PROJECTION);
		glPopMatrix(); // Pop projection matrix
		glPopAttrib();
	}
}

void loadFloorTexture(const char* filename) {
	unsigned char* data = stbi_load(filename, &texWidth, &texHeight, &texChannels,
		0);
	GLenum format = (texChannels == 1) ? GL_RED :
		(texChannels == 3) ? GL_RGB :
		(texChannels == 4) ? GL_RGBA : GL_RGB;
	if (data) {
		glGenTextures(1, &floorTexID);
		glBindTexture(GL_TEXTURE_2D, floorTexID);
		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format,
			GL_UNSIGNED_BYTE, data);
		stbi_image_free(data); // Free image data after uploading to GPU
	}
	else {
		std::cerr << "Failed to load texture: " << filename << std::endl;
	}
}

void loadSkinTexture(const char* filename) {
	unsigned char* data = stbi_load(filename, &texWidth, &texHeight, &texChannels,
		0);
	GLenum format = (texChannels == 1) ? GL_RED :
		(texChannels == 3) ? GL_RGB :
		(texChannels == 4) ? GL_RGBA : GL_RGB;
	if (data) {
		glGenTextures(1, &skinTexID);
		glBindTexture(GL_TEXTURE_2D, skinTexID);
		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format,
			GL_UNSIGNED_BYTE, data);
		stbi_image_free(data); // Free image data after uploading to GPU
	}
	else {
		std::cerr << "Failed to load texture: " << filename << std::endl;
	}
}

void loadShirtTexture(const char* filename) {
	unsigned char* data = stbi_load(filename, &texWidth, &texHeight, &texChannels,
		0);
	GLenum format = (texChannels == 1) ? GL_RED :
		(texChannels == 3) ? GL_RGB :
		(texChannels == 4) ? GL_RGBA : GL_RGB;
	if (data) {
		glGenTextures(1, &shirtTexID);
		glBindTexture(GL_TEXTURE_2D, shirtTexID);
		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format,
			GL_UNSIGNED_BYTE, data);
		stbi_image_free(data); // Free image data after uploading to GPU
	}
	else {
		std::cerr << "Failed to load texture: " << filename << std::endl;
	}
}

void loadLegsTexture(const char* filename) {
	unsigned char* data = stbi_load(filename, &texWidth, &texHeight, &texChannels,
		0);
	GLenum format = (texChannels == 1) ? GL_RED :
		(texChannels == 3) ? GL_RGB :
		(texChannels == 4) ? GL_RGBA : GL_RGB;
	if (data) {
		glGenTextures(1, &legsTexID);
		glBindTexture(GL_TEXTURE_2D, legsTexID);
		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format,
			GL_UNSIGNED_BYTE, data);
		stbi_image_free(data); // Free image data after uploading to GPU
	}
	else {
		std::cerr << "Failed to load texture: " << filename << std::endl;
	}
}

void loadBoxTexture(const char* filename) {
	unsigned char* data = stbi_load(filename, &texWidth, &texHeight, &texChannels,
		0);
	GLenum format = (texChannels == 1) ? GL_RED :
		(texChannels == 3) ? GL_RGB :
		(texChannels == 4) ? GL_RGBA : GL_RGB;
	if (data) {
		glGenTextures(1, &boxTexID);
		glBindTexture(GL_TEXTURE_2D, boxTexID);
		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format,
			GL_UNSIGNED_BYTE, data);
		stbi_image_free(data); // Free image data after uploading to GPU
	}
	else {
		std::cerr << "Failed to load texture: " << filename << std::endl;
	}
}

void loadRedTexture(const char* filename) {
	unsigned char* data = stbi_load(filename, &texWidth, &texHeight, &texChannels,
		0);
	GLenum format = (texChannels == 1) ? GL_RED :
		(texChannels == 3) ? GL_RGB :
		(texChannels == 4) ? GL_RGBA : GL_RGB;
	if (data) {
		glGenTextures(1, &redTexID);
		glBindTexture(GL_TEXTURE_2D, redTexID);
		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format,
			GL_UNSIGNED_BYTE, data);
		stbi_image_free(data); // Free image data after uploading to GPU
	}
	else {
		std::cerr << "Failed to load texture: " << filename << std::endl;
	}
}

void loadRockTexture(const char* filename) {
	unsigned char* data = stbi_load(filename, &texWidth, &texHeight, &texChannels,
		0);
	GLenum format = (texChannels == 1) ? GL_RED :
		(texChannels == 3) ? GL_RGB :
		(texChannels == 4) ? GL_RGBA : GL_RGB;
	if (data) {
		glGenTextures(1, &rockTexID);
		glBindTexture(GL_TEXTURE_2D, rockTexID);
		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format,
			GL_UNSIGNED_BYTE, data);
		stbi_image_free(data); // Free image data after uploading to GPU
	}
	else {
		std::cerr << "Failed to load texture: " << filename << std::endl;
	}
}

class Bullet {
public:
	Bullet(float bx = x, float bz = z) : bx_(bx), bz_(bz), by_(y - 0.55f + (jumpForce * 1.5f)) {}
	void DrawBullet();
	float getDist() {
		return sqrt(((bx_ - x) * (bx_ - x)) +
			((bz_ - z) * (bz_ - z)));
	}
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
	bx_ += lx * bulletSpeed*2;
	bz_ += lz * bulletSpeed*2;

	glPushMatrix();
	glTranslatef(bx_, by_, bz_);
	if (!wire) {
		glColor3f(0.5, 0, 0);
		glutSolidSphere(0.08f, 10, 10);
	}
	else {
		glColor3f(1, 1, 1);
		glutWireSphere(0.08f, 10, 10);
	}
	glPopMatrix();
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
Vector2 storePos[10];

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
		storePos[i] = tmp;
	}
}

bool gameDone = false;
int seconds = 30;
bool killed[10];

bool checkWin() {//game end logic
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
	glDisable(GL_LIGHTING);
	glScalef(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glBegin(GL_LINES);
	glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(10, 0, 0);
	glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 10, 0);
	glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 10);
	glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

//all this stuff from the slide examples, but i did change
//around some of the phi/theta stuff so it lines up with the
//spherical to cartesian coordinate formulas
float cameraTheta, cameraPhi, cameraRadius = 1.0f;
void recomputeOrientation()
{
	az = (cameraRadius * sinf(cameraTheta) * sinf(cameraPhi));
	ax = (cameraRadius * cosf(cameraPhi) * sinf(cameraTheta));
	ay = cameraRadius * cosf(cameraTheta);//changed this from phi to theta
	glutPostRedisplay();
}

void DrawCamera(float x, float y, float z, float width, float height, float depth) {
	glPushMatrix();
	glTranslatef(x, y + jumpForce, z);
	glScalef(width, height, depth);
	glRotatef(-angle * 57.5f, 0, 1, 0);//57.5 just because it works

	//main camera body
	glColor3f(1.0f, 1.0f, 1.0f);
	if (!wire)glutSolidCube(1.0f);
	else glutWireCube(1.0f);

	//drawing the gun
	glPushMatrix();
	glTranslatef(0.0f, -0.2f, -0.5f);
	glScalef(1, 1, 4);
	glColor3f(0.5f, 0.5f, 0.5f);
	if(!wire)glutSolidCube(0.2);
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireCube(0.2);
	glPopMatrix();

	//outline for easy viewing
	glutWireCube(1.01f);

	glPopMatrix();
}

void DrawBox(bool wire, float width, float height, float depth, float x, float y, float z) {
	glPushMatrix();

	//move to the x y z parameters
	glTranslatef(x, y, z);

	//scale according to parameters
	glScalef(width, height, depth);

	if (!wire) {
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

//anything about buttons is from GEC class
class Button {
public:
	string text;
	float x_=0, y_=0, h_=0, w_=0;
	function<void(Button&)> buttonAction;
	Button(function<void(Button&)> act, const string& t):buttonAction(act), text(t) {}

	void set(float x1, float y1, float h1, float w1) {
		x_ = x1; y_ = y1; h_ = h1; w_ = w1;
	}

	void print() {
		cout << text << endl;
	}
	 
	void draw() {
		float centerX = x_ + w_ / 2;
		float centerY = y_ + h_ / 2;

		glColor3f(0.9f, 0.9f, 0.9f);
		glBegin(GL_QUADS);
		glVertex3f(x_,y_, 0);
		glVertex3f(x_+w_, y_, 0);
		glVertex3f(x_+w_, y_+h_, 0);
		glVertex3f(x_, y_+h_, 0);
		glEnd();

		float textX = centerX;
		float textY = centerY;

		glColor3f(0, 0, 0);
		glRasterPos3f(textX, textY, 1);
		for (int i = 0; i < text.length(); i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
		}
	}

	void handleClick() {buttonAction(*this);}

	bool mouseInside(int mouseX, int mouseY) {
		float leftBoundary = x_;
		float rightBoundary = x_ + w_;
		float bottomBoundary = y_ ;
		float topBoundary = y_ + h_;

		bool insideX = (mouseX >= leftBoundary && mouseX <= rightBoundary);
		bool insideY = (mouseY >= bottomBoundary && mouseY <= topBoundary);

		return insideX && insideY;
	}

};
list<Button*> buttons;

bool dontShow[10];
int score = 0; // update this score in real time
int robotsKilled = 0; // update this number in real time

void newGameAction(Button&) {
	for (int i = 0; i < 10; ++i) {
		killed[i] = false;
		dontShow[i] = false;
		score = 0; robotsKilled = 0;
		seconds = 30;
		gameDone = false;
		randPos[i] = storePos[i];
	}
	//resetting positions and player rotation
	showMenu = false;
	paused = false;
	ax = 3.63335, az = 9.26316;
	lx = 0, lz = -1;
}

void resumeAction(Button&) {
	showMenu = false;
	paused = false;
}

void exitAction(Button&) {
	exit(0);
}


Button* b1;
Button* b2;
Button* b3;

void init(void) {
	setRandPos(); //generate random positions once at the start  
	for (int i = 0; i < 10; ++i) {  //set death arrays to false
		killed[i] = false;
		dontShow[i] = false;
	}

	b1 = new Button(newGameAction, "NEW GAME");
	b2 = new Button(resumeAction, "RESUME");
	b3 = new Button(exitAction, "EXIT");

	buttons.push_back(b1);
	buttons.push_back(b2);
	buttons.push_back(b3);


	glEnable(GL_NORMALIZE);
	//from slides
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
}

//timer for removing robot after death
int killTime = 0;

//for changing colors of robot parts
float headColorR = 0.133f, headColorG = 0.545f, headColorB = 0.133f;
float armsColorR = 0.133f, armsColorG = 0.545f, armsColorB = 0.133f;
float legsColorR = 0.4f, legsColorG = 0.4f, legsColorB = 0.4f;
float bodyColorR = 0.408f, bodyColorG = 0.341f, bodyColorB = 0.094f;

bool showCollider = false;
GLuint tmpTex;

//from slides
void texturedBox(int i) {
	if (i == 0) tmpTex = skinTexID;
	else if (i == 1) tmpTex = shirtTexID;
	else if (i == 2) tmpTex = legsTexID;
	else if (i == 3) tmpTex = boxTexID;
	else if (i == 4) tmpTex = redTexID;

	if (!wire) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tmpTex);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
		// Back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
		// Top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
		// Bottom face
		glNormal3f(0.0f, -1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
		// Right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
		// Left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
		glEnd();
		glDisable(GL_TEXTURE_2D); // Disable texture mapping
	}
	else {
		glColor3f(1, 1, 1);
		glutWireCube(2.0f);
	}
}

//handles drawing the 6 pieces of robot
void DrawRobot(int i) {
	if (killed[i] && killTime == 0) dontShow[i] = true;

	//only draw if it is not dead and the red visual is gone
	if (!dontShow[i]) {
		glTranslated(0, 1.5f, 0);//just moves it up so the robot is more centered

		//legs
		glPushMatrix();
		glPushMatrix();
		glTranslatef(-0.1f, -0.975f, 0.0f);
		glScalef(0.09f, 0.325f, 0.15f);
		if (killed[i]) texturedBox(4);
		else texturedBox(2);
		
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.1f, -0.975f, 0.0f);
		glScalef(0.09f, 0.325f, 0.15f);
		if (killed[i]) texturedBox(4);
		else texturedBox(2);
		glPopMatrix();

		glPopMatrix();//end both legs

		//head
		glPushMatrix();

		//this applies to head, arms, body, not legs
		if (dance) {
			glTranslatef(0, headY * 1.3, 0);
			glRotatef(rotateTwo, 0, 1, 0);
		}

		glPushMatrix();
		glScalef(0.15f, 0.15f, 0.15f);
		if (killed[i]) texturedBox(4);
		else texturedBox(0);
		glPopMatrix();

		//arms
		if (dance) armsY = 0.10f; //put arms up for motion
		else armsY = -0.45f;

		glPushMatrix();
		glTranslatef(0.274f, armsY, 0.0f);
		glScalef(0.075f, 0.3f, 0.15f);
		if (killed[i]) texturedBox(4);
		else texturedBox(0);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-0.274f, armsY, 0.0f);
		glScalef(0.075f, 0.3f, 0.15f);
		if (killed[i]) texturedBox(4);
		else texturedBox(0);
		glPopMatrix();

		//body
		glPushMatrix();
		glTranslatef(0.0f, -0.40f, 0.0f);
		glScalef(0.20f, 0.25f, 0.15f);
		if (killed[i]) texturedBox(4);
		else texturedBox(1);
		glPopMatrix();

		if (showCollider) {
			glDisable(GL_LIGHTING);
			glColor3f(0.678f, 0.847f, 0.902f);//hitbox
			glTranslatef(0, -0.6f + (headY * 0.1f), 0);
			glutWireSphere(0.4f, 7, 7);
			glEnable(GL_LIGHTING);
		}
		glPopMatrix();

	}

}

//collision logic from example at:
//https://www.swiftless.com/tutorials/opengl/collision.html
float d;
void collision(void) {
	for (auto bullet : bullets) {
		for (int i = 0; i < 10; ++i) {
			d = sqrt(((bullet->getBX() - randPos[i].x * 1.5f) * (bullet->getBX() - randPos[i].x * 1.5f)) +
				((bullet->getBY() - (0.9f + (i / 5) + (headY))) * (bullet->getBY() - (0.9f + (i / 5) + (headY)))) +
				((bullet->getBZ() - randPos[i].z * 1.5f) * (bullet->getBZ() - randPos[i].z * 1.5f)));
			if (d <= 0.08f + 0.6f && !killed[i]) {//do this if collision occurs AND the demon isnt dead
				killTime = 1; //set kill timer for red visual
				killed[i] = true; //set the robot to dead
				score += 10; //increase score
				robotsKilled++; //increase robots killed
				bullet->destroy(); //destroy the bullet

				//play one of two demon sounds
				if (sound) {
					if (i % 2 == 0) SoundEngine->play2D("kill1.wav", false);
					else SoundEngine->play2D("kill2.wav", false);
				}
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

int tmp; //for randomness of AI movement
int moveTimer = 0; //goes to 50, robots change movement at 25

//for my simple game AI
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

//From the slides!!!
// Function to draw text at a specific position
void drawString(float x, float y, void* font, const char* string) {
	glRasterPos2f(x, y); // Set the raster position for drawing text
	for (char* c = (char*)string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c); // Draw each character
	}
}

// text to be displayed
char displayText[100];
char timeDisplay[50];
char speedDisplay[17];
char scoringText[60];
char winText[35];
char loseText[35];
char toggleText[250];
char controlsText[105];
bool showControls = true;

//draws a static gun and aim overtop the FPV
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

	glDisable(GL_LIGHTING);
	//the gun
	glColor3f(0.5f, 0.5f, 0.5f);

	if (!wire) {
		//just do size 1.0, glScalef resizes it
		glutSolidCube(1.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		glutWireCube(1.01f);
		glColor3f(1, 0, 0); //red dot sight
		glTranslatef(0, 2.5f, -2);
		glutSolidSphere(0.25f, 10, 10);
	}
	else {
		glColor3f(1.0f, 1.0f, 1.0f);
		glutWireCube(1.0f);
		glTranslatef(0, 2.5f, -2);
		glutWireSphere(0.25f, 10, 10);
	}

	glPopMatrix();
	glEnable(GL_LIGHTING);
}

//for viewport 1
//from slides
void DrawUI() {
	glDisable(GL_LIGHTING);
	glViewport(0, 0, windowW, windowW / 8); //always want this in the same place
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-2.0, 2.0, -2.0, 2.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(0.3f, 0.3f, 0.3f);
	glRectf(-5.0, 5.0, 5.0, -5.0);

	//setting speed display 
	if (*it == 0.01f) speed = 1;
	else if (*it == 0.035f) speed = 2;
	else if (*it == 0.1f) speed = 3;


	//making all the strings
	snprintf(displayText, sizeof(displayText), "Score:%d      Demons Killed : % d / 10", score, robotsKilled);
	snprintf(speedDisplay, sizeof(speedDisplay), "Bullet Speed: %d", speed);
	snprintf(timeDisplay, sizeof(timeDisplay), "Seconds Left: %d", seconds);
	snprintf(winText, sizeof(winText), "Mission Complete!");
	snprintf(loseText, sizeof(loseText), "Mission Fail!");
	snprintf(toggleText, sizeof(toggleText), "w-wireframe | s-solid | c-collider | a-axes | b-bullet speed | m-motion | o-model | F1-fullscreen | F2-FPV/ESV | F3-sound | F4-shading");
	snprintf(controlsText, sizeof(controlsText), "up-forwards | down-backwards | left-rotate left | right-rotate right | space-shoot | ESC-pause | l-lighting");
	snprintf(scoringText, sizeof(scoringText), "+10 points for hitting enemy | 30 seconds to kill them all!");

	glColor3f(1.0f, 0.0f, 0.0f); //red text color

	if (!gameDone) { //if game is in progress
		if (showControls) {//shows keyboard controls and gameplay stats
			drawString(-2.0, -1.0, GLUT_BITMAP_HELVETICA_10, toggleText);
			drawString(-2.0, -1.5, GLUT_BITMAP_HELVETICA_10, controlsText);
			drawString(-2.0, -2.0, GLUT_BITMAP_HELVETICA_10, scoringText);
			drawString(-1.0, 1.25, GLUT_BITMAP_HELVETICA_18, displayText);
			drawString(-0.5, 0.45, GLUT_BITMAP_HELVETICA_18, speedDisplay);
			drawString(-0.5, -0.30, GLUT_BITMAP_HELVETICA_18, timeDisplay);
		}
		else {//shows just gameplay stats
			drawString(-1.0, 1.0, GLUT_BITMAP_HELVETICA_18, displayText);
			drawString(-0.5, 0.0, GLUT_BITMAP_HELVETICA_18, speedDisplay);
			drawString(-0.5, -1.0, GLUT_BITMAP_HELVETICA_18, timeDisplay);
		}
	}
	else { //did we win or lose?
		if (checkWin()) drawString(-0.5, -0.5, GLUT_BITMAP_HELVETICA_18, winText);
		else drawString(-0.5, -0.5, GLUT_BITMAP_HELVETICA_18, loseText);
	}
	glEnable(GL_LIGHTING);
}

//for viewport 2
void DrawNormal() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glClear(GL_DEPTH_BUFFER_BIT);//so viewports dont overlap, from https://stackoverflow.com/questions/13710791/multiple-viewports-interfering
	gluPerspective(60.0, (double)(8 * windowW) / (windowH * 7), 0.1, 100.0);//from slides
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//draw camera and make it look like a jump too
	glTranslatef(0, -jumpForce * 1.5f, 0);
	gluLookAt(x, y, z, x + lx, 1.0f, z + lz, 0.0f, 1.0f, 0.0f);

	if (!lightType) {
		GLfloat pos[] = { 0.0f, 5.0f, 0.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, pos);

		GLfloat dir[] = { 0.0f, -1.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
	}
	else {
		GLfloat pos[] = { 1.0f, 1.0f, 1.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, pos);
	}

	//from example
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("Rock1.3ds", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenUVCoords);

	//just drawing the thing
	glPushMatrix();
	glTranslatef(0, -0.1f, -5);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, rockTexID);
	recursive_render(scene, scene->mRootNode);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	if (!wire) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorTexID);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glPushMatrix();
		glTranslatef(-25, 0, -25);
		//to make a tiled texture, i used two for loops
		for (float i = 1; i < 51; ++i) {
			glPushMatrix();
			glTranslatef(i, 0, 0);
			for (float j = 1; j < 51; ++j) {
				glPushMatrix();
				glTranslatef(0, 0, j);
				glBegin(GL_QUADS);
				glNormal3f(0.0f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5, 0.0f, -0.5);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, 0.0f, 0.5);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5, 0.0f, 0.5);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.0f, -0.5);
				glEnd();
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
	}
	else {
		glColor3f(1, 1, 1);
		glBegin(GL_LINES);
		glVertex3f(-25, 0, -25); glVertex3f(-25, 0, 25);
		glVertex3f(-25, 0, 25); glVertex3f(25, 0, 25);
		glVertex3f(25, 0, 25); glVertex3f(25, 0, -25);
		glVertex3f(25, 0, -25); glVertex3f(-25, 0, -25);
		glEnd();
	}

	for (int i = 0; i < 10; i++) {
		glPushMatrix();
		glTranslatef(randPos[i].x * 1.5f, (i / 5) - 0.25f, randPos[i].z * 1.5f);//idea from slides
		if (i / 5 == 0) movement(i);//make robots on the floor move
		if (i / 5 == 1) {
			glPushMatrix();
			glTranslatef(0.0f, -0.25f, 0.0f);
			glScalef(0.5f, 0.5f, 0.5f);
			texturedBox(3);
			glPopMatrix();
		}
		DrawRobot(i);
		glPopMatrix();
	}

	for (auto bullet : bullets) {
		bullet->DrawBullet();
		//if the bullet gets too far or there are 10, start removing them
		if (bullet->getDist() > 15.0f || bullets.size() > 10) { bullets.pop_front(); }
	}

	collision();//collision check loop

	if (showAxes) { DrawAxes(); }
}

//for viewport 3
void DrawBirdEye() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glClear(GL_DEPTH_BUFFER_BIT);
	gluPerspective(60.0, (double)(windowW / 5) / (windowH / 5), 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(ax, ay, az, 0, 0, 0, 0, 1, 0); //a good view of the scene

	if (!lightType) {
		GLfloat pos[] = { 0.0f, 5.0f, 0.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, pos);

		GLfloat dir[] = { 0.0f, -1.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
	}
	else {
		GLfloat pos[] = { 1.0f, 1.0f, 1.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, pos);
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("Rock1.3ds", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenUVCoords);

	glPushMatrix();
	glTranslatef(0, -0.1f, -5);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, rockTexID);
	recursive_render(scene, scene->mRootNode);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	if (!wire) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorTexID);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glPushMatrix();
		glTranslatef(-25, 0, -25);
		//to make a tiled texture, i used two for loops
		for (float i = 1; i < 51; ++i) {
			glPushMatrix();
			glTranslatef(i, 0, 0);
			for (float j = 1; j < 51; ++j) {
				glPushMatrix();
				glTranslatef(0, 0, j);
				glBegin(GL_QUADS);
				glNormal3f(0.0f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5, 0.0f, -0.5);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5, 0.0f, 0.5);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5, 0.0f, 0.5);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.0f, -0.5);
				glEnd();
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();

		glDisable(GL_TEXTURE_2D);
	}
	else {
		glColor3f(1, 1, 1);
		glBegin(GL_LINES);
		glVertex3f(-25, 0, -25); glVertex3f(-25, 0, 25);
		glVertex3f(-25, 0, 25); glVertex3f(25, 0, 25);
		glVertex3f(25, 0, 25); glVertex3f(25, 0, -25);
		glVertex3f(25, 0, -25); glVertex3f(-25, 0, -25);
		glEnd();
	}

	for (int i = 0; i < 10; i++) {
		glPushMatrix();
		glTranslatef(randPos[i].x * 1.5f, (i / 5) - 0.25f, randPos[i].z * 1.5f);//idea from slides
		if (i / 5 == 0) movement(i);//make robots on the floor move
		if (i / 5 == 1) {
			glPushMatrix();
			glTranslatef(0.0f, -0.25f, 0.0f);
			glScalef(0.5f, 0.5f, 0.5f);
			texturedBox(3);
			glPopMatrix();
		}
		DrawRobot(i);
		glPopMatrix();
	}

	glColor3f(255.0f, 255.0f, 255.0f);
	DrawCamera(x, y - (y * 0.5), z, 1.0f, 1.0f, 1.0f);

	for (auto bullet : bullets) {
		bullet->DrawBullet();
		if (bullet->getDist() > 15.0f || bullets.size() > 10) { bullets.pop_front(); }
	}
}

//for my little pause menu
void DrawMenu() {
	glViewport(0, 0, windowW, windowH);

	glDisable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, windowW, 0, windowH, -10, 10);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	b1->draw();
	b2->draw();
	b3->draw();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);
}


bool played = false;

void MyDisplay(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	b1->set(windowW / 4, windowH / 3 + windowH / 3, windowH / 4, windowW / 2);
	b2->set(windowW / 4, windowH / 3              , windowH / 4, windowW / 2);
	b3->set(windowW / 4, windowH / 3 - windowH / 3, windowH / 4, windowW / 2);

	if (sound && !played) {
		SoundEngine->setSoundVolume(0.5f);
		SoundEngine->play2D("song.wav", true);
		played = true;
	}

	if (!showMenu) {
		paused = false;
		if (mainOrCorner) {
			LoadMainView();//make main viewport
			drawBackground();
			DrawNormal();  //and draw FPV
			DrawGun(x, 0.5, z, 0.3f, 0.3f, 3.0f);//put gun in main view
			DrawUI();//put UI in main view

			LoadCornerView();//make corner viewport
			DrawBirdEye();   //and draw ESV

		}
		else {
			LoadMainView();
			drawBackground();
			DrawBirdEye();
			DrawUI();//put UI in main view still

			LoadCornerView();
			DrawNormal();
			DrawGun(x, 0.5, z, 0.3f, 0.3f, 3.0f);//put gun in corner view	
		}
	}

	else {
		paused = true;
		LoadMainView();
		DrawMenu();
	}

	glDisable(GL_TEXTURE_2D);

	checkWin();//checking win

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
		glDisable(GL_LIGHTING);
		break;
	case 'o'://outline
		model = !model;
		break;
	case 's': //solid
		vertex = false;
		wire = false;
		glEnable(GL_LIGHTING);
		break;
	case 'p': //vertex
		vertex = true;
		break;
	case 'c': //collider
		showCollider = !showCollider;
		break;
	case 'z': //jump
		jump = true;
		break;
	case 'd': //show controls
		showControls = !showControls;
		break;
	case 'm': //motion
		dance = !dance;
		break;
	case 'l': //light
		lightType = !lightType;
		break;
	case 'b': //bullet speeds
		if (*it == 0.1f) { it = speeds.begin(); }
		else { ++it; }
		break;
	case 27: //pause
		showMenu = !showMenu;
		break;
	case ' ': //shooting
		Bullet * bul = new Bullet;
		bullets.push_back(bul);
		if(sound) SoundEngine->play2D("shoot.wav", false);
		break;
	}
	glutPostRedisplay();
}

int mouseX = 0, mouseY = 0;
#define PI 3.141592
//initialized these so they dont start down
GLint rightMouseButton = GLUT_UP, leftMouseButton = GLUT_UP;

//from slides example
void MouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		leftMouseButton = state;
	}
	if (button == GLUT_RIGHT_BUTTON) {
		rightMouseButton = state;
	}
	mouseX = x;
	mouseY = y;
	int fixedY = windowH - mouseY;//to adapt mouse posiiton
	//from GEC
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		for (auto& button : buttons) {
			if (button->mouseInside(x, fixedY)) {
				if (paused) {
					button->handleClick();
					button->print();
				}

				break;
			}
		}
	}
}

//from slides example
void mouseMotion(int x, int y) {
	if (mainOrCorner) return;
	if (leftMouseButton == GLUT_DOWN) {
		cameraPhi += (mouseX - x) * -0.005;
		cameraTheta += (mouseY - y) * 0.005;
		recomputeOrientation();
	}
	if (rightMouseButton == GLUT_DOWN) {
		double totalChangeSq = (x - mouseX) + (y - mouseY);
		cameraRadius += totalChangeSq * 0.1;
		if (cameraRadius < 2.0) cameraRadius = 2.0;
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

	gluPerspective(60.0, (double)(8 * windowW) / (windowH * 7), 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//sound engine just for the random demon sound so
//i can make it louder then the other sound engine
ISoundEngine* SoundEngine2 = createIrrKlangDevice();

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
		x += (lx * fraction) * 3;
		z += (lz * fraction) * 3;
		break;
	case GLUT_KEY_DOWN:
		x -= (lx * fraction) * 3;
		z -= (lz * fraction) * 3;
		break;

		//key names from https://stackoverflow.com/questions/15435715/opengl-glut-buttons-and-keys

	case GLUT_KEY_F1:
		//from slides
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
	case GLUT_KEY_F2:
		mainOrCorner = !mainOrCorner;
		break;
	case GLUT_KEY_F3:
		sound = !sound;
		if (sound) {
			SoundEngine = createIrrKlangDevice();
			SoundEngine2 = createIrrKlangDevice();
		}
		else {
			SoundEngine->drop();
			SoundEngine2->drop();
		}
		played = false;
		break;
	case GLUT_KEY_F4:
		if (bGouraud) glShadeModel(GL_SMOOTH);
		else glShadeModel(GL_FLAT);
		bGouraud = !bGouraud;
		glutPostRedisplay();
		break;

	}
}

//various trackers and flags
int danceFrame = 0;
int timeFrame = 0;
bool once = false;
bool bulletOnce = false;
bool jumpDone = false;
bool moveOnce = false;
bool moveTwice = true;

void Timer(int v) {
	++danceFrame;
	++moveTimer;
	++timeFrame;
	++killTime;
	if (danceFrame >= 10) danceFrame = 0;//loop through 10 dance frames
	if (timeFrame >= 10) timeFrame = 0;  //loop through 10 time frames
	if (moveTimer >= 50) moveTimer = 0;  //loop through 50 movement frames
	if (killTime >= 10) killTime = 0;    //loop through 10 kill frames

	//for counting down seconds
	if (timeFrame == 9 && seconds != 0 && !gameDone  && !paused) { seconds--; }

	rotateTwo = danceFrame * 36.0f;//for spinning motion

	//not used in this assignment, only keeping for next one
	superSpin = danceFrame * 108.0f; //faster spin speed
	rotateOne = danceFrame * -36.0f;//legsSpin so it rotates

	if (jump && !jumpDone) { jumpForce += 0.3f; }//jump up
	else if (jump && jumpDone) { jumpForce -= 0.3f; }//go down

	if (jumpForce >= 2.0f) { jumpDone = true; }//start going down at jump's peak
	if (jumpForce == 0.0f) {//reset flags
		jump = false;
		jumpDone = false;
	}

	//random setting of tmp and moving flags
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

	//to make sure dancing variables begin at the right place, 
	//regardless of current frame
	if (!once) danceFrame = 0;
	if (dance) {
		if (danceFrame < 5) headY += 0.075f; //make head rise/fall
		else headY -= 0.075f;
		once = true;
		if (danceFrame < 5) fullRot += 1.0f;
		else fullRot -= 1.0f;
	}
	else { headY = 0.0f; fullRot = -2.5f; once = false; }//to make sure these start at the right place

	if (danceFrame == 0 && rand() % 150 == 0 && !gameDone) { //play demon sound at random
		if(sound)SoundEngine2->setSoundVolume(5.0f);
		if (sound)SoundEngine2->play2D("idle.wav", false);
	}

	glutPostRedisplay();
	glutTimerFunc(100, Timer, v);
}

void menuFunc(int i) {
	if (i == 1) { exit(0); }//exit
	else {//reset
		for (int i = 0; i < 10; ++i) {
			killed[i] = false;
			dontShow[i] = false;
			score = 0; robotsKilled = 0;
			seconds = 30;
			gameDone = false;
		}
		//resetting positions and player rotation
		randPos[i] = storePos[i];
		ax = 3.63335, az = 9.26316;
		lx = 0, lz = -1;
	}
}

int main(int argc, char** argv) {
	
	//std::cout << "Model loaded successfully!" << std::endl;
	
	
	glutInit(&argc, argv);
	srand(time(nullptr));
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE); // RGB mode
	glutInitWindowSize(WINDOW_W, WINDOW_H); // window size
	glutInitWindowPosition(windowPosx, windowPosy);
	glutCreateWindow("Assignment 4 - Ben Ams 811254818 - Demon Hunter!");

	//from the examples, but i changed the values for a good starting position
	cameraRadius = 10.0f;
	cameraTheta = 1.471f;
	cameraPhi = 1.197f;
	recomputeOrientation();

	init();

	glShadeModel(GL_SMOOTH);

	loadFloorTexture("floor.png");
	loadSkinTexture("skin.jpg");
	loadShirtTexture("shirt.png");
	loadLegsTexture("legs.png");
	loadBoxTexture("wall.png");
	loadRockTexture("Rock-Texture-Surface.jpg");
	loadRedTexture("red.png");
	loadImage("sky.jpeg");

	cout << "=================================" << endl;
	cout << "Computer Graphics Assignment 4" << endl;
	cout << "Ben Ams" << endl;
	cout << "=================================" << endl << endl;
	cout << "Press [d] to show commands" << endl << endl;

	glClearColor(0.0, 0.0, 0.0, 1.0); // clear the window screen

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

	//making the menu, use middle mouse button
	auto MainMenu = glutCreateMenu(menuFunc);
	glutAddMenuEntry("RESUME", 0);
	glutAddMenuEntry("EXIT", 1);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);

	glutMainLoop();

	stbi_image_free(skyIMG);
	return 0;
}


