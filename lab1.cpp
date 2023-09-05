//modified by: John Pocasangre
//date: 8/29/2023
//
//author: Gordon Griesel
//date: Spring 2022
//purpose: get openGL working on your personal computer
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>


//some structures

class Global {
public:
	int xres, yres;	
    float w;
    float dir;
	float pos[2];
	float color[3];
	clock_t lastBounceTime;
    float minBounceInterval;
    float maxBounceInterval;
	bool showBox;
	clock_t currentBounceTime;
    Global();
}g;

class X11_wrapper 
{
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	~X11_wrapper();
	X11_wrapper();
	void set_title();
	bool getXPending();
	XEvent getXNextEvent();
	void swapBuffers();
	void reshape_window(int width, int height);
	void check_resize(XEvent *e);
	void check_mouse(XEvent *e);
	int check_keys(XEvent *e);
} x11;

//Function prototypes
void init_opengl(void);
void physics(void);
void render(void);



//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
	init_opengl();
	//Main loop
	int done = 0;
	while (!done) {
		//Process external events.
		while (x11.getXPending()) 
		{
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			x11.check_mouse(&e);
			done = x11.check_keys(&e);
		}
		physics();
		render();
		x11.swapBuffers();
		usleep(200);
	}
	return 0;
}

Global::Global()
{

	xres = 400;
	yres = 200;
    w= 35.0f;
    dir = 5.0f;
    pos[0] = 0.0f + w;
    pos[1] = yres /2.0f;
	color[0] = 0.0f;
	color[1] = 1.0f;
	color[2] = 0.0f;
	 lastBounceTime = clock();
    minBounceInterval = 0.003f;  
    maxBounceInterval = 0.4f; 
	showBox = true;
}

X11_wrapper::~X11_wrapper()
{
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w = g.xres, h = g.yres;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) 
	{
		cout << "\n\tcannot connect to X server\n" << endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) 
	{
		cout << "\n\tno appropriate visual found\n" << endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "3350 Lab1");
}

bool X11_wrapper::getXPending()
{
	//See if there are pending events.
	return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
	//Get a pending event.
	XEvent e;
	XNextEvent(dpy, &e);
	return e;
}

void X11_wrapper::swapBuffers()
{
	glXSwapBuffers(dpy, win);
}

void X11_wrapper::reshape_window(int width, int height)
{
	//window has been resized.
	g.xres = width;
	g.yres = height;
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
}

void X11_wrapper::check_resize(XEvent *e)
{
	//The ConfigureNotify is sent by the
	//server if the window is resized.
	if (e->type != ConfigureNotify)
		return;
	XConfigureEvent xce = e->xconfigure;
	if (xce.width != g.xres || xce.height != g.yres) {
		//Window size did change.
		reshape_window(xce.width, xce.height);
	}
}
//-----------------------------------------------------------------------------

void X11_wrapper::check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	//Weed out non-mouse events
	if (e->type != ButtonRelease &&
		e->type != ButtonPress &&
		e->type != MotionNotify) 
	{
		//This is not a mouse event that we care about.
		return;
	}
	//
	if (e->type == ButtonRelease) 
	{
		return;
	}
	if (e->type == ButtonPress) 
	{
		if (e->xbutton.button==1) 
		{
			//Left button was pressed.
			//int y = g.yres - e->xbutton.y;
			return;
		}
		if (e->xbutton.button==3) 
		{
			//Right button was pressed.
			return;
		}
	}
	if (e->type == MotionNotify) 
	{
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) 
		{
			savex = e->xbutton.x;
			savey = e->xbutton.y;
			//Code placed here will execute whenever the mouse moves.


		}
	}
}

int X11_wrapper::check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) 
	{
		switch (key) 
		{
			case XK_1:
				//Key 1 was pressed
				break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
		}
	}
	return 0;
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
}

void physics()
{
    if (g.xres < 2 * g.w) 
	{
        g.showBox = false;
        return;
    } else {
        g.showBox = true;
    }

    g.pos[0] += g.dir;

    if (g.pos[0] >= (g.xres-g.w) || g.pos[0] <= g.w) 
	{
        if (g.pos[0] >= (g.xres-g.w)) g.pos[0] = (g.xres-g.w);
        if (g.pos[0] <= g.w) g.pos[0] = g.w;
        g.dir = -g.dir;

        // Turn red when bouncing
        g.color[0] = 1.0f;
        g.color[1] = 0.0f;
        g.color[2] = 0.0f;
    } else {
        // Calculate the fraction of the distance covered since the last bounce
        float fraction;
        if (g.dir > 0) {
            fraction = (g.pos[0] - g.w) / (g.xres - 2 * g.w);
        } else 
		{
            fraction = (g.xres - g.w - g.pos[0]) / (g.xres - 2 * g.w);
        }

        // Interpolate colors based on the fraction of the distance covered
        g.color[0] = 1.0f - fraction;
        g.color[2] = fraction;
    }
}




void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (!g.showBox) 
	{
        return;
    }

    glPushMatrix();
    glColor3f(g.color[0], g.color[1], g.color[2]);
    glTranslatef(g.pos[0], g.pos[1], 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(-g.w, -g.w);
        glVertex2f(-g.w, g.w);
        glVertex2f(g.w, g.w);
        glVertex2f(g.w, -g.w);
    glEnd();
    glPopMatrix();
}

