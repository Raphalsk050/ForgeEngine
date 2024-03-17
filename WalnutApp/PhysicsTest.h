#pragma once
#include <iostream>
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

using namespace std;
class PhysicsTest
{
public:
	PhysicsTest();
	void InitPhysics();
	btDynamicsWorld* dynamicsWorld;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	btRigidBody* create_rigid_body_object(float mass, btCollisionShape* collider, btVector3 origin = btVector3(0, 0, 0));
	btCollisionShape* CreateBoxCollider(btVector3 &half_extent);
	btCollisionShape* CreateSphereCollider(btScalar radius);
	
	void Update() const;
	void CleanUp();
};

