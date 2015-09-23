#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <boost/format.hpp>

GLFWwindow* g_window;

int g_running = 1,
    g_width   = 256,
    g_height  = 256,
    g_fullscreen = 0,
    g_drawCageEdges = 1,
    g_drawCageVertices = 1,
    g_prev_x = 0,
    g_prev_y = 0,
    g_mbutton[3] = {0, 0, 0},
    g_frame=0,
    g_freeze=0,
    g_repeatCount;

float g_rotate[2] = {0, 0},
    g_dolly = 5,
    g_pan[2] = {0, 0},
    g_center[3] = {0, 0, 0},
    g_size = 0,
    g_moveScale = 0.0f;

float g_ratio = 1.0;

struct Transform {
    float ModelViewMatrix[16];
    float ProjectionMatrix[16];
    float ModelViewProjectionMatrix[16];
} g_transformData;

inline void
multMatrix(float *d, const float *a, const float *b)
 {

    for (int i=0; i<4; ++i)
    {
        for (int j=0; j<4; ++j)
        {
            d[i*4 + j] =
                a[i*4 + 0] * b[0*4 + j] +
                a[i*4 + 1] * b[1*4 + j] +
                a[i*4 + 2] * b[2*4 + j] +
                a[i*4 + 3] * b[3*4 + j];
        }
    }
}

inline void
rotate(float *m, float angle, float x, float y, float z)
{
    float r = 2 * (float) M_PI * angle/360.0f;
    float c = cos(r);
    float s = sin(r);
    float t[16];
    t[0] = x*x*(1-c)+c;
    t[1] = y*x*(1-c)+z*s;
    t[2] = x*z*(1-c)-y*s;
    t[3] = 0;
    t[4] = x*y*(1-c)-z*s;
    t[5] = y*y*(1-c)+c;
    t[6] = y*z*(1-c)+x*s;
    t[7] = 0;
    t[8] = x*z*(1-c)+y*s;
    t[9] = y*z*(1-c)-x*s;
    t[10] = z*z*(1-c)+c;
    t[11] = 0;
    t[12] = t[13] = t[14] = 0;
    t[15] = 1;
    float o[16];
    for(int i = 0; i < 16; i++) o[i] = m[i];
    multMatrix(m, t, o);
}

inline void
perspective(float *m, float fovy, float aspect, float znear, float zfar)
{
    float r = 2 * (float)M_PI * fovy / 360.0F;
    float t = 1.0f / tan(r*0.5f);
    m[0] = t/aspect;
    m[1] = m[2] = m[3] = 0.0;
    m[4] = 0.0;
    m[5] = t;
    m[6] = m[7] = 0.0;
    m[8] = m[9] = 0.0;
    m[10] = (zfar + znear) / (znear - zfar);
    m[11] = -1;
    m[12] = m[13] = 0.0;
    m[14] = (2*zfar*znear)/(znear - zfar);
    m[15] = 0.0;
}

inline void
identity(float *m)
{
    m[0] = 1; m[1] = 0; m[2] = 0; m[3] = 0;
    m[4] = 0; m[5] = 1; m[6] = 0; m[7] = 0;
    m[8] = 0; m[9] = 0; m[10] = 1; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

inline void
translate(float *m, float x, float y, float z)
{
    float t[16];
    identity(t);
    t[12] = x;
    t[13] = y;
    t[14] = z;
    float o[16];
    for(int i = 0; i < 16; i++) o[i] = m[i];
    multMatrix(m, t, o);
}

/* static */ void
motion(GLFWwindow *, double dx, double dy)
{
    int x=(int)dx, y=(int)dy;

    if (g_mbutton[0] && !g_mbutton[1] && !g_mbutton[2]) {
        // orbit
        // printf("orbit\n");
        g_rotate[0] += x - g_prev_x;
        g_rotate[1] += y - g_prev_y;
    } else if (!g_mbutton[0] && !g_mbutton[1] && g_mbutton[2]) {
        // pan
        // printf("pan\n");
        g_pan[0] -= g_dolly*(x - g_prev_x)/g_width;
        g_pan[1] += g_dolly*(y - g_prev_y)/g_height;
    } else if ((g_mbutton[0] && !g_mbutton[1] && g_mbutton[2]) ||
               (!g_mbutton[0] && g_mbutton[1] && !g_mbutton[2])) {
        // dolly
        g_dolly -= g_dolly*0.01f*(x - g_prev_x);
        if(g_dolly <= 0.01) g_dolly = 0.01f;
        // printf("dolly g_dolly=%f\n",g_dolly);
    }

    g_prev_x = x;
    g_prev_y = y;
}

/* static */ void
mouse(GLFWwindow *, int button, int state, int /* mods */)
{

    // if (button == 0 && state == GLFW_PRESS)
    // return;

    if (button < 3) {
        g_mbutton[button] = (state == GLFW_PRESS);
    }
}


/* static */ void
reshape(GLFWwindow *, int width, int height)
{

    g_width = width;
    g_height = height;

    int windowWidth = g_width, windowHeight = g_height;
    // window size might not match framebuffer size on a high DPI display
    glfwGetWindowSize(g_window, &windowWidth, &windowHeight);
}

/* static */ void
keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		g_running = 0;
        glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

/* static */ void idle()
{
}

/* static */ void display()
{


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, g_width, g_height);

    double aspect = g_width/(double)g_height;
    identity(g_transformData.ModelViewMatrix);
    translate(g_transformData.ModelViewMatrix, -g_pan[0], -g_pan[1], -g_dolly);
    rotate(g_transformData.ModelViewMatrix, g_rotate[1], 1, 0, 0);
    rotate(g_transformData.ModelViewMatrix, g_rotate[0], 0, 1, 0);
    rotate(g_transformData.ModelViewMatrix, -90, 1, 0, 0);
    translate(g_transformData.ModelViewMatrix,
              -g_center[0], -g_center[1], -g_center[2]);
    perspective(g_transformData.ProjectionMatrix,
                45.0f, (float)aspect, 0.01f, 500.0f);
    multMatrix(g_transformData.ModelViewProjectionMatrix,
               g_transformData.ModelViewMatrix,
               g_transformData.ProjectionMatrix);
    glFinish();

    glPushMatrix();
    glMultMatrixf(g_transformData.ModelViewProjectionMatrix);

// #define DRAW_TRIANGLE
#ifdef DRAW_TRIANGLE
    glBegin(GL_TRIANGLES);
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(-0.6f, -0.4f, 0.f);
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.6f, -0.4f, 0.f);
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(-0.6f, 0.6f, 0.f);
    glEnd();
#else // DRAW_TRIANGLE
    glBegin(GL_QUADS);
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(-0.6f, -0.4f, 0.f);
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.6f, -0.4f, 0.f);
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.6f, 0.6f, 0.f);
    glColor3f(1.f, 1.f, 1.f);
    glVertex3f(-0.6f, 0.6f, 0.f);
    glEnd();
#endif // DRAW_TRIANGLE

    // g_ogl2.draw_ogl(OpenGL2Translator::DRAW_AS_WIREFRAME);

    glPopMatrix();
    glFinish();

}

int main(int argc, char **argv)
{
	if (argc!=2)
	{
		std::cerr << boost::format("Usage : %1% <bgeo-file>") % argv[0] << std::endl;
        exit(EXIT_FAILURE);
	}
	std::string bgeo_file(argv[1]);
    // glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);

// #if defined(__APPLE__) || defined (_WIN32)
//#if defined(__APPLE__)
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//#endif // defined(__APPLE__) || defined (_WIN32)

    std::string windows_title = (boost::format("HouGeo Viewer")).str();
    g_window = glfwCreateWindow(640, 480, windows_title.c_str(), NULL, NULL);
    if (!g_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Setup the geometry extractor
//    g_ge.initialize(bgeo_file);
//    g_ogl2.initialize(g_ge.getHeader(),
//    		g_ge.getInfo(),
//			g_ge.getTopology(),
//			g_ge.getAttributes(),
//			g_ge.getPrimitives(),
//			g_ge.getAttributeTypeMap());
    // g_ogl2.dump();
    glfwMakeContextCurrent(g_window);

    glfwGetFramebufferSize(g_window, &g_width, &g_height);
    g_ratio = g_width / (float) g_height;

    glfwSetKeyCallback(g_window, keyboard);
    glfwSetCursorPosCallback(g_window, motion);
    glfwSetMouseButtonCallback(g_window, mouse);
    glfwSetFramebufferSizeCallback(g_window, reshape);

    // MUST do this AFTER a GL context i.e. glfw context is available
    // MUST call glewInit() BEFORE any calls to additional API e.g. OpenGL 3.3 calls, are made
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

    }

    // g_ogl2.setup_ogl();
    std::cout << boost::format("HouGeo Viewer : OpenGL version supported by this platform (%1%): ") % glGetString(GL_VERSION) << std::endl;

    glfwSwapInterval(0);
    while (g_running)
    {
        idle();
        display();
        glfwPollEvents();
        glfwSwapBuffers(g_window);
    }
    glfwDestroyWindow(g_window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
