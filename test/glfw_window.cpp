#include "glfw_window.hpp"

using namespace std ;

glfwRenderWindow::~glfwRenderWindow()
{

}

bool glfwRenderWindow::run(size_t width, size_t height, const string &wname) {

    glfwSetErrorCallback(errorCallback);

    if( !glfwInit() ) return false ;
    // With an intel card with this glxinfo I have replaced GLFW_OPENGL_COMPAT_PROFILE
    // to GLFW_OPENGL_CORE_PROFILE
    // OpenGL renderer string: Mesa DRI Intel(R) HD Graphics 5500 (Broadwell GT2)
    // OpenGL core profile version string: 3.3 (Core Profile) Mesa 10.5.9
    // OpenGL core profile shading language version string: 3.30

    //    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);\
    if ( !( handle_ = glfwCreateWindow(width, height, wname.c_str(), 0, 0) )) {
        glfwTerminate();
        return false ;
    }

    glfwSetWindowUserPointer(handle_, this) ;

    glfwMakeContextCurrent(handle_);
    glfwSwapInterval(1);

    glfwSetCursorPosCallback(handle_, moveCallback);
    glfwSetKeyCallback(handle_, keyCallback);
    glfwSetMouseButtonCallback(handle_, buttonCallback);
    glfwSetScrollCallback(handle_, scrollCallback);
    glfwSetWindowSizeCallback(handle_, sizeCallback);

    onInit() ;

    sizeCallback(handle_, width, height); // Set initial size.

    while (!glfwWindowShouldClose(handle_))  {

        onRender() ;
        glfwSwapBuffers(handle_);
        glfwPollEvents();
    }

    glfwDestroyWindow(handle_);

    glfwTerminate();

    return true ;
}

void glfwRenderWindow::buttonCallback(GLFWwindow *window, int button, int action, int mods) {

    glfwRenderWindow *instance = (glfwRenderWindow *)glfwGetWindowUserPointer(window) ;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    switch(action)
    {
    case GLFW_PRESS:
        instance->onMouseButtonPressed(button, xpos, ypos, mods) ;
        break ;
    case GLFW_RELEASE:
        instance->onMouseButtonReleased(button, xpos, ypos, mods) ;
        break ;
    default: break;
    }
}

void glfwRenderWindow::errorCallback(int error, const char *description) {

}

void glfwRenderWindow::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    glfwRenderWindow *instance = (glfwRenderWindow *)glfwGetWindowUserPointer(window) ;

    switch(action) {
    case GLFW_PRESS:
        switch(key)
        {
        case GLFW_KEY_ESCAPE:
            // Exit app on ESC key.
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        default:
            instance->onKeyPressed(key, mods);
        }
        break;
    case GLFW_RELEASE:
        instance->onKeyReleased(key, mods) ;
    default: break;
    }

}

void glfwRenderWindow::moveCallback(GLFWwindow *window, double xpos, double ypos) {
     glfwRenderWindow *instance = (glfwRenderWindow *)glfwGetWindowUserPointer(window) ;

     instance->onMouseMoved(xpos, ypos);
}

void glfwRenderWindow::scrollCallback(GLFWwindow *window, double xpos, double ypos) {
    glfwRenderWindow *instance = (glfwRenderWindow *)glfwGetWindowUserPointer(window) ;
    instance->onMouseWheel(xpos + ypos);
}

void glfwRenderWindow::sizeCallback(GLFWwindow *window, int width, int height) {
    glfwRenderWindow *instance = (glfwRenderWindow *)glfwGetWindowUserPointer(window) ;
    instance->onResize(width, height) ;
}
