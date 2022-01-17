#include <bits/stdc++.h>

using namespace std;

#include <GL/glut.h>
#include "include/def.h"
#include "include/geo3d.h"
#include "include/camera.h"
#include "include/classes.h"
#include "include/bitmap_image.hpp"

int window;
bool isDrawnAxes;

bool showCamLogs;
Camera cam;

bool hasTextures;;
unsigned char* texDat[2];
int tex_width[2], tex_height[2], tex_channels[2];

int REC_LEV;
int IMG_WIDTH, IMG_HEIGHT;

vector<Object*> objects;
vector<Light> lights;

void loadData() {
	objects.clear();
	lights.clear();

	ifstream in("scene.txt");
	if(!in.is_open()) {
		cerr << "Couldn't load data from ./scene.txt\n";
		exit(1);
	}

	in >> REC_LEV;
	in >> IMG_WIDTH;
	IMG_HEIGHT = IMG_WIDTH;

	Floor* flr = new Floor(1000, 20);
	flr->setCoeffs(0.4, 0.2, 0.3, 0.3, 5);
	objects.push_back(flr);

	int n_objs;
	in >> n_objs;

	while(n_objs--) {
		string type;
		in >> type;

		if(type == "sphere") {
			Sphere* sp = new Sphere();
			in >> *sp;
			objects.push_back(sp);
		}
		else if(type == "triangle") {
			Triangle* tr = new Triangle();
			in >> *tr;
			objects.push_back(tr);
		}
		else if(type == "general") {
			Quadratic* qd = new Quadratic();
			in >> *qd;
			objects.push_back(qd);
		}
	}

	int n_lights;
	in >> n_lights;

	while(n_lights--) {
		Light l;
		in >> l;
		lights.push_back(l);
	}
	
	in.close();

	stbi_set_flip_vertically_on_load(true);
	texDat[1] = stbi_load("img/white.jpg", &tex_width[1], &tex_height[1], &tex_channels[1], 0);
	texDat[0] = stbi_load("img/black.jpg", &tex_width[0], &tex_height[0], &tex_channels[0], 0);
	if(!texDat[0] or !texDat[1]) {
		cerr << "Could not load textures\n";
	}

	cerr << "Loaded data\n";
}

void cleanUp() {
	while(!objects.empty()) {
		delete objects.back();
		objects.pop_back();
	}

	while(!lights.empty()) {
		lights.pop_back();
	}

	stbi_image_free(texDat[0]);
	stbi_image_free(texDat[1]);

	cerr << "Cleaned up\n";
}

void capture() {
	// Althought the image dimension are mentioned square
	// a landscape AR provides better quality
	// IMG_WIDTH = 1280;
	// IMG_HEIGHT = 720;

	static int img_cnt = 0;

	bitmap_image img(IMG_WIDTH, IMG_HEIGHT);
	img.clear();
	
	Tf plane_distance = (WINDOW_HEIGHT / 2.0) / tan(toRadians(VIEW_ANGLE / 2.0));
	Tf dx = (Tf) WINDOW_WIDTH / IMG_WIDTH;
	Tf dy = (Tf) WINDOW_HEIGHT / IMG_HEIGHT;

	Pt3 topleft = cam.pos
		+ cam.look * plane_distance
		- cam.right * WINDOW_WIDTH / 2
		+ cam.up * WINDOW_HEIGHT / 2
		+ cam.right * dx * 0.5
		- cam.up * dy * 0.5;

	for(int i=0; i<IMG_WIDTH; ++i) {
		for(int j=0; j<IMG_HEIGHT; ++j) {
			Pt3 cur = topleft + cam.right * dx * i- cam.up * dy * j;
			Ray ray(cam.pos, cur - cam.pos);
			rgb col;
			Tf t_min = INF;
			Object* o_min = nullptr;

			for(Object* obj : objects) {
				Tf t = obj->intersect(ray, col, 0);
				if(t > 0 and dcmp(t, t_min) < 0) {
					o_min = obj;
					t_min = t;
				}
			}

			if(o_min) {
				col = rgb();
				t_min = o_min->intersect(ray, col, REC_LEV);
				col = col * 255;
				img.set_pixel(i, j, col.r, col.g, col.b);
			}
		}
	}

	img.save_image(string("out/output") + to_string(++img_cnt) + ".bmp");
}

void drawAxes() {
	if(!isDrawnAxes) return ;

	glColor3f(1, 0, 0);
	glBegin(GL_LINES);

	glVertex3f(-1000, 0, 0);
	glVertex3f(1000, 0, 0);

	glEnd();
	
	glColor3f(0, 1, 0);
	glBegin(GL_LINES);

	glVertex3f(0, -1000, 0);
	glVertex3f(0, 1000, 0);

	glEnd();
	
	glColor3f(0, 0, 1);
	glBegin(GL_LINES);

	glVertex3f(0, 0, -1000);
	glVertex3f(0, 0, 1000);
	
	glEnd();
}

void drawRect(Tf width, Tf height, rgb c, Tf z) {
	glColor3f(c.r, c.g, c.b);
	glBegin(GL_QUADS);

	glVertex3f(-width / 2, -height / 2, z);
	glVertex3f(width / 2, -height / 2, z);
	glVertex3f(width / 2, height / 2, z);
	glVertex3f(-width / 2, height / 2, z);

	glEnd();
}

void drawSetUp() {
	drawAxes();

	for(Object* pobj : objects) {
		pobj->draw();
	}

	for(Light l : lights) {
		l.draw();
	}
}

void display() {
	// clear display
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/***** set up camera ******
	** load MODEL-VIEW matrix
	** initialize matrix
	** 1. where is the camera placed at?
	** 2. where is the camera looking at?
	** 3. which direction is the camera's UP?
	**************************/

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		cam.pos.x, cam.pos.y, cam.pos.z,
		cam.pos.x + cam.look.x, cam.pos.y + cam.look.y, cam.pos.z + cam.look.z,
		cam.up.x, cam.up.y, cam.up.z
	);
	glMatrixMode(GL_MODELVIEW);

	/********* custom drawings start here *********/

	drawSetUp();

	/********* custom drawings end here **********/

	/***************  glutSwapBuffers ************
	** swaps the buffers of the current window if double buffered.
	** https://www.opengl.org/resources/libraries/glut/spec3/node21.html
	*********************************************/
	glutSwapBuffers();
}

void animate() {
	/***********  glutPostRedisplay **************
	** marks the current window as needing to be redisplayed.
	** https://www.opengl.org/resources/libraries/glut/spec3/node20.html
	**********************************************/
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch(key) {
		case '0':
			capture();
			break;
		case '1':
			cam.lookLeft();
			if(showCamLogs) cam.prnt();
			break;
		case '2':
			cam.lookRight();
			if(showCamLogs) cam.prnt();
			break;
		case '3':
			cam.lookUp();
			if(showCamLogs) cam.prnt();
			break;
		case '4':
			cam.lookDown();
			if(showCamLogs) cam.prnt();
			break;
		case '5':
			cam.tiltCW();
			if(showCamLogs) cam.prnt();
			break;
		case '6':	
			cam.tiltCCW();
			if(showCamLogs) cam.prnt();
			break;
		case 27: 	// ESC
			glutDestroyWindow(window);
			cleanUp();
			break;
		default:
			break;
	}
}

void specialKey(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_DOWN:
			cam.moveBackward();
			if(showCamLogs) cam.prnt();
			break;
		case GLUT_KEY_UP:
			cam.moveForward();
			if(showCamLogs) cam.prnt();
			break;
		case GLUT_KEY_RIGHT:
			cam.moveRight();
			if(showCamLogs) cam.prnt();
			break;
		case GLUT_KEY_LEFT:
			cam.moveLeft();
			if(showCamLogs) cam.prnt();
			break;
		case GLUT_KEY_PAGE_UP:
			cam.moveUp();
			if(showCamLogs) cam.prnt();
			break;
		case GLUT_KEY_PAGE_DOWN:
			cam.moveDown();
			if(showCamLogs) cam.prnt();
			break;
		case GLUT_KEY_INSERT:
			break;
		case GLUT_KEY_HOME:
			break;
		case GLUT_KEY_END:
			break;
		default:
			break;
	}
}

void mouse(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_RIGHT_BUTTON:
			if(state == GLUT_DOWN) {
				isDrawnAxes ^= 1;
			}
			break;
		default:
			break;
	}
}

void init() {
	/*********** custom initializations *************/
	isDrawnAxes = false;
	showCamLogs = false;
	cam = Camera();

	loadData();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	hasTextures = true;
	if(!texDat[0] or !texDat[1]) hasTextures = false;

	/*********** routine initializations ***********/

	// clear screen
	glClearColor(0, 0, 0, 0);

	/***** set up projection ********
	** load PROJECTION matrix
	** initialize the matrix
	** provide PERSPECTIVE parameters
	** params: vertical FOV, aspect ratio, near sight, far sight
	********************************/

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(VIEW_ANGLE, 1, 1, 1000);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(200, 100);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
	window = glutCreateWindow("Ray Tracing");
	
	init();

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
	glutIdleFunc(animate);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKey);
	glutMouseFunc(mouse);

	glutMainLoop();

	return 0;
}
