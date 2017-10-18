#ifndef __VSIM_ENVIRONMENT_HPP__
#define __VSIM_ENVIRONMENT_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <Eigen/Core>

#include <vsim/env/scene.hpp>

namespace vsim {

class Environment ;
typedef std::shared_ptr<Environment> EnvironmentPtr ;

// class defining a complete scene

class Environment {
public:

    Environment() {}

    void loadXML(const std::string &path) ;
public:

    ScenePtr scene_ ;
};

class EnvironmentLoadException: public std::runtime_error {
public:
    EnvironmentLoadException(const std::string &msg): std::runtime_error(msg) {}
};

}
#endif
