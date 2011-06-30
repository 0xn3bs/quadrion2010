// This code contains NVIDIA Confidential Information and is disclosed to you 
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and 
// any modifications thereto. Any use, reproduction, disclosure, or 
// distribution of this software and related documentation without an express 
// license agreement from NVIDIA Corporation is strictly prohibited.
// 
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2011 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "PxTkFactory.h"
#include "PxSimpleFactory.h"
#include "deprecated/PxRigidStaticDesc.h"
#include "deprecated/PxRigidDynamicDesc.h"
#include "PxSphereGeometry.h"
#include "PxBoxGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PxConvexMeshGeometry.h"
#include "PxRigidBodyExt.h"
#include "PxRigidStatic.h"
#include "PxScene.h"
#include "PxShape.h"
#include "PxRigidDynamic.h"


/*
namespace
{
template<class A>
 A* setShape(A* actor, const PxGeometry& geometry, PxMaterial& material)
{
	if(!actor)
		return NULL;

	PxShape* shape = actor->createShape(geometry, material);
	if(!shape)
	{ 
		actor->release();
		return NULL;
	}
	return actor;
}

PxRigidDynamic* PxCreateDynamic(PxPhysics& sdk, const PxTransform& transform, const PxGeometry& geometry,
							    PxMaterial& material, PxReal density)
{
	PX_CHECK_VALID(transform);

	PxRigidDynamic* actor = setShape(sdk.createRigidDynamic(transform), geometry, material);
	if(actor)
		PxRigidBodyExt::updateMassAndInertia(*actor, density);
	return actor;
}


PxRigidDynamic* PxCreateDynamicWithNonSimShape(PxPhysics& sdk, const PxTransform& transform, const PxGeometry& geometry, PxMaterial& material)
{
	PX_CHECK_VALID(transform);

	PxRigidDynamic* actor = setShape(sdk.createRigidDynamic(transform), geometry, material);
	if(actor)
	{
		actor->setMass(1);
		actor->setMassSpaceInertiaTensor(PxVec3(1,1,1));
	}
	return actor;
}


PxRigidStatic* PxCreateStatic(PxPhysics& sdk, const PxTransform& transform, const PxGeometry& geometry, PxMaterial& material)
{
	PX_CHECK_VALID(transform);

	return setShape(sdk.createRigidStatic(transform), geometry, material);
}

}

*/

PxTkStaticCreationParams::PxTkStaticCreationParams() :
	mSceneQueryShape	(false),
	mDebugVisualization	(false),
	mTrigger			(false)
{
}

PxTkDynamicCreationParams::PxTkDynamicCreationParams() :
	mLinearDamping		(0.0f),
	mAngularDamping		(0.05f),
	mWakeCounter		(20.0f*0.02f),
	mDensity			(1.0f),
	mUseSweptBounds		(false),
	mKinematic			(false)
{
}

static void finishActor(PxScene* scene, PxRigidActor* actor, const PxTkStaticCreationParams& params, bool useSweptBounds)
{
	PxShape* shape;
	actor->getShapes(&shape, 1);
	shape->setFlags(PxShapeFlags(params.mTrigger ? PxShapeFlag::eTRIGGER_SHAPE : 0)
			      | PxShapeFlags(!params.mTrigger ? PxShapeFlag::eSIMULATION_SHAPE : 0)
				  | PxShapeFlags(params.mSceneQueryShape ? PxShapeFlag::eSCENE_QUERY_SHAPE : 0)
				  | PxShapeFlags(params.mDebugVisualization ? PxShapeFlag::eVISUALIZATION : 0)
				  | PxShapeFlags(useSweptBounds ? PxShapeFlag::eUSE_SWEPT_BOUNDS : 0));

	actor->setActorFlag(PxActorFlag::eVISUALIZATION, params.mDebugVisualization);
	if(scene)
		scene->addActor(*actor);
}

static PxRigidDynamic* createDynamicWithParams(PxPhysics& sdk, PxScene* scene, const PxTransform &transform, 
											  const PxGeometry &geometry, PxMaterial &material, const PxTkDynamicCreationParams& params)
{
	PxRigidDynamic *actor = !params.mKinematic ? PxCreateDynamic(sdk, transform, geometry, material, params.mDensity)
											   : PxCreateKinematic(sdk, transform, geometry, material, params.mDensity);
	if(actor)
	{
		actor->setLinearDamping(params.mLinearDamping);
		actor->setAngularDamping(params.mAngularDamping);
		actor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC,params.mKinematic);
		actor->wakeUp(params.mWakeCounter);
		finishActor(scene, actor, params, params.mUseSweptBounds);
	}
	
	return actor;
}

static PxRigidStatic* createStaticWithParams(PxPhysics& sdk, PxScene* scene, const PxTransform& transform, 
											 const PxGeometry& geometry, PxMaterial& material, const PxTkStaticCreationParams& params)
{
	PxRigidStatic* actor = PxCreateStatic(sdk, transform, geometry, material);
	if(actor)
		finishActor(scene, actor, params, false);

	return actor;
}

PxRigidDynamic* PxTkCreateDynamicSphere(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkDynamicCreationParams& params,
											const PxVec3& pos, float radius)
{
	return createDynamicWithParams(sdk, scene, PxTransform(pos), PxSphereGeometry(radius), *material, params);
}

PxRigidStatic* PxTkCreateStaticSphere(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkStaticCreationParams& params,
										const PxVec3& pos, float radius)
{
	return createStaticWithParams(sdk, scene, PxTransform(pos), PxSphereGeometry(radius), *material, params);
}

PxRigidDynamic* PxTkCreateDynamicBox(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkDynamicCreationParams& params,
										const PxVec3& pos, const PxQuat& rot, const PxVec3& dims)
{
	return createDynamicWithParams(sdk, scene, PxTransform(pos, rot), PxBoxGeometry(dims), *material, params);
}

PxRigidStatic* PxTkCreateStaticBox(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkStaticCreationParams& params,
									const PxVec3& pos, const PxQuat& rot, const PxVec3& dims)
{
	return createStaticWithParams(sdk, scene, PxTransform(pos, rot), PxBoxGeometry(dims), *material, params);
}

PxRigidDynamic* PxTkCreateDynamicCapsule(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkDynamicCreationParams& params,
											const PxVec3& pos, const PxQuat& rot, float radius, PxReal halfHeight)
{
	return createDynamicWithParams(sdk, scene, PxTransform(pos, rot), PxCapsuleGeometry(radius, halfHeight), *material, params);
}

PxRigidStatic* PxTkCreateStaticCapsule(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkStaticCreationParams& params,
										const PxVec3& pos, const PxQuat& rot, float radius, PxReal halfHeight)
{
	return createStaticWithParams(sdk, scene, PxTransform(pos, rot), PxCapsuleGeometry(radius, halfHeight), *material, params);
}

PxRigidDynamic* PxTkCreateDynamicConvex(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkDynamicCreationParams& params,
											PxConvexMesh* convexMesh, const PxVec3& pos, const PxQuat& rot)
{
	PX_ASSERT(convexMesh);
	return createDynamicWithParams(sdk, scene, PxTransform(pos, rot), PxConvexMeshGeometry(convexMesh), *material, params);
}

PxRigidStatic* PxTkCreateStaticConvex(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkStaticCreationParams& params,
										PxConvexMesh* convexMesh, const PxVec3& pos, const PxQuat& rot)
{
	PX_ASSERT(convexMesh);
	return createStaticWithParams(sdk, scene, PxTransform(pos, rot), PxConvexMeshGeometry(convexMesh), *material, params);
}
