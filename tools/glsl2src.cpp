/* http://hamelot.co.uk/visualization/opengl-glsl-shader-as-a-string/ */

#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>

#include <vsim/util/xml_sax_parser.hpp>

using namespace vsim::util ;
using namespace std;

struct ShaderCode {
    ShaderCode(const string &name, const string &type, const string &code):
        name_(name), type_(type), code_(code) {}

    string name_, type_, code_ ;
};

struct ShaderBundleConfig {

    ShaderBundleConfig(const string &name): name_(name) {}
    string name_ ;

    vector<ShaderCode> shaders_ ;
};

class BundleXml: public XMLSAXParser {
public:
    BundleXml(istream &strm): XMLSAXParser(strm) {}

    void startElement(const std::string &qname, const Dictionary &attr_list) override {
        if ( qname == "bundle" ) {
            string bname = attr_list.get("name") ;
            bundles_.emplace_back(bname) ;
        } else if ( qname == "shader" ) {
            string sname = attr_list.get("name") ;
            string stype = attr_list.get("type") ;
            string src = attr_list.get("src") ;
            bundles_.back().shaders_.emplace_back(sname, stype, src) ;
        }
    }
    void endElement(const std::string &qname) {

    }

    void characters(const std::string &text_data) {
    }

    virtual void error(ErrorCode code, uint line, uint column ) {
        std::cout << "error near line: " << line << ", col: " << column << ", code: " << code << std::endl ;
    }

    vector<ShaderBundleConfig> bundles_ ;
};

std::string make_c_string( const string & in) {
    string out;
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if ('"' == c)
            out += "\\\"";
        else if ('\\' == c)
            out += "\\\\";
        else
            out += c;
    }
    return out ;
}

void write_quoted_string(ostream &strm, const string &str) {
    strm << "\"" << str << "\"";
}


int main(int argc, char** args) {
    if (argc < 4) {
        printf("syntax error, usage:  glsl2src rootdir cfgfile outfile");
        exit(0xff);
    }

    string root = args[1];
    string cfg = args[2] ;
    string out = args[3] ;

    ifstream strm(cfg) ;
    BundleXml config(strm) ;
    if ( !config.parse() ) {
        cerr << "Error parsing config file: "  << cfg << endl ;
        exit(1) ;
    }

    ofstream ostrm(out) ;

    for( const auto &b: config.bundles_ ) {

    }



}
