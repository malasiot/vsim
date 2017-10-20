#include <vsim/env/environment.hpp>
#include <vsim/env/model.hpp>
#include <vsim/env/geometry.hpp>
#include <vsim/env/node.hpp>
#include <vsim/env/pose.hpp>
#include <vsim/env/drawable.hpp>

#include <vsim/util/format.hpp>
#include <vsim/util/filesystem.hpp>
#include <vsim/util/strings.hpp>

#include <pugixml/pugixml.hpp>

#include <fstream>
#include <Eigen/Geometry>
#include <stack>

using namespace Eigen ;
using namespace std ;

namespace vsim {


using namespace pugi ;

class EnvLoader {
public:

    EnvLoader(Environment &env): env_(env) {}
    void load(const string &fname) ;

private:

    void parse(const xml_node &root) ;
    void parseWorld(const xml_node &root) ;
    void parseEnvironment(const xml_node &root) ;
    void parseLibrary(const xml_node &root) ;
    NodePtr parseVisual(const xml_node &root) ;
    void parseShape(const xml_node &root) ;
    void parseLink(const xml_node &root) ;
    void parseRigidBody(const xml_node &root) ;
    void parseRigidConstraint(const xml_node &root) ;
    void parseModel(const xml_node &root) ;
    NodePtr parseNode(const xml_node &root) ;
    NodePtr parseNodeInstance(const xml_node &root) ;
    NodePtr parseImportNode(const xml_node &root) ;
    MaterialPtr parseMaterial(const xml_node &root) ;
    MaterialPtr parseMaterialInstance(const xml_node &root) ;
    DrawablePtr parseDrawable(const xml_node &root) ;
    GeometryPtr parseGeometry(const xml_node &root) ;
    GeometryPtr parseGeometryInstance(const xml_node &root) ;
    GeometryPtr parseBox(const xml_node &root) ;
    GeometryPtr parseMesh(const xml_node &root) ;
    void parsePose(const xml_node &root, Pose &p) ;


private:
    Environment &env_ ;
    string root_dir_ ;
    map<string, NodePtr> node_instances_ ;
    map<string, MaterialPtr> material_instances_ ;
    map<string, GeometryPtr> geometry_instances_ ;
};

void Environment::loadXML(const string &path) {
    EnvLoader loader(*this) ;
    loader.load(path);
}


struct find_by_id_walker: pugi::xml_tree_walker
{
    find_by_id_walker(const string &tag, const string &id): tag_(tag), id_(id) {
    }

    virtual bool for_each(pugi::xml_node& node)
    {
        if ( pugi::node_element != node.type() ) return true ;

        string ename = node.name() ;
        string id = node.attribute("id").as_string() ;

        if ( ename == tag_ && id == id_ ) {
            node_ = node ;
            return false ;
        }
        return true; // continue traversal
    }

    string tag_, id_ ;
    xml_node node_ ;
};

void EnvLoader::load(const string &file_name) {
    xml_document doc ;

    string dir, base, ext ;
    util::split_path(file_name, dir, base, ext) ;

    xml_parse_result result = doc.load_file(file_name.c_str()) ;

    if ( !result )
        throw EnvironmentLoadException(result.description()) ;

    xml_node root = doc.child("vsim") ;

    root_dir_ = root.attribute("root_dir").as_string(dir.c_str()) ;

    parse(root) ;
}

void EnvLoader::parse(const xml_node &root) {

    if ( xml_node library = root.child("library") ) parseLibrary(library) ;
    if ( xml_node world = root.child("world") ) parseWorld(world) ;
    else throw EnvironmentLoadException("No <world> defined");
    if ( xml_node env = root.child("environment") ) parseEnvironment(env) ;
}

void EnvLoader::parseWorld(const xml_node &parent) {

    for( xml_node &child: parent.children() ) {
        string tag_name = child.name() ;

        if ( tag_name == "rigid_body" ) {
            parseRigidBody(child) ;
        } else if ( tag_name == "model" ) {
            parseModel(child) ;
        } else
            throw EnvironmentLoadException(util::format("Invalid element <%> inside <world>", tag_name));
    }
}

void EnvLoader::parseEnvironment(const xml_node &root)
{

}

void EnvLoader::parseLibrary(const xml_node &root) {

}

NodePtr EnvLoader::parseVisual(const xml_node &parent) {
    return parseNode(parent) ;
}

NodePtr EnvLoader::parseNode(const xml_node &parent)
{
    NodePtr p(new Node) ;

    string id = parent.attribute("id").as_string() ;
    if ( !id.empty() ) node_instances_.insert({id, p}) ;

    bool is_first_pose = true ;
    for( xml_node &child: parent.children() ) {
        string tag_name = child.name() ;

        if ( tag_name == "node" ) {
            auto child_node = parseNode(child) ;
            p->children_.push_back(child_node) ;
        } else if ( tag_name == "include" ) {
            parseImportNode(child) ;
        } else if ( tag_name == "material" ) {
            parseMaterial(child) ;
        } else if ( tag_name == "drawable" ) {
            parseDrawable(child) ;
        } else if ( tag_name == "pose" ) {
            if ( is_first_pose ) {
                parsePose(child, p->pose_) ;
                is_first_pose = false ;
            }
            else
                throw EnvironmentLoadException("Only one <pose> expected inside <node>");
        } else if ( tag_name == "node_instance" ) {
            return parseNodeInstance(child) ;
        } else
            throw EnvironmentLoadException(util::format("Invalid element <%> inside <node>", tag_name));
    }

    return p;
}

NodePtr EnvLoader::parseNodeInstance(const xml_node &parent) {
    string ref = parent.attribute("uri").as_string() ;
    if ( ref.empty() )
        throw EnvironmentLoadException("Attribute \"uri\" is required for <node_instance>");
    string id = ref.substr(1) ;
    auto it = node_instances_.find(id) ;
    if ( it == node_instances_.end() )
        throw EnvironmentLoadException(util::format("Unresolved reference \"%\" of <node_instance>", ref));
    return it->second ;
}

MaterialPtr EnvLoader::parseMaterialInstance(const xml_node &parent)
{
    string ref = parent.attribute("uri").as_string() ;
    if ( ref.empty() )
        throw EnvironmentLoadException("Attribute \"uri\" is required for <material_instance>");
    string id = ref.substr(1) ;
    auto it = material_instances_.find(id) ;
    if ( it == material_instances_.end() )
        throw EnvironmentLoadException(util::format("Unresolved reference \"%\" of <material_instance>", ref));
    return it->second ;

}


NodePtr EnvLoader::parseImportNode(const xml_node &root)
{
return nullptr;
}

MaterialPtr EnvLoader::parseMaterial(const xml_node &root)
{
return nullptr;
}

DrawablePtr EnvLoader::parseDrawable(const xml_node &parent)
{
    DrawablePtr drawable(new Drawable) ;

    for( xml_node &child: parent.children() ) {
        string tag_name = child.name() ;

        if ( tag_name == "geometry" ) {
            if ( !drawable->geometry_ )
                drawable->geometry_ = parseGeometry(child) ;
       } else if ( tag_name == "geometry_instance" ) {
            if ( !drawable->geometry_ )
                drawable->geometry_ = parseGeometryInstance(child) ;
       } else if ( tag_name == "material" ) {
            if ( !drawable->material_ )
                drawable->material_ = parseMaterial(child) ;
       } else if ( tag_name == "material_instance" ) {
            if ( !drawable->material_ )
                drawable->material_ = parseMaterialInstance(child) ;
       } else
                throw EnvironmentLoadException(util::format("Invalid element <%> inside <node>", tag_name));
    }

    if ( !drawable->geometry_ )
        throw EnvironmentLoadException("No geometry defined inside <drawable>");

    return drawable ;
}

GeometryPtr EnvLoader::parseGeometry(const xml_node &parent)
{
    GeometryPtr geom ;

    for( xml_node &child: parent.children() ) {
        string tag_name = child.name() ;

        if ( tag_name == "box" ) {
            geom = parseBox(child) ;
        } else if ( tag_name == "mesh" ) {
            geom = parseMesh(child) ;
        } else
            throw EnvironmentLoadException(util::format("Invalid element <%> inside <geometry>", tag_name));
    }

    if ( !geom )
        throw EnvironmentLoadException("Empty <geometry> element");
    return geom ;
}

GeometryPtr EnvLoader::parseGeometryInstance(const xml_node &parent)
{
    string ref = parent.attribute("uri").as_string() ;
    if ( ref.empty() )
        throw EnvironmentLoadException("Attribute \"uri\" is required for <geometry_instance>");
    string id = ref.substr(1) ;
    auto it = geometry_instances_.find(id) ;
    if ( it == geometry_instances_.end() )
        throw EnvironmentLoadException(util::format("Unresolved reference \"%\" of <geometry_instance>", ref));
    return it->second ;
}

GeometryPtr EnvLoader::parseBox(const xml_node &parent)
{
    GeometryPtr geom ;

    if ( xml_node extents = parent.child("extents") ) {
        BoxGeometry *box = new BoxGeometry ;
        istringstream strm(extents.child_value()) ;
        Vector3f &e = box->half_extents_ ;
        strm >> e.x() >> e.y() >> e.z() ;
        geom.reset(box) ;
    }
    else
        throw EnvironmentLoadException("<extents> is missing from <box>");

    return geom ;

}

GeometryPtr EnvLoader::parseMesh(const xml_node &parent)
{
    GeometryPtr geom ;

    Mesh *m = new Mesh ;

    string src = parent.attribute("url").as_string() ;
    try {
        m->load(root_dir_ + '/' + src) ;
    }
    catch ( ModelLoaderException &e ) {
        throw EnvironmentLoadException(e.what()) ;
    }

    return GeometryPtr(m) ;
}

void EnvLoader::parsePose(const xml_node &parent, Pose &p) {

    for( xml_node &child: parent.children() ) {
        string tag_name = child.name() ;
        string cont = child.child_value() ;
        istringstream sstrm(cont) ;

        if ( tag_name == "translate" ) {
            float tx, ty, tz ;
            sstrm >> tx >> ty >> tz  ;
            p.mat_.translate(Vector3f(tx, ty, tz)) ;
        } else if ( tag_name == "rotate" ) {
            float ax, ay, az, angle ;
            sstrm >> ax >> ay >> az >> angle ;
            p.mat_.rotate(AngleAxisf(angle * M_PI/180.0, Vector3f(ax, ay, az))) ;
        } else if ( tag_name == "scale" ) {
            float sx, sy, sz ;
            sstrm >> sx >> sy >> sz  ;
            p.mat_.scale(Vector3f(sx, sy, sz)) ;
        } else if ( tag_name == "matrix" ) {
            Matrix4f m ;

            sstrm >> m(0, 0) >> m(0, 1) >> m(0, 2) >> m(0, 3) ;
            sstrm >> m(1, 0) >> m(1, 1) >> m(1, 2) >> m(1, 3) ;
            sstrm >> m(2, 0) >> m(2, 1) >> m(2, 2) >> m(2, 3) ;
            sstrm >> m(3, 0) >> m(3, 1) >> m(3, 2) >> m(3, 3) ;

            p.mat_ *= m ;
        } else
            throw EnvironmentLoadException(util::format("Invalid element <%> inside <pose>", tag_name));
    }
}

void EnvLoader::parseShape(const xml_node &root)
{

}

void EnvLoader::parseLink(const xml_node &root)
{

}

void EnvLoader::parseRigidBody(const xml_node &parent) {
    for( xml_node &child: parent.children() ) {
        string tag_name = child.name() ;

        if ( tag_name == "visual" ) {
            parseVisual(child) ;
        } else if ( tag_name == "shape" ) {
            parseShape(child) ;
        } else if ( tag_name == "link" ) {
            parseLink(child) ;
        } else
            throw EnvironmentLoadException(util::format("Invalid element <%> inside <rigid_body>", tag_name));
    }

}

void EnvLoader::parseRigidConstraint(const xml_node &root)
{

}

void EnvLoader::parseModel(const xml_node &parent)
{
    for( xml_node &child: parent.children() ) {
        string tag_name = child.name() ;

        if ( tag_name == "rigid_body" ) {
            parseRigidBody(child) ;
        }
        else if ( tag_name == "rigid_constraint" ) {
            parseRigidConstraint(child) ;
        }
        else
            throw EnvironmentLoadException(util::format("Invalid element <%> inside <model>", tag_name));
    }
}




#if 0
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
#endif
}
