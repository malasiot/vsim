#ifndef __VSIM_POSE_HPP__
#define __VSIM_POSE_HPP__

#include <Eigen/Geometry>
#include <memory>

namespace vsim {

struct Frame ;
typedef std::shared_ptr<Frame> FramePtr ;

struct Pose {
public:

    Pose(): mat_(Eigen::Matrix4f::Identity()) {}

    Eigen::Matrix4f absolute() const ; // accumulate transform of parent frame

    Eigen::Affine3f mat_ ;                 // transformation matrix to apply to child nodes and attached geometries
    FramePtr frame_ ;
};

}
#endif
