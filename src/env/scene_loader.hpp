#ifndef __SCENE_IMPORTER_HPP__
#define __SCENE_IMPORTER_HPP__

#include <string>
#include <stdexcept>
#include <vector>
#include <memory>

#include <vsim/env/model.hpp>

namespace vsim {

class ModelLoader ;
typedef std::shared_ptr<ModelLoader> ModelLoaderPtr ;

class ModelLoader {
protected:

    friend class ModelLoaderRegistry ;

    virtual bool canLoad(const std::string &fname) const = 0 ;

public:

    // should throw exception if failed
    virtual ModelPtr load(const std::string &fname) = 0 ;

    /** Return a ';' separated list of valid file extensions for this driver. */
    virtual const char *getExtensions() const =0 ;

    /** Determines the file format from the file name extension */
    static ModelLoaderPtr findByFileName(const std::string &fname) ;

    static ModelLoaderPtr findDriver(const std::string &name) ;
} ;

class ModelLoaderRegistry
{
    ModelLoaderRegistry() ;
public:

    std::vector<ModelLoaderPtr> driver_list_ ;

    static ModelLoaderRegistry &instance()
    {
        static ModelLoaderRegistry instance_ ;

        return instance_ ;
    }
} ;

} // namespace vsim

#endif
