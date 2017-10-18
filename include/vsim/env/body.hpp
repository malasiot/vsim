#ifndef __VSIM_BODY_HPP__
#define __VSIM_BODY_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>


#include <Eigen/Core>

#include <vsim/env/model.hpp>

namespace vsim {

class Body ;
typedef std::shared_ptr<Body> BodyPtr ;

// class defining a rigid body

class Body {
public:

    Body(): pose_(Eigen::Matrix4f::Identity()) {}

public:

    Eigen::Matrix4f pose_ ;
    ModelPtr model_ ;
};


}
#endif
