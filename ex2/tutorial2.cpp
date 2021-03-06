//  
//  cgp-projects
//
//  Created by HUJI Computer Graphics course staff, 2013.
//  Tweaked by HUJI Computer Games Programming staff 2014

#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

// GLM headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // for glm::value_ptr

using namespace glm;

#include "Model.h"
#include "ShaderIO.h"
#include "PPBuffer.h"


#include <iostream>

/** Internal Definitions */

#define	WINDOW_SIZE         (600) // initial size of the window               //
#define	WINDOW_POS_X        (100) // initial X position of the window         //
#define	WINDOW_POS_Y        (100) // initial Y position of the window         //

#define RC_OK                 (0) // Everything went ok                       //
#define RC_INVALID_ARGUMENTS  (1) // Invalid arguments given to the program   //
#define RC_INPUT_ERROR        (2) // Invalid input to the program             //

#define	ARGUMENTS_PROGRAM     (0) // program name position on argv            //
#define	ARGUMENTS_INPUTFILE   (1) // given input file position on argv        //
#define	ARGUMENTS_REQUIRED    (2) // number of required arguments             //

/** Key definitions */

#define KEY_ANIMATE         ('a') // Key used to start animation              //
#define KEY_ESC            ('\e') // Key used to terminate the program - ESC  //
#define KEY_QUIT            ('q') // Key used to terminate the program        //
#define KEY_RESET           ('r') // Key used to reset the applied TX's	      //
#define KEY_RELOAD          ('l') // Key used to reload the shaders 	      //
#define KEY_FORWARD			('w')
#define KEY_BACKWORD		('s')
#define KEY_RIGHT			('d')
#define KEY_LEFT			('a')
#define KEY_FAULT			('f')

/** display callback */
void display(void);

/** window reshape callback  */
void windowResize(int width, int height);

/** keyboard callback  */
void keyboard(unsigned char key, int x, int y);

/** keyboard up callback  */
void keyboardUp(unsigned char key, int x, int y);

/** mouse click callback */
void mouse(int button, int state, int x, int y) ;

/** mouse dragging callback  */
void motion(int x, int y) ;

/** mouse moving callback  */
void passiveMotion(int x, int y) ;

/** timer callback */
void timer(int value) ;

/** idle callback */
void idle() ;
/** Global variables */

int     g_nFPS = 0, g_nFrames = 0;              // FPS and FPS Counter
int     g_dwLastFPS = 0;                        // Last FPS Check Time
bool    g_startAnimation = false;
bool    g_duringAnimation = false;

// A global variable for our model (a better practice would be to use a singletone that holds all model):
Model _model;
PPBuffer _ppbuffer;

// camera params (again, a better practice is to have a camera singleton)
vec3 dir = vec3(0.0f, 0.0f, 1.0f);
vec3 pos = vec3(0.0f, 1.0f, 0.0f);
vec3 up = vec3(0.0f, 1.0f, 0.0f);
vec2 angles;

mat4 World = mat4(1.0);
mat4 View = lookAt(pos, pos+dir, up);
mat4 Projection = perspective(45.0f, 1.0f, 0.1f, 100.0f);

mat4 wvp = Projection * View * World;

const int moveLeft = 1;
const int moveRight = 2;
const int moveForward = 4;
const int moveBackward = 8;
const int moveUp = 16;
const int moveDown = 32;

int moveMask = 0;

/** main function */
int main(int argc, char* argv[])
{
    std::cout << "Starting ex2..." << std::endl;
	
    // Initialize GLUT
    glutInit(&argc, argv) ;
#ifdef __APPLE__
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE) ;
#else
    glutInitContextVersion(3, 3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
#endif
    glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);
    glutInitWindowPosition(WINDOW_POS_X, WINDOW_POS_Y);
    glutCreateWindow(argv[0]);
	
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    int glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "Unable to initialize GLEW ... exiting" << std::endl;
        exit(1);
    }
	
#ifdef __APPLE__
    GLint sync = 1;
    CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &sync);
#endif
		
	// Set callback functions:
	glutDisplayFunc(display) ;
	glutReshapeFunc(windowResize) ;
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutMouseFunc(mouse);
	glutMotionFunc(passiveMotion);
	glutPassiveMotionFunc(passiveMotion);
	glutTimerFunc(100, timer, 0);   // uint millis int value
	glutIdleFunc(idle);
	
	glutSetCursor(GLUT_CURSOR_NONE);

    // Init anything that can be done once and for all:
    _model.init();
    _ppbuffer.init(WINDOW_SIZE, WINDOW_SIZE);
  
		
    // Start events/drawing loop
    glutMainLoop();
	
    return 0;
}

void display(void) {
  _ppbuffer.setup();
  
  // Clear the screen buffer
  glClearColor(0.0, 0.3, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  // tell the model to draw itself...
  _model.draw(World, View, Projection);
  
  _ppbuffer.render();
  
  // Swap those buffers so someone will actually see the results... //
  glutSwapBuffers();
}

// This method is called when the window is resized
void windowResize(int w, int h)
{
    _ppbuffer.resize(w,h);

    // Update model to fit the new resolution
    _model.resize(w, h);
    
    // set the new viewport //
    glViewport(0, 0, w, h);
    
    // Refresh the display //
    glutPostRedisplay();
}


void terrainHorizontalMove()
{
	//get average height of four closest vertices to the camera
	pos.y = _model.getPosHeight(pos) + 1;
	View = lookAt(pos, pos + dir, up);
}

/********************************************************************
 * Function  :	keyboard
 * Arguments :	key : the key that was pressed
 *              x   : x value of the current mouse location
 *              y   : y value of the current mouse location
 * Returns   : n/a
 * Throws    : n/a
 *
 * Purpose   : This function handles all the keyboard input from the user.
 *             It supports terminating the application when the KEY_QUIT is pressed.
 *
 \******************************************************************/
void keyboard(unsigned char key, int x, int y)
{
    unsigned int lower_key = tolower(key);
	
    switch(lower_key)
    {
      case KEY_RESET:
          // reset to initial view of the object
          // For use in a future exercise
          break;
      case KEY_RELOAD:
          // Reload the shading programs of the object
          // For use in a future exercise
          break;
	case KEY_FORWARD:
		moveMask |= moveForward;
		break;
	case KEY_BACKWORD:
		moveMask |= moveBackward;
		break;
	case KEY_LEFT:
		moveMask |= moveLeft;
		break;
	case KEY_RIGHT:
		moveMask |= moveRight;
		break;
	case KEY_FAULT:
		_model.createFault();
		terrainHorizontalMove();
		glutPostRedisplay();
		break;
      case KEY_QUIT:
      case KEY_ESC:
          // Terminate the program:
          exit(RC_OK);
          break;
      default:
          std::cerr << "Key " << lower_key << " undefined\n";
          break;
    }
    
    return;
}

/********************************************************************
 * Function  :   mouse
 * Arguments :   button  : the button that was engaged in some action
 *               state   : the new state of that button
 *               x       : x value of the current mouse location
 *               y       : y value of the current mouse location
 * Returns   :   n/a
 * Throws    :   n/a
 *
 * Purpose   :   This function handles mouse actions.
 *
 \******************************************************************/
void mouse(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON)
    {
		
    }
    else if (button == GLUT_RIGHT_BUTTON)
    {
        
    }
    
    return;
}


/********************************************************************
 * Function  :   motion
 * Arguments :   x   : x value of the current mouse location
 *               y   : y value of the current mouse location
 * Returns   :   n/a
 * Throws    :   n/a
 *
 * Purpose   :   This function handles mouse dragging events.
 *
 \******************************************************************/
void motion(int x, int y)
{
    return;
}



void keyboardUp(unsigned char key, int x, int y) {
	if(key == KEY_LEFT)
		moveMask &= ~moveLeft;
	if(key == KEY_RIGHT)
		moveMask &= ~moveRight;
	if(key == KEY_FORWARD)
		moveMask &= ~moveForward;
	if(key == KEY_BACKWORD)
		moveMask &= ~moveBackward;
}


void passiveMotion(int x, int y)
{
	static bool wrap = false;
	//perform the movement only once per two calls of this function
	//because glutWarpPointer also generates this callback (in which case we don't move)
	if(!wrap)
	{
		int ww = glutGet(GLUT_WINDOW_WIDTH);
		int wh = glutGet(GLUT_WINDOW_HEIGHT);

		int dx = x - ww / 2;
		int dy = y - wh / 2;

		const float mousespeed = 0.005;

		angles.x += dx * mousespeed;
		angles.y += dy * mousespeed;

		//don't want too much vertical angle 1.5 ~ PI / 2
		if(angles.y > 1.5) angles.y = 1.5;
		if(angles.y < -1.5) angles.y = -1.5;

		if(angles.x < -M_PI)
			angles.x += M_PI * 2;
		else if(angles.x > M_PI)
			angles.x -= M_PI * 2;

		if(angles.y < -M_PI / 2)
			angles.y = -M_PI / 2;
		if(angles.y > M_PI / 2)
			angles.y = M_PI / 2;

		dir.x = -sinf(angles.x) * cosf(angles.y);
		dir.y = -sinf(angles.y);
		dir.z = cosf(angles.x) * cosf(angles.y);

		View = glm::lookAt(pos, pos + dir, up);

		wrap = true;
		glutWarpPointer(ww / 2, wh / 2);
		glutPostRedisplay();
	}
	else
	{
		wrap = false;
	}
}


void timer(int value) {

}

void idle() {
	static int pt = 0;
	const float movespeed = 8;

	//	 Calculate time since last call to idle()
	int t = glutGet(GLUT_ELAPSED_TIME);
	float dt = (t - pt) * 1.0e-3;
	pt = t;

	// Calculate movement vectors
	vec3 forward_dir = vec3(dir.x, 0, dir.z);
	vec3 right_dir = cross(forward_dir, up);

	// Update camera position
	if(moveMask & moveLeft)
		pos -= right_dir * movespeed * dt;
	if(moveMask & moveRight)
		pos += right_dir * movespeed * dt;
	if(moveMask & moveForward)
		pos += forward_dir * movespeed * dt;
	if(moveMask & moveBackward)
		pos -= forward_dir * movespeed * dt;

	//terrain horizontal move depending on height of four closest vertices
	if(moveMask)
	{
		pos.y = _model.getPosHeight(pos) + 1.0;
	}


	View = glm::lookAt(pos, pos + dir, up);

	// Redraw the scene
	if(moveMask != 0)
	{
		glutPostRedisplay();
	}
}
