#ifndef __VSIM_MODEL_HPP__
#define __VSIM_MODEL_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <Eigen/Core>

#include <vsim/util/format.hpp>
#include <vsim/env/scene_fwd.hpp>
#include <vsim/env/indexed_container.hpp>
#include <vsim/env/pose.hpp>
#include <assimp/scene.h>

namespace vsim {

// class defining a scene model. A model is a container of other models as well as geometry nodes and resources such as materials, lights
// that may be loaded from a 3D model file. Nodes can also form a hierarchy. Conceptually a model is a collection of objects while nodes are object parts

class Model {
public:

    // load scene from file using one of registered drivers (falling back to Assimp if no valid driver found)
    static ModelPtr load(const std::string &fname) ;

    static ModelPtr loadAssimp(const std::string &fname) ;
    static ModelPtr loadAssimp(const aiScene *sc, const std::string &fname) ;

    // add light to scene
    void addLight(const LightPtr &light) {
        lights_.push_back(light) ;
    }

    void addMaterial(const MaterialPtr &mat) {
        materials_.push_back(mat) ;
    }

    void addMesh(const MeshPtr &mesh, const std::string &id = std::string()) {
        meshes_.add(mesh, id) ;
    }

    void addNode(const NodePtr &node) {
        nodes_.push_back(node) ;
    }

public:

    Pose pose_ ;

    std::vector<ModelPtr> children_ ;
    std::vector<NodePtr> nodes_ ;           // root nodes of the hierarchy
    std::vector<MaterialPtr> materials_ ;   // shared materials
    IndexedContainer<MeshPtr> meshes_ ;          // shared meshes

    std::vector<CameraPtr> cameras_ ;       // list of cameras
    std::vector<LightPtr> lights_ ;         // list of lights
};


class ModelLoaderException: public std::runtime_error {

public:

    ModelLoaderException(const std::string &driver_name, const std::string &message, const std::string &fname, int line = -1):
        std::runtime_error(util::format("%:% (%)", driver_name, message, fname)) {}
};


}
#endif
