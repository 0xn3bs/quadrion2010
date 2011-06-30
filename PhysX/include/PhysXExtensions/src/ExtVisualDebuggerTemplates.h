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


#ifndef EXT_VISUAL_DEBUGGER_TEMPLATES_H
#define EXT_VISUAL_DEBUGGER_TEMPLATES_H

#if PX_SUPPORT_VISUAL_DEBUGGER

#include "CmPhysXCommon.h"
#include "PvdConnectionTraits.h"
#include "ExtPvdClassDefinitions.h"
#include "ExtD6Joint.h"
#include "ExtDistanceJoint.h"
#include "ExtFixedJoint.h"
#include "ExtPrismaticJoint.h"
#include "ExtRevoluteJoint.h"
#include "ExtSphericalJoint.h"


namespace Ext
{
	template<typename TDataType> inline Pvd::PvdClassKeys::Enum GetPvdClassKeyForType() { PX_ASSERT( false ); return 0; }

	template<> Pvd::PvdClassKeys::Enum GetPvdClassKeyForType<PxJointLimitCone>() { return Pvd::PvdClassKeys::JointLimitCone; }
	template<> Pvd::PvdClassKeys::Enum GetPvdClassKeyForType<PxJointLimitPair>() { return Pvd::PvdClassKeys::JointLimitPair; }
	template<> Pvd::PvdClassKeys::Enum GetPvdClassKeyForType<PxJointLimit>() { return Pvd::PvdClassKeys::JointLimit; }
	template<> Pvd::PvdClassKeys::Enum GetPvdClassKeyForType<PxD6JointDrive>() { return Pvd::PvdClassKeys::JointDrive; }
	template<> Pvd::PvdClassKeys::Enum GetPvdClassKeyForType<PxJointLimitParameters>() { return Pvd::PvdClassKeys::JointLimitParameters; }


#define DEFINE_JOINT_SUBCLASS_IDS( joint )	\
		PxU64 jointId = PxU64(joint);		\
		PxU64 linearLimitId = jointId+1;	\
		PxU64 swingLimitId	= jointId+2;	\
		PxU64 twistLimitId	= jointId+3;	\
		PxU64 coneLimitId	= jointId+4;	\
		PxU64 drivexId		= jointId+5;	\
		PxU64 driveyId		= jointId+6;	\
		PxU64 drivezId		= jointId+7;	\
		PxU64 driveSwingId	= jointId+8;	\
		PxU64 driveTwistId	= jointId+9;	\
		PxU64 driveSlerpId	= jointId+10;

	template<class J>
	static PxU32 updateJoint(PVD::PvdDataStream& pvdConnection, const PxConstraint* c, const J* joint, PVD::PvdCommLayerValue* values, PxU32 maxValues) 
	{ 
		PX_ASSERT(0);
		return 0; 
	}


	template<class Base, PxJointType::Enum t>
	static void updateJointBase(const Joint<Base, t>* joint, const PxConstraint* c, PVD::PvdCommLayerValue* values)
	{
		PxRigidActor* actor0;
		PxRigidActor* actor1;
		PxReal brkForce, brkTorque;
		c->getActors(actor0, actor1);
		c->getBreakImpulse(brkForce, brkTorque);

		values[Pvd::JointProp::Actor0]			= PVD::createInstanceId(PxU64(actor0));
		values[Pvd::JointProp::Actor1]			= PVD::createInstanceId(PxU64(actor1));
		values[Pvd::JointProp::LocalPose0]		= PVD::createTransform(joint->getLocalPose(0));
		values[Pvd::JointProp::LocalPose1]		= PVD::createTransform(joint->getLocalPose(1));
		values[Pvd::JointProp::BreakForce]		= brkForce;
		values[Pvd::JointProp::BreakTorque]		= brkTorque;
		values[Pvd::JointProp::Name]			= PVD::createString(joint->getName());
		values[Pvd::JointProp::ConstraintFlags]	= PVD::createBitflag(c->getFlags());
	}

	template<>
	PxU32 updateJoint<D6JointT>(PVD::PvdDataStream& pvdConnection, const PxConstraint* c, const D6JointT* joint, PVD::PvdCommLayerValue* values, PxU32 maxValues)
	{
		PX_ASSERT(Pvd::D6JointProp::NUM_ELEMENTS <= maxValues);

		updateJointBase(joint, c, values);
		PxVec3 force, torque;
		c->getForce(force, torque);
		values[Pvd::JointProp::Force]			= PVD::createFloat3(force);
		values[Pvd::JointProp::Torque]			= PVD::createFloat3(torque);

		DEFINE_JOINT_SUBCLASS_IDS(joint);

		VisualDebugger::updateJointLimit(pvdConnection, joint->getLinearLimit(), linearLimitId);
		VisualDebugger::updateJointLimitCone(pvdConnection, joint->getSwingLimit(), swingLimitId);
		VisualDebugger::updateJointLimitPair(pvdConnection, joint->getTwistLimit(), twistLimitId);
		VisualDebugger::updateDrive(pvdConnection, joint->getDrive(PxD6Drive::eX), drivexId);
		VisualDebugger::updateDrive(pvdConnection, joint->getDrive(PxD6Drive::eY), driveyId);
		VisualDebugger::updateDrive(pvdConnection, joint->getDrive(PxD6Drive::eZ), drivezId);
		VisualDebugger::updateDrive(pvdConnection, joint->getDrive(PxD6Drive::eSWING), driveSwingId);
		VisualDebugger::updateDrive(pvdConnection, joint->getDrive(PxD6Drive::eTWIST), driveTwistId);
		VisualDebugger::updateDrive(pvdConnection, joint->getDrive(PxD6Drive::eSLERP), driveSlerpId);

		PxVec3 linVel, angVel;
		joint->getDriveVelocity(linVel, angVel);

		values[Pvd::D6JointProp::Motion_X]						= PVD::createEnumerationValue(joint->getMotion(PxD6Axis::eX));
		values[Pvd::D6JointProp::Motion_Y]						= PVD::createEnumerationValue(joint->getMotion(PxD6Axis::eY));
		values[Pvd::D6JointProp::Motion_Z]						= PVD::createEnumerationValue(joint->getMotion(PxD6Axis::eZ));
		values[Pvd::D6JointProp::Motion_Twist]					= PVD::createEnumerationValue(joint->getMotion(PxD6Axis::eTWIST));
		values[Pvd::D6JointProp::Motion_Swing1]					= PVD::createEnumerationValue(joint->getMotion(PxD6Axis::eSWING1));
		values[Pvd::D6JointProp::Motion_Swing2]					= PVD::createEnumerationValue(joint->getMotion(PxD6Axis::eSWING2));
		values[Pvd::D6JointProp::LinearLimit]					= PVD::createInstanceId(linearLimitId);
		values[Pvd::D6JointProp::SwingLimit]					= PVD::createInstanceId(swingLimitId);
		values[Pvd::D6JointProp::TwistLimit]					= PVD::createInstanceId(twistLimitId);
		values[Pvd::D6JointProp::LimitCone]						= PVD::createInstanceId(coneLimitId);
		values[Pvd::D6JointProp::Drive_X]						= PVD::createInstanceId(drivexId);
		values[Pvd::D6JointProp::Drive_Y]						= PVD::createInstanceId(driveyId);
		values[Pvd::D6JointProp::Drive_Z]						= PVD::createInstanceId(drivezId);
		values[Pvd::D6JointProp::Drive_Swing]					= PVD::createInstanceId(driveSwingId);
		values[Pvd::D6JointProp::Drive_Twist]					= PVD::createInstanceId(driveTwistId);
		values[Pvd::D6JointProp::Drive_Slerp]					= PVD::createInstanceId(driveSlerpId);
		values[Pvd::D6JointProp::DrivePosition]					= PVD::createTransform(joint->getDrivePosition());
		values[Pvd::D6JointProp::DriveLinearVelocity]			= PVD::createFloat3(linVel);
		values[Pvd::D6JointProp::DriveAngularVelocity]			= PVD::createFloat3(angVel);
		values[Pvd::D6JointProp::ProjectionLinearTolerance]		= joint->getProjectionLinearTolerance();
		values[Pvd::D6JointProp::ProjectionAngularTolerance]	= joint->getProjectionAngularTolerance();

		return Pvd::D6JointProp::NUM_ELEMENTS;
	}

	template<>
	PxU32 updateJoint<DistanceJointT>(PVD::PvdDataStream& pvdConnection, const PxConstraint* c, const DistanceJointT* joint, PVD::PvdCommLayerValue* values, PxU32 maxValues)
	{
		PX_ASSERT(Pvd::DistanceJointProp::NUM_ELEMENTS <= maxValues);

		updateJointBase(joint, c, values);
		PxVec3 force, torque;
		c->getForce(force, torque);
		values[Pvd::JointProp::Force]			= PVD::createFloat3(force);
		values[Pvd::JointProp::Torque]			= PVD::createFloat3(torque);

		values[Pvd::DistanceJointProp::MinDistance]		= joint->getMaxDistance();
		values[Pvd::DistanceJointProp::MaxDistance]		= joint->getMinDistance();
		values[Pvd::DistanceJointProp::Spring]			= joint->getSpring();
		values[Pvd::DistanceJointProp::Damping]			= joint->getDamping();
		values[Pvd::DistanceJointProp::Tolerance]		= joint->getTolerance();
		values[Pvd::DistanceJointProp::JointFlags]		= PVD::createBitflag(joint->getDistanceJointFlags());

		return Pvd::DistanceJointProp::NUM_ELEMENTS;
	}

	template<>
	PxU32 updateJoint<FixedJointT>(PVD::PvdDataStream& pvdConnection, const PxConstraint* c, const FixedJointT* joint, PVD::PvdCommLayerValue* values, PxU32 maxValues)
	{
		PX_ASSERT(Pvd::FixedJointProp::NUM_ELEMENTS <= maxValues);

		updateJointBase(joint, c, values);
		PxVec3 force, torque;
		c->getForce(force, torque);
		values[Pvd::JointProp::Force]			= PVD::createFloat3(force);
		values[Pvd::JointProp::Torque]			= PVD::createFloat3(torque);		
		
		values[Pvd::FixedJointProp::ProjectionLinearTolerance]	= joint->getProjectionLinearTolerance();
		values[Pvd::FixedJointProp::ProjectionAngularTolerance]	= joint->getProjectionAngularTolerance();

		return Pvd::FixedJointProp::NUM_ELEMENTS;
	}

	template<>
	PxU32 updateJoint<PrismaticJointT>(PVD::PvdDataStream& pvdConnection, const PxConstraint* c, const PrismaticJointT* joint, PVD::PvdCommLayerValue* values, PxU32 maxValues)
	{
		PX_ASSERT(Pvd::PrismaticJointProp::NUM_ELEMENTS <= maxValues);

		updateJointBase(joint, c, values);
		PxVec3 force, torque;
		c->getForce(force, torque);
		values[Pvd::JointProp::Force]			= PVD::createFloat3(force);
		values[Pvd::JointProp::Torque]			= PVD::createFloat3(torque);

		
		DEFINE_JOINT_SUBCLASS_IDS(joint);

		VisualDebugger::updateJointLimitPair(pvdConnection, joint->getLimit(), twistLimitId);

		values[Pvd::PrismaticJointProp::ProjectionLinearTolerance]	= joint->getProjectionLinearTolerance();
		values[Pvd::PrismaticJointProp::ProjectionAngularTolerance]	= joint->getProjectionAngularTolerance();
		values[Pvd::PrismaticJointProp::Limit]						= PVD::createInstanceId(twistLimitId);
		values[Pvd::PrismaticJointProp::JointFlags]					= PVD::createBitflag(joint->getPrismaticJointFlags());

		return Pvd::PrismaticJointProp::NUM_ELEMENTS;
	}

	template<>
	PxU32 updateJoint<RevoluteJointT>(PVD::PvdDataStream& pvdConnection, const PxConstraint* c, const RevoluteJointT* joint, PVD::PvdCommLayerValue* values, PxU32 maxValues)
	{
		PX_ASSERT(Pvd::RevoluteJointProp::NUM_ELEMENTS <= maxValues);

		updateJointBase(joint, c, values);
		PxVec3 force, torque;
		c->getForce(force, torque);
		values[Pvd::JointProp::Force]			= PVD::createFloat3(force);
		values[Pvd::JointProp::Torque]			= PVD::createFloat3(torque);

		DEFINE_JOINT_SUBCLASS_IDS(joint);
		PxU64 limit = jointId+3;

		VisualDebugger::updateJointLimitPair(pvdConnection, joint->getLimit(), twistLimitId);

		values[Pvd::RevoluteJointProp::Limit]						= PVD::createInstanceId(twistLimitId);
		values[Pvd::RevoluteJointProp::DriveVelocity]				= joint->getDriveVelocity();
		values[Pvd::RevoluteJointProp::DriveGearRatio]				= joint->getDriveGearRatio();
		values[Pvd::RevoluteJointProp::DriveForceLimit]				= joint->getDriveForceLimit();
		values[Pvd::RevoluteJointProp::ProjectionLinearTolerance]	= joint->getProjectionLinearTolerance();
		values[Pvd::RevoluteJointProp::ProjectionAngularTolerance]	= joint->getProjectionAngularTolerance();
		values[Pvd::RevoluteJointProp::JointFlags]					= PVD::createBitflag(joint->getRevoluteJointFlags());

		return Pvd::RevoluteJointProp::NUM_ELEMENTS;
	}

	template<>
	PxU32 updateJoint<SphericalJointT>(PVD::PvdDataStream& pvdConnection, const PxConstraint* c, const SphericalJointT* joint, PVD::PvdCommLayerValue* values, PxU32 maxValues)
	{
		PX_ASSERT(Pvd::SphericalJointProp::NUM_ELEMENTS <= maxValues);
		const SphericalJoint* j = static_cast<const SphericalJoint*>(joint);

		updateJointBase(joint, c, values);
		PxVec3 force, torque;
		c->getForce(force, torque);
		values[Pvd::JointProp::Force]			= PVD::createFloat3(force);
		values[Pvd::JointProp::Torque]			= PVD::createFloat3(torque);

		DEFINE_JOINT_SUBCLASS_IDS(joint);

		VisualDebugger::updateJointLimitCone(pvdConnection, joint->getLimitCone(), coneLimitId);

		values[Pvd::SphericalJointProp::LimitCone]					= PVD::createInstanceId(coneLimitId);
		values[Pvd::SphericalJointProp::ProjectionLinearTolerance]	= joint->getProjectionLinearTolerance();
		values[Pvd::SphericalJointProp::JointFlags]					= PVD::createBitflag(joint->getSphericalJointFlags());

		return Pvd::SphericalJointProp::NUM_ELEMENTS;
	}


	template<class Base, PxJointType::Enum t>
	void VisualDebugger::updatePvdProperties(PVD::PvdDataStream& pvdConnection, const PxConstraint* c, const Joint<Base, t>* joint)
	{
		if(!pvdConnection.isConnected())
			return;
		
		static const PxU32 maxValues = Pvd::D6JointProp::NUM_ELEMENTS;
		PVD::PvdCommLayerValue inValues[maxValues];
		PVD::PvdCommLayerDatatype dataTypes[maxValues];
		PxU32 propList[maxValues];

		PxU32 numValues = updateJoint(pvdConnection, c, joint, inValues, maxValues);
		Pvd::PvdClassKeys::Enum classKey = Pvd::PvdClassKeys::Enum(Pvd::PvdClassKeys::D6Joint + t);

		for(PxU32 i = 0; i < numValues; i++)
		{
			dataTypes[i] = inValues[i].getDatatype();
			propList[i] = i+1;
		}

		PvdNameSpace ns(pvdConnection, "physx3Ext");
		pvdConnection.beginPropertyBlock(classKey+1, propList, dataTypes, numValues);
		pvdConnection.sendPropertyBlock(PxU64(joint), inValues);
		pvdConnection.endPropertyBlock();
	}

	template<class Base, PxJointType::Enum t>
	bool VisualDebugger::simUpdate(PVD::PvdDataStream& pvdConnection, const PxConstraint* c, const Joint<Base, t>* joint)
	{
		if(!pvdConnection.isConnected())
			return false;

		PxVec3 linear, angular;
		PxU32 propList[4] = { Pvd::JointProp::Force+1, Pvd::JointProp::Torque+1, Pvd::JointProp::ConstraintFlags+1, 0};
		Pvd::PvdClassKeys::Enum classKey = Pvd::PvdClassKeys::Enum(Pvd::PvdClassKeys::D6Joint + t);
		c->getForce(linear, angular);

		PVD::PvdCommLayerValue values[4];
		PVD::PvdCommLayerDatatype types[4] = { PVD::PvdCommLayerDatatype::Float3, PVD::PvdCommLayerDatatype::Float3, PVD::PvdCommLayerDatatype::Bitflag, PVD::PvdCommLayerDatatype::Bitflag };
		
		values[0] = PVD::createFloat3(linear);
		values[1] = PVD::createFloat3(angular);
		values[2] = PVD::createBitflag(c->getFlags());
		
		PxU32 propertyCount = 3;
		const PxJoint *jointbase = static_cast<const PxJoint*>(joint);
		switch(t)
		{
			case PxJointType::eDISTANCE:
				values[3] = PVD::createBitflag(static_cast<const PxDistanceJoint*>(jointbase)->getDistanceJointFlags());
				propList[3] = Pvd::DistanceJointProp::JointFlags + 1;
				propertyCount = 4;
				break;
			case PxJointType::eREVOLUTE:
				values[3] = PVD::createBitflag(static_cast<const PxRevoluteJoint*>(jointbase)->getRevoluteJointFlags());
				propList[3] = Pvd::RevoluteJointProp::JointFlags + 1;
				propertyCount = 4;
				break;
			case PxJointType::eSPHERICAL:
				propertyCount = 3;
				break;
		}

		PVD::PvdCommLayerError error;
		PvdNameSpace ns(pvdConnection, "physx3Ext");
		error = pvdConnection.beginPropertyBlock(classKey+1, propList, types, propertyCount);
		error = pvdConnection.sendPropertyBlock(PxU64(joint), values);
		error = pvdConnection.endPropertyBlock();
		
		return (error == PVD::PvdCommLayerError::None);
	}

	template<class Base, PxJointType::Enum t>
	void VisualDebugger::createPvdInstance(PVD::PvdDataStream& pvdConnection, const PxConstraint* c, const Joint<Base, t>* joint)
	{
		if(!pvdConnection.isConnected())
			return;

		DEFINE_JOINT_SUBCLASS_IDS(joint);

		PvdNameSpace ns(pvdConnection, "physx3Ext");
		Pvd::PvdClassKeys::Enum classKey = Pvd::PvdClassKeys::Enum(Pvd::PvdClassKeys::D6Joint + t);
		pvdConnection.createInstance(classKey+1, PxU64(joint), PVD::EInstanceUIFlags::TopLevel);
		pvdConnection.addChild(PxU64(c->getScene())+JOINT_GROUP+1, jointId);

		// create additional instances for aggregates structs:
		pvdConnection.createInstance(GetPvdClassKeyForType<PxJointLimit>()+1, linearLimitId, PVD::EInstanceUIFlags::Hidden);
		pvdConnection.createInstance(GetPvdClassKeyForType<PxJointLimitCone>()+1, swingLimitId, PVD::EInstanceUIFlags::Hidden);
		pvdConnection.createInstance(GetPvdClassKeyForType<PxJointLimitPair>()+1, twistLimitId, PVD::EInstanceUIFlags::Hidden);
		pvdConnection.createInstance(GetPvdClassKeyForType<PxJointLimitCone>()+1, coneLimitId, PVD::EInstanceUIFlags::Hidden);
		pvdConnection.createInstance(GetPvdClassKeyForType<PxD6JointDrive>()+1, drivexId, PVD::EInstanceUIFlags::Hidden);
		pvdConnection.createInstance(GetPvdClassKeyForType<PxD6JointDrive>()+1, driveyId, PVD::EInstanceUIFlags::Hidden);
		pvdConnection.createInstance(GetPvdClassKeyForType<PxD6JointDrive>()+1, drivezId, PVD::EInstanceUIFlags::Hidden);
		pvdConnection.createInstance(GetPvdClassKeyForType<PxD6JointDrive>()+1, driveSwingId, PVD::EInstanceUIFlags::Hidden);
		pvdConnection.createInstance(GetPvdClassKeyForType<PxD6JointDrive>()+1, driveTwistId, PVD::EInstanceUIFlags::Hidden);
		pvdConnection.createInstance(GetPvdClassKeyForType<PxD6JointDrive>()+1, driveSlerpId, PVD::EInstanceUIFlags::Hidden);

		updatePvdProperties(pvdConnection, c, joint);
	}
}


#endif // EXT_VISUAL_DEBUGGER_TEMPLATES_H
#endif // EXT_VISUAL_DEBUGGER_H
