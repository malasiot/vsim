#include <vsim/renderer/renderer.hpp>
#include "glfw_window.hpp"
#include "trackball.hpp"

#include <iostream>

using namespace vsim::renderer ;
using namespace vsim::util ;
using namespace std ;
using namespace Eigen ;

class glfwGUI: public glfwRenderWindow {
public:

    glfwGUI(ScenePtr scene): glfwRenderWindow(), rdr_(scene), camera_(1.0, 50*M_PI/180, 0.01, 1000) {
    }

    void onInit() {
        rdr_.init() ;

        trackball_.setCamera(&camera_) ;
        trackball_.initCamera({0.0, 100.0, 500}, {0, 0, 0}, {0, 1, 0}) ;
    }

    void onResize(int width, int height) {
        float ratio;
        ratio = width / (float) height;

        camera_.setAspectRatio(ratio) ;
        camera_.setViewport(width, height)  ;

        trackball_.setScreenSize(width, height);
    }

    void onMouseButtonPressed(uint button, size_t x, size_t y, uint flags) override {
        switch ( button ) {
            case GLFW_MOUSE_BUTTON_LEFT:
                trackball_.setLeftClicked(true) ;
                break ;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                trackball_.setMiddleClicked(true) ;
                break ;
            case GLFW_MOUSE_BUTTON_RIGHT:
                trackball_.setRightClicked(true) ;
                break ;
        }
        trackball_.setClickPoint(x, y) ;
    }

    void onMouseButtonReleased(uint button, size_t x, size_t y, uint flags) override {
        switch ( button ) {
            case GLFW_MOUSE_BUTTON_LEFT:
                trackball_.setLeftClicked(false) ;
                break ;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                trackball_.setMiddleClicked(false) ;
                break ;
            case GLFW_MOUSE_BUTTON_RIGHT:
                trackball_.setRightClicked(false) ;
                break ;
        }

    }

    void onMouseMoved(double xpos, double ypos) override {
        trackball_.setClickPoint(xpos, ypos) ;
    }

    void onMouseWheel(double x) {
        trackball_.setScrollDirection(x>0);
    }


    void onRender() {
        trackball_.update() ;
        rdr_.render(camera_, Renderer::RENDER_SMOOTH) ;
    }

    Renderer rdr_ ;
    ScenePtr scene_ ;
    TrackBall trackball_ ;
    PerspectiveCamera camera_ ;
};

int main(int argc, char *argv[]) {


    ScenePtr scene = Scene::load("/home/malasiot/Downloads/greek_column.obj") ;
    scene->addLight(LightPtr(new DirectionalLight(Vector3f(0.5, 0.5, 1), Vector3f(1, 1, 1)))) ;

    glfwGUI gui(scene) ;

    gui.run(500, 500) ;

}
