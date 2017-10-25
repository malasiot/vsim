#ifndef __VSIM_DRAWABLE_HPP__
#define __VSIM_DRAWABLE_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <Eigen/Core>
#include <vsim/env/scene_fwd.hpp>
#include <vsim/env/base_element.hpp>

namespace vsim {

class Drawable ;
typedef std::shared_ptr<Drawable> DrawablePtr ;

// class defining a complete scene

class Drawable: public BaseElement {
public:

    Drawable() {}

public:

    GeometryPtr geometry_ ;
    MaterialPtr material_ ;
};


}
#endif
