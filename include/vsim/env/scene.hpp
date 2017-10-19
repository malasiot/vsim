#ifndef __VSIM_SCENE_HPP__
#define __VSIM_SCENE_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <vsim/util/variant.hpp>
#include <boost/optional.hpp>
#include <Eigen/Core>

#include <vsim/env/body.hpp>
#include <vsim/env/model.hpp>
#include <vsim/env/mesh.hpp>
#include <vsim/env/camera.hpp>
#include <vsim/env/light.hpp>

namespace vsim {

// class defining a complete scene

class Scene {
public:

    // load scene from file using one of registered drivers (falling back to Assimp if no valid driver found)
    static ScenePtr load(const std::string &fname) ;


public:

    std::vector<BodyPtr> bodies_ ;

    std::vector<ModelPtr> models_ ;           // shared models
    std::vector<MaterialPtr> materials_ ;   // shared materials
    std::vector<MeshPtr> meshes_ ;          // shared meshes
    std::vector<CameraPtr> cameras_ ;       // list of cameras
    std::vector<LightPtr> lights_ ;         // list of lights
};


}
#endif
