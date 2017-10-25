#ifndef __VSIM_PHYSICS_MODEL_HPP__
#define __VSIM_PHYSICS_MODEL_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <Eigen/Core>

#include <vsim/env/scene_fwd.hpp>
#include <vsim/env/pose.hpp>
#include <vsim/env/base_element.hpp>

namespace vsim {

// a physics model contains bodies and their constraints

struct PhysicsModel: public BaseElement {
public:

    PhysicsModel() = default ;

    std::vector<RigidBodyPtr> bodies_ ;
    std::vector<RigidBodyConstraintPtr> constraints_ ;
};

}
#endif
