#include "renderer_impl.hpp"
#include "tools.hpp"

#include <iostream>
#include <cstring>

#include <Eigen/Dense>

#include <fstream>
#include <memory>

#include <vsim/util/format.hpp>
#include <vsim/env/material.hpp>
#include <vsim/env/camera.hpp>
#include <vsim/env/node.hpp>
#include <vsim/env/model.hpp>
#include <vsim/env/drawable.hpp>
#include <vsim/env/geometry.hpp>
#include <vsim/env/environment.hpp>
#include <vsim/env/light.hpp>

#include <FreeImage.h>

#include "ftgl/texture-font.h"

using namespace std ;
using namespace Eigen ;

extern vector<vsim::renderer::OpenGLShaderLibrary::ShaderConfig> vsim_opengl_shaders_library_shaders ;
extern vector<vsim::renderer::OpenGLShaderLibrary::ProgramConfig> vsim_opengl_shaders_library_programs ;

namespace vsim { namespace renderer {


void RendererImpl::makeVertexBuffers(const ScenePtr &scene) {

/*    for( MeshPtr mesh: scene->meshes_ ) {
        MeshData data ;
        initBuffersForMesh(data, *mesh);
        buffers_[mesh] = data ;
    }

    for( ModelPtr m: scene->models_ )
        makeVertexBuffers(m);
        */
}

void RendererImpl::makeVertexBuffers(const ModelPtr &model) {
    /*
    for( MeshPtr mesh: model->meshes_ ) {
        MeshData data ;
        initBuffersForMesh(data, *mesh);
        buffers_[mesh] = data ;
    }

    for( ModelPtr child: model->children_ )
        makeVertexBuffers(child) ;
*/
}

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

    makeVertexBuffers(scene_) ;

    // load textures

    initTextures(scene_) ;

    initFontData() ;

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

    // inefficient but only done at startup
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

#if 0
    glGenBuffers(1, &data.buffers_[TF_VB]);
    glBindBuffer(GL_ARRAY_BUFFER, data.buffers_[TF_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat) * 3, 0, GL_STATIC_READ);
#endif

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RendererImpl::render(const Camera &cam, Renderer::RenderMode mode) {

    glEnable(GL_DEPTH_TEST) ;
    glDepthFunc(GL_LESS);

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

    // render model hierarchy
/*
    for( uint i=0 ; i<scene_->bodies_.size() ; i++ ) {
        const BodyPtr &b = scene_->bodies_[i] ;
        render(b->model_, cam, b->pose_.absolute(), mode) ;

    }
    */
}

void RendererImpl::render(const ModelPtr &model, const Camera &cam, const Matrix4f &tf, Renderer::RenderMode mode) {

    Matrix4f mat = model->pose_.absolute(),
            tr = tf * mat ; // accumulate transform

    for( uint i=0 ; i<model->nodes_.size() ; i++ ) {
        const NodePtr &n = model->nodes_[i] ;
        render(n, cam, tr, mode) ;
    }

    for( uint i=0 ; i<model->children_.size() ; i++ ) {
        const ModelPtr &m = model->children_[i] ;
        render(m, cam, tr, mode) ;
    }
}

void RendererImpl::render(const NodePtr &node, const Camera &cam, const Matrix4f &tf, Renderer::RenderMode mode) {
    Matrix4f mat = node->pose_.absolute(),
            tr = tf * mat ; // accumulate transform

    for( uint i=0 ; i<node->drawables_.size() ; i++ ) {
        const DrawablePtr &m = node->drawables_[i] ;
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
    if ( material->type_ == Material::CONSTANT )
        prog_->setUniform("g_material.is_constant", true) ;
    else
        prog_->setUniform("g_material.is_constant", false) ;

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
    else
        prog_->setUniform("g_material.diffuse", Vector4f(0.5, 0.5, 0.5, 1)) ;

    if ( material->texture_.valid() ) {
        Sampler2D &ts = material->texture_.get<Sampler2D>() ;

        prog_->setUniform("texUnit", 0) ;
        prog_->setUniform("g_material.diffuse_map", true) ;

        if ( textures_.count(ts.image_url_) ) {
            glBindTexture(GL_TEXTURE_2D, textures_[ts.image_url_]) ;
        }
    }

    if (  material->specular_.is<Vector4f>() ) {
        Vector4f clr = material->specular_.get<Vector4f>() ;
        prog_->setUniform("g_material.specular", clr) ;
    }
    else
        prog_->setUniform("g_material.specular", Vector4f(0.5, 0.5, 0.5, 1)) ;

    if (  material->shininess_.valid() )
        prog_->setUniform("g_material.shininess", material->shininess_.get<float>()) ;
    else
        prog_->setUniform("g_material.shininess", (float)1.0) ;
}


#define MAX_LIGHTS 10

void RendererImpl::setLights(const ScenePtr &scene) {

    light_index_ = 0 ;

    setLights(scene->environment_->lights_) ;
/*
    for( ModelPtr m: scene->models_ )
        setLights(m) ;

    for(uint i=light_index_ ; i<MAX_LIGHTS ; i++ ) {
        string vname = util::format("g_light_source[%]", i) ;
        prog_->setUniform(vname + ".light_type", -1) ;
        continue ;
    }
    */
}

void RendererImpl::setLights(const ModelPtr &model) {
    setLights(model->lights_) ;

    for( ModelPtr m: model->children_ )
        setLights(m) ;
}


void RendererImpl::setLights(const vector<LightPtr> &lights)
{

    for( uint i=0 ; i< lights.size() ; i++  ) {

        if ( light_index_ >= MAX_LIGHTS ) return ;

        string vname = util::format("g_light_source[%]", light_index_++) ;

        const LightPtr &light = lights[i] ;

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

void RendererImpl::initTextures(const ScenePtr &scene) {
    /*
    for( MaterialPtr mat: scene->materials_)
        initTexture(mat) ;

    for( ModelPtr model: scene->models_)
        initTextures(model) ;
        */
}

void RendererImpl::initTextures(const ModelPtr &model) {
    for( MaterialPtr mat: model->materials_)
        initTexture(mat) ;
}

void RendererImpl::initTexture(const MaterialPtr &m)
{

    if ( m->texture_.valid() ) {
        Sampler2D &texture = m->texture_.get<Sampler2D>() ;

        FIBITMAP* image = FreeImage_Load(
                    FreeImage_GetFileType(texture.image_url_.c_str(), 0),
                    texture.image_url_.c_str());

        if ( image ) {
            GLuint texture_id ;
            glEnable(GL_TEXTURE_2D) ;
            glActiveTexture(GL_TEXTURE0);
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Set texture clamping method
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
            glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
            glPixelStorei( GL_UNPACK_SKIP_ROWS, 0 );

            int w = FreeImage_GetWidth(image);
            int h = FreeImage_GetHeight(image);

            FIBITMAP* im_rgba = FreeImage_ConvertTo32Bits(image);

            FreeImage_Unload(image);

            glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                         0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                         GL_RGBA8,            // Internal colour format to convert to
                         w,
                         h,
                         0,                 // Border width in pixels (can either be 1 or 0)
                         GL_BGRA, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                         GL_UNSIGNED_BYTE,  // Image data type
                         (void*)FreeImage_GetBits(im_rgba)
                         );        // The actual image data itself

            glGenerateMipmap(GL_TEXTURE_2D);

            textures_[texture.image_url_] = texture_id ;

            FreeImage_Unload(im_rgba);
        }


    }
}

static void make_text_buffer_data( ftgl::texture_font_t * font,  const char *text, float x, float y, const Vector4f &color, vector<Vector3f> &vertices, vector<Vector2f> &uvs, vector<Vector4f> &colors, vector<GLuint> &indices )
{
    float r = color.x(), g = color.y(), b = color.z(), a = color.w();
    float penx = x, peny = y ;

    using namespace ftgl ;

    for( size_t i = 0 ; i<strlen(text) ; i++ ) {

        texture_glyph_t *glyph = texture_font_get_glyph( font, text + i );

        if ( glyph  ) {
            float kerning = 0.0f;
            if ( i > 0 )
                kerning = texture_glyph_get_kerning( glyph, text + i - 1 );

            penx += kerning;

            float x0  = penx + glyph->offset_x ;
            float y0  = peny + glyph->offset_y ;
            float x1  = x0 + glyph->width ;
            float y1  = y0 - glyph->height ;
            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;

            float scale = 1.0 ;
            vertices.emplace_back(x0*scale, y0*scale,0) ; uvs.emplace_back(s0,t0) ; colors.emplace_back(r,g,b,a) ;
            vertices.emplace_back(x1*scale,y0*scale,0) ; uvs.emplace_back(s1,t0) ; colors.emplace_back(r,g,b,a) ;
            vertices.emplace_back(x1*scale,y1*scale,0) ; uvs.emplace_back(s1,t1) ; colors.emplace_back(r,g,b,a) ;
            vertices.emplace_back(x0*scale,y1*scale,0) ; uvs.emplace_back(s0,t1) ; colors.emplace_back(r,g,b,a) ;

            size_t idx = 4*i ;
            indices.push_back(idx) ;
            indices.push_back(idx+1) ;
            indices.push_back(idx+2) ;
            indices.push_back(idx) ;
            indices.push_back(idx+2) ;
            indices.push_back(idx+3) ;

            penx += glyph->advance_x;
        }
    }
}

void
mat4_set_orthographic( Matrix4f &mat,
                       float left,   float right,
                       float bottom, float top,
                       float znear,  float zfar )
{
    if (left == right || bottom == top || znear == zfar)  return;

    mat.setZero() ;

    mat(0, 0) = +2.0f/(right-left);
    mat(1, 1) = +2.0f/(top-bottom);
    mat(2, 2) = +2.0f/(zfar-znear);
    mat(3, 0) = -(right+left)/(right-left);
    mat(3, 1) = -(top+bottom)/(top-bottom);
    mat(3, 2) = -(zfar+znear)/(zfar-znear);
    mat(3, 3) = 1.0f;
}

void RendererImpl::renderText(const string &text, float x, float y)
{
    Vector4f color(1, 1, 1, 1) ;

    text_prog_->use() ;

    Matrix4f projection ;

    GLint v[4];
    glGetIntegerv( GL_VIEWPORT, v );
    GLint width  = v[2];
    GLint height = v[3];

    //    glViewport(0, 0, width, height);

    projection.setIdentity() ;
    projection(0, 0) = 1.0/width ;
    projection(1, 1) = 1.0/height ;
    projection(3, 3) = 1 ;

    Affine3f model ;
    model.setIdentity();

    Affine3f view ;
    view.setIdentity();

    model.translate(Vector3f(width * x, height * y, 0.0)) ;
    model.scale(Vector3f(0.5, 0.5, 0)) ;

    text_prog_->setUniform("u_texture", 0);
    text_prog_->setUniform("u_color", color) ;
    text_prog_->setUniform("u_model", model.matrix()) ;
    text_prog_->setUniform("u_view", view.matrix()) ;
    text_prog_->setUniform("u_projection", projection) ;

    vector<Vector3f> vertices ;
    vector<Vector4f> colors ;
    vector<Vector2f> uvs ;
    vector<GLuint> indices ;

    make_text_buffer_data(font_data_.font_, text.c_str(), 0, 0, color, vertices, uvs, colors, indices);

    GLuint vao, vbo_vtx, vbo_uvs, vbo_colors, vbo_indices ;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo_vtx);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vtx);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat) * 3, &vertices[0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glGenBuffers(1, &vbo_uvs);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(GLfloat) * 2, (GLfloat *)uvs.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glGenBuffers(1, &vbo_colors);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat) * 4, (GLfloat *)colors.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    glGenBuffers(1, &vbo_indices);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo_indices );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_DYNAMIC_DRAW );

    glDisable(GL_DEPTH_TEST) ;

    glDisable(GL_CULL_FACE) ;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_data_.texture_id_);
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    /*
    for( uint i=0 ; i<vertices.size() ; i++ ) {
        Vector4f p(vertices[i].x(), vertices[i].y(), vertices[i].z(), 1.0) ;
        Vector4f o = projection * view * model * p ;
        cout << o.adjoint() << endl;
    }
    */
    if ( 0 ) {
        GLuint vbo_tf ;
        glGenBuffers(1, &vbo_tf);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_tf);

        vector<Vector4f> tf_data ;
        tf_data.resize(vertices.size()) ;
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float)* 4, nullptr, GL_STATIC_READ);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo_tf);

        glEnable(GL_RASTERIZER_DISCARD);

        glBeginTransformFeedback(GL_TRIANGLES);
        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *)0 );
        glEndTransformFeedback();

        glFlush();
        glDisable(GL_RASTERIZER_DISCARD);

        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vertices.size() * sizeof(float) * 4, tf_data.data());

        glBindVertexArray(0) ;
    }
    else {
        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *)0 );
        glFlush() ;

    }

    glDeleteVertexArrays(1, &vao) ;


}

static const string sdf_text_shader_vs = R"(
                                         #version 330

                                         layout (location = 0) in vec3 vertex;
                                         layout (location = 1) in vec2 tex_coord;
                                         layout (location = 2) in vec4 color;

                                         uniform mat4 u_model;
                                         uniform mat4 u_view;
                                         uniform mat4 u_projection;
                                         uniform vec4 u_color;

                                         out vec2 uvs ;
                                         out vec4 v_colors ;
                                         out vec4 v_position ;

                                         void main(void)
                                         {
                                         uvs = tex_coord ;
                                         v_colors = color * u_color;
                                         v_position = u_projection*(u_view*(u_model*vec4(vertex,1.0)));
                                         gl_Position = v_position ;
                                         }
                                         )" ;

static const string sdf_text_shader_fs = R"(
                                         #version 330
                                         uniform sampler2D u_texture;

                                         vec3 glyph_color    = vec3(1.0,1.0,1.0);
                                         const float glyph_center   = 0.50;
                                         vec3 outline_color  = vec3(0.0,0.0,0.0);
                                         const float outline_center = 0.55;
                                         vec3 glow_color     = vec3(1.0,1.0,1.0);
                                         const float glow_center    = 1.25;

                                         in vec2 uvs;
                                         in vec4 v_colors ;

                                         out vec4 fragColor;

                                         void main(void)
                                         {
                                         vec4  color = texture2D(u_texture, uvs);
                                         float dist  = color.r;
                                         float width = fwidth(dist);
                                         float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);

                                         // Smooth
                                         // fragColor = vec4(glyph_color, alpha);
                                         //fragColor = vec4(1, 0, 1, 1) ;
                                         // Outline
                                         // float mu = smoothstep(outline_center-width, outline_center+width, dist);
                                         // vec3 rgb = mix(outline_color, glyph_color, mu);
                                         // fragColor = vec4(rgb, max(alpha,mu));

                                         // Glow
                                         //vec3 rgb = mix(glow_color, glyph_color, alpha);
                                         //float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
                                         //gl_FragColor = vec4(rgb, max(alpha,mu));

                                         // Glow + outline
                                         vec3 rgb = mix(glow_color, glyph_color, alpha);
                                         float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
                                         color = vec4(rgb, max(alpha,mu));
                                         float beta = smoothstep(outline_center-width, outline_center+width, dist);
                                         rgb = mix(outline_color, color.rgb, beta);
                                         fragColor = vec4(rgb, max(color.a,beta));


                                         fragColor = vec4(v_colors.rgb, alpha);

                                         }
                                         )" ;

void RendererImpl::initFontData()
{

    const char *filename = "fonts/Vera.ttf";
    const char * cache = " !\"#$%&'()*+,-./0123456789:;<=>?"
                         "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                         "`abcdefghijklmnopqrstuvwxyz{|}~";

    font_data_.atlas_ = ftgl::texture_atlas_new( 512, 512, 1 );
    font_data_.font_ = ftgl::texture_font_new_from_file( font_data_.atlas_, 72, "/usr/share/fonts/truetype/roboto/hinted/Roboto-Light.ttf" );
    font_data_.font_->rendermode = ftgl::RENDER_SIGNED_DISTANCE_FIELD;

    ftgl::texture_font_load_glyphs( font_data_.font_, cache );

    glGenTextures( 1, &font_data_.texture_id_ );
    glBindTexture( GL_TEXTURE_2D, font_data_.texture_id_ );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, font_data_.atlas_->width, font_data_.atlas_->height, 0, GL_RED, GL_UNSIGNED_BYTE, font_data_.atlas_->data );

    OpenGLShader::Ptr fs(new OpenGLShader(OpenGLShader::Fragment)) ;
    fs->compileString(sdf_text_shader_fs, "sdf_texture_fs") ;

    OpenGLShader::Ptr vs(new OpenGLShader(OpenGLShader::Vertex)) ;
    vs->compileString(sdf_text_shader_vs, "sdf_texture_vs") ;

    text_prog_.reset(new OpenGLShaderProgram) ;
    text_prog_->addShader(vs);
    text_prog_->addShader(fs);

    const GLchar* feedbackVaryings[] = { "v_position" };
    glTransformFeedbackVaryings(text_prog_->handle(), 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

    text_prog_->link() ;

}


void RendererImpl::render(const DrawablePtr &geom, const Camera &cam, const Matrix4f &mat, Renderer::RenderMode mode)
{
    if ( !geom->geometry_ ) return ;

    MeshData &data = buffers_[geom->geometry_] ;

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

    light_index_ = 0 ;
    setLights(scene_) ;

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

    MeshPtr mesh = std::dynamic_pointer_cast<Mesh>(geom->geometry_) ;

    if ( mesh ) {
        if ( mesh->ptype_ == Mesh::Triangles ) {
            glDrawArrays(GL_TRIANGLES, 0, data.elem_count_) ;
        }
        else if ( mesh->ptype_ == Mesh::Lines ) {
            glDrawArrays(GL_LINES, 0, data.elem_count_) ;
        }
        else if ( mesh->ptype_ == Mesh::Points ) {
            glDrawArrays(GL_POINTS, 0, data.elem_count_) ;
        }

    } else {
        glDrawArrays(GL_TRIANGLES, 0, data.elem_count_) ;
    }

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
