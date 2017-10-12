#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <set>

#include <vsim/util/xml_sax_parser.hpp>
#include <vsim/util/filesystem.hpp>

using namespace vsim::util ;
using namespace std;

string escape_string(const string &src) {
    return '"' + src + '"' ;
}

struct ShaderConfig {
    ShaderConfig(const string &id, const string &type, const string &src, const string &code):
        id_(id), type_(type), src_(src), code_(code) {}

    void dumb(ostream &os) const {
        os << '{' << escape_string(id_) << ',' << escape_string(type_) << ',' << escape_string(src_) << ',';
        os << "R\"\"(" << code_ << ")\"\"" ;
        os << '}' ;
    }

    string id_, type_, src_, code_ ;
};

struct ProgramConfig {
    ProgramConfig(const string &id): id_(id) {}

    void dumb(ostream &os) const {
        os << '{' << escape_string(id_) << ',' ;
        os << '{' ;
        for ( uint i=0 ; i<shaders_.size() ; i++ ) {
            if ( i > 0 ) os << ',' ;
            os << escape_string(shaders_[i]) ;
        }
        os << '}' ;
        os << '}' ;
    }

    string id_ ;
    vector<string> shaders_ ;
};

struct LibraryConfig {

    LibraryConfig(const string &id): id_(id) {}

    string id_ ;

    map<string, ShaderConfig> shaders_ ;
    map<string, ProgramConfig> programs_ ;
};


class ShaderConfigException: public std::runtime_error {
public:
    ShaderConfigException(const string &msg): std::runtime_error(msg) {}
};

class ShaderLibraryConfigParser: public XMLSAXParser {
public:
    ShaderLibraryConfigParser(istream &strm, const string &src_root): XMLSAXParser(strm), src_root_(src_root) {}

    void startElement(const std::string &qname, const Dictionary &attr_list) override {
        if ( qname == "library" ) {
            string id = attr_list.get("id") ;
            auto res = libraries_.emplace(id, id) ;
            if ( res.second )
                current_library_ = id ;
            else
                throw ShaderConfigException("Library id \"" + id + "\", already defined") ;
        } else if ( qname == "shader" ) {
            auto cl = libraries_.find(current_library_) ;
            assert ( cl != libraries_.end() ) ;

            if ( current_program_.empty() ) {
                string id = attr_list.get("id") ;
                string stype = attr_list.get("type") ;
                string src = attr_list.get("src") ;

                static set<string> stypes{"vertex", "fragment", "compute"} ;


                if ( stype.empty() )
                     throw ShaderConfigException("Attribute 'type' is missing for shader with id \"" + id + "\"") ;

                if ( stypes.count(stype) == 0 )
                    throw ShaderConfigException("Value of attribute 'type' is invalid for shader with id \"" + id + "\"") ;

                if ( src.empty() )
                     throw ShaderConfigException("Attribute 'src' is missing for shader with id \"" + id + "\"") ;

                string code = get_file_contents(src_root_ + src) ;

                if ( code.empty() )
                    throw ShaderConfigException("Cannot ready shader source code from \"" + src_root_ + src + "\"") ;

                auto res = cl->second.shaders_.emplace(id, ShaderConfig{id, stype, src, code}) ;

                if ( !res.second )
                     throw ShaderConfigException("Shader with id \"" + id + "\", already defined") ;
            }
            else {
                auto it = cl->second.programs_.find(current_program_) ;
                if ( it != cl->second.programs_.end() ) {
                    string ref = attr_list.get("ref") ;

                    if ( ref.empty() )
                         throw ShaderConfigException("Attribute 'ref' is missing for shader attached to program \"" + current_program_ + "\"") ;

                    auto itref = cl->second.shaders_.find(ref) ;
                    if ( itref == cl->second.shaders_.end() )
                        throw ShaderConfigException("No such shader \"" + ref + "\" in program \"" + current_program_ + "\"") ;

                    it->second.shaders_.emplace_back(ref) ;
                }
            }
        }
        else if ( qname == "program" ) {
            auto cl = libraries_.find(current_library_) ;
            assert ( cl != libraries_.end() ) ;
            string id = attr_list.get("id") ;

            if ( id.empty() )
                 throw ShaderConfigException("Attribute 'id' is missing for program declared in \"" + current_library_ + "\"") ;

            auto res = cl->second.programs_.emplace(id, ProgramConfig{id}) ;

            if ( !res.second )
                 throw ShaderConfigException("Program with id \"" + id + "\", already defined") ;
            else
                current_program_ = id ;
        }
    }

    void endElement(const std::string &qname) {
        if ( qname == "program" )
            current_program_.clear() ;
        else if ( qname == "library" )
            current_library_.clear() ;
    }

    void characters(const std::string &text_data) {
    }

    virtual void error(ErrorCode code, uint line, uint column ) {
        std::cerr << "error near line: " << line << ", col: " << column << ", code: " << code << std::endl ;
    }

    map<std::string, LibraryConfig> libraries_ ;
    string current_library_, current_program_, src_root_ ;
};


int main(int argc, char** args) {
    if (argc < 4) {
        printf("syntax error, usage:  glsl2src rootdir cfgfile outfile");
        exit(1);
    }

    string root = args[1];
    string cfg = args[2] ;
    string out = args[3] ;

    ifstream strm(cfg) ;
    ShaderLibraryConfigParser config(strm, root + '/') ;
    try {
        if ( !config.parse() ) {
            cerr << "Error parsing config file: "  << cfg << endl ;
            exit(1) ;
        }
    }
    catch ( ShaderConfigException &e ) {
        cerr << "Error parsing config file: "  << cfg << endl ;
        cerr << e.what() << endl ;
        exit(1) ;
    }

    ofstream os(out) ;

    os << R"(
#include <vsim/renderer/ogl_shaders.hpp>
using namespace std ;
using vsim::renderer::OpenGLShaderLibrary ;
)" ;

    for( const auto &l: config.libraries_ ) {
        os << R"(vector<OpenGLShaderLibrary::ShaderConfig> )" << l.first << "_shaders{" << endl;

        for( auto it = l.second.shaders_.begin() ; it != l.second.shaders_.end(); it++) {
            const ShaderConfig &shader = it->second ;
            if ( it != l.second.shaders_.begin() ) os << ',' ;
            shader.dumb(os) ;
        }

        os << "};" << endl ;

        os << R"(vector<OpenGLShaderLibrary::ProgramConfig> )" << l.first << "_programs{" << endl;

        for( auto it = l.second.programs_.begin() ; it != l.second.programs_.end(); it++) {
            const ProgramConfig &program = it->second ;
            if ( it != l.second.programs_.begin() ) os << ',' ;
            program.dumb(os) ;
        }

        os << "};" << endl ;
    }



}
