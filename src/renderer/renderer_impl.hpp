#ifndef __VSIM_RENDERER_IMPL_HPP__
#define __VSIM_RENDERER_IMPL_HPP__

#include <memory>

#include <vsim/renderer/scene.hpp>
#include <vsim/renderer/ogl_shaders.hpp>
#include <vsim/renderer/renderer.hpp>

#include <GL/glew.h>

namespace vsim { namespace renderer {

class RendererImpl {
public:

    RendererImpl(const ScenePtr &scene): scene_(scene) {}
    ~RendererImpl() ;

    // initialize renderer
    bool init() ;
    void setBackgroundColor(const Eigen::Vector4f &clr) { bg_clr_ = clr ; }


    static const int MAX_TEXTURES = 10 ;

    enum VB_TYPES {
        INDEX_BUFFER,
        POS_VB,
        NORMAL_VB,
        COLOR_VB,
        TEXCOORD_VB,
        TF_VB = TEXCOORD_VB + MAX_TEXTURES,
        NUM_VBs
    };



    struct MeshData {
        MeshData() ;
        GLuint buffers_[10];
        GLuint texture_id_, vao_ ;
        GLuint elem_count_ ;
    };


    void clear(MeshData &data);
    void initBuffersForMesh(MeshData &data, Mesh &mesh) ;

    void render(const Camera &cam, Renderer::RenderMode mode) ;
    void render(const NodePtr &node, const Camera &cam, const Eigen::Matrix4f &mat, Renderer::RenderMode mode) ;
    void render(const GeometryPtr &geom, const Camera &cam, const Eigen::Matrix4f &mat, Renderer::RenderMode mode) ;
    void setModelTransform(const Eigen::Matrix4f &tf);
    void setMaterial(const MaterialPtr &material) ;
    void setProgram(Renderer::RenderMode rm) ;
    void setLights() ;
    void initTextures() ;

private:

    OpenGLShaderLibrary shaders_ ;

    std::map<MeshPtr, MeshData> buffers_ ;
    std::map<std::string, GLuint> textures_ ;
    ScenePtr scene_ ;
    Eigen::Matrix4f perspective_, proj_ ;
    GLuint query_ ;
    Eigen::Vector4f bg_clr_= { 0, 0, 0, 1 } ;
    float znear_, zfar_ ;
    MaterialPtr default_material_ ;
    OpenGLShaderProgram::Ptr prog_ ;

} ;


}}

#endif
