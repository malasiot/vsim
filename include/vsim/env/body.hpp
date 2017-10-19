#ifndef __VSIM_BODY_HPP__
#define __VSIM_BODY_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>


#include <Eigen/Core>

#include <vsim/env/model.hpp>
#include <vsim/env/pose.hpp>

namespace vsim {

class Body ;
typedef std::shared_ptr<Body> BodyPtr ;

// class defining a rigid body

class Body {
public:

    Body() = default ;

public:

    Pose pose_ ;
    ModelPtr model_ ;
};


}
#endif
