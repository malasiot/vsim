#ifndef __VSIM_ENVIRONMENT_HPP__
#define __VSIM_ENVIRONMENT_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <Eigen/Core>

#include <vsim/env/scene_fwd.hpp>

namespace vsim {

class Environment ;
typedef std::shared_ptr<Environment> EnvironmentPtr ;

// class defining attributes of scene visualization

class Environment {
public:

    Environment() {}

public:

    CameraPtr camera_ ;
    std::vector<LightPtr> lights_ ;
};


}
#endif
