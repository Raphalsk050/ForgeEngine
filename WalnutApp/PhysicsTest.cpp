#include "PhysicsTest.h"

PhysicsTest::PhysicsTest()
{
    
}

void PhysicsTest::InitPhysics()
{
    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    dispatcher = new btCollisionDispatcher(collisionConfiguration);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    overlappingPairCache = new btDbvtBroadphase();

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

    dynamicsWorld->setGravity(btVector3(0, -10, 0));

    //add a ground to world
    auto groundCol = CreateBoxCollider(btVector3(50, 0.5, 50));
    create_rigid_body_object(0., groundCol, btVector3(0, -56, 0));

    //add a sphere to world
    auto sphere_collider = CreateSphereCollider(1);
    create_rigid_body_object(10, sphere_collider, btVector3(0,50,0));
}


btRigidBody* PhysicsTest::create_rigid_body_object(float mass, btCollisionShape* collider, btVector3 origin)
{
    btTransform rbTransform;
    rbTransform.setIdentity();
    rbTransform.setOrigin(origin);
    btScalar bodyMass(mass);

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);

    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
        collider->calculateLocalInertia(bodyMass, localInertia);
    auto* myMotionState = new btDefaultMotionState(rbTransform);
    const btRigidBody::btRigidBodyConstructionInfo rb_info(mass, myMotionState, collider, localInertia);
    auto* body = new btRigidBody(rb_info);
    dynamicsWorld->addRigidBody(body);
    return body;
}

btCollisionShape* PhysicsTest::CreateBoxCollider(btVector3& half_extent)
{
    auto col_shape = new btStaticPlaneShape(btVector3(btScalar(0), btScalar(1), btScalar(0)), btScalar(0));
    collisionShapes.push_back(col_shape);
    return col_shape;
}

btCollisionShape* PhysicsTest::CreateSphereCollider(const btScalar radius)
{
    //btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
    auto col_shape = new btSphereShape(radius);
    collisionShapes.push_back(col_shape);
    return col_shape;
}

void PhysicsTest::Update() const
{
    dynamicsWorld->stepSimulation(1.f / 60.f, 0);

    //print positions of all objects
    for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
        btRigidBody* body = btRigidBody::upcast(obj);
        btTransform trans;
        if (body && body->getMotionState())
        {
            body->getMotionState()->getWorldTransform(trans);
        }
        else
        {
            trans = obj->getWorldTransform();
        }
        printf("world pos object %d = (%f,%f,%f)\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
    }
}

void PhysicsTest::CleanUp()
{
    //remove the rigidbodies from the dynamics world and delete them
    for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }

    //delete collision shapes
    for (int j = 0; j < collisionShapes.size(); j++)
    {
        btCollisionShape* shape = collisionShapes[j];
        collisionShapes[j] = 0;
        delete shape;
    }

    //delete dynamics world
    delete dynamicsWorld;

    //delete solver
    delete solver;

    //delete broadphase
    delete overlappingPairCache;

    //delete dispatcher
    delete dispatcher;

    delete collisionConfiguration;

    //next line is optional: it will be cleared by the destructor when the array goes out of scope
    collisionShapes.clear();
}
