#include <sol/sol.hpp>

#include <memory>
#include <iostream>
#include <cassert>

using namespace sol ;
using namespace std ;

struct holy {

      ~holy() {}
private:
    holy() : data() {}
    holy(int value) : data(value) {}


public:
    struct deleter {
        void operator()(holy* p) const {
            destroy(*p);
        }
    };

    vector<std::shared_ptr<holy>> data;

    static std::shared_ptr<holy> create(sol::table t) {

        std::shared_ptr<holy> res(new holy());


            for( auto c: t ) {
                cout << c.first.as<int>()  << endl ;
                if ( c.second.is<holy>() ) {
                    std::shared_ptr<holy> &h = c.second.as<std::shared_ptr<holy>>() ;
                    res->data.push_back(h) ;
                }

            }

        return res;
    }

    static void initialize(holy& uninitialized_memory) {
        std::cout << "initializing 'holy' userdata at " << static_cast<void*>(&uninitialized_memory) << std::endl;
        // receive uninitialized memory from Lua:
        // properly set it by calling a constructor
        // on it
        // "placement new"
        new (&uninitialized_memory) holy();
    }

    static void destroy(holy& memory_from_lua) {
        std::cout << "destroying 'holy' userdata at " << static_cast<void*>(&memory_from_lua) << std::endl;
        memory_from_lua.~holy();
    }
};

/*

    b1 = Body {
        mass=2.0,
        inertia = { 1, 2, 3 },
        frame=Pose{} ;
        Shape {
            pose=Pose{}
            geometry =
                Plane { }
                Box {}
                Sphere {}
                Cylinder {}
                Capsule {}
                Mesh { src="sksk" }
                ConvexMesh { src = "sks" }
        }

        Shape {
        }

    }

    b2=  Body {}

    Model {
        b1, b2,
        Constraint {
            Hinge {
                parent = p2, child = b2,

            }
        }

    }

  */
int main() {
    std::shared_ptr<holy> h1,h2 ;
    std::cout << "=== usertype_initializers example ===" << std::endl;
    { // additional scope to make usertype destroy earlier
        sol::state lua;
        lua.open_libraries();

        lua.new_usertype<holy>("holy",
            sol::call_constructor, sol::factories(&holy::create),
            sol::meta_function::garbage_collect, sol::destructor(&holy::destroy),
            "data", &holy::data
        );

        lua.script(R"(

                   h1 = holy{};

                   h2 = holy{
                    h1,
                    holy{},
                    pose = {1, 2, 3}
                }


)");

        h2 = lua["h2"];
        h1 = lua["h1"];

std::cout << std::endl;
    }
    std::cout << std::endl;
}
