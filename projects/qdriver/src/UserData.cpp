// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                          USER DATA
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include "UserData.h"

void AddUserDataToActors(void* scene)
{
	unsigned int i = 0;

	void** actors = scene->getActors();
	unsigned int nbActors = scene->getNbActors();
	while (nbActors--)
	{
		void* actor = actors[nbActors];
		actor->userData = new ActorUserData;
		((ActorUserData *)(actor->userData))->id = i++;
		// FIXME: After setting it, actorDesc.managedHwSceneIndex gets cleared for some reason...
#if 0
		voidDesc actorDesc;
		actor->saveToDesc(actorDesc);
		if (actorDesc.managedHwSceneIndex > 0)  
		{
			// To identify the actor when render it, set a bit in the userdata flags
			((ActorUserData *)(actor->userData))->flags |= UD_IS_HW_ACTOR;
		}
#endif
		AddUserDataToShapes(actor);
	}
}

void AddUserDataToShapes(void* actor)
{
	unsigned int i = 0;

	NxShape*const* shapes = actor->getShapes();
	unsigned int nbShapes = actor->getNbShapes();
	while (nbShapes--)
	{
		NxShape* shape = shapes[nbShapes];
		shape->userData = new ShapeUserData;
		ShapeUserData* sud = (ShapeUserData*)(shape->userData);
		sud->id = i++;
		if (shape->getType() == NX_SHAPE_CONVEX)
		{
			sud->mesh = new NxConvexMeshDesc;
			shape->isConvexMesh()->getConvexMesh().saveToDesc(*(NxConvexMeshDesc*)sud->mesh);
		}
		if (shape->getType() == NX_SHAPE_MESH)
		{
			sud->mesh = new NxTriangleMeshDesc;
			shape->isTriangleMesh()->getTriangleMesh().saveToDesc(*(NxTriangleMeshDesc*)sud->mesh);
		}
	}
}

void ReleaseUserDataFromActors(void* scene)
{
	void** actors = scene->getActors();
	unsigned int nbActors = scene->getNbActors();
	while (nbActors--)
	{
		void* actor = actors[nbActors];
		if (actor->userData)  delete actor->userData;
		ReleaseUserDataFromShapes(actor);
	}
}

void ReleaseUserDataFromShapes(void* actor)
{
	NxShape*const* shapes = actor->getShapes();
	unsigned int nbShapes = actor->getNbShapes();
	while (nbShapes--)
	{
		NxShape* shape = shapes[nbShapes];
		if (shape->userData)
		{
			ShapeUserData* sud = (ShapeUserData*)(shape->userData);
			if (sud && sud->mesh)
			{
				delete sud->mesh;
			}
		}
	}
}


