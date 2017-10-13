#include "scene_loader.hpp"
#include <vsim/util/filesystem.hpp>
#include <vsim/util/strings.hpp>

#include <vector>

using namespace std ;

namespace vsim { namespace renderer {

SceneLoaderRegistry::SceneLoaderRegistry() {
    // put here all drivers
}

SceneLoaderPtr SceneLoader::findDriver(const std::string &name)
{
    const vector<SceneLoaderPtr> &drivers = SceneLoaderRegistry::instance().driver_list_ ;
    for ( uint i=0 ; i<drivers.size() ; i++ )
        if ( drivers[i]->canLoad(name) ) return drivers[i] ;

    return SceneLoaderPtr() ;
}

SceneLoaderPtr SceneLoader::findByFileName(const std::string &fname)
{
    string dir, base, fext ;
    util::split_path(fname, dir, base, fext);

    if ( fext.empty() ) return SceneLoaderPtr() ;

    util::toLower(fext) ;

    const vector<SceneLoaderPtr> &drivers = SceneLoaderRegistry::instance().driver_list_ ;

    for ( uint i=0 ; i<drivers.size() ; i++ ) {
        SceneLoaderPtr d = drivers[i] ;

        string extensions = d->getExtensions() ;

        vector<string> tokens = util::split(extensions, ";") ;
        if ( std::find(tokens.begin(), tokens.end(), fext) != tokens.end() ) return d ;
    }

    return SceneLoaderPtr() ;
}


} // namespace renderer
} // namespace vsim
