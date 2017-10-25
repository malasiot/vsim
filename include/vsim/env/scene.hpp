#ifndef __VSIM_SCENE_HPP__
#define __VSIM_SCENE_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <vsim/util/variant.hpp>
#include <boost/optional.hpp>
#include <Eigen/Core>

#include <vsim/env/scene_fwd.hpp>


namespace vsim {

// class defining a complete scene

class Scene {
public:
    void loadFromFile(const std::string &script_path) ;
    static ScenePtr loadFromString(const std::string &script_source) ;
public:

    PhysicsScenePtr physics_scene_ ;
    EnvironmentPtr environment_ ;
};


}
#endif
