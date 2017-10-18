#include "scene_loader.hpp"
#include <vsim/util/filesystem.hpp>
#include <vsim/util/strings.hpp>

#include <vector>

using namespace std ;

namespace vsim {

ModelLoaderRegistry::ModelLoaderRegistry() {
    // put here all drivers
}

ModelLoaderPtr ModelLoader::findDriver(const std::string &name)
{
    const vector<ModelLoaderPtr> &drivers = ModelLoaderRegistry::instance().driver_list_ ;
    for ( uint i=0 ; i<drivers.size() ; i++ )
        if ( drivers[i]->canLoad(name) ) return drivers[i] ;

    return ModelLoaderPtr() ;
}

ModelLoaderPtr ModelLoader::findByFileName(const std::string &fname)
{
    string dir, base, fext ;
    util::split_path(fname, dir, base, fext);

    if ( fext.empty() ) return ModelLoaderPtr() ;

    util::toLower(fext) ;

    const vector<ModelLoaderPtr> &drivers = ModelLoaderRegistry::instance().driver_list_ ;

    for ( uint i=0 ; i<drivers.size() ; i++ ) {
        ModelLoaderPtr d = drivers[i] ;

        string extensions = d->getExtensions() ;

        vector<string> tokens = util::split(extensions, ";") ;
        if ( std::find(tokens.begin(), tokens.end(), fext) != tokens.end() ) return d ;
    }

    return ModelLoaderPtr() ;
}


} // namespace vsim
