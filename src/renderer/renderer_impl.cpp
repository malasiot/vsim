#include "renderer_impl.hpp"
#include "tools.hpp"

#include <iostream>
#include <cstring>

#include <Eigen/Dense>

#include <fstream>
#include <memory>

#include <vsim/util/format.hpp>
#include "soil/SOIL.h"

using namespace std ;
using namespace Eigen ;

extern vector<vsim::renderer::OpenGLShaderLibrary::ShaderConfig> vsim_opengl_shaders_library_shaders ;
extern vector<vsim::renderer::OpenGLShaderLibrary::ProgramConfig> vsim_opengl_shaders_library_programs ;

namespace vsim { namespace renderer {

bool RendererImpl::init() {

    // this is needed for non core profiles or instead use gl3w
    glewExperimental = GL_TRUE ;

    GLenum err ;
    if ( ( err = glewInit() ) != GLEW_OK ) {
        cerr << glewGetErrorString(err) << endl;
        return false ;
    }

    // init stock shaders

    try {
        shaders_.build(vsim_opengl_shaders_library_shaders,
                       vsim_opengl_shaders_library_programs) ;
    }
    catch ( const OpenGLShaderError &e ) {
        cerr << e.what() << endl ;
        return false ;
    }

    // create vertex buffers

    for( uint m=0 ; m<scene_->meshes_.size() ; m++ )
    {
        MeshData data ;
        MeshPtr mesh = scene_->meshes_[m] ;
        initBuffersForMesh(data, *scene_->meshes_[m]);
        buffers_[mesh] = data ;
    }

    // load textures

    initTextures() ;

    default_material_.reset(new Material) ;

    default_material_->type_ = Material::PHONG ;
    default_material_->ambient_.set<Vector4f>(0.0, 0.0, 0.0, 1) ;
    default_material_->diffuse_.set<Vector4f>(0.5, 0.5, 0.5, 1) ;

    return true ;
}

void RendererImpl::clear(MeshData &data) {
    glDeleteVertexArrays(1, &data.vao_) ;
}

#define POSITION_LOCATION    0
#define NORMALS_LOCATION    1
#define COLORS_LOCATION    2
#define BONE_ID_LOCATION    3
#define BONE_WEIGHT_LOCATION    4
#define UV_LOCATION 5

RendererImpl::MeshData::MeshData() {}

void RendererImpl::initBuffersForMesh(MeshData &data, Mesh &mesh)
{
    // Create the VAO
    glGenVertexArrays(1, &data.vao_);
    glBindVertexArray(data.vao_);

    vector<Vector3f> vertices, normals, colors ;
    vector<Vector2f> tex_coords[MAX_TEXTURES] ;
    vector<GLuint> indices ;

    flatten_mesh(mesh, vertices, normals, colors, tex_coords) ;
    data.elem_count_ = vertices.size() ;

    glGenBuffers(1, &data.buffers_[POS_VB]);
    glBindBuffer(GL_ARRAY_BUFFER, data.buffers_[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat) * 3, &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    if ( !normals.empty() ) {
        glGenBuffers(1, &data.buffers_[NORMAL_VB]);
        glBindBuffer(GL_ARRAY_BUFFER, data.buffers_[NORMAL_VB]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat) * 3, (GLfloat *)normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(NORMALS_LOCATION);
        glVertexAttribPointer(NORMALS_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    if ( !colors.empty() ) {
        glGenBuffers(1, &data.buffers_[COLOR_VB]);
        glBindBuffer(GL_ARRAY_BUFFER, data.buffers_[COLOR_VB]);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat) * 3, (GLfloat *)colors.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(COLORS_LOCATION);
        glVertexAttribPointer(COLORS_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    for( uint t = 0 ; t<MAX_TEXTURES ; t++ ) {
        if ( !tex_coords[t].empty() ) {
            glGenBuffers(1, &data.buffers_[TEXCOORD_VB + t]);
            glBindBuffer(GL_ARRAY_BUFFER, data.buffers_[TEXCOORD_VB + t]);
            glBufferData(GL_ARRAY_BUFFER, tex_coords[t].size() * sizeof(GLfloat) * 2, (GLfloat *)tex_coords[t].data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(UV_LOCATION + t);
            glVertexAttribPointer(UV_LOCATION + t, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        }
    }

    if ( !indices.empty() ) {
        glGenBuffers(1, &data.buffers_[INDEX_BUFFER]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.buffers_[INDEX_BUFFER]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    }

    glGenBuffers(1, &data.buffers_[TF_VB]);
    glBindBuffer(GL_ARRAY_BUFFER, data.buffers_[TF_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat) * 3, 0, GL_STATIC_READ);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static Matrix4f perspective(float fovy, float aspect, float zNear, float zFar)	{
    assert(abs(aspect - std::numeric_limits<float>::epsilon()) > static_cast<float>(0));

    float const d = 1/tan(fovy / static_cast<float>(2));

    Matrix4f Result ;
    Result.setZero() ;

    Result(0, 0) = d / aspect ;
    Result(1, 1) = d ;
    Result(2, 2) =  (zFar + zNear) / (zNear - zFar);
    Result(2, 3) =  2 * zFar * zNear /(zNear - zFar) ;
    Result(3, 2) = -1 ;

    return Result;
}

void RendererImpl::render(const Camera &cam, Renderer::RenderMode mode) {

    glEnable(GL_DEPTH_TEST) ;

    glEnable(GL_CULL_FACE) ;
    glCullFace(GL_BACK) ;
    glFrontFace(GL_CCW) ;
    glClearColor(bg_clr_.x(), bg_clr_.y(), bg_clr_.z(), bg_clr_.w()) ;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if ( cam.type() == Camera::Perspective ) {
        const PerspectiveCamera &pcam = static_cast<const PerspectiveCamera &>(cam) ;
        perspective_ = pcam.projectionMatrix() ;
        znear_ = pcam.zNear() ;
        zfar_ = pcam.zFar() ;
    }

    const Viewport &vp = cam.getViewport() ;
    glViewport(vp.x_, vp.y_, vp.width_, vp.height_);

    proj_ = cam.getViewMatrix() ;

    // render node hierarchy

    for( uint i=0 ; i<scene_->nodes_.size() ; i++ ) {
        const NodePtr &n = scene_->nodes_[i] ;
        render(n, cam, Matrix4f::Identity(), mode) ;
    }
}

void RendererImpl::render(const NodePtr &node, const Camera &cam, const Matrix4f &tf, Renderer::RenderMode mode) {
    Matrix4f mat = node->mat_,
            tr = tf * mat ; // accumulate transform

    for( uint i=0 ; i<node->geometries_.size() ; i++ ) {
        const GeometryPtr &m = node->geometries_[i] ;
        render(m, cam, tr, mode) ;
    }

    for( uint i=0 ; i<node->children_.size() ; i++ ) {
        const NodePtr &n = node->children_[i] ;
        render(n, cam, tr, mode) ;
    }
}


RendererImpl::~RendererImpl() {

}

void RendererImpl::setModelTransform(const Matrix4f &tf)
{
    Matrix4f mvp =  perspective_ * proj_ * tf;
    Matrix4f mv =   proj_ * tf;
    Matrix3f wp = mv.block<3, 3>(0, 0).transpose().inverse() ;

    prog_->setUniform("gProj", mvp) ;
    prog_->setUniform("gModel", mv) ;
    prog_->setUniform("gNormal", wp) ;
}

void RendererImpl::setMaterial(const MaterialPtr &material)
{
    if ( material->ambient_.is<Vector4f>() ) {
        Vector4f clr = material->ambient_.get<Vector4f>() ;
        prog_->setUniform("g_material.ambient", clr) ;
    }
    else
        prog_->setUniform("g_material.ambient", Vector4f(0, 0, 0, 1)) ;

    if (  material->diffuse_.is<Vector4f>() ) {
        Vector4f clr = material->diffuse_.get<Vector4f>() ;
        prog_->setUniform("g_material.diffuse", clr) ;
        prog_->setUniform("g_material.diffuse_map", false) ;
    }
    else if ( material->diffuse_.is<Sampler2D>() ) {
        Sampler2D &ts = material->diffuse_.get<Sampler2D>() ;

        prog_->setUniform("texUnit", 0) ;
        prog_->setUniform("g_material.diffuse_map", true) ;

        if ( textures_.count(ts.image_url_) ) {
            glBindTexture(GL_TEXTURE_2D, textures_[ts.image_url_]) ;
        }
    }
    else
        prog_->setUniform("g_material.diffuse", Vector4f(0.5, 0.5, 0.5, 1)) ;


    if (  material->specular_.is<Vector4f>() ) {
        Vector4f clr = material->specular_.get<Vector4f>() ;
        prog_->setUniform("g_material.specular", clr) ;
    }
    else
        prog_->setUniform("g_material.specular", Vector4f(0.5, 0.5, 0.5, 1)) ;

    if (  material->shininess_.is<Vector4f>() )
        prog_->setUniform("g_material.shininess", material->shininess_.get<Vector4f>()) ;
    else
        prog_->setUniform("g_material.shininess", 0) ;
}

#define MAX_LIGHTS 10

void RendererImpl::setLights()
{
    for( uint i=0 ; i< MAX_LIGHTS ; i++  ) {

        string vname = util::format("g_light_source[%]", i) ;

        if ( i >= scene_->lights_.size() ) {
            prog_->setUniform(vname + ".light_type", -1) ;
            continue ;
        }

        const LightPtr &light = scene_->lights_[i] ;

        if ( light->type_ == Light::AMBIENT ) {
            const AmbientLight &alight = (const AmbientLight &)*light ;

            prog_->setUniform(vname + ".light_type", 0) ;
            prog_->setUniform(vname + ".color", alight.color_) ;
        }
        else if ( light->type_ == Light::DIRECTIONAL ) {
            const DirectionalLight &dlight = (const DirectionalLight &)*light ;

            prog_->setUniform(vname + ".light_type", 1) ;
            prog_->setUniform(vname + ".color", dlight.color_) ;
            prog_->setUniform(vname + ".direction", dlight.direction_) ;
        }
        else if ( light->type_ == Light::SPOT) {
            const SpotLight &slight = (const SpotLight &)*light ;

            prog_->setUniform(vname + ".light_type", 2) ;
            prog_->setUniform(vname + ".color", slight.color_) ;
            prog_->setUniform(vname + ".direction", slight.direction_) ;
            prog_->setUniform(vname + ".position", slight.position_) ;
            prog_->setUniform(vname + ".constant_attenuation", slight.constant_attenuation_) ;
            prog_->setUniform(vname + ".linear_attenuation", slight.linear_attenuation_) ;
            prog_->setUniform(vname + ".quadratic_attenuation", slight.quadratic_attenuation_) ;
            prog_->setUniform(vname + ".spot_exponent", slight.falloff_exponent_) ;
            prog_->setUniform(vname + ".spot_cos_cutoff", (float)cos(M_PI*slight.falloff_angle_/180.0)) ;
        }
        else if ( light->type_ == Light::POINT) {
            const PointLight &plight = (const PointLight &)*light ;

            prog_->setUniform(vname + ".light_type", 3) ;
            prog_->setUniform(vname + ".color", plight.color_) ;
            prog_->setUniform(vname + ".position", plight.position_) ;
            prog_->setUniform(vname + ".constant_attenuation", plight.constant_attenuation_) ;
            prog_->setUniform(vname + ".linear_attenuation", plight.linear_attenuation_) ;
            prog_->setUniform(vname + ".quadratic_attenuation", plight.quadratic_attenuation_) ;
        }
    }

}

void RendererImpl::initTextures()
{
    for( uint i=0 ; i<scene_->materials_.size() ; i++ ) {

        MaterialPtr &m = scene_->materials_[i] ;

        if ( m->diffuse_.is<Sampler2D>() ) {
            Sampler2D &texture = m->diffuse_.get<Sampler2D>() ;

            glActiveTexture(GL_TEXTURE0);
            GLuint texture_id = SOIL_load_OGL_texture(texture.image_url_.c_str(),
                SOIL_LOAD_RGB,
                SOIL_CREATE_NEW_ID,
                SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS
              //| SOIL_FLAG_MULTIPLY_ALPHA
              //| SOIL_FLAG_COMPRESS_TO_DXT
                | SOIL_FLAG_DDS_LOAD_DIRECT
              //| SOIL_FLAG_NTSC_SAFE_RGB
              //| SOIL_FLAG_CoCg_Y
              //| SOIL_FLAG_TEXTURE_RECTANGLE
            );

            if ( texture_id ) {
                // Set texture clamping method
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

                textures_[texture.image_url_] = texture_id ;
            }
        }

    }
}


void RendererImpl::render(const GeometryPtr &geom, const Camera &cam, const Matrix4f &mat, Renderer::RenderMode mode)
{
    MeshData &data = buffers_[geom->mesh_] ;

    if ( mode == Renderer::RENDER_FLAT )
        prog_ = shaders_.get("rigid_flat") ;
    else if ( mode == Renderer::RENDER_SMOOTH )
        prog_ = shaders_.get("rigid_smooth") ;
    else if ( mode == Renderer::RENDER_GOURAUD )
        prog_ = shaders_.get("rigid_gouraud") ;

    assert( prog_ ) ;

    prog_->use() ;

    setModelTransform(mat) ;

    if ( geom->material_) setMaterial(geom->material_) ;
    else setMaterial(default_material_) ;

    setLights() ;

#if 0
    glBindVertexArray(data.vao_);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, data.buffers_[TF_VB]);

    glEnable(GL_RASTERIZER_DISCARD);



    glBeginTransformFeedback(GL_TRIANGLES);


    glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query_ );
    glDrawArrays(GL_TRIANGLES, 0, data.elem_count_) ;
    glEndQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN );
    glEndTransformFeedback();

    GLuint primitives_written = 0 ;
    glGetQueryObjectuiv( query_, GL_QUERY_RESULT, &primitives_written );



    glDisable(GL_RASTERIZER_DISCARD);

    glFlush();

    vector<GLfloat> fdata(36, 0) ;

    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 36*sizeof(GLfloat), fdata.data());

    glBindVertexArray(0) ;
#else
    glBindVertexArray(data.vao_);
    glDrawArrays(GL_TRIANGLES, 0, data.elem_count_) ;
    glBindVertexArray(0) ;

    glFlush();
#endif
    glUseProgram(0) ;
}
/*
cv::Mat RendererImpl::getColor(bool alpha)
{
    if ( alpha )
    {
        cv::Mat_<cv::Vec4b> mask(ctx_->height_, ctx_->width_) ;
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, ctx_->width_, ctx_->height_, GL_RGBA, GL_UNSIGNED_BYTE, mask.ptr());

        cv::flip(mask, mask, 0) ;

        cv::cvtColor(mask, mask, CV_RGBA2BGRA);

        return mask ;
    }
    else
    {
        cv::Mat_<cv::Vec3b> mask(ctx_->height_, ctx_->width_) ;
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, ctx_->width_, ctx_->height_, GL_RGB, GL_UNSIGNED_BYTE, mask.ptr());

        cv::flip(mask, mask, 0) ;

        cv::cvtColor(mask, mask, CV_RGB2BGR);

        return mask ;
    }
}

cv::Mat RendererImpl::getColor(cv::Mat &bg, float alpha)
{
    cv::Mat_<cv::Vec3b> mask(ctx_->height_, ctx_->width_) ;
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, ctx_->width_, ctx_->height_, GL_RGB, GL_UNSIGNED_BYTE, mask.ptr());
    cv::flip(mask, mask, 0) ;
    cv::cvtColor(mask, mask, CV_RGB2BGR);

    cv::Mat dst ;
    cv::addWeighted( mask, alpha, bg, (1 - alpha), 0.0, dst);

    return dst ;
}

cv::Mat RendererImpl::getDepth() {

    cv::Mat_<float> depth(ctx_->height_, ctx_->width_);

    glReadBuffer(GL_DEPTH_ATTACHMENT);

    glReadPixels(0, 0, ctx_->width_, ctx_->height_, GL_DEPTH_COMPONENT, GL_FLOAT, depth.ptr());

    cv::Mat_<float>::iterator it = depth.begin(), end = depth.end();
    float max_allowed_z = zfar_ * 0.99;

    unsigned int i_min = ctx_->width_, i_max = 0, j_min = ctx_->height_, j_max = 0;

    for (unsigned int j = 0; j < ctx_->height_; ++j)
        for (unsigned int i = 0; i < ctx_->width_; ++i, ++it)
        {
            //need to undo the depth buffer mapping
            //http://olivers.posterous.com/linear-depth-in-glsl-for-real
            float z  = 2 * zfar_ * znear_ / (zfar_ + znear_ - (zfar_ - znear_) * (2 * (*it) - 1));

            if (z > max_allowed_z) *it = 0;
            else {
                *it = z ;
            }
        }
    // Rescale the depth to be in millimeters
    cv::Mat depth_scale(cv::Size(ctx_->width_, ctx_->height_), CV_16UC1);
    depth.convertTo(depth_scale, CV_16UC1, 1e3);

    cv::flip(depth_scale, depth_scale, 0) ;

    // cv::imwrite("/tmp/dmap.png", vl::depthViz(depth_scale)) ;
    return depth_scale;

}
#endif
*/

}}