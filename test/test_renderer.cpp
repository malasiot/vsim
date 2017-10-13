#include <vsim/renderer/renderer.hpp>
#include <GLFW/glfw3.h>

#include <vsim/util/format.hpp>
#include <iostream>

using namespace vsim::renderer ;
using namespace vsim::util ;
using namespace std ;

class glfwRenderingContext: public RenderingContext {
public:

    glfwRenderingContext(uint32_t w, uint32_t h): RenderingContext(w, h) {
        if ( !init()  ) throw std::runtime_error("failed to initialize GL context") ;
    }

    bool init() {
        if( !glfwInit() ) return false ;

//        glfwSetErrorCallback(error_callback);

        // With an intel card with this glxinfo I have replaced GLFW_OPENGL_COMPAT_PROFILE
        // to GLFW_OPENGL_CORE_PROFILE
        // OpenGL renderer string: Mesa DRI Intel(R) HD Graphics 5500 (Broadwell GT2)
        // OpenGL core profile version string: 3.3 (Core Profile) Mesa 10.5.9
        // OpenGL core profile shading language version string: 3.30

        //    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        if ( !( handle_ = glfwCreateWindow(width_, height_, "vsim-test", 0, 0) )) {
            glfwTerminate();
            return false ;
        }


        return true ;
    }

    ~glfwRenderingContext() {
        glfwDestroyWindow(handle_);
        glfwTerminate();
    }


    void attach() {
        glfwMakeContextCurrent(handle_);
    }

    void detach() {
       glfwMakeContextCurrent(0);
    }

    GLFWwindow *handle_ ;
};

int main(int argc, char *argv[]) {

    glfwRenderingContext ctx(100, 100) ;

    ctx.attach() ;

    ScenePtr scene = Scene::load("/home/malasiot/Downloads/greek_column.obj") ;
    Renderer rdr(scene) ;

    rdr.init() ;

    ctx.detach() ;

}
