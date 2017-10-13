#ifndef __VSIM_SCENE_HPP__
#define __VSIM_SCENE_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <vsim/util/variant.hpp>
#include <boost/optional.hpp>
#include <Eigen/Core>

#include <vsim/renderer/scene_fwd.hpp>
#include <assimp/scene.h>

namespace vsim { namespace renderer {

// class defining a complete scene

class Scene {
public:

    // load scene from file using one of registered drivers (falling back to Assimp if no valid driver found)
    static ScenePtr load(const std::string &fname) ;

    static ScenePtr loadAssimp(const std::string &fname) ;
    static ScenePtr loadAssimp(const aiScene *sc, const std::string &fname) ;

    // add light to scene
    void addLight(const LightPtr &light) {
        lights_.push_back(light) ;
    }

public:

    std::vector<NodePtr> nodes_ ;           // root nodes of the hierarchy
    std::vector<MaterialPtr> materials_ ;   // shared materials
    std::vector<MeshPtr> meshes_ ;          // shared meshes

    std::vector<CameraPtr> cameras_ ;       // list of cameras
    std::vector<LightPtr> lights_ ;         // list of lights
};

// a hieracrchy of nodes. each node applies a transformation to the attached geometries

struct Node {
public:

    Node(): mat_(Eigen::Matrix4f::Identity()) {}
    std::string name_ ;
    Eigen::Matrix4f mat_ ;                 // transformation matrix to apply to child nodes and attached geometries
    std::vector<NodePtr> children_ ;       // child nodes
    std::vector<GeometryPtr> geometries_ ; // meshes associated with this node

    NodePtr parent_ ;                      // parent node
};


static const int MAX_TEXTURES = 4 ;

// mesh data of triangular mesh

struct Mesh {
    enum PrimitiveType { Triangles, Lines, Points } ;

    // vertex attribute arrays, they can be of different size if attributes (e.g. normals) are shared for multiple vertices (e.g. per face normals)
    std::vector<Eigen::Vector3f> vertices_, normals_, colors_ ;
    std::vector<Eigen::Vector2f> tex_coords_[MAX_TEXTURES] ;

    // triplets of vertex attribute indices corresponding to the triangles of the mesh (all same size)
    std::vector<uint32_t> vertex_indices_, normal_indices_, color_indices_, tex_coord_indices_[MAX_TEXTURES] ;

    PrimitiveType ptype_ ;
};


// texture and its parameters
struct Sampler2D {
    std::string image_url_ ;
    std::string wrap_s_, wrap_t_ ;
};

struct None {} ;

typedef util::variant<Eigen::Vector4f, Sampler2D> ColorOrTexture ;

struct Material {
    enum Type { PHONG, LAMBERTIAN, BLINN, CONSTANT } ;

    std::string name_ ;
    Type type_ ;        // type of material
    ColorOrTexture emission_, ambient_, diffuse_,
    specular_, reflective_, transparent_ ; // material component color or associated texture map

    util::variant<float> reflectivity_, shininess_, transparency_ ;
};


// Geometry structure associates a mesh instance with material.

struct Geometry {
    MeshPtr mesh_ ;
    MaterialPtr material_ ;
};


// Abstract camera

class Camera {
public:
    enum Type { Perspective, Orthographic } ;

    Camera(Type t): type_(t), mat_(Eigen::Matrix4f::Identity()) {}
    virtual ~Camera() {}

    void setViewport(size_t w, size_t h) {
        vp_width_ = w ; vp_height_ = h ;
    }

    void setViewport(size_t x, size_t y, size_t w, size_t h) {
        vp_width_ = w ; vp_height_ = h ;
        vp_x_ = x ; vp_y_ = y ;
    }

    setViewTransform(const Eigen::Matrix4f &vt) {
        mat_ = vt ;
    }

    Type type() const { return type_ ; }
    Eigen::Matrix4f getViewMatrix() const { return mat_ ; }
    Viewport getViewport() const { return

private:

    Type type_ ;
    Eigen::Matrix4f mat_ ; // view transformation
    float vp_x_ = 0, vp_y_ = 0, vp_width_ = 0, vp_height_ = 0 ;
};

// Perspective camera

class PerspectiveCamera: public Camera {

    PerspectiveCamera(float aspect, float yfov): Camera(Perspective) {
        aspect_ratio_ = aspect ;
        yfov_ = yfov  ;
        xfov_ = aspect * yfov ;
        znear_ = 0.01 ;
        zfar_ = 10 ;
    }


    float xfov_, yfov_, aspect_ratio_, znear_, zfar_ ;
};

// Orthographic camera

struct OrthographicCamera: public Camera {

    OrthographicCamera(): Camera(Orthographic) {}

    float xmag_, ymag_, znear_, zfar_ ;
};



// Abstract light

struct Light {
    enum Type { AMBIENT, POINT, DIRECTIONAL, SPOT } ;
    Light(Type t): type_(t) {}

    Type type_ ;
};

struct AmbientLight: public Light {
    AmbientLight(): Light(AMBIENT) {}
    AmbientLight(const Eigen::Vector3f &clr): Light(AMBIENT), color_(clr) {}

    Eigen::Vector3f color_ ;
};

struct PointLight: public Light {
    PointLight(): Light(POINT), constant_attenuation_(1.0), linear_attenuation_(0.0) {}
    PointLight(const Eigen::Vector3f &pos, const Eigen::Vector3f &clr): Light(POINT), position_(pos), color_(clr), constant_attenuation_(1.0), linear_attenuation_(0.0) {}

    Eigen::Vector3f position_ ;
    Eigen::Vector3f color_ ;
    float constant_attenuation_ ;
    float linear_attenuation_ ;
    float quadratic_attenuation_ ;
};

struct DirectionalLight: public Light {
    DirectionalLight(): Light(DIRECTIONAL) {}
    DirectionalLight(const Eigen::Vector3f &dir, const Eigen::Vector3f &clr): Light(DIRECTIONAL), direction_(dir), color_(clr) {}

    Eigen::Vector3f color_, direction_ ;
};

struct SpotLight: public Light {
    SpotLight(): Light(SPOT), constant_attenuation_(1.0),
    linear_attenuation_(0.0), quadratic_attenuation_(0.0),
    falloff_angle_(M_PI), falloff_exponent_(0){}

    Eigen::Vector3f color_, direction_, position_ ;
    float constant_attenuation_ ;
    float linear_attenuation_ ;
    float quadratic_attenuation_ ;
    float falloff_angle_, falloff_exponent_ ;
};


}}
#endif
