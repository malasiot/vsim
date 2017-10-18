#ifndef __VSIM_MATERIAL_HPP__
#define __VSIM_MATERIAL_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <vsim/util/variant.hpp>
#include <Eigen/Core>

namespace vsim {

struct Material ;
typedef std::shared_ptr<Material> MaterialPtr ;

// texture and its parameters
struct Sampler2D {
    std::string image_url_ ;
    std::string wrap_s_, wrap_t_ ;
};

struct Material {
    enum Type { PHONG, LAMBERTIAN, BLINN, CONSTANT } ;

    std::string name_ ;
    Type type_ ;        // type of material
    util::variant<Eigen::Vector4f> emission_, ambient_, diffuse_,
    specular_, reflective_, transparent_ ;

    util::variant<float> reflectivity_, shininess_, transparency_ ;
    util::variant<Sampler2D> texture_ ;
};


}
#endif
