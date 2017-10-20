#ifndef __VSIM_NODE_HPP__
#define __VSIM_NODE_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <Eigen/Core>

#include <vsim/env/scene_fwd.hpp>
#include <vsim/env/pose.hpp>

namespace vsim {
// a hieracrchy of nodes. each node applies a transformation to the attached geometries

struct Node {
public:

    Node() = default ;

    std::string name_ ;
    Pose pose_ ;                 // transformation matrix to apply to child nodes and attached geometries
    std::vector<NodePtr> children_ ;       // child nodes
    std::vector<DrawablePtr> drawables_ ; // meshes associated with this node

    NodePtr parent_ ;

};

}
#endif
