#ifndef __VSIM_SCENE_FWD_HPP__
#define __VSIM_SCENE_FWD_HPP__

#include <memory>

namespace vsim {

class Model ;
typedef std::shared_ptr<Model> ModelPtr ;

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

struct Model ;
typedef std::shared_ptr<Model> ModelPtr ;

struct Drawable ;
typedef std::shared_ptr<Drawable> DrawablePtr ;

struct CollisionShape ;
typedef std::shared_ptr<CollisionShape> CollisionShapePtr ;

struct RigidBody ;
typedef std::shared_ptr<RigidBody> RigidBodyPtr ;

struct RigidBodyConstraint ;
typedef std::shared_ptr<RigidBodyConstraint> RigidBodyConstraintPtr ;

struct PhysicsModel ;
typedef std::shared_ptr<PhysicsModel> PhysicsModelPtr ;

struct PhysicsScene ;
typedef std::shared_ptr<PhysicsScene> PhysicsScenePtr ;

struct Environment ;
typedef std::shared_ptr<Environment> EnvironmentPtr ;

}

#endif
