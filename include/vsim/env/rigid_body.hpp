#ifndef __VSIM_RIGID_BODY_HPP__
#define __VSIM_RIGID_BODY_HPP__

#include <string>
#include <vector>
#include <memory>

#include <Eigen/Core>

#include <vsim/env/scene_fwd.hpp>
#include <vsim/env/pose.hpp>
#include <vsim/env/base_element.hpp>

namespace vsim {

class RigidBody ;
typedef std::shared_ptr<RigidBody> BodyPtr ;

// class defining a rigid body

class RigidBody: public BaseElement {
public:

    RigidBody() = default ;

public:

    std::vector<CollisionShapePtr> shapes_ ;

    Pose pose_ ;
    float mass_ ;
    Eigen::Vector3f velocity_, angular_velocity_ ;

    NodePtr visual_ ;
};


}
#endif
