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

#ifndef PX_TOOLKIT_FACTORY_H
#define PX_TOOLKIT_FACTORY_H

#include "common/PxPhysXCommon.h"

	class PxPhysics;
	class PxScene;
	class PxMaterial;
	class PxRigidDynamic;
	class PxRigidStatic;
	class PxConvexMesh;

	struct PxTkStaticCreationParams
	{
				PxTkStaticCreationParams();

		bool	mSceneQueryShape;
		bool	mDebugVisualization;
		bool	mTrigger;
	};

	struct PxTkDynamicCreationParams : PxTkStaticCreationParams
	{
				PxTkDynamicCreationParams();

		PxReal	mLinearDamping;
		PxReal	mAngularDamping;
		PxReal	mWakeCounter;
		PxReal	mDensity;
		bool	mUseSweptBounds;
		bool	mKinematic;
	};

	PxRigidDynamic* PxTkCreateDynamicSphere(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkDynamicCreationParams& params,
																		const PxVec3& pos, float radius);
	PxRigidStatic*	PxTkCreateStaticSphere(		PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkStaticCreationParams& params,
																		const PxVec3& pos, float radius);

	PxRigidDynamic* PxTkCreateDynamicBox(		PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkDynamicCreationParams& params,
																		const PxVec3& pos, const PxQuat& rot, const PxVec3& dims);
	PxRigidStatic*	PxTkCreateStaticBox(		PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkStaticCreationParams& params,
																		const PxVec3& pos, const PxQuat& rot, const PxVec3& dims);

	PxRigidDynamic* PxTkCreateDynamicCapsule(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkDynamicCreationParams& params,
																		const PxVec3& pos, const PxQuat& rot, float radius, PxReal halfHeight);
	PxRigidStatic*	PxTkCreateStaticCapsule(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkStaticCreationParams& params,
																		const PxVec3& pos, const PxQuat& rot, float radius, PxReal halfHeight);

	PxRigidDynamic* PxTkCreateDynamicConvex(	PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkDynamicCreationParams& params,
																		PxConvexMesh* convexMesh, const PxVec3& pos, const PxQuat& rot);
	PxRigidStatic*	PxTkCreateStaticConvex(		PxPhysics& sdk, PxScene* scene, PxMaterial* material, const PxTkStaticCreationParams& params,
																		PxConvexMesh* convexMesh, const PxVec3& pos, const PxQuat& rot);

#endif