#include <vsim/util/format.hpp>
#include <vector>

using namespace std ;

namespace vsim { namespace util {

string formatFloat(double arg, int fieldWidth, char format, int precision, char fill_char)
{
    stringstream strm ;
    if ( fieldWidth > 0 ) strm.width(fieldWidth) ;
    if ( precision > 0 ) strm.precision(precision) ;

    strm.fill(fill_char) ;

    switch ( format ) {
    case 'E':
        strm.setf(ios::uppercase);
    case 'e':
        strm.setf(std::ios::scientific, std::ios::floatfield);
        strm.setf(std::ios::dec, std::ios::basefield);
        break ;
    case 'F':
        strm.setf(ios::uppercase);
    case 'f':
        strm.setf(ios::fixed, ios::floatfield);
        break;
    case 'G':
        strm.setf(std::ios::uppercase);
    case 'g':
        strm.setf(std::ios::dec, std::ios::basefield);
        strm.flags(strm.flags() & ~std::ios::floatfield);
        break;
    default: break ;
    }

    strm << arg ;

    return strm.str() ;
}

string formatDecimal(long long arg, int fieldWidth, char format, char fill_char)
{
    stringstream strm ;

    if ( fieldWidth > 0 ) strm.width(fieldWidth) ;
    else if ( fieldWidth < 0 ) {
        strm.setf(ios::left, ios::adjustfield);
        strm.width(-fieldWidth) ;
    }

    strm.fill(fill_char) ;

    switch ( format ) {
        case 'u': case 'd': case 'i':   strm.setf(ios::dec, ios::basefield); break;
        case 'o':                       strm.setf(ios::oct, ios::basefield); break;
        case 'X':                       strm.setf(ios::uppercase);
        case 'x':                       strm.setf(ios::hex, ios::basefield); break ;
        default:                        break ;
    }

    strm << arg ;

    return strm.str() ;
}



} // namespace util
} // namespace vsim
