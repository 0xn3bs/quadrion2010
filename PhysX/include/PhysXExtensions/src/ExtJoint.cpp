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

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "PxPhysics.h"
#include "PxConstraint.h"
#include "PxJoint.h"
#include "ExtJoint.h"
#include "ExtDistanceJoint.h"
#include "ExtD6Joint.h"
#include "ExtFixedJoint.h"
#include "ExtPrismaticJoint.h"
#include "ExtRevoluteJoint.h"
#include "ExtSphericalJoint.h"
// PX_SERIALIZATION
#include "CmSerialFramework.h"
//~PX_SERIALIZATION

using namespace Ext;

PxJoint* PxJointCreate(PxPhysics& physics, const PxJointDesc& desc)
{
	PxConstraintDesc nxDesc;
	nxDesc.actor[0]				= desc.actor[0];
	nxDesc.actor[1]				= desc.actor[1];
	nxDesc.flags				= desc.constraintFlags;
	nxDesc.linearBreakImpulse	= desc.breakForce;
	nxDesc.angularBreakImpulse	= desc.breakTorque;

	Ext::ConstraintInternalInterface* joint;

	switch(desc.getType())
	{
	case PxJointType::eDISTANCE:
		joint = Ext::DistanceJoint::create(static_cast<const PxDistanceJointDesc&>(desc));
		break;
	case PxJointType::eFIXED:
		joint = Ext::FixedJoint::create(static_cast<const PxFixedJointDesc&>(desc));
		break;
	case PxJointType::ePRISMATIC:
		joint = Ext::PrismaticJoint::create(static_cast<const PxPrismaticJointDesc&>(desc));
		break;
	case PxJointType::eREVOLUTE:
		joint = Ext::RevoluteJoint::create(static_cast<const PxRevoluteJointDesc&>(desc));
		break;
	case PxJointType::eSPHERICAL:
		joint = Ext::SphericalJoint::create(static_cast<const PxSphericalJointDesc&>(desc));
		break;
	case PxJointType::eD6:
		joint = Ext::D6Joint::create(static_cast<const PxD6JointDesc&>(desc));
		break;

	default: PX_ASSERT(0); break;
		//TODO: fill in break impulses for joints which have them
	}

	if(joint==0)
		return 0;

	joint->finishDesc(nxDesc);
	nxDesc.connector = joint->getConnector();
	PxConstraint* shader = physics.createConstraint(nxDesc);
	if(!shader)
	{
		delete joint;
		return 0;
	}

	joint->setShader(shader);

	return joint->getPxJoint();
}

// PX_SERIALIZATION
using namespace Ext;

PxConstraint* resolveConstraintPtr(PxRefResolver& v, 
								   PxConstraint* old, 
								   PxConstraintConnector* connector,
								   PxConstraintSolverPrep solverPrep,
								   void* solvelrPrepSpu,
								   PxU32 solverPrepSpuByteSize,
								   PxConstraintProject project,
								   PxConstraintVisualize visualize)
{
	PxSerializable* s = v.newAddress(old);
	if(!s)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "resolveConstraintPtr: constraint not found.");
		return NULL;
	}
	PxConstraint* new_nx = static_cast<PxConstraint*>(s);
	new_nx->setConstraintFunctions(connector, solverPrep, solvelrPrepSpu, solverPrepSpuByteSize, project, visualize);
	return new_nx;
}
//~PX_SERIALIZATION


static void normalToTangents(const PxVec3& n, PxVec3& t1, PxVec3& t2)
{
	const PxReal m_sqrt1_2 = PxReal(0.7071067811865475244008443621048490);
	if(fabsf(n.z) > m_sqrt1_2)
	{
		const PxReal a = n.y*n.y + n.z*n.z;
		const PxReal k = PxReal(1.0)/PxSqrt(a);
		t1 = PxVec3(0,-n.z*k,n.y*k);
		t2 = PxVec3(a*k,-n.x*t1.z,n.x*t1.y);
	}
	else 
	{
		const PxReal a = n.x*n.x + n.y*n.y;
		const PxReal k = PxReal(1.0)/PxSqrt(a);
		t1 = PxVec3(-n.y*k,n.x*k,0);
		t2 = PxVec3(-n.z*t1.y,n.z*t1.x,a*k);
	}
	t1.normalize();
	t2.normalize();
}

#include "PxMat33Legacy.h"
void PxSetJointGlobalFrame(PxJoint& joint, const PxVec3* wsAnchor, const PxVec3* axisIn)
{
	PxRigidActor* actors[2];
	joint.getActors(actors[0], actors[1]);

	PxTransform localPose[2];
	for(PxU32 i=0; i<2; i++)
		localPose[i] = PxTransform::createIdentity();

	// 1) global anchor
	if(wsAnchor)
	{
		//transform anchorPoint to local space
		for(PxU32 i=0; i<2; i++)
			localPose[i].p = actors[i] ? actors[i]->getGlobalPose().transformInv(*wsAnchor) : *wsAnchor;
	}

	// 2) global axis
	if(axisIn)
	{
		PxVec3 localAxis[2], localNormal[2];

		//find 2 orthogonal vectors.
		//gotta do this in world space, if we choose them
		//separately in local space they won't match up in worldspace.
		PxVec3 axisw = *axisIn;
		axisw.normalize();

		PxVec3 normalw, binormalw;
		::normalToTangents(axisw, binormalw, normalw);
		//because axis is supposed to be the Z axis of a frame with the other two being X and Y, we need to negate
		//Y to make the frame right handed. Note that the above call makes a right handed frame if we pass X --> Y,Z, so 
		//it need not be changed.

		for(PxU32 i=0; i<2; i++)
		{
			if(actors[i])
			{
				const PxTransform& m = actors[i]->getGlobalPose();
				PxMat33Legacy mM(m.q);
				localAxis[i]   = mM % axisw;
				localNormal[i] = mM % normalw;
			}
			else
			{
				localAxis[i] = axisw;
				localNormal[i] = normalw;
			}

			PxMat33Legacy rot;
			rot.setColumn(0, localAxis[i]);
			rot.setColumn(1, localNormal[i]);
			rot.setColumn(2, localAxis[i].cross(localNormal[i]));

			localPose[i].q = rot.toQuat();
			localPose[i].q.normalize();
		}
	}

	for(PxU32 i=0; i<2; i++)
		joint.setLocalPose(i, localPose[i]);
}
