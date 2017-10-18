#include <vsim/env/scene.hpp>
#include <iostream>
#include <fstream>

#include <Eigen/Geometry>

#include "scene_loader.hpp"

using namespace std ;
using namespace Eigen ;

namespace vsim {

ModelPtr Model::load(const std::string &fname)
{
    ModelLoaderPtr driver = ModelLoader::findDriver(fname) ;

    if ( !driver )
        return Model::loadAssimp(fname) ;

    try {
        return driver->load(fname) ;
    }
    catch ( ModelLoaderException &e ) {
        cerr << e.what() << endl ;
        return ModelPtr() ;
    }
}



}
