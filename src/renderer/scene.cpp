#include <vsim/renderer/scene.hpp>
#include <iostream>

#include <Eigen/Geometry>

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

static Matrix4f look_at(const Vector3f & eye,  const Vector3f &center, const Vector3f &up)
{
    Vector3f f = (center - eye).normalized();
    Vector3f s = f.cross(up).normalized();
    Vector3f u = s.cross(f) ;

    Matrix4f res ;
    res << s.x(), s.y(), s.z(), -s.dot(eye),
            u.x(), u.y(), u.z(), -u.dot(eye),
            -f.x(), -f.y(), -f.z(), f.dot(eye),
            0, 0, 0, 1 ;

    return res ;
}

void Camera::lookAt(const Vector3f &eye, const Vector3f &center, const Vector3f &up)
{
    mat_ = look_at(eye, center, up) ;

}

void Camera::lookAt(const Vector3f &eye, const Vector3f &center, float roll)
{
    Matrix4f lr = look_at(eye, center, Vector3f(0, 1, 0)) ;

    Affine3f rot ;
    rot.setIdentity();
    rot.rotate(AngleAxisf(roll, Eigen::Vector3f::UnitZ())) ;
    mat_ = lr * rot.matrix() ;

}

}}
