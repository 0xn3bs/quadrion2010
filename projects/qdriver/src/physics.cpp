#include "fps.h"
#include "physics.h"
#include "qtimer.h"
#include "scene.h"
#include "cGUI.h"

////// Bullet /////////////
#include "BspLoader.h"
///////////////////////////


// temp
#include "serverBrowser.h"
#include "input.h"

// Actor globals
NxActor* groundPlane = NULL;
NxActor* box = NULL;

// Simulation globals
NxReal	gDeltaTime = 1.0/60.0;
CTimer	physTimer;
vec3f	gDefaultGravity(0, -23.0, 0); // Meters per second

// tmp
int		lblCam;
bool	gJump = false;
NxF32	gV0;
NxF32	jumpTime;


struct EventInformation
{
	int		size;
	NxU32	events;
};



/////////////////////////////////////////////////
// Bullet Debug Render class implementation 
/////////////////////////////////////////////////


DebugRender::DebugRender()
:m_debugMode(0)
{
	m_debugMode = btIDebugDraw::DBG_DrawConstraints+btIDebugDraw::DBG_DrawConstraintLimits+btIDebugDraw::DBG_DrawWireframe+btIDebugDraw::DBG_DrawContactPoints;/*+btIDebugDraw::DBG_DrawAabb;*/
}


void DebugRender::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
{
	vec3f fromVec(from.getX(), from.getY(), from.getZ());
	vec3f toVec(to.getX(), to.getY(), to.getZ());

	
	
	unsigned char fromR = (unsigned char)255 * (fromColor.getX() / 1.0F);
	unsigned char fromG = (unsigned char)255 * (fromColor.getY() / 1.0F);
	unsigned char fromB = (unsigned char)255 * (fromColor.getZ() / 1.0F);
	unsigned char toR = (unsigned char)255 * (toColor.getX() / 1.0F);
	unsigned char toG = (unsigned char)255 * (toColor.getY() / 1.0F);
	unsigned char toB =	(unsigned char)255 * (toColor.getZ() / 1.0F);
	
	g_pRender->RenderLine(fromVec, toVec, QRENDER_MAKE_ARGB(0xFF, fromR, fromG, fromB), QRENDER_MAKE_ARGB(0xFF, toR, toG, toB));
}


void DebugRender::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	vec3f fromVec(from.getX(), from.getY(), from.getZ());
	vec3f toVec(to.getX(), to.getY(), to.getZ());

	unsigned char r = (unsigned char)255 * (color.getX() / 1.0F);
	unsigned char g = (unsigned char)255 * (color.getY() / 1.0F);
	unsigned char b = (unsigned char)255 * (color.getZ() / 1.0F);
	
	g_pRender->RenderLine(fromVec, toVec, QRENDER_MAKE_ARGB(0xFF, r, g, b));	
}

void DebugRender::drawSphere(const btVector3& p, btScalar radius, const btVector3& color)
{
	mat4 S, T, F, tmp;
	vec3f center(p.getX(), p.getY(), p.getZ());
	QMATH_MATRIX_LOADSCALE(S, vec3f(radius, radius, radius));
	QMATH_MATRIX_LOADTRANSLATION(T, center);
	QMATH_MATRIX_MULTIPLY(T, S, F);
	
	g_pRender->GetMatrix(QRENDER_MATRIX_MODEL, tmp);
	g_pRender->SetMatrix(QRENDER_MATRIX_MODEL, F);
	
	g_pRender->RenderSphere();
	
	g_pRender->SetMatrix(QRENDER_MATRIX_MODEL, tmp);
}

void DebugRender::drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3& color, btScalar)
{
	vec3f a(v0.getX(), v0.getY(), v0.getZ());
	vec3f b(v1.getX(), v1.getY(), v1.getZ());
	vec3f c(v2.getX(), v2.getY(), v2.getZ());
	
	unsigned char r = (unsigned char)255 * (color.getX() / 1.0F);
	unsigned char g = (unsigned char)255 * (color.getY() / 1.0F);
	unsigned char blue = (unsigned char)255 * (color.getZ() / 1.0F);
	
	g_pRender->RenderTriangle(a, b, c, QRENDER_MAKE_ARGB(0xFF, r, g, blue));	
}

void DebugRender::drawBox(const btVector3& boxMin, const btVector3& boxMax, const btVector3& color, btScalar alpha)
{
	vec3f mins(boxMin.getX(), boxMin.getY(), boxMin.getZ());
	vec3f maxs(boxMax.getX(), boxMax.getY(), boxMax.getZ());
	
	unsigned char r = (unsigned char)255 * (color.getX() / 1.0F);
	unsigned char g = (unsigned char)255 * (color.getY() / 1.0F);
	unsigned char b = (unsigned char)255 * (color.getZ() / 1.0F);
	
	g_pRender->RenderBox(mins, maxs, QRENDER_MAKE_ARGB(0xFF, r, g, b));	
}



/////////// Bullet ///////////////////////////////

class BspToBulletConverter : public BspConverter
{
public:



		virtual void	addConvexVerticesCollider(btAlignedObjectArray<btVector3>& vertices, bool isEntity, const btVector3& entityTargetLocation)
		{
			
			///perhaps we can do something special with entities (isEntity)
			///like adding a collision Triggering (as example)
			
			if (vertices.size() > 0)
			{
				vec3f normal_basis(0, 1, 0);
				vec3f bsp_basis(0, 0, -1);
		
				mat4 desired_rotation;
				mat3 flip_rotation;
				
				QMATH_MATRIX_VECTORTOVECTOR(flip_rotation, normal_basis, bsp_basis);
				QMATH_MATRIX_COPY3TO4(desired_rotation, flip_rotation);
			
			
				//btAlignedObjectArray<btCollisionShape*>	*shapes = gPhys->GetCollisionShapes();
				float mass = 0.f;
				btTransform startTransform;
				startTransform.setFromOpenGLMatrix((const btScalar*)desired_rotation);
				
				//can use a shift
//				startTransform.setIdentity();
//				startTransform.setRotation(btQuaternion(0, -1, 1, 0));
				//startTransform.setOrigin(btVector3(0,0,-10.f));
				//this create an internal copy of the vertices
				
				btCollisionShape* shape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
				//*shapes.push_back(shape);

				//btRigidBody* body = m_demoApp->localCreateRigidBody(mass, startTransform,shape);
				gPhys->localCreateRigidBody(mass, startTransform,shape);
				
			}
		}
};




btRigidBody*	cPhysics::localCreateRigidBody(float mass, const btTransform& startTransform,btCollisionShape* shape)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);

	btRigidBody* body = new btRigidBody(cInfo);

#else
	btRigidBody* body = new btRigidBody(mass,0,shape,localInertia);	
	body->setWorldTransform(startTransform);
#endif//

	m_dynamicsWorld->addRigidBody(body);

	return body;
}

void cPhysics::RenderDebug()
{
	vec3f normal_basis(0, 1, 0);
	vec3f bsp_basis(0, 0, -1);
	
	mat4 previous_world, desired_rotation;
	g_pRender->GetMatrix(QRENDER_MATRIX_MODEL, previous_world);
	
	mat3 flip_rotation;
	QMATH_MATRIX_VECTORTOVECTOR(flip_rotation, normal_basis, bsp_basis);
	QMATH_MATRIX_COPY3TO4(desired_rotation, flip_rotation);
	
	//g_pRender->SetMatrix(QRENDER_MATRIX_MODEL, desired_rotation);

	m_dynamicsWorld->debugDrawWorld();
	
	//g_pRender->SetMatrix(QRENDER_MATRIX_MODEL, previous_world);
}

////////////////////////////////////////////////////










class TriggerCallback : public NxUserTriggerReport
{
public:
	virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
	{
		// other actor is a trigger too?
		if ((NxI32)(otherShape.getActor().userData) < 0)
			return;

		NxActor& triggerActor = triggerShape.getActor();
		NxI32 triggerNumber = -(NxI32)triggerActor.userData;
		NxI32 triggerIndex = triggerNumber - 1;

		if(status & NX_TRIGGER_ON_ENTER)
		{
			// A body entered the trigger area for the first time
			//gNbTouchedBodies[triggerIndex]++;
		}
		if(status & NX_TRIGGER_ON_LEAVE)
		{
			// A body left the trigger area
			//gNbTouchedBodies[triggerIndex]--;
		}
		//NX_ASSERT(gNbTouchedBodies[triggerIndex]>=0);	//Shouldn't go negative
	}

} TriggerReport;



NxActor	*CreateTrigger(cQBSPEntity *entity, const vec3f& pos, NxF32 size, const vec3f* initial_velocity, bool kinematic)
{
	return NULL;
}


NxCCDSkeleton *cPhysics::CreateCCDBox(vec3f dimensions)
{
	return NULL;
}

NxActor *cPhysics::CreateBoundingBox(const vec3f& pos, vec3f min, vec3f max, const vec3f* initialVelocity)
{
	return NULL;
}

NxActor *cPhysics::CreateCustomCapsule(const vec3f& pos, float height, float radius, const vec3f* initialVelocity, bool ccd)
{
	return NULL;
}


void physCmdCB(std::vector<std::string> argv)
{
	
}



cPhysics::cPhysics()
{
	
} 

cPhysics::~cPhysics()
{
	
}

bool cPhysics::InitNX()
{
	return false;
}

void cPhysics::ReleaseNX()
{
	

}


void cPhysics::UpdatePhysics()
{
	
}

void cPhysics::StartPhysics()
{
	
}


void cPhysics::SetActorCollisionGroup(NxActor* actor, NxCollisionGroup group)
{
	
}


NxActor *cPhysics::BuildCollisionTree(NxTriangleMeshDesc *brushData, const char* mapName)
{
	return NULL;
}

void cPhysics::UpdateCharacters(NxReal deltaTime)
{
	
}



float cPhysics::castRayDist(vec3f origin, vec3f dir)
{
	return -1;
}

void cPhysics::PostUpdateCharacter()
{
	
}

NxActor *cPhysics::createDynamicObject( char *modelName, vec3f pos )
{
	return NULL;
}

NxCapsuleController* cPhysics::CreateCharacterController(vec3f pos, NxReal scale, NxScene& scene)
{
	return NULL;
}

void cPhysics::ReleaseCharacterControllers()
{
	
}

NxU32 cPhysics::MoveCharacter(NxController* character, NxScene& scene, const vec3f& dispVector, NxF32 elapsedTime, NxU32 collisionGroups, NxF32 heightDelta)
{
	return NULL;
}

unsigned int cPhysics::MoveCharacter(btKinematicCharacterController* character, const vec3f dispVector, float elapsedTime)
{
	return 0;
}

const NxExtendedVec3& cPhysics::GetCharacterPos(NxController *character)
{
		return NxExtendedVec3(0,0,0);
}

void cPhysics::BeginPhysicsSimulation()
{
	
}


void cPhysics::SetGravity(vec3f newGrav)
{
	
}