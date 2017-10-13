#include <vsim/renderer/scene.hpp>
#include <iostream>

#include "scene_loader.hpp"

using namespace std ;
using namespace Eigen ;

namespace vsim { namespace renderer {

ScenePtr Scene::load(const std::string &fname)
{
    SceneLoaderPtr driver = SceneLoader::findDriver(fname) ;

    if ( !driver )
        return Scene::loadAssimp(fname) ;

    try {
        return driver->load(fname) ;
    }
    catch ( SceneLoaderException &e ) {
        cerr << e.what() << endl ;
        return ScenePtr() ;
    }
}

Eigen::Matrix4f PerspectiveCamera::projectionMatrix() const
{
    assert(abs(aspect_ - std::numeric_limits<float>::epsilon()) > static_cast<float>(0));

    float xfov = aspect_ * yfov_ ;
    float const d = 1/tan(xfov / static_cast<float>(2));

    Matrix4f result ;
    result.setZero() ;

    result(0, 0) = d / aspect_ ;
    result(1, 1) = d ;
    result(2, 2) =  (zfar_ + znear_) / (znear_ - zfar_);
    result(2, 3) =  2 * zfar_ * znear_ /(znear_ - zfar_) ;
    result(3, 2) = -1 ;

    return result;

}

}}
