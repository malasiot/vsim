#ifndef __VSIM_LIGHT_HPP__
#define __VSIM_LIGHT_HPP__

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <vsim/util/variant.hpp>

#include <Eigen/Core>

namespace vsim {


// Abstract light

struct Light {
    enum Type { AMBIENT, POINT, DIRECTIONAL, SPOT } ;
    Light(Type t): type_(t) {}

    Type type_ ;
};

struct AmbientLight: public Light {
    AmbientLight(): Light(AMBIENT) {}
    AmbientLight(const Eigen::Vector3f &clr): Light(AMBIENT), color_(clr) {}

    Eigen::Vector3f color_ ;
};

struct PointLight: public Light {
    PointLight(): Light(POINT), constant_attenuation_(1.0), linear_attenuation_(0.0) {}
    PointLight(const Eigen::Vector3f &pos, const Eigen::Vector3f &clr): Light(POINT), position_(pos), color_(clr), constant_attenuation_(1.0), linear_attenuation_(0.0) {}

    Eigen::Vector3f position_ ;
    Eigen::Vector3f color_ ;
    float constant_attenuation_ ;
    float linear_attenuation_ ;
    float quadratic_attenuation_ ;
};

struct DirectionalLight: public Light {
    DirectionalLight(): Light(DIRECTIONAL) {}
    DirectionalLight(const Eigen::Vector3f &dir, const Eigen::Vector3f &clr): Light(DIRECTIONAL), direction_(dir), color_(clr) {}

    Eigen::Vector3f color_, direction_ ;
};

struct SpotLight: public Light {
    SpotLight(): Light(SPOT), constant_attenuation_(1.0),
    linear_attenuation_(0.0), quadratic_attenuation_(0.0),
    falloff_angle_(M_PI), falloff_exponent_(0){}

    Eigen::Vector3f color_, direction_, position_ ;
    float constant_attenuation_ ;
    float linear_attenuation_ ;
    float quadratic_attenuation_ ;
    float falloff_angle_, falloff_exponent_ ;
};


}
#endif
