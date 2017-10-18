#include <vsim/renderer/renderer.hpp>
#include <vsim/env/camera.hpp>
#include <vsim/env/light.hpp>
#include <vsim/env/mesh.hpp>
#include <vsim/env/material.hpp>
#include <vsim/env/geometry.hpp>
#include <vsim/env/node.hpp>
#include <vsim/env/environment.hpp>

#include "glfw_window.hpp"
#include "trackball.hpp"

#include <iostream>

using namespace vsim::renderer ;
using namespace vsim ;
using namespace std ;
using namespace Eigen ;

class glfwGUI: public glfwRenderWindow {
public:

    glfwGUI(ModelPtr scene): glfwRenderWindow(), rdr_(scene), camera_(1.0, 50*M_PI/180, 0.01, 1000) {
    }

    void onInit() {
        rdr_.init() ;

        trackball_.setCamera(&camera_, {0.0, 100.0, 500}, {0, 0, 0}, {0, 1, 0}) ;
        //  trackball_.setCamera(&camera_, {0.0, 1, 5}, {0, 0, 0}, {0, 1, 0}) ;
        trackball_.setZoomScale(20) ;
    }

    void onResize(int width, int height) {
        float ratio;
        ratio = width / (float) height;

        trackball_.setScreenSize(width, height);

        camera_.setAspectRatio(ratio) ;
        camera_.setViewport(width, height)  ;


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
        ostringstream s ;
        s << xpos << ',' << ypos ;
        text_ = s.str() ;

        trackball_.setClickPoint(xpos, ypos) ;
    }

    void onMouseWheel(double x) {
        trackball_.setScrollDirection(x>0);
    }


    void onRender() {
        trackball_.update() ;
        rdr_.render(camera_, Renderer::RENDER_SMOOTH) ;
        rdr_.renderText(text_, 0.5, 0.5) ;
    }

    string text_ ;
    Renderer rdr_ ;

    TrackBall trackball_ ;
    PerspectiveCamera camera_ ;
};

int main(int argc, char *argv[]) {

    Environment env ;
    env.loadXML("/home/malasiot/tmp/env.xml") ;

    ModelPtr scene = Model::load("/home/malasiot/Downloads/greek_column.obj") ;
     //ScenePtr scene = Scene::load("/home/malasiot/Downloads/cube.obj") ;
    scene->addLight(LightPtr(new DirectionalLight(Vector3f(0.5, 0.5, 1), Vector3f(1, 1, 1)))) ;

    MeshPtr cube = Mesh::createWireCylinder(50, 100, 10, 10) ;
    MaterialPtr cube_mat(new Material) ;
    cube_mat->type_ = Material::CONSTANT ;
    cube_mat->diffuse_.set<Vector4f>(1, 0, 0, 1) ;
    scene->addMaterial(cube_mat) ;
    GeometryPtr cube_geom(new Geometry) ;
    cube_geom->material_ = cube_mat ;
    cube_geom->mesh_ = cube ;
    NodePtr cube_node(new Node) ;
    cube_node->geometries_.push_back(cube_geom) ;
    cube_node->mat_.setIdentity() ;
    scene->addMesh(cube) ;
    scene->addNode(cube_node) ;

    glfwGUI gui(scene) ;

    gui.run(500, 500) ;

}
