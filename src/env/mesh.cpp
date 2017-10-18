#include <vsim/env/mesh.hpp>

using namespace std ;

namespace vsim {

MeshPtr Mesh::createWireCube(float hs) {

    MeshPtr m(new Mesh) ;
    m->vertices_ = {{ -hs, +hs, +hs }, { +hs, +hs, +hs }, { +hs, -hs, +hs }, { -hs, -hs, +hs },
                    { -hs, +hs, -hs }, { +hs, +hs, -hs }, { +hs, -hs, -hs }, { -hs, -hs, -hs } } ;
    m->vertex_indices_ = {  0, 1, 1, 2, 2, 3, 3, 0,  4, 5, 5, 6, 6, 7,  7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };

    m->ptype_ = Lines ;

    return m ;
}

MeshPtr Mesh::createSolidCube(float hs) {
    MeshPtr m(new Mesh) ;
    m->normals_ = {{ 0.0, -1.0, 0.0 }, {0.0, 1.0, 0.0}, { 1.0, 0.0, 0.0 }, {0.0, 0.0, 1.0}, {-1.0, 0.0, 0.0}, { 0.0, 0.0, -1.0}} ;
    m->vertices_ = {{ +hs, -hs, -hs }, { +hs, -hs, +hs }, { -hs, -hs, +hs }, { -hs, -hs, -hs },
                    { +hs, +hs, -hs }, { +hs, +hs, +hs }, { -hs, +hs, +hs }, { -hs, +hs, -hs } } ;
    m->vertex_indices_ = {  1, 3, 0, 7, 5, 4, 4, 1, 0, 5, 2, 1, 2, 7, 3, 0, 7, 4, 1, 2, 3, 7, 6, 5, 4, 5, 1, 5, 6, 2, 2, 6, 7, 0, 3, 7};
    m->normal_indices_ = {  0, 0, 0,  1, 1, 1,  2, 2, 2,  3, 3, 3,  4, 4, 4,  5, 5, 5,  0, 0, 0,  1, 1, 1,  2, 2, 2,  3, 3, 3,  4, 4, 4, 5, 5, 5};
    m->ptype_ = Triangles ;
    return m ;
}

// adapted from freeglut

static void makeCircleTable(vector<float> &sint, vector<float> &cost, int n) {

    /* Table size, the sign of n flips the circle direction */

    const size_t size = abs(n);

    /* Determine the angle between samples */

    const float angle = 2*M_PI/(float)( ( n == 0 ) ? 1 : n );

    sint.resize(size+1) ; cost.resize(size+1) ;

    /* Compute cos and sin around the circle */

    sint[0] = 0.0;
    cost[0] = 1.0;

    for ( size_t i =1 ; i<size; i++ ) {
        sint[i] = sin(angle*i);
        cost[i] = cos(angle*i);
    }

    /* Last sample is duplicate of the first */

    sint[size] = sint[0];
    cost[size] = cost[0];
}

MeshPtr Mesh::createSolidCone(float radius, float height, size_t slices, size_t stacks)
{
    MeshPtr m(new Mesh) ;
    m->ptype_ = Mesh::Triangles ;

    float z0,z1;
    float r0,r1;

    const float zStep = height / std::max(stacks, (size_t)1) ;
    const float rStep = radius / std::max(stacks, (size_t)1) ;

    const float cosn = ( height / sqrt ( height * height + radius * radius ));
    const float sinn = ( radius / sqrt ( height * height + radius * radius ));

    vector<float> sint, cost;
    makeCircleTable( sint, cost, slices);

    /* Cover the circular base with a triangle fan... */

    z0 = 0.0;
    z1 = z0+ zStep;

    r0 = radius ;
    r1 = r0 - rStep;

    // the base of the cone is on (0, 0, 0) aligned with the z-axis and pointing towards positive z

    m->vertices_.push_back({0, 0, z0}) ;
    m->normals_.push_back({0, 0, -1}) ;

    for( uint i=0 ; i<slices ; i++ ) {
        m->vertices_.push_back({cost[i]*r0, sint[i]*r0, z0}) ;
    }

    for( uint i=0 ; i<slices ; i++ ) {

        m->vertex_indices_.push_back(i+1) ;
        m->vertex_indices_.push_back(0) ;
        m->vertex_indices_.push_back(i == slices-1 ? 1 : i+2) ;

        m->normal_indices_.push_back(0) ;
        m->normal_indices_.push_back(0) ;
        m->normal_indices_.push_back(0) ;
    }

    // normals shared by all side vertices

    for( uint i=0 ; i<slices ; i++ ) {
        m->normals_.push_back({cost[i]*sinn, sint[i]*sinn, cosn}) ;
    }

    for( size_t j = 1;  j < stacks; j++ ) {

        for( uint i=0 ; i<slices ; i++ ) {
            m->vertices_.push_back({cost[i]*r1, sint[i]*r1, z1}) ;
        }

        for( uint i=0 ; i<slices ; i++ ) {
            size_t pn = ( i == slices - 1 ) ? 0 : i+1 ;
            m->vertex_indices_.push_back((j-1)*slices + i + 1) ;
            m->vertex_indices_.push_back((j-1)*slices + pn + 1) ;
            m->vertex_indices_.push_back((j)*slices + pn + 1) ;

            m->vertex_indices_.push_back((j-1)*slices + i + 1) ;
            m->vertex_indices_.push_back((j)*slices + pn + 1) ;
            m->vertex_indices_.push_back((j)*slices + i + 1) ;

            m->normal_indices_.push_back(i + 1) ;
            m->normal_indices_.push_back(pn + 1) ;
            m->normal_indices_.push_back(pn + 1) ;

            m->normal_indices_.push_back(i + 1) ;
            m->normal_indices_.push_back(pn + 1) ;
            m->normal_indices_.push_back(i + 1) ;
        }

        z1 += zStep;
        r1 -= rStep ;
    }

    // link apex with last stack

    size_t offset = (stacks - 1)*slices + 1;

    m->vertices_.push_back({0, 0, z1}) ;

    for( uint i=0 ; i<slices ; i++ ) {
        size_t pn = ( i == slices - 1 ) ? 0 : i+1 ;
        m->vertex_indices_.push_back(offset + i) ;
        m->vertex_indices_.push_back(offset + pn) ;
        m->vertex_indices_.push_back(offset + slices) ;

        m->normal_indices_.push_back(i + 1) ;
        m->normal_indices_.push_back(pn + 1) ;
        m->normal_indices_.push_back(i + 1) ;
    }

    return m ;
}

MeshPtr Mesh::createWireCone(float radius, float height, size_t slices, size_t stacks)
{
    MeshPtr m(new Mesh) ;
    m->ptype_ = Mesh::Lines ;

    float z0,z1;
    float r0;

    vector<float> sint, cost;
    makeCircleTable( sint, cost, slices);

    z0 = 0.0;
    r0 = radius ;
    z1 = z0 + height ;

    for( uint i=0 ; i<slices ; i++ ) {
        m->vertices_.push_back({cost[i]*r0, sint[i]*r0, z0}) ;
    }

    m->vertices_.push_back({0, 0, z1}) ;

    for( uint i=0 ; i<slices-1 ; i++ ) {
        m->vertex_indices_.push_back(i) ;
        m->vertex_indices_.push_back(i+1) ;
    }
    m->vertex_indices_.push_back(slices-1) ;
    m->vertex_indices_.push_back(0) ;

    for( uint i=0 ; i<slices ; i++ ) {
        m->vertex_indices_.push_back(i) ;
        m->vertex_indices_.push_back(slices) ;
    }

    return m ;
}

MeshPtr Mesh::createSolidCylinder(float radius, float height, size_t slices, size_t stacks)
{
    MeshPtr m(new Mesh) ;
    m->ptype_ = Mesh::Triangles ;

    float z0,z1;

    const float zStep = height / std::max(stacks, (size_t)1) ;

    vector<float> sint, cost;
    makeCircleTable( sint, cost, slices);

    /* Cover the circular base with a triangle fan... */

    z0 = 0.0;
    z1 = z0 + zStep;

    m->vertices_.push_back({0, 0, z0}) ;
    m->normals_.push_back({0, 0, -1}) ;

    for( uint i=0 ; i<slices ; i++ ) {
        m->vertices_.push_back({cost[i]*radius, sint[i]*radius, z0}) ;
    }

    for( uint i=0 ; i<slices ; i++ ) {

        m->vertex_indices_.push_back(i+1) ;
        m->vertex_indices_.push_back(0) ;
        m->vertex_indices_.push_back(i == slices-1 ? 1 : i+2) ;

        m->normal_indices_.push_back(0) ;
        m->normal_indices_.push_back(0) ;
        m->normal_indices_.push_back(0) ;
    }

    // normals shared by all side vertices

    for( uint i=0 ; i<slices ; i++ ) {
        m->normals_.push_back({cost[i], sint[i], 1.0}) ;
    }

    for( size_t j = 1;  j <= stacks; j++ ) {

        for( uint i=0 ; i<slices ; i++ ) {
            m->vertices_.push_back({cost[i]*radius, sint[i]*radius, z1}) ;
        }

        for( uint i=0 ; i<slices ; i++ ) {
            size_t pn = ( i == slices - 1 ) ? 0 : i+1 ;
            m->vertex_indices_.push_back((j-1)*slices + i + 1) ;
            m->vertex_indices_.push_back((j-1)*slices + pn + 1) ;
            m->vertex_indices_.push_back((j)*slices + pn + 1) ;

            m->vertex_indices_.push_back((j-1)*slices + i + 1) ;
            m->vertex_indices_.push_back((j)*slices + pn + 1) ;
            m->vertex_indices_.push_back((j)*slices + i + 1) ;

            m->normal_indices_.push_back(i + 1) ;
            m->normal_indices_.push_back(pn + 1) ;
            m->normal_indices_.push_back(pn + 1) ;

            m->normal_indices_.push_back(i + 1) ;
            m->normal_indices_.push_back(pn + 1) ;
            m->normal_indices_.push_back(i + 1) ;
        }

        z1 += zStep;
    }

    // link apex with last stack

    size_t offset = (stacks)*slices + 1;

    m->vertices_.push_back({0, 0, height}) ;
    m->normals_.push_back({0, 0, 1}) ;

    for( uint i=0 ; i<slices ; i++ ) {
        size_t pn = ( i == slices - 1 ) ? 0 : i+1 ;
        m->vertex_indices_.push_back(offset + i) ;
        m->vertex_indices_.push_back(offset + pn) ;
        m->vertex_indices_.push_back(offset + slices) ;

        m->normal_indices_.push_back(slices+1) ;
        m->normal_indices_.push_back(slices+1) ;
        m->normal_indices_.push_back(slices+1) ;
    }

    return m ;
}

MeshPtr Mesh::createWireCylinder(float radius, float height, size_t slices, size_t stacks)
{
    MeshPtr m(new Mesh) ;
    m->ptype_ = Mesh::Lines ;

    float z0,z1;

    const float zStep = height / std::max(stacks, (size_t)1) ;

    vector<float> sint, cost;
    makeCircleTable( sint, cost, slices);

    z0 = 0.0;

    for( uint i=0 ; i<slices ; i++ ) {
        m->vertices_.push_back({cost[i]*radius, sint[i]*radius, z0}) ;
    }

    for( uint i=0 ; i<slices ; i++ ) {
        m->vertices_.push_back({cost[i]*radius, sint[i]*radius, z0 + height}) ;
    }

    for( uint i=0 ; i<slices-1 ; i++ ) {
        m->vertex_indices_.push_back(i) ;
        m->vertex_indices_.push_back(i+1) ;
    }
    m->vertex_indices_.push_back(slices-1) ;
    m->vertex_indices_.push_back(0) ;

    uint offset = slices ;

    for( uint i=0 ; i<slices-1 ; i++ ) {
        m->vertex_indices_.push_back(offset + i) ;
        m->vertex_indices_.push_back(offset + i+1) ;
    }
    m->vertex_indices_.push_back(offset + slices-1) ;
    m->vertex_indices_.push_back(offset) ;

    for( uint i=0 ; i<slices ; i++ ) {

        m->vertex_indices_.push_back(i) ;
        m->vertex_indices_.push_back(i + offset) ;
    }

    return m ;
}

}
