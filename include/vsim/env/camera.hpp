#ifndef __VSIM_CAMERA_HPP__
#define __VSIM_CAMERA_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <Eigen/Core>


namespace vsim {


// Abstract camera

struct Viewport {
    Viewport() = default ;
    size_t x_ = 0, y_ = 0, width_, height_ ;
};

class Camera {
public:
    enum Type { Perspective, Orthographic } ;

    Camera(Type t): type_(t), mat_(Eigen::Matrix4f::Identity()) {}
    virtual ~Camera() {}

    void setViewport(size_t w, size_t h) {
        vp_.width_ = w ; vp_.height_ = h ;
    }

    void setViewport(size_t x, size_t y, size_t w, size_t h) {
        vp_.width_ = w ; vp_.height_ = h ;
        vp_.x_ = x ; vp_.y_ = y ;
    }

    void setViewTransform(const Eigen::Matrix4f &vt) {
        mat_ = vt ;
    }

    Type type() const { return type_ ; }
    Eigen::Matrix4f getViewMatrix() const { return mat_ ; }
    const Viewport &getViewport() const { return vp_ ; }

    void lookAt(const Eigen::Vector3f &eye, const Eigen::Vector3f &center, float roll = 0);
    void lookAt(const Eigen::Vector3f &eye, const Eigen::Vector3f &center, const Eigen::Vector3f &up);
protected:

    Type type_ ;
    Eigen::Matrix4f mat_ ; // view transformation
    Viewport vp_ ;
};

// Perspective camera

class PerspectiveCamera: public Camera {
public:
    PerspectiveCamera(float aspect, float yfov, float znear = 0.01, float zfar = 10.0):
        Camera(Perspective), aspect_(aspect), yfov_(yfov), znear_(znear), zfar_(zfar) {
    }


    void setAspectRatio(float asp) {
        aspect_ = asp ;
    }

    Eigen::Matrix4f projectionMatrix() const ;
    float zNear() const { return znear_ ; }
    float zFar() const { return zfar_ ; }

protected:

    float yfov_, aspect_, znear_, zfar_ ;
};

// Orthographic camera

struct OrthographicCamera: public Camera {

    OrthographicCamera(): Camera(Orthographic) {}

    float xmag_, ymag_, znear_, zfar_ ;
};


}
#endif
