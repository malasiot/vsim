#include <vsim/env/environment.hpp>
#include <vsim/env/model.hpp>
#include <vsim/env/geometry.hpp>
#include <vsim/env/node.hpp>
#include <vsim/util/xml_sax_parser.hpp>
#include <vsim/util/format.hpp>
#include <vsim/util/filesystem.hpp>

#include <fstream>
#include <Eigen/Geometry>
#include <stack>

using namespace Eigen ;
using namespace std ;

namespace vsim {

struct MeshReference {
    std::string id_ ;
    GeometryPtr geom_ ;
};

struct ModelReference {
    std::string id_ ;
    ModelPtr parent_ ;
};

struct MaterialReference {
    std::string id_ ;
    GeometryPtr parent_ ;
};


class EnvParser: public util::XMLSAXParser {
public:
    enum ElementType { ModelElement, PoseElement, MeshElement, NodeElement, RootElement, SceneElement, BodyElement,
                       GeometryElement, MaterialElement, TranslationElement, RotationElement, SkewElement, ScaleElement } ;

    EnvParser(const string &fname, istream &strm, Environment &env): util::XMLSAXParser(strm), fname_(fname), env_(env) {
        string base, ext ;
        util::split_path(fname, dir_, base, ext) ;
    }

    void startElement(const std::string &qname, const util::Dictionary &attr_list) override ;
    void endElement(const std::string &qname) override ;
    void characters(const std::string &text_data) override {
        text_ = text_data ;
    }

    void error(ErrorCode code, uint line, uint column ) {
        throw EnvironmentLoadException(util::format("Error parsing file: %\nXML parsing error near line: %, col: %, code: %", fname_, line, column, code));
    }

    Environment &env_ ;

    string fname_, dir_ ;
    stack<ElementType> elements_ ;
    stack<ModelPtr> model_stack_ ;
    stack<NodePtr> node_stack_ ;
    ScenePtr scene_ ;
    MeshPtr mesh_ ;

    BodyPtr body_ ;
    GeometryPtr geom_ ;
    MaterialPtr material_ ;

    map<string, ModelPtr> model_map_ ;
    map<string, MeshPtr> mesh_map_ ;
    map<string, MaterialPtr> material_map_ ;
    string text_ ;
};



/*
string EnvParser::resolveUri(const string &uri) {

    find_by_id_walker walker("image", uri) ;
    root_doc_->traverse(walker) ;

    xml_node img_node = walker.node_ ;

    if ( img_node ) {
        string img_uri = img_node.child("init_from").child_value() ;

        boost::regex exrp( "^(?:([^:/?#]+):)?(?://([^/?#]*))?([^?#]*)(?:\\?([^#]*))?(?:#(.*))?" );
        boost::match_results<string::const_iterator> what;

        string schema, authority, path, query, fragment ;

        if( !regex_search( img_uri, what, exrp ) ) return string() ;

        schema = what.str(1) ;
        authority = what.str(2) ;
        path = what.str(3) ;
        query = what.str(4) ;
        fragment = what.str(5) ;

        if ( !path.empty() ) {
            fs::path p(path) ;

            if ( schema == "file" )  // absolute path
                return p.native() ;
            else if ( schema.empty() ) // no schema, relative path
                return fs::absolute(p, fs::path(base_dir_).parent_path()).native() ;
        }

    }

    return string() ;
}

Matrix4f EnvParser::parseTransform(const string &text)
{
    Affine3f mat ;
    mat.setIdentity();

    for( xml_node &child: parent.children() ) {

        string tagName = child.name() ;

        if ( tagName == "rotate" )
        {
            string cont = child.child_value() ;

            istringstream sstrm(cont) ;

            float ax, ay, az, angle ;
            sstrm >> ax >> ay >> az >> angle ;

            mat.rotate(AngleAxisf(angle * M_PI/180.0, Vector3f(ax, ay, az))) ;
        }
        else if ( tagName == "translate" )
        {
            string cont = child.child_value() ;

            istringstream sstrm(cont) ;
            float tx, ty, tz ;
            sstrm >> tx >> ty >> tz  ;

            mat.translate(Vector3f(tx, ty, tz)) ;
        }
        else if ( tagName == "matrix" )
        {
            string cont = child.child_value() ;

            istringstream sstrm(cont);

            Matrix4f m ;

            sstrm >> m(0, 0) >> m(0, 1) >> m(0, 2) >> m(0, 3) ;
            sstrm >> m(1, 0) >> m(1, 1) >> m(1, 2) >> m(1, 3) ;
            sstrm >> m(2, 0) >> m(2, 1) >> m(2, 2) >> m(2, 3) ;
            sstrm >> m(3, 0) >> m(3, 1) >> m(3, 2) >> m(3, 3) ;

            mat *= m ;

        }
        else if ( tagName == "scale" )
        {
            string cont = child.child_value() ;

            istringstream sstrm(cont) ;
            float sx, sy, sz ;
            sstrm >> sx >> sy >> sz  ;

            mat.scale(Vector3f(sx, sy, sz)) ;

        }
        else if ( tagName == "skew" )
        {
            // not implemented
        }

    }

    return mat.matrix() ;

}
*/


void Environment::loadXML(const string &path) {
    ifstream strm(path) ;
    EnvParser parser(path, strm, *this) ;
    parser.parse() ;

}

void EnvParser::startElement(const string &qname, const util::Dictionary &attrs)
{
    if ( qname == "vsim" ) {
        elements_.push(RootElement) ;
    }
    else if ( qname == "scene" ) {
        if ( elements_.top() != RootElement || scene_ ) {
            throw EnvironmentLoadException("Scene declared outside of root element or already defined") ;
        }
        elements_.push(SceneElement) ;
        scene_.reset(new Scene) ;
        env_.scene_ = scene_ ;
    } else if ( qname == "body" ) {
        if ( elements_.top() != SceneElement ) {
            throw EnvironmentLoadException("<body> declared outside of <scene> element") ;
        }
        elements_.push(BodyElement) ;
        body_ = std::make_shared<Body>() ;
        scene_->bodies_.push_back(body_) ;
    } else if ( qname == "geometry" ) {
        if ( elements_.top() != NodeElement ) {
            throw EnvironmentLoadException("<geometry> declared outside of <scene> element") ;
        }
        elements_.push(GeometryElement) ;
        geom_.reset(new Geometry) ;
        node_stack_.top()->geometries_.push_back(geom_) ;
    }
    else if ( qname == "model" ) {
        if ( elements_.top() != SceneElement &&
             elements_.top() != BodyElement &&
             elements_.top() != ModelElement ) {
            throw EnvironmentLoadException("<model> should be declared only inside <scene> or <body> or <model> element") ;
        }

        string uri = attrs.get("uri") ;

        ModelPtr model ;

        if ( !uri.empty() ) {
            auto it = model_map_.find(uri.substr(1))  ;
            if ( it == model_map_.end() )
               throw EnvironmentLoadException(util::format("Unresolved reference % of <model>", uri)) ;
            else model = it->second ;
        }
        else {
            string id = attrs.get("id") ;
            string src = attrs.get("src") ;

            if ( !src.empty() ) {
                try {
                    model = Model::load(dir_ + '/' + src) ;
                }
                catch ( ModelLoaderException &e ) {
                    throw EnvironmentLoadException(e.what()) ;
                }
            } else model.reset(new Model) ;

            if ( !id.empty() ) model_map_.insert({id, model}) ;
        }

        if ( elements_.top() == BodyElement ) {
            body_->model_ = model ;
        } else if ( elements_.top() == SceneElement ) {
            scene_->models_.push_back(model) ;
        } else if ( elements_.top() == ModelElement ) {
            ModelPtr parent = model_stack_.top() ;
            parent->children_.push_back(model) ;
        }

        elements_.push(ModelElement) ;
        model_stack_.push(model) ;

    } else if ( qname == "node" ) {

        if ( elements_.top() != ModelElement  &&
             elements_.top() != NodeElement ) {
            throw EnvironmentLoadException("<node> should be declared only inside <model> or <node> element") ;
        }

        string id = attrs.get("id") ;

        NodePtr node(new Node) ;

        if ( !node_stack_.empty() ) {
            NodePtr parent = node_stack_.top() ;
            parent->children_.push_back(node) ;
        }
        node_stack_.push(node) ;

        if ( elements_.top() == ModelElement ) {
            model_stack_.top()->nodes_.push_back(node) ;
        }

        elements_.push(NodeElement) ;
    }  else if ( qname == "mesh" ) {
        if ( elements_.top() != SceneElement &&
             elements_.top() != GeometryElement &&
             elements_.top() != ModelElement ) {
                throw EnvironmentLoadException("<mesh> should be declared only inside <scene> or <geometry> or <model> element") ;
        }

        string uri = attrs.get("uri") ;

        MeshPtr mesh ;

        if ( !uri.empty() ) {
            string rid = uri.substr(1) ;

            auto it = mesh_map_.find(rid)  ;
            if ( it == mesh_map_.end() )
               throw EnvironmentLoadException(util::format("Unresolved reference \"%\" of <mesh>", uri)) ;
            else mesh = it->second ;
        }
        else {
            string id = attrs.get("id") ;
            mesh.reset(new Mesh) ;
            if ( !id.empty() )
                mesh_map_.insert({id, mesh}) ;
        }

        if ( elements_.top() == GeometryElement ) {
            if ( geom_->mesh_ )
                throw EnvironmentLoadException(util::format("Only one instance of <mesh> is allowed within <geometry>")) ;
            geom_->mesh_ = mesh ;
        }
        else if ( elements_.top() == SceneElement ) {
            scene_->meshes_.push_back(mesh) ;
        }
        else if ( elements_.top() == ModelElement ) {
            model_stack_.top()->meshes_.push_back(mesh) ;
        }

        elements_.push(MeshElement) ;
        mesh_ = mesh ;
    } else if ( qname == "material" ) {
        if ( elements_.top() != SceneElement &&
             elements_.top() != GeometryElement &&
             elements_.top() != ModelElement ) {
                throw EnvironmentLoadException("<material> should be declared only inside <scene> or <geometry> or <model> element") ;
        }

        string uri = attrs.get("uri") ;

        MaterialPtr material ;

        if ( !uri.empty() ) {
            string rid = uri.substr(1) ;

            auto it = material_map_.find(rid)  ;
            if ( it == material_map_.end() )
               throw EnvironmentLoadException(util::format("Unresolved reference \"%\" of <material>", uri)) ;
            else material = it->second ;
        }
        else {
            string id = attrs.get("id") ;
            material.reset(new Material) ;
            if ( !id.empty() )
                material_map_.insert({id, material}) ;
        }

        if ( elements_.top() == GeometryElement ) {
            if ( geom_->material_ )
                throw EnvironmentLoadException(util::format("Only one instance of <material> is allowed within <geometry>")) ;
            geom_->material_ = material ;
        }
        else if ( elements_.top() == SceneElement ) {
            scene_->materials_.push_back(material) ;
        }
        else if ( elements_.top() == ModelElement ) {
            model_stack_.top()->materials_.push_back(material) ;
        }

        elements_.push(MaterialElement) ;
        material_ = material ;
    }
}

void EnvParser::endElement(const string &qname)
{
    if ( qname == "vsim" ) {
        elements_.pop() ;
    }
    else if ( qname == "scene" ) {
        elements_.pop() ;
    } else if ( qname == "body" ) {
        elements_.pop() ;
        body_.reset() ;
    } else if ( qname == "model" ) {
        elements_.pop() ;
        model_stack_.pop() ;
    } else if ( qname == "node" ) {
        elements_.pop() ;
        node_stack_.pop() ;
    } else if ( qname == "mesh" ) {
        elements_.pop() ;
        mesh_.reset() ;
    } else if ( qname == "geometry" ) {
        elements_.pop() ;
        geom_.reset() ;
    } else if ( qname == "material" ) {
        elements_.pop() ;
        material_.reset() ;
    }

}

}
