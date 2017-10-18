#ifndef __VSIM_NODE_HPP__
#define __VSIM_NODE_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <Eigen/Core>

namespace vsim {

struct Node ;
typedef std::shared_ptr<Node> NodePtr ;

struct Geometry ;
typedef std::shared_ptr<Geometry> GeometryPtr ;

struct Model ;
typedef std::shared_ptr<Model> ModelPtr ;
// a hieracrchy of nodes. each node applies a transformation to the attached geometries

struct Node {
public:

    Node(): mat_(Eigen::Matrix4f::Identity()) {}
    std::string name_ ;
    Eigen::Matrix4f mat_ ;                 // transformation matrix to apply to child nodes and attached geometries
    std::vector<NodePtr> children_ ;       // child nodes
    std::vector<GeometryPtr> geometries_ ; // meshes associated with this node

    NodePtr parent_ ;                      // parent node
    ModelPtr model_ ;
};

}
#endif
