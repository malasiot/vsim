#include "glsl.hpp"

#include <cstring>
#include <fstream>

using namespace std ;
using namespace Eigen ;

namespace vsim { namespace renderer {

OpenGLShader::OpenGLShader(Type t, const std::string &code): type_(t) {
    compileString(code) ;
}

void OpenGLShader::compileString(const std::string &code, const string &resource_name) {
    if ( ( handle_ = glCreateShader(GLenum(type_)) ) == 0 )
        throw Error("cannot create shader") ;

    const GLchar* p[1];
    p[0] = code.c_str() ;
    GLint lengths[1] = { (GLint)code.length() };

    glShaderSource(handle_, 1, p, lengths);
    glCompileShader(handle_);

    GLint success;
    glGetShaderiv(handle_, GL_COMPILE_STATUS, &success);

    if ( !success ) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(handle_, 1024, NULL, InfoLog);

        stringstream estrm ;
        estrm << "Error compiling shader " ;
        if ( !resource_name.empty() ) estrm << "(" << resource_name << ")" ;
        estrm << ": " << InfoLog ;

        throw Error(estrm.str());
    }
}


static string basename( const string &pathname ) {
    return std::string(
        std::find_if( pathname.rbegin(), pathname.rend(), [] (char ch){ return ch == '\\' || ch == '/'; }).base(),
        pathname.end() );
}

static string get_file_contents(const std::string &fname) {
    ifstream strm(fname) ;

    strm.seekg(0, ios::end);
    size_t length = strm.tellg();
    strm.seekg(0,std::ios::beg);

    string res ;
    res.resize(length) ;
    strm.read(&res[0], length) ;

    return res ;
}

void OpenGLShader::compileFile(const std::string &fname, const string &resource_name) {

    string contents = get_file_contents(fname) ;

    if ( contents.empty() ) {
        stringstream estrm ;
        estrm << "Error reading shader file " ;
        if ( !resource_name.empty() ) estrm << "(" << resource_name << ")" ;
        estrm << ": " << fname ;
        throw Error(estrm.str());
    }

    if ( resource_name.empty() )
        compileString(contents, basename(fname)) ;
    else
        compileString(contents, resource_name) ;
}

OpenGLShader::~OpenGLShader() {
    glDeleteShader(handle_) ;
}

OpenGLShaderProgram::OpenGLShaderProgram(const char *vshader_code, const char *fshader_code)
{
    handle_ = glCreateProgram();

    OpenGLShader::Ptr vertex_shader = make_shared<OpenGLShader>(OpenGLShader::Vertex, vshader_code) ;
    OpenGLShader::Ptr fragment_shader = make_shared<OpenGLShader>(OpenGLShader::Fragment, fshader_code) ;

    shaders_.push_back(vertex_shader) ;
    shaders_.push_back(fragment_shader) ;

    link();
}
/*
OpenGLShaderProgram::OpenGLShaderProgram(std::initializer_list<ShaderPtr> &shaders)
{
    handle_ = glCreateOpenGLShaderProgram();
    std::copy(shaders.begin(), shaders.end(), std::back_inserter(shaders_)) ;
    link_and_validate_OpenGLShaderProgram() ;
}
*/
void OpenGLShaderProgram::setUniform(const string &name, float v)
{
    GLint loc = glGetUniformLocation(handle_, name.c_str()) ;
    if ( loc != -1 ) glUniform1f(loc, v) ;
}

void OpenGLShaderProgram::setUniform(const string &name, GLuint v)
{
    GLint loc = glGetUniformLocation(handle_, name.c_str()) ;
    if ( loc != -1 ) glUniform1ui(loc, v) ;
}

void OpenGLShaderProgram::setUniform(const string &name, GLint v)
{
    GLint loc = glGetUniformLocation(handle_, name.c_str()) ;
    if ( loc != -1 ) glUniform1i(loc, v) ;
}

void OpenGLShaderProgram::setUniform(const string &name, const Vector3f &v)
{
    GLint loc = glGetUniformLocation(handle_, name.c_str()) ;
    if ( loc != -1 ) glUniform3fv(loc, 1, v.data()) ;
}

void OpenGLShaderProgram::setUniform(const string &name, const Vector4f &v)
{
    GLint loc = glGetUniformLocation(handle_, name.c_str()) ;
    if ( loc != -1 ) glUniform4fv(loc, 1, v.data()) ;
}

void OpenGLShaderProgram::setUniform(const string &name, const Matrix3f &v)
{
    GLint loc = glGetUniformLocation(handle_, name.c_str()) ;
    if ( loc != -1 ) glUniformMatrix3fv(loc, 1, GL_FALSE, v.data()) ;
}

void OpenGLShaderProgram::setUniform(const string &name, const Matrix4f &v)
{
    GLint loc = glGetUniformLocation(handle_, name.c_str()) ;
    if ( loc != -1 ) glUniformMatrix4fv(loc, 1, GL_FALSE, v.data()) ;
}

OpenGLShaderProgram::~OpenGLShaderProgram() {
    glDeleteProgram(handle_) ;
}

void OpenGLShaderProgram::addShader(const OpenGLShader::Ptr &shader) {
    shaders_.push_back(shader) ;
}

void OpenGLShaderProgram::addShaderFromString(OpenGLShader::Type t, const string &code, const string &resource_name) {
    auto shader = std::make_shared<OpenGLShader>(t) ;
    shader->compileString(code, resource_name) ;
    addShader(shader) ;
}

void OpenGLShaderProgram::addShaderFromFile(OpenGLShader::Type t, const string &fname, const string &resource_name) {
    auto shader = std::make_shared<OpenGLShader>(t) ;
    shader->compileFile(fname, resource_name) ;
    addShader(shader) ;
}


void OpenGLShaderProgram::link() {

    /*

    const GLchar* feedbackVaryings[] = { "gl_Position" };
    glTransformFeedbackVaryings(id, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
*/

    for( auto &&shader: shaders_ )
        glAttachShader(handle_, shader->handle()) ;

    GLchar error_log[1024] = { 0 };

    glLinkProgram(handle_) ;

    GLint success;
    glGetProgramiv(handle_, GL_LINK_STATUS, &success);

    if ( success == 0 ) {
        glGetProgramInfoLog(handle_, sizeof(error_log), NULL, error_log);
        throwError("Error linking shader program", error_log) ;
    }

    glValidateProgram(handle_);
    glGetProgramiv(handle_, GL_VALIDATE_STATUS, &success);

    if ( !success ) {
        glGetProgramInfoLog(handle_, sizeof(error_log), NULL, error_log);
        throwError("Invalid shader program", error_log);
    }
}

void OpenGLShaderProgram::throwError(const char *error_str, const char *error_desc) {
    stringstream estrm ;
    estrm << error_str << ':' << error_desc << endl ;
    throw Error(estrm.str()) ;
}

} // namespace renderer
} // namespace vsim
