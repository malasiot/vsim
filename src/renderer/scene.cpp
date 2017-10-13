#include <vsim/renderer/scene.hpp>
#include <iostream>

#include "scene_loader.hpp"

using namespace std ;

namespace vsim { namespace renderer {

ScenePtr Scene::load(const std::string &fname)
{
    SceneLoaderPtr driver = SceneLoader::findDriver(fname) ;

    if ( !driver )
        return Scene::loadAssimp(fname) ;

    try {
        return driver->load(fname) ;
    }
    catch ( SceneLoaderException &e ) {
        cerr << e.what() << endl ;
        return ScenePtr() ;
    }
}

}}
