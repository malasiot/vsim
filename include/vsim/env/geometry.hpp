#ifndef __VSIM_GEOMETRY_HPP__
#define __VSIM_GEOMETRY_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <Eigen/Core>

#include <vsim/env/mesh.hpp>
#include <vsim/env/material.hpp>
#include <vsim/env/node.hpp>

namespace vsim {

struct Geometry ;
typedef std::shared_ptr<Geometry> GeometryPtr ;

// Geometry structure associates a mesh instance with material.

struct Geometry {
    Geometry(const MeshPtr &mesh, const MaterialPtr &mat): mesh_(mesh), material_(mat) {}
    Geometry() = default ;

    MeshPtr mesh_ ;
    MaterialPtr material_ ;
    NodePtr parent_ ;
};




}
#endif
