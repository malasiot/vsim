#ifndef __VSIM_PHYSICS_SCENE_HPP__
#define __VSIM_PHYSICS_SCENE_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <Eigen/Core>

#include <vsim/env/scene_fwd.hpp>
#include <vsim/env/base_element.hpp>

namespace vsim {

// a physics scene contains all bodies and models comprising the simulation

struct PhysicsScene: public BaseElement {
public:

    PhysicsScene() = default ;

    std::vector<RigidBodyPtr> bodies_ ;
    std::vector<PhysicsModelPtr> models_ ;
};

}
#endif
