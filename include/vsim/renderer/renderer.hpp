#ifndef __VSIM_RENDERER_HPP__
#define __VSIM_RENDERER_HPP__

#include <memory>

#include <vsim/renderer/scene.hpp>

namespace vsim { namespace renderer {

class RendererImpl ;

class Renderer {
public:

    enum RenderMode {   RENDER_FLAT, // no lighting, makes a mask of each object using its diffuse material color
                        RENDER_SMOOTH, // phong
                        RENDER_GOURAUD } ;

    Renderer(const ScenePtr &scene) ;
    ~Renderer() ;

    // initialize renderer
    bool init() ;

    // set free space color
    void setBackgroundColor(const Eigen::Vector4f &clr);

    // render scene
    void render(const Camera &cam, RenderMode mode) ;

  //  cv::Mat getColor(bool alpha = true);
  //  cv::Mat getColor(cv::Mat &bg, float alpha);
  //  cv::Mat getDepth();


private:

    std::unique_ptr<RendererImpl> impl_ ;

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
