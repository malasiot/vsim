#ifndef __VSIM_COLLISION_SHAPE_HPP__
#define __VSIM_COLLISION_SHAPE_HPP__

#include <Eigen/Geometry>
#include <memory>

#include <vsim/env/pose.hpp>
#include <vsim/env/base_element.hpp>
#include <vsim/env/scene_fwd.hpp>

namespace vsim {

struct CollisionShape ;
typedef std::shared_ptr<CollisionShape> CollisionShapePtr ;

struct CollisionShape: public BaseElement {
public:

    CollisionShape() = default ;

    GeometryPtr geom_ ;
    Pose pose_ ;
};

}
#endif
