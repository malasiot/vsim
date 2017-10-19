#ifndef __VSIM_FRAME_HPP__
#define __VSIM_FRAME_HPP__

#include <Eigen/Geometry>

#include <vsim/env/pose.hpp>

namespace vsim {

struct Frame ;
typedef std::shared_ptr<Frame> FramePtr ;

struct Frame {
public:

    Frame() ;

    Eigen::Matrix4f transform() const {
        return pose_.absolute() ;
    }

    Pose pose_ ;
};

}
#endif
