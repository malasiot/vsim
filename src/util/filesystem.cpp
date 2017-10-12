#include <vsim/util/filesystem.hpp>
#include <algorithm>
#include <fstream>

using namespace std ;

namespace vsim { namespace util {

void split_path(const string &path, string &dir, string &basename, string &extension) {

    size_t pos = path.find_last_of("\\/");

    if ( pos != std::string::npos ) {
        dir = path.substr(0, pos);
        pos ++ ;
    } else
        pos = 0 ;

    string filename = path.substr(pos) ;

    size_t dot = filename.find_last_of(".") ;
    if ( dot != std::string::npos ) {
        basename = filename.substr(0, dot);
        extension  = filename.substr(dot);
    }
    else basename = filename ;
}

string basename( const string &pathname ) {
    return std::string(
        std::find_if( pathname.rbegin(), pathname.rend(), [] (char ch){ return ch == '\\' || ch == '/'; }).base(),
        pathname.end() );
}

string get_file_contents(const std::string &fname) {
    ifstream strm(fname) ;

    if ( !strm.good() ) return string() ;
    strm.seekg(0, ios::end);
    size_t length = strm.tellg();
    strm.seekg(0,std::ios::beg);

    string res ;
    res.resize(length) ;
    strm.read(&res[0], length) ;

    return res ;
}

} // namespace util
} // namespace vsim
