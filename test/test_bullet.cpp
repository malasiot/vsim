#include <btBulletDynamicsCommon.h>

#include <GL/glew.h>
#include "glfw_window.hpp"

#include <memory>
#include <vector>
#include <iostream>

using namespace std ;

struct BulletBody {
    std::unique_ptr<btDefaultMotionState> motion_state_ ;
    std::unique_ptr<btCollisionShape> shape_ ;
    std::unique_ptr<btRigidBody> body_ ;

    virtual void render() = 0 ;

    void setId(const std::string &id) {
        id_ = id ;
    }

    std::string id_ ;
};

struct PhysicsWorld {

  PhysicsWorld();

  void addBody(BulletBody *body) {
        dynamics_world_->addRigidBody(body->body_.get()) ;
        bodies_.push_back(std::unique_ptr<BulletBody>(body)) ;
  }

  void step(float seconds) {
      dynamics_world_->stepSimulation(seconds, 1, seconds) ;
  }

  void render() {

      glClear(GL_COLOR_BUFFER_BIT);

      // Reset the matrix
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
;
      for( auto &b: bodies_) {
          b->render() ;
      }

      glFlush() ;
  }

  std::vector<std::unique_ptr<BulletBody>> bodies_ ;



  std::unique_ptr<btDefaultCollisionConfiguration> collision_conf_ ;
  std::unique_ptr<btCollisionDispatcher> collision_dispatcher_ ;
  std::unique_ptr<btBroadphaseInterface> broadphase_interface_ ;
  std::unique_ptr<btSequentialImpulseConstraintSolver> solver_ ;
  std::unique_ptr<btDiscreteDynamicsWorld> dynamics_world_ ;
};


struct BoxBody: public BulletBody {


    BoxBody(float mass, const btVector3 &sz, const btVector3 &pos, float margin=0.01): sz_(sz) {
        motion_state_ = std::make_unique<btDefaultMotionState>(btTransform(
            btQuaternion(0.0f, 0.0f, 0.0f, 1.0f),  pos));

        shape_ = std::make_unique<btBoxShape>(sz);
        shape_->setMargin(btScalar(margin));

        btVector3 box_inertia(0.0f, 0.0f, 0.0f);
        shape_->calculateLocalInertia(mass, box_inertia);

        body_ = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(
            mass, motion_state_.get(), shape_.get(), box_inertia));

        body_->setFriction(1.f);
        body_->setRestitution(0.1f);
        body_->setDamping(0, 0);
      }

    void render() {
        btTransform trans;
        body_->getMotionState()->getWorldTransform(trans);

        float w = sz_.x(), h = sz_.y() ;
        float x = trans.getOrigin().getX(), y = trans.getOrigin().getY() ;
        float angle = trans.getRotation().getAngle() ;

        cout << id_ << ' ' << x << ' ' << y << ' ' << angle << endl ;

        glPushMatrix();
            glTranslatef(x, y, 0);
            glRotatef(angle * 180/M_PI, 0, 0, -1);
        //    glTranslatef(-x, -y, 0);

        glColor3ub( 255, 0, 0 );
        glBegin(GL_LINE_LOOP);
            glVertex2f( -w, -h );
            glVertex2f( -w, +h );
            glVertex2f( +w, +h );
            glVertex2f( +w, -h );
       glEnd();

       glPopMatrix() ;
    }

    btVector3 sz_ ;

};

struct GroundPlane: public BulletBody {

    GroundPlane() {
        motion_state_ = std::make_unique<btDefaultMotionState>(btTransform(
            btQuaternion(0.0f, 0.0f, 0.0f, 1.0f),  btVector3(0.0, 0.0, 0.0)));

        shape_ = std::make_unique<btStaticPlaneShape>(btVector3(0, 1, 0), 0);

        body_ = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(
            0, motion_state_.get(), shape_.get(), btVector3(0, 0, 0)));

        shape_->setMargin(btScalar(0.001f));
        body_->setFriction(0.7f);
    }

    void render() override {
        glColor3ub( 0, 0, 255 );
        glBegin(GL_LINE);
            glVertex2f( 0.0, 100.0);
            glVertex2f( 100, 100 );
       glEnd();

    }
};

PhysicsWorld::PhysicsWorld()
          : collision_conf_(new btDefaultCollisionConfiguration()),
            collision_dispatcher_(new btCollisionDispatcher(collision_conf_.get())),
            broadphase_interface_(new btDbvtBroadphase()),
            solver_(new btSequentialImpulseConstraintSolver()),
            dynamics_world_(new btDiscreteDynamicsWorld(collision_dispatcher_.get(), broadphase_interface_.get(),
                                                      solver_.get(), collision_conf_.get())) {

        dynamics_world_->setGravity(btVector3(0.0f, -9.8, 0.0f));
}


struct HingeConstraint {
    std::unique_ptr<btHingeConstraint> constraint_;

    HingeConstraint(PhysicsWorld &world, BulletBody *bA, BulletBody *bB, btVector3 pivA, btVector3 pivB) {
       constraint_ = std::make_unique<btHingeConstraint>(
            *bA->body_, *bB->body_, pivA, pivB,  btVector3(0.0f, 0.0f, 1.0f), btVector3(0.0f, 0.0f, 1.0f), false);
       constraint_->setLimit(-M_PI, M_PI, 0.1f, 1.0) ;
                             /*btScalar _biasFactor = 0.3f, btScalar _relaxationFactor = 1.0f*/
       world.dynamics_world_->addConstraint(constraint_.get()) ;
    }
};


class glfwGUI: public glfwRenderWindow {
public:

    glfwGUI(PhysicsWorld &world): glfwRenderWindow(), world_(world) {
    }

    void onInit() {
        // this is needed for non core profiles or instead use gl3w
        glewExperimental = GL_TRUE ;

        GLenum err ;
        if ( ( err = glewInit() ) != GLEW_OK ) {
            cerr << glewGetErrorString(err) << endl;
            return ;
        }


         glClearColor(1, 1, 1, 1) ;

        glEnable(GL_SMOOTH);		// Enable (gouraud) shading

            glDisable(GL_DEPTH_TEST); 	// Disable depth testing

            glEnable(GL_BLEND);		// Enable blending (used for alpha) and blending function to use
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glLineWidth(5.0f);		// Set a 'chunky' line width

            glEnable(GL_LINE_SMOOTH);	// Enable anti-aliasing on lines

            glPointSize(5.0f);		// Set a 'chunky' point size

            glEnable(GL_POINT_SMOOTH);	// Enable anti-aliasing on points

    }

    void onResize(int width, int height) {
        glViewport(0, 0, width, height)  ;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, 0, height,  0, 1);
    }

    void onMouseButtonPressed(uint button, size_t x, size_t y, uint flags) override {

    }

    void onMouseButtonReleased(uint button, size_t x, size_t y, uint flags) override {

    }

    void onMouseMoved(double xpos, double ypos) override {
    }

    void onMouseWheel(double x) {

    }

    void onKeyPressed(size_t key_code, uint mods) {
        btVector3 impulse(0, 0, 0) ;

        if ( key_code == GLFW_KEY_LEFT ) {
            impulse = btVector3(-200, 0, 0) ;
        } else if ( key_code == GLFW_KEY_RIGHT ) {
            impulse = btVector3(200, 0, 0) ;
        }

        world_.bodies_[0]->body_->applyImpulse(impulse, btVector3(0, 0, 0)) ;
    }


    void onRender() {
        // Clear the screen

        world_.render() ;
        world_.step(0.05) ;
    }

   PhysicsWorld &world_ ;
};


int main(int argc, char *argv[]) {

    PhysicsWorld world ;

    GroundPlane *plane = new GroundPlane ;
    BoxBody *body1 = new BoxBody(10, btVector3(100, 100, 100), btVector3(150, 180, 0)) ;
    BoxBody *body2 = new BoxBody(6, btVector3(70, 70, 70), btVector3(320, 350, 0)) ;

    body1->setId("b1") ; body2->setId("b2") ;

    world.addBody(body1) ;
    world.addBody(body2) ;
    world.addBody(plane) ;
    HingeConstraint hinge(world, body1, body2, btVector3(100, 100, 0), btVector3(-70, -70, 0)) ;



    glfwGUI gui(world) ;
    gui.run(500, 500, "gui") ;

}
