#ifndef __VSIM_OPENGL_SHADERS_HPP__
#define __VSIM_OPENGL_SHADERS_HPP__

#include <GL/glew.h>
#include <string>
#include <stdexcept>
#include <memory>
#include <vector>
#include <map>

#include <Eigen/Core>
#include <boost/shared_ptr.hpp>

namespace vsim {
namespace renderer {

class OpenGLShader {
public:
    typedef std::shared_ptr<OpenGLShader> Ptr ;

    enum Type { Vertex = GL_VERTEX_SHADER,  Fragment = GL_FRAGMENT_SHADER, Geometry = GL_GEOMETRY_SHADER,
                Compute = GL_COMPUTE_SHADER, TessControl = GL_TESS_CONTROL_SHADER, TessEvaluation = GL_TESS_EVALUATION_SHADER
    } ;

    OpenGLShader(Type t): type_(t) {}

    // Compile shader from source code string. A resource name may be passed to be able to identify the code in error messages
    void compileString(const std::string &code, const std::string &resource_name = std::string()) ;
    // Loads the code from the designated file and calls compile string. If no resource name the filename will be used.
    void compileFile(const std::string &fileName, const std::string &resource_name = std::string()) ;

    OpenGLShader(Type t, const std::string &code) ;
    ~OpenGLShader() ;

    GLuint handle() const { return handle_; }

private:
    GLuint handle_ ;
    Type type_ ;
};


class OpenGLShaderProgram {
public:

    typedef std::shared_ptr<OpenGLShaderProgram> Ptr ;

    OpenGLShaderProgram() ;

    void addShader(const OpenGLShader::Ptr &shader) ;
    void addShaderFromString(OpenGLShader::Type t, const std::string &code, const std::string &resource_name = std::string()) ;
    void addShaderFromFile(OpenGLShader::Type t, const std::string &fname, const std::string &resource_name = std::string()) ;

    void link() ;
    void use() { glUseProgram(handle_) ; }

    int attributeLocation(const std::string &attr_name) ;
    void bindAttributeLocation(const std::string &attr_name, int loc) ;

    int uniformLocation(const std::string &uni_name) ;

    OpenGLShaderProgram(const char *vshader, const char *fshader) ;

    void setUniform(const std::string &name, float v) ;
    void setUniform(const std::string &name, GLint v) ;
    void setUniform(const std::string &name, GLuint v) ;
    void setUniform(const std::string &name, const Eigen::Vector3f &v) ;
    void setUniform(const std::string &name, const Eigen::Vector4f &v) ;
    void setUniform(const std::string &name, const Eigen::Matrix3f &v) ;
    void setUniform(const std::string &name, const Eigen::Matrix4f &v) ;

    ~OpenGLShaderProgram() ;

private:

    void throwError(const char *error_str, const char *error_desc) ;

    GLuint handle_ ;
    std::vector<OpenGLShader::Ptr> shaders_ ;
};


class OpenGLShaderError: public std::runtime_error {
public:
    OpenGLShaderError(const std::string &msg):
        std::runtime_error(msg), id_(glGetError()) {}

    GLenum glError() const { return id_ ; }

private:
    GLenum id_ ;
};


class OpenGLShaderLibrary {
public:
    OpenGLShaderLibrary() = default ;

    struct ShaderConfig {
        std::string id_, type_, path_, src_ ;
    };

    struct ProgramConfig {
        std::string id_ ;
        std::vector<std::string> shaders_ ;
    };

    void build(const std::vector<ShaderConfig> &shaders, const std::vector<ProgramConfig> &programs) ;

private:

    std::map<std::string, OpenGLShader::Ptr> shaders_ ;
    std::map<std::string, OpenGLShaderProgram::Ptr> programs_ ;
};

} // namespace renderer
} // namespace vsim

#endif // GLSL_HPP
