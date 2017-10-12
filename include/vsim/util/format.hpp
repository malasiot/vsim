#ifndef __VSIM_UTIL_FORMAT_HPP__
#define __VSIM_UTIL_FORMAT_HPP__

#include <memory>

namespace vsim { namespace util {

class FormatImpl ;

class Format  {
public:

    Format(const char *fmtStr) ;

    ~Format() ;
    std::unique_ptr<FormatImpl> fmt_ ;
};



} // namespace util
} // namespace vsim

#endif
