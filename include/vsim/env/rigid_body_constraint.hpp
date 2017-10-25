#ifndef __VSIM_RIGID_BODY_CONSTRAINT_HPP__
#define __VSIM_RIGID_BODY_CONSTRAINT_HPP__

#include <string>
#include <vector>
#include <memory>

#include <Eigen/Core>

#include <vsim/env/scene_fwd.hpp>
#include <vsim/env/base_element.hpp>

namespace vsim {

class RigidBodyConstraint ;
typedef std::shared_ptr<RigidBodyConstraint> RigidBodyConstraintPtr ;

class RigidBodyConstraint: public BaseElement {
public:

    RigidBodyConstraint() = default ;

public:

    RigidBodyPtr a_, b_ ;
};

struct HingeConstraint: public RigidBodyConstraint {
    HingeConstraint() ;

    float min_angle_, max_angle_ ;
};


}
#endif
