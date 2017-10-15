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

Eigen::Matrix4f PerspectiveCamera::projectionMatrix() const {
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


void Camera::lookAt(const Vector3f &eye, const Vector3f &center, const Vector3f &up) {
    Vector3f f = (center - eye).normalized();
    Vector3f s = f.cross(up).normalized();
    Vector3f u = s.cross(f) ;

    mat_ << s.x(), s.y(), s.z(), -s.dot(eye),
            u.x(), u.y(), u.z(), -u.dot(eye),
            -f.x(), -f.y(), -f.z(), f.dot(eye),
            0, 0, 0, 1 ;
}

void Camera::lookAt(const Vector3f &eye, const Vector3f &center, float roll) {
    lookAt(eye, center, Vector3f(0, 1, 0)) ;

    Affine3f rot ;
    rot.setIdentity();
    rot.rotate(AngleAxisf(roll, Eigen::Vector3f::UnitZ())) ;
    mat_ = mat_ * rot.matrix() ;
}

MeshPtr Mesh::createWireCube(float hs) {

    MeshPtr m(new Mesh) ;
    m->vertices_ = {{ -hs, +hs, +hs }, { +hs, +hs, +hs }, { +hs, -hs, +hs }, { -hs, -hs, +hs },
                 { -hs, +hs, -hs }, { +hs, +hs, -hs }, { +hs, -hs, -hs }, { -hs, -hs, -hs } } ;
    m->vertex_indices_ = {  0, 1, 1, 2, 2, 3, 3, 0,  4, 5, 5, 6, 6, 7,  7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };

    m->ptype_ = Lines ;

    return m ;
}

/*
v 1.000000 -1.000000 -1.000000
v 1.000000 -1.000000 1.000000
v -1.000000 -1.000000 1.000000
v -1.000000 -1.000000 -1.000000
v 1.000000 1.000000 -0.999999
v 0.999999 1.000000 1.000001
v -1.000000 1.000000 1.000000
v -1.000000 1.000000 -1.000000
vn 0.0000 -1.0000 0.0000
vn 0.0000 1.0000 0.0000
vn 1.0000 -0.0000 0.0000
vn 0.0000 -0.0000 1.0000
vn -1.0000 -0.0000 -0.0000
vn 0.0000 0.0000 -1.0000
usemtl Material
s off
f 2//1 4//1 1//1
f 8//2 6//2 5//2
f 5//3 2//3 1//3
f 6//4 3//4 2//4
f 3//5 8//5 4//5
f 1//6 8//6 5//6
f 2//1 3//1 4//1
f 8//2 7//2 6//2
f 5//3 6//3 2//3
f 6//4 7//4 3//4
f 3//5 7//5 8//5
f 1//6 4//6 8//6
*/
MeshPtr Mesh::createSolidCube(float hs) {
    MeshPtr m(new Mesh) ;
    m->normals_ = {{ 0.0, -1.0, 0.0 }, {0.0, 1.0, 0.0}, { 1.0, 0.0, 0.0 }, {0.0, 0.0, 1.0}, {-1.0, 0.0, 0.0}, { 0.0, 0.0, -1.0}} ;
    m->vertices_ = {{ +hs, -hs, -hs }, { +hs, -hs, +hs }, { -hs, -hs, +hs }, { -hs, -hs, -hs },
                    { +hs, +hs, -hs }, { +hs, +hs, +hs }, { -hs, +hs, +hs }, { -hs, +hs, -hs } } ;
    m->vertex_indices_ = {  1, 3, 0, 7, 5, 4, 4, 1, 0, 5, 2, 1, 2, 7, 3, 0, 7, 4, 1, 2, 3, 7, 6, 5, 4, 5, 1, 5, 6, 2, 2, 6, 7, 0, 3, 7};
    m->normal_indices_ = {  0, 0, 0,  1, 1, 1,  2, 2, 2,  3, 3, 3,  4, 4, 4,  5, 5, 5,  0, 0, 0,  1, 1, 1,  2, 2, 2,  3, 3, 3,  4, 4, 4, 5, 5, 5};
    m->ptype_ = Triangles ;
    return m ;
}

}}
