#include <vsim/util/format.hpp>
#include <vector>

using namespace std ;

namespace vsim { namespace util {

class FormatSpec {
public:

    string literal_ ;
    char flags_ = 0, length_ = 0, spec_ = 0 ;
    int width_ = -2, precision_ = -2 ;
};


class FormatParser {
public:
    FormatParser(const char *fmt): p_(fmt) {}

    operator bool () { return *p_ != 0 ; }
    bool parseLiteral(string &res) ;
    bool parseSpec(FormatSpec &spec) ;
    bool parseInteger(int &val) ;

private:

    const char *p_ ;
};

class FormatImpl {
public:
    FormatImpl() = default ;

    bool parse(const char *fmt) ;

    vector<std::unique_ptr<FormatSpec>> specs_ ;
};


bool FormatParser::parseLiteral(string &res) {
    while (1) {
        switch(*p_)
        {
            case '\0': return true ;
            case '%':
                if ( (*p_ + 1) != '%' ) return true ;
                else { res.push_back(*p_) ; ++p_ ; }
                break;
            default:
                res.push_back(*p_) ;
         }
        ++p_ ;
    }

    return true ;
}

bool FormatParser::parseInteger(int &res) {
    res =  0;

    while ( *p_ >= '0' && *p_ <= '9' ) {
        res = res * 10 + ( *p_ - '0' ) ;
        ++p_ ;
    }

    return true ;
}

bool FormatParser::parseSpec(FormatSpec &spec) {

    if ( *p_++ != '%' ) return false ;

    // parse flag

    switch(*p_) {
        case '#':
        case '0':
        case '-':
        case ' ':
        spec.flags_ = *p_++ ;
    }

    // parse width

    if ( *p_ == '*' ) {
        spec.width_ = -1 ;
        ++p_ ;
    }
    else
        parseInteger(spec.width_) ;

    // parse precision

    if ( *p_ == '.' ) {
        ++p_ ;

        if ( *p_ == '*' ) {
               spec.precision_ = -1 ;
               ++p_ ;
        } else
            parseInteger(spec.precision_) ;
    }

    // ignore any C99 length modifier

    while(*p_ == 'l' || *p_ == 'h' || *p_ == 'L' ||
             *p_ == 'j' || *p_ == 'z' || *p_ == 't')
           ++p_;

    switch( *p_ ) {
        case 'u': case 'd': case 'i':
        case 'o': case 'X':
        case 'x': case 'p':
        case 'E': case 'e':
        case 'F': case 'f':
        case 'G': case 'g':
        case 'a': case 'A':
        case 'c':
        case 's': case 'n':
            spec.spec_ = *p_++ ;
            break ;
        case '\0':
            return false ;
        default:
            break;
    }

    return true ;
}

bool FormatImpl::parse(const char *fmt) {

    FormatParser parser(fmt) ;

    while (parser) {
        string lit ;
        if ( !parser.parseLiteral(lit) ) return false ;
        if ( !lit.empty() ) {
            std::unique_ptr<FormatSpec> spec(new FormatSpec) ;
            spec->literal_ = std::move(lit) ;
            specs_.emplace_back(std::move(spec)) ;
        }
        if ( !parser ) break ;
        std::unique_ptr<FormatSpec> spec(new FormatSpec) ;

        if ( !parser.parseSpec(*spec) ) return false ;
        else {
            specs_.emplace_back(std::move(spec)) ;
        }

    }

    return true ;
}

Format::Format(const char *fmt_str): fmt_(new FormatImpl) {
    fmt_->parse(fmt_str) ;
}

Format::~Format() {

}




} // namespace util
} // namespace vsim
