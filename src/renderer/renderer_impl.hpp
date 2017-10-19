#ifndef __VSIM_RENDERER_IMPL_HPP__
#define __VSIM_RENDERER_IMPL_HPP__

#include <memory>

#include <vsim/env/scene.hpp>
#include <vsim/renderer/ogl_shaders.hpp>
#include <vsim/renderer/renderer.hpp>

#include <GL/glew.h>

#include "ftgl/texture-font.h"
#include "ftgl/texture-atlas.h"

namespace vsim { namespace renderer {

class RendererImpl {
public:

    RendererImpl(const ScenePtr &scene): scene_(scene) {}
    ~RendererImpl() ;

    // initialize renderer
    bool init() ;
    void setBackgroundColor(const Eigen::Vector4f &clr) { bg_clr_ = clr ; }


    static const int MAX_TEXTURES = 4 ;

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

    struct FontData {
        ftgl::texture_atlas_t *atlas_ = nullptr ;
        ftgl::texture_font_t *font_ = nullptr ;
        GLuint texture_id_ = 0 ;

        ~FontData() {
            if ( font_ )
                ftgl::texture_font_delete(font_) ;
            if ( atlas_ )
                ftgl::texture_atlas_delete(atlas_) ;
        }
    };


    void makeVertexBuffers(const ScenePtr &scene) ;
    void makeVertexBuffers(const ModelPtr &model) ;

    void clear(MeshData &data);
    void initBuffersForMesh(MeshData &data, Mesh &mesh) ;

    void render(const Camera &cam, Renderer::RenderMode mode) ;
    void render(const NodePtr &node, const Camera &cam, const Eigen::Matrix4f &mat, Renderer::RenderMode mode) ;
    void render(const GeometryPtr &geom, const Camera &cam, const Eigen::Matrix4f &mat, Renderer::RenderMode mode) ;
    void render(const ModelPtr &model, const Camera &cam, const Eigen::Matrix4f &tf, Renderer::RenderMode mode);

    void setModelTransform(const Eigen::Matrix4f &tf);
    void setMaterial(const MaterialPtr &material) ;
    void setProgram(Renderer::RenderMode rm) ;

    void setLights(const ScenePtr &scene) ;
    void setLights(const ModelPtr &model) ;
    void setLights(const std::vector<LightPtr> &lights) ;
    void initTextures(const ScenePtr &scene) ;
    void initTextures(const ModelPtr &scene) ;
    void initTexture(const MaterialPtr &mat) ;
    void renderText(const std::string &text, float x, float y) ;
    void initFontData() ;

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
    OpenGLShaderProgram::Ptr text_prog_ ;
    FontData font_data_ ;
    uint light_index_ = 0 ;
} ;


}}

#endif
