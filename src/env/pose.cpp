#include <vsim/env/pose.hpp>
#include <vsim/env/frame.hpp>

using namespace std ;
using namespace Eigen ;

namespace vsim {
Matrix4f Pose::absolute() const {
    if ( !frame_ ) return mat_.matrix() ;
    else return frame_->transform() * mat_.matrix() ;
}

}
