#ifndef __VSIM_RENDERER_HPP__
#define __VSIM_RENDERER_HPP__

#include <memory>
#include <map>
#include <Eigen/Core>

#include <vsim/renderer/ogl_shaders.hpp>

namespace vsim { namespace renderer {


class Renderer {
public:

    typedef std::shared_ptr<Renderer> Ptr ;

    Renderer() ;
    ~Renderer() ;

    bool init() ;

//    void render(const Camera &cam, RenderMode mode) ;

  //  cv::Mat getColor(bool alpha = true);
  //  cv::Mat getColor(cv::Mat &bg, float alpha);
  //  cv::Mat getDepth();

private:

    static const int MAX_TEXTURES = 10 ;
    enum VB_TYPES {
        INDEX_BUFFER,
        POS_VB,
        NORMAL_VB,
        COLOR_VB,
        TEXCOORD_VB,
        BONE_VB = TEXCOORD_VB + MAX_TEXTURES,
        TF_VB,
        NUM_VBs
    };

    struct MeshData {
        MeshData() ;
        GLuint buffers_[10];
        GLuint texture_id_, vao_ ;
        GLuint elem_count_ ;
    };

//    void release() ;
//    void clear(MeshData &data);
//    void init_buffers_for_mesh(MeshData &data, Mesh &mesh) ;
 //   void init_buffers_for_skinning(MeshData &data, SkinningModifier &a) ;
//    void render(const NodePtr &node, const Camera &cam, const Eigen::Matrix4f &mat, RenderMode mode) ;
//    void render(const GeometryPtr &geom, const Camera &cam, const Eigen::Matrix4f &mat, RenderMode mode) ;
//    void set_model_transform(const Eigen::Matrix4f &tf);
//    void set_material(const MaterialPtr &material) ;
//    void set_program(RenderMode rm) ;
//    void set_lights() ;
//    void init_textures() ;
//    void set_bone_transforms(const SkinningModifierPtr &sk) ;
//    bool importAssimpRecursive(const struct aiScene *sc, const std::string &path, const struct aiNode* nd, const Eigen::Matrix4f &ptf) ;

private:


    OpenGLShaderLibrary shaders_ ;
/*
    std::map<MeshPtr, MeshData> buffers_ ;
    std::map<std::string, GLuint> textures_ ;
    ScenePtr scene_ ;
    Eigen::Matrix4f perspective_, proj_ ;
    GLuint query_ ;
    Eigen::Vector4f bg_clr_ ;
    float znear_, zfar_ ;
    MaterialPtr default_material_ ;
    */
} ;



class RenderingContext {
public:
    RenderingContext(uint32_t width, uint32_t height): width_(width), height_(height) {}
    virtual ~RenderingContext() { }

    virtual void attach() = 0;
    virtual void detach() = 0;

    uint32_t width_, height_ ;
};


}}

#endif
