#include <vsim/env/scene.hpp>
#include <vsim/env/environment.hpp>
#include <vsim/env/physics_scene.hpp>
#include <vsim/env/rigid_body.hpp>
#include <vsim/env/collision_shape.hpp>
#include <vsim/env/geometry.hpp>

#include <iostream>
#include <sol/sol.hpp>


using namespace std ;
using namespace Eigen ;

namespace vsim {

static ScenePtr lua_create_scene(sol::table t) {

    ScenePtr scene(new Scene());

    for( auto c: t ) {
        if ( c.second.is<Environment>() ) {
            EnvironmentPtr e = c.second.as<EnvironmentPtr>() ;
            scene->environment_ = e ;
        } else if ( c.second.is<PhysicsScene>() ) {
            PhysicsScenePtr e = c.second.as<PhysicsScenePtr>() ;
            scene->physics_scene_ = e ;
        }
    }

    return scene ;
}

static EnvironmentPtr lua_create_environment(sol::table t) {

    EnvironmentPtr env(new Environment());

    for( auto c: t ) {
        if ( c.second.is<Light>() ) {
            LightPtr e = c.second.as<LightPtr>() ;
            env->lights_.push_back(e) ;
        }
    }

    return env ;
}

static PhysicsScenePtr lua_create_physics_scene(sol::table t) {

    PhysicsScenePtr p(new PhysicsScene());

    for( auto c: t ) {
        if ( c.second.is<PhysicsModel>() ) {
            PhysicsModelPtr e = c.second.as<PhysicsModelPtr>() ;
            p->models_.push_back(e) ;
        } else if ( c.second.is<RigidBody>() ) {
            RigidBodyPtr e = c.second.as<RigidBodyPtr>() ;
            p->bodies_.push_back(e) ;
        }
    }

    return p ;
}

static RigidBodyPtr lua_create_rigid_body(sol::table t) {

    RigidBodyPtr p(new RigidBody());

    for( auto c: t ) {
        if ( c.second.is<CollisionShape>() ) {
            CollisionShapePtr e = c.second.as<CollisionShapePtr>() ;
            p->shapes_.push_back(e) ;
        } else if ( c.second.is<Pose>() ) {
            Pose e = c.second.as<Pose>() ;
            p->pose_ = e ;
        }
        else if ( c.first.is<string>() ) {
            string attr = c.first.as<string>() ;
            if ( attr == "visual" ) {
                NodePtr e = c.second.as<NodePtr>() ;
                p->visual_ = e ;
            }
        }
    }

    return p ;
}

static CollisionShapePtr lua_create_collision_shape(sol::table t) {

    CollisionShapePtr p(new CollisionShape());

    for( auto c: t ) {
        if ( c.second.is<BoxGeometry>() ) {
            GeometryPtr e = c.second.as<std::shared_ptr<BoxGeometry>>() ;
            p->geom_ = e ;
        } else if ( c.first.is<string>() ) {
            string attr = c.first.as<string>() ;
        }
    }

    return p ;
}

static bool lua_read_vec3(sol::table &t, Vector3f &v) {
    if ( t.size() < 3 ) return false ;

    sol::optional<float> x = t[1], y = t[2], z = t[3] ;
    if (  x && y && z  ) {
        v.x() = x.value() ;
        v.y() = y.value() ;
        v.z() = z.value() ;
        return true ;
    }

    return false ;
}

static bool lua_read_vec4(sol::table &t, Vector4f &v) {
    if ( t.size() < 4 ) return false ;

    sol::optional<float> x = t[1], y = t[2], z = t[3], w = t[4] ;
    if (  x && y && z  ) {
        v.x() = x.value() ;
        v.y() = y.value() ;
        v.z() = z.value() ;
        v.w() = w.value() ;
        return true ;
    }

    return false ;
}

static BoxGeometryPtr lua_create_box_geometry(sol::table t) {

    BoxGeometryPtr p(new BoxGeometry());

    if ( !lua_read_vec3(t, p->half_extents_) ) return nullptr ;

    return p ;
}

static PlaneGeometryPtr lua_create_plane_geometry(sol::table t) {

    PlaneGeometryPtr p(new PlaneGeometry());

    if ( !lua_read_vec4(t, p->coeffs_) ) return nullptr ;

    return p ;
}

struct MTranslate {
    Vector3f translation_ ;
};

struct MRotate {
    Quaternionf rotation_ ;
};

struct MScale {
    Vector3f scale_ ;
};


static MTranslate lua_create_pose_translate(sol::table t) {

    MTranslate res ;
    lua_read_vec3(t, res.translation_ ) ;
    return res ;
}

static MScale lua_create_pose_scale(sol::table t) {

    MScale res ;
    lua_read_vec3(t, res.scale_ ) ;
    return res ;
}

static MRotate lua_create_pose_rotate(sol::table t) {

    MRotate res ;

    Vector4f r ;
    lua_read_vec4(t, r) ;
    res.rotation_ = AngleAxisf(r.w() * M_PI/180.0, Vector3f(r.x(), r.y(), r.z())) ;
    return res ;
}

static Pose lua_create_pose(sol::table t) {

    Pose p ;

    for( auto c: t ) {
        if ( c.second.is<MTranslate>() ) {
            MTranslate e = c.second.as<MTranslate>() ;
            p.mat_.translate(e.translation_) ;
        } else if ( c.second.is<MRotate>() ) {
            MRotate e = c.second.as<MRotate>() ;
            p.mat_.rotate(e.rotation_) ;
        } else if ( c.second.is<MScale>() ) {
            MScale e = c.second.as<MScale>() ;
            p.mat_.scale(e.scale_) ;
        }
    }

    return p ;
}

ScenePtr Scene::loadFromString(const string &src) {
    sol::state lua;
    lua.open_libraries();

    lua.new_usertype<Scene>("Scene",
        sol::call_constructor, sol::factories(&lua_create_scene)
    );

    lua.new_usertype<Environment>("Environment",
        sol::call_constructor, sol::factories(&lua_create_environment)
    );

    lua.new_usertype<PhysicsScene>("PhysicsScene",
        sol::call_constructor, sol::factories(&lua_create_physics_scene)
    );

    lua.new_usertype<RigidBody>("RigidBody",
        sol::call_constructor, sol::factories(&lua_create_rigid_body)
    );

    lua.new_usertype<CollisionShape>("CollisionShape",
        sol::call_constructor, sol::factories(&lua_create_collision_shape)
    );

    lua.new_usertype<BoxGeometry>("Box",
        sol::call_constructor, sol::factories(&lua_create_box_geometry)
    );

    lua.new_usertype<BoxGeometry>("Plane",
        sol::call_constructor, sol::factories(&lua_create_plane_geometry)
    );

    lua.new_usertype<Pose>("Pose",
        sol::call_constructor, sol::factories(&lua_create_pose)
    );

    lua.new_usertype<MRotate>("rotate",
        sol::call_constructor, sol::factories(&lua_create_pose_rotate)
    );

    lua.new_usertype<MTranslate>("translate",
        sol::call_constructor, sol::factories(&lua_create_pose_translate)
    );

    lua.new_usertype<MScale>("scale",
        sol::call_constructor, sol::factories(&lua_create_pose_scale)
    );

    auto res = lua.script(src) ;

    ScenePtr p = res ;

    return p ;
}

}
