#ifndef __VSIM_GEOMETRY_HPP__
#define __VSIM_GEOMETRY_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <Eigen/Core>

namespace vsim {

struct Geometry ;
typedef std::shared_ptr<Geometry> GeometryPtr ;

// Geometry structure associates a mesh instance with material.

struct Geometry {

    Geometry() = default ;
    virtual ~Geometry() {}
};

struct BoxGeometry: public Geometry {
    Eigen::Vector3f half_extents_ ;

};

struct SphereGeometry: public Geometry {
    float radius_ ;
};

struct CylinderGeometry: public Geometry {
    float radius_, height_ ;
};

struct ConeGeometry: public Geometry {
    float radius_, height_ ;
};


static const int MAX_MESH_TEXTURES = 4 ;

class Mesh ;
typedef std::shared_ptr<Mesh> MeshPtr ;


// mesh data of triangular mesh

class Mesh: public Geometry {
public:

    enum PrimitiveType { Triangles, Lines, Points } ;

    // vertex attribute arrays, they can be of different size if attributes (e.g. normals) are shared for multiple vertices (e.g. per face normals)
    std::vector<Eigen::Vector3f> vertices_, normals_, colors_ ;
    std::vector<Eigen::Vector2f> tex_coords_[MAX_MESH_TEXTURES] ;

    // triplets of vertex attribute indices corresponding to the triangles of the mesh (all same size)
    std::vector<uint32_t> vertex_indices_, normal_indices_, color_indices_, tex_coord_indices_[MAX_MESH_TEXTURES] ;

    PrimitiveType ptype_ ;

    void load(const std::string &fpath) {}

    // a 3d cube

    static MeshPtr createWireCube(float sz) ;
    static MeshPtr createSolidCube(float sz) ;

    static MeshPtr createWireSphere(float radius, size_t slices, size_t stacks) ;
    static MeshPtr createSolidSphere(float radius, size_t slices, size_t stacks) ;

    // the base of the cone is on (0, 0, 0) aligned with the z-axis and pointing towards positive z

    static MeshPtr createWireCone(float radius, float height, size_t slices, size_t stacks) ;
    static MeshPtr createSolidCone(float radius, float height, size_t slices, size_t stacks) ;

    static MeshPtr createWireCylinder(float radius, float height, size_t slices, size_t stacks) ;
    static MeshPtr createSolidCylinder(float radius, float height, size_t slices, size_t stacks) ;
};


}
#endif
