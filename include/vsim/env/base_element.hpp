#ifndef __VSIM_BASE_ELEMENT_HPP__
#define __VSIM_BASE_ELEMENT_HPP__

#include <string>

namespace vsim {

// base class of scene graph objects

class BaseElement {
public:

    BaseElement() {}

public:

    std::string id_ ;
};


}
#endif
