#include <vsim/env/environment.hpp>
#include <vsim/env/model.hpp>
#include <vsim/env/geometry.hpp>
#include <vsim/env/node.hpp>
#include <vsim/env/pose.hpp>
#include <vsim/util/xml_sax_parser.hpp>
#include <vsim/util/format.hpp>
#include <vsim/util/filesystem.hpp>
#include <vsim/util/strings.hpp>

#include <fstream>
#include <Eigen/Geometry>
#include <stack>

using namespace Eigen ;
using namespace std ;

namespace vsim {

using util::Dictionary ;

class EnvParser: public util::XMLSAXParser {
public:
    enum ElementType { ModelElement, ModelElementReference, PoseElement, MeshElement, MeshElementReference, NodeElement, NodeElementReference,
                       RootElement, SceneElement, BodyElement, BodyElementReference,
                       GeometryElement, MaterialElement, MaterialElementReference, TranslationElement, RotationElement, SkewElement, ScaleElement,
                     MatrixElement } ;

    EnvParser(const string &fname, istream &strm, Environment &env): util::XMLSAXParser(strm), fname_(fname), env_(env) {
        string base, ext ;
        util::split_path(fname, dir_, base, ext) ;
    }

    void startElement(const std::string &qname, const util::Dictionary &attr_list) override ;
    void endElement(const std::string &qname) override ;
    void characters(const std::string &text_data) override {
        string trimmed = util::trimCopy(text_data) ;
        if ( !trimmed.empty() ) text_ = trimmed ;
    }

    void error(ErrorCode code, uint line, uint column ) {
        throw EnvironmentLoadException(util::format("Error parsing file: %\nXML parsing error near line: %, col: %, code: %", fname_, line, column, code));
    }

    void createModelElement(const Dictionary &attrs) ;
    void createNodeElement(const Dictionary &attrs) ;
    void createMaterialElement(const Dictionary &attrs) ;
    void createPrimitiveElement(const string &type, const Dictionary &attrs) ;
    void createPoseElement(const Dictionary &attrs) ;


    void parsePoseTranslation() ;
    void parsePoseScale() ;
    void parsePoseRotation(const string &type) ;
    void parsePoseMatrix() ;

    Environment &env_ ;

    string fname_, dir_ ;
    stack<ElementType> elements_ ;
    stack<ModelPtr> model_stack_ ;
    stack<NodePtr> node_stack_ ;
    ScenePtr scene_ ;
    MeshPtr mesh_ ;
    Pose pose_ ;

    BodyPtr body_ ;
    GeometryPtr geom_ ;
    MaterialPtr material_ ;

    map<string, ModelPtr> model_map_ ;
    map<string, MeshPtr> mesh_map_ ;
    map<string, MaterialPtr> material_map_ ;
    map<string, FramePtr> frame_map_ ;
    string text_ ;
};

void Environment::loadXML(const string &path) {
    ifstream strm(path) ;
    EnvParser parser(path, strm, *this) ;
    parser.parse() ;
}

void EnvParser::createModelElement(const Dictionary &attrs) {
    ElementType et = elements_.top() ;

    if ( et != SceneElement && et != BodyElement && et != ModelElement )
        throw EnvironmentLoadException("<model> should be declared only inside <scene> or <body> or <model> element") ;

    string uri = attrs.get("uri") ;

    ModelPtr model ;

    if ( !uri.empty() ) { // this is a reference to a model

        auto it = model_map_.find(uri.substr(1))  ;
        if ( it == model_map_.end() )
           throw EnvironmentLoadException(util::format("Unresolved reference % of <model>", uri)) ;
        else model = it->second ;

        elements_.push(ModelElementReference) ;
    }
    else { // this is an inline model or a model loaded from file
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

        elements_.push(ModelElement) ;

        if ( et == BodyElement )
            scene_->models_.push_back(model) ;
        else if ( et == SceneElement )
            scene_->models_.push_back(model) ;
        else if ( et == ModelElement ) {
            ModelPtr parent = model_stack_.top() ;
            parent->children_.push_back(model) ;
        }
    }

    if ( et == BodyElement ) {
        body_->model_ = model ;
    }

    model_stack_.push(model) ;
}

void EnvParser::createNodeElement(const Dictionary &attrs) {
    ElementType et = elements_.top() ;
    if ( et != ModelElement && et != NodeElement )
        throw EnvironmentLoadException("<node> should be declared only inside <model> or <node> element") ;

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
}

void EnvParser::createPrimitiveElement(const std::string &pname, const Dictionary &attrs) {
    ElementType et = elements_.top() ;
    if ( et != SceneElement && et != GeometryElement && et != ModelElement )
            throw EnvironmentLoadException(util::format("<%> should be declared only inside <scene> or <geometry> or <model> element", pname)) ;

    string uri = attrs.get("uri") ;

    MeshPtr mesh ;

    if ( !uri.empty() ) {
        string rid = uri.substr(1) ;

        auto it = mesh_map_.find(rid)  ;
        if ( it == mesh_map_.end() )
           throw EnvironmentLoadException(util::format("Unresolved reference \"%\" of <%>", pname)) ;
        else mesh = it->second ;

        elements_.push(MeshElementReference) ;
    }
    else {
        string id = attrs.get("id") ;

        if ( pname == "cube" ) {
            float sz = attrs.value<float>("size", 1.0) ;
            mesh = Mesh::createSolidCube(sz) ;
        } else if ( pname == "cylinder" ) {
            float radius = attrs.value<float>("radius", 1.0) ;
            float height = attrs.value<float>("height", 1.0) ;
            int slices = attrs.value<int>("slices", 12) ;
            int stacks = attrs.value<int>("stacks", 4) ;
            mesh = Mesh::createSolidCylinder(radius, height, slices, stacks) ;
        } else if ( pname == "cone" ) {
            float radius = attrs.value<float>("radius", 1.0) ;
            float height = attrs.value<float>("height", 1.0) ;
            int slices = attrs.value<int>("slices", 12) ;
            int stacks = attrs.value<int>("stacks", 4) ;
            mesh = Mesh::createSolidCone(radius, height, slices, stacks) ;
        } else if ( pname == "sphere" ) {
            float radius = attrs.value<float>("radius", 1.0) ;
            int slices = attrs.value<int>("slices", 12) ;
            int stacks = attrs.value<int>("stacks", 12) ;
            mesh = Mesh::createSolidSphere(radius, slices, stacks) ;
        }

        if ( !id.empty() )
            mesh_map_.insert({id, mesh}) ;

        elements_.push(MeshElement) ;

        if ( et == GeometryElement )
            geom_->parent_->model_->meshes_.push_back(mesh_) ;
        else if ( et == SceneElement )
            scene_->meshes_.push_back(mesh) ;
        else if ( et == ModelElement )
            model_stack_.top()->meshes_.push_back(mesh) ;
    }

    if ( et == GeometryElement ) {
        if ( geom_->mesh_ )
            throw EnvironmentLoadException(util::format("Only one instance of <mesh> is allowed within <geometry>")) ;
        geom_->mesh_ = mesh ;
    }

    mesh_ = mesh ;
}

void EnvParser::parsePoseTranslation() {

    istringstream sstrm(text_) ;
    float tx, ty, tz ;
    sstrm >> tx >> ty >> tz  ;

    pose_.mat_.translate(Vector3f(tx, ty, tz)) ;
}

void EnvParser::parsePoseRotation(const string &rtype) {

    if ( rtype == "axis" )
    {
        istringstream sstrm(text_) ;

        float ax, ay, az, angle ;
        sstrm >> ax >> ay >> az >> angle ;

        pose_.mat_.rotate(AngleAxisf(angle * M_PI/180.0, Vector3f(ax, ay, az))) ;
    }
}

void EnvParser::parsePoseScale() {

    istringstream sstrm(text_) ;

    float ax, ay, az ;
    sstrm >> ax >> ay >> az  ;

    pose_.mat_.scale(Vector3f(ax, ay, az)) ;
}

void EnvParser::parsePoseMatrix() {

    istringstream sstrm(text_) ;
    Matrix4f m ;

    sstrm >> m(0, 0) >> m(0, 1) >> m(0, 2) >> m(0, 3) ;
    sstrm >> m(1, 0) >> m(1, 1) >> m(1, 2) >> m(1, 3) ;
    sstrm >> m(2, 0) >> m(2, 1) >> m(2, 2) >> m(2, 3) ;
    sstrm >> m(3, 0) >> m(3, 1) >> m(3, 2) >> m(3, 3) ;


    pose_.mat_ *= m ;
}


void EnvParser::createMaterialElement(const Dictionary &attrs) {
    ElementType et = elements_.top() ;

    if ( et != SceneElement && et != GeometryElement && et != ModelElement )
            throw EnvironmentLoadException("<material> should be declared only inside <scene> or <geometry> or <model> element") ;

    string uri = attrs.get("uri") ;

    MaterialPtr material ;

    if ( !uri.empty() ) {
        string rid = uri.substr(1) ;

        auto it = material_map_.find(rid)  ;
        if ( it == material_map_.end() )
           throw EnvironmentLoadException(util::format("Unresolved reference \"%\" of <material>", uri)) ;
        else material = it->second ;

        elements_.push(MaterialElementReference) ;
    }
    else {
        string id = attrs.get("id") ;
        material.reset(new Material) ;
        if ( !id.empty() )
            material_map_.insert({id, material}) ;

        elements_.push(MaterialElement) ;
    }

    if ( et == GeometryElement ) {
        if ( geom_->material_ )
            throw EnvironmentLoadException(util::format("Only one instance of <material> is allowed within <geometry>")) ;
        geom_->material_ = material ;
    }
    else if ( et == SceneElement )
        scene_->materials_.push_back(material) ;
    else if ( et == ModelElement )
        model_stack_.top()->materials_.push_back(material) ;

    material_ = material ;

}

void EnvParser::createPoseElement(const Dictionary &attrs) {

    ElementType et = elements_.top() ;

    if ( et != BodyElement && et != NodeElement && et != ModelElement )
            throw EnvironmentLoadException("<pose> should be declared only inside <body> or <node> or <model> element") ;

    string parent_id = attrs.get("frame") ;
    if ( !parent_id.empty() ) {
        auto it = frame_map_.find(parent_id.substr(1))  ;
        if ( it == frame_map_.end() )
           throw EnvironmentLoadException(util::format("Unresolved reference \"%\" of \"frame\" in <pose>", parent_id)) ;
        else {
            pose_.frame_ = it->second ;
        }
    }
    else pose_.frame_.reset() ;



    elements_.push(PoseElement) ;


}

void EnvParser::startElement(const string &qname, const util::Dictionary &attrs)
{
    if ( qname == "vsim" ) {
        elements_.push(RootElement) ;
    }
    else if ( qname == "scene" ) {
        if ( elements_.top() != RootElement || scene_ ) {
            throw EnvironmentLoadException("<scene> declared outside of <root> element or already defined") ;
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
        createModelElement(attrs);

    } else if ( qname == "node" ) {
        createNodeElement(attrs) ;
    }  else if ( qname == "cube" || qname == "sphere" || qname == "cylinder" || qname == "cone" ) {
        createPrimitiveElement(qname, attrs) ;
    } else if ( qname == "material" ) {
        createMaterialElement(attrs) ;
    } else if ( qname == "pose" ) {
       createPoseElement(attrs) ;
    } else if ( qname == "translate" ) {
        if ( elements_.top() != PoseElement )
            throw EnvironmentLoadException("<translate> declared outside of <pose> element") ;
        elements_.push(TranslationElement) ;
    } else if ( qname == "rotate" ) {
        if ( elements_.top() != PoseElement )
            throw EnvironmentLoadException("<rotate> declared outside of <pose> element") ;
        elements_.push(RotationElement) ;
    } else if ( qname == "scale" ) {
        if ( elements_.top() != PoseElement )
            throw EnvironmentLoadException("<scale> declared outside of <pose> element") ;
        elements_.push(ScaleElement) ;
    } else if ( qname == "matrix" ) {
        if ( elements_.top() != PoseElement )
            throw EnvironmentLoadException("<scale> declared outside of <pose> element") ;
        elements_.push(MatrixElement) ;
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
    } else if ( qname == "cube" || qname == "sphere" || qname == "cylinder" || qname == "cone") {
        elements_.pop() ;
        mesh_.reset() ;
    } else if ( qname == "geometry" ) {
        elements_.pop() ;
        geom_.reset() ;
    } else if ( qname == "material" ) {
        elements_.pop() ;
        material_.reset() ;
    } else if ( qname == "translate" ) {
        elements_.pop() ;
        parsePoseTranslation();
    }  else if ( qname == "rotate" ) {
        elements_.pop() ;
        parsePoseRotation("axis");
    }  else if ( qname == "scale" ) {
        elements_.pop() ;
        parsePoseScale();
    } else if ( qname == "matrix" ) {
        elements_.pop() ;
        parsePoseMatrix();
    } else if ( qname == "pose" ) {
        elements_.pop() ;
        ElementType et = elements_.top() ;

        if ( et == BodyElement )
            body_->pose_ = pose_ ;
        else if ( et == NodeElement )
            node_stack_.top()->pose_ = pose_ ;
        else if ( et == ModelElement )
            model_stack_.top()->pose_ = pose_ ;

        pose_.frame_.reset() ;
        pose_.mat_.setIdentity() ;
    }


}

}
