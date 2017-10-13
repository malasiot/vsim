#ifndef __VSIM_SCENE_FWD_HPP__
#define __VSIM_SCENE_FWD_HPP__

#include <memory>

namespace vsim { namespace renderer {

struct Material ;
typedef std::shared_ptr<Material> MaterialPtr ;

struct Mesh ;
typedef std::shared_ptr<Mesh> MeshPtr ;

struct SkinningModifier ;
typedef std::shared_ptr<SkinningModifier> SkinningModifierPtr ;

struct Geometry ;
typedef std::shared_ptr<Geometry> GeometryPtr ;

struct Bone ;
typedef std::shared_ptr<Bone> BonePtr ;

struct Skeleton ;
typedef std::shared_ptr<Skeleton> SkeletonPtr ;

struct Node ;
typedef std::shared_ptr<Node> NodePtr ;

struct Camera ;
typedef std::shared_ptr<Camera> CameraPtr ;

struct Light ;
typedef std::shared_ptr<Light> LightPtr ;

class Scene ;
typedef std::shared_ptr<Scene> ScenePtr ;

}}

#endif