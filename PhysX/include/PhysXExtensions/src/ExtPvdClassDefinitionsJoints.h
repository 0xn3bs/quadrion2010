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



#ifndef EXT_PVD_CLASSDEFINITIONS_JOINTS_H
#define EXT_PVD_CLASSDEFINITIONS_JOINTS_H

#if PX_SUPPORT_VISUAL_DEBUGGER

#include "PxConstraintDesc.h"
#include "PxD6Joint.h"
#include "PxDistanceJoint.h"
#include "PxPrismaticJoint.h"

#include "PxRevoluteJoint.h"
#include "PxSphericalJoint.h"

namespace Pvd
{


// ----------------------------- PxD6JointDrive -------------------------------- //

static const PVD::NamedValueDefinition gJointDriveFlags[1] =
{
	{ "PxD6JointDrive", PxD6JointDriveFlag::eACCELERATION }
};

struct JointDriveProp
{
	enum Enum
	{
		Spring,
		Damping,
		ForceLimit,
		Flags,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gJointDriveProp[JointDriveProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(Spring, Float),
	PROPERTY_ROW(Damping, Float),
	PROPERTY_ROW(ForceLimit, Float),
	BITFLAG_PROPERTY_ROW(Flags, gJointDriveFlags),
};

// ----------------------------- PxJointLimitParameters -------------------------------- //

struct JointLimitParametersProp
{
	enum Enum
	{
		Restitution,
		Spring,
		Damping,
		ContactDistance,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gJointLimitParametersProp[JointLimitParametersProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(Restitution, Float),
	PROPERTY_ROW(Spring, Float),
	PROPERTY_ROW(Damping, Float),
	PROPERTY_ROW(ContactDistance, Float),
};


// ----------------------------- PxJointLimit -------------------------------- //

struct JointLimitProp
{
	enum Enum
	{
		Value = JointLimitParametersProp::NUM_ELEMENTS,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gJointLimitProp[JointLimitProp::NUM_ELEMENTS-JointLimitParametersProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(Value, Float),
};

// ----------------------------- PxJointLimitPair -------------------------------- //

struct JointLimitPairProp
{
	enum Enum
	{
		Upper = JointLimitParametersProp::NUM_ELEMENTS,
		Lower,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gJointLimitPairProp[JointLimitPairProp::NUM_ELEMENTS-JointLimitParametersProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(Upper, Float),
	PROPERTY_ROW(Lower, Float),
};

// ----------------------------- PxJointLimitCone -------------------------------- //

struct JointLimitConeProp
{
	enum Enum
	{
		Y_Angle = JointLimitParametersProp::NUM_ELEMENTS,
		Z_Angle,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gJointLimitConeProp[JointLimitConeProp::NUM_ELEMENTS-JointLimitParametersProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(Y_Angle, Float),
	PROPERTY_ROW(Z_Angle, Float),
};

// ----------------------------- PxJoint -------------------------------- //

static const PVD::NamedValueDefinition gConstraintFlags[5] =
{
	FLAG_ROW(PxConstraintFlag, eBROKEN),
	FLAG_ROW(PxConstraintFlag, ePROJECTION),
	FLAG_ROW(PxConstraintFlag, eCOLLISION_ENABLED),
	FLAG_ROW(PxConstraintFlag, eREPORTING),
	FLAG_ROW(PxConstraintFlag, eVISUALIZATION),
};

static const PVD::NamedValueDefinition gJointFlags[5] =
{
	FLAG_ROW(PxConstraintFlag, eBROKEN),
	FLAG_ROW(PxConstraintFlag, ePROJECTION),
	FLAG_ROW(PxConstraintFlag, eCOLLISION_ENABLED),
	FLAG_ROW(PxConstraintFlag, eREPORTING),
	FLAG_ROW(PxConstraintFlag, eVISUALIZATION),
};

struct JointProp
{
	enum Enum
	{
		Actor0,
		Actor1,
		LocalPose0,
		LocalPose1,
		BreakForce,
		BreakTorque,
		Force,
		Torque,
		Name,
		ConstraintFlags,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gJointProp[JointProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(Actor0, ObjectId),
	PROPERTY_ROW(Actor1, ObjectId),
	PROPERTY_ROW(LocalPose0, Transform),
	PROPERTY_ROW(LocalPose1, Transform),
	PROPERTY_ROW(BreakForce, Float),
	PROPERTY_ROW(BreakTorque, Float),
	PROPERTY_ROW(Force, Float3),
	PROPERTY_ROW(Torque, Float3),
	PROPERTY_ROW(Name, String),
	BITFLAG_PROPERTY_ROW(ConstraintFlags, gConstraintFlags)
};

// ----------------------------- PxD6Joint -------------------------------- //

static const PVD::NamedValueDefinition gD6MotionEnum[3] =
{
	FLAG_ROW(PxD6Motion, eLOCKED),
	FLAG_ROW(PxD6Motion, eLIMITED),
	FLAG_ROW(PxD6Motion, eFREE),
};

struct D6JointProp
{
	enum Enum
	{
		Motion_X = JointProp::NUM_ELEMENTS,
		Motion_Y,
		Motion_Z,
		Motion_Twist,
		Motion_Swing1,
		Motion_Swing2,
		LinearLimit,
		SwingLimit,
		TwistLimit,
		LimitCone,
		Drive_X,
		Drive_Y,
		Drive_Z,
		Drive_Swing,
		Drive_Twist,
		Drive_Slerp,
		DrivePosition,
		DriveLinearVelocity,
		DriveAngularVelocity,
		ProjectionLinearTolerance,
		ProjectionAngularTolerance,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gD6JointProp[D6JointProp::NUM_ELEMENTS-JointProp::NUM_ELEMENTS] =
{
	ENUM_PROPERTY_ROW(Motion.X, gD6MotionEnum),
	ENUM_PROPERTY_ROW(Motion.Y, gD6MotionEnum),
	ENUM_PROPERTY_ROW(Motion.Z, gD6MotionEnum),
	ENUM_PROPERTY_ROW(Motion.Twist, gD6MotionEnum),
	ENUM_PROPERTY_ROW(Motion.Swing1, gD6MotionEnum),
	ENUM_PROPERTY_ROW(Motion.Swing2, gD6MotionEnum),
	PROPERTY_ROW(LinearLimit, ObjectId),
	PROPERTY_ROW(SwingLimit, ObjectId),
	PROPERTY_ROW(TwistLimit, ObjectId),
	PROPERTY_ROW(LimitCone, ObjectId),
	PROPERTY_ROW(Drive.X, ObjectId),
	PROPERTY_ROW(Drive.Y, ObjectId),
	PROPERTY_ROW(Drive.Z, ObjectId),
	PROPERTY_ROW(Drive.Swing, ObjectId),
	PROPERTY_ROW(Drive.Twist, ObjectId),
	PROPERTY_ROW(Drive.Slerp, ObjectId),
	PROPERTY_ROW(Drive.Position, Transform),
	PROPERTY_ROW(Drive.LinearVelocity, Float3),
	PROPERTY_ROW(Drive.AngularVelocity, Float3),
	PROPERTY_ROW(ProjectionLinearTolerance, Float),
	PROPERTY_ROW(ProjectionAngularTolerance, Float),
};

// ----------------------------- PxDistanceJoint -------------------------------- //

static const PVD::NamedValueDefinition gDistanceJointFlags[8] =
{
	FLAG_ROW(PxDistanceJointFlag, eMAX_DISTANCE_ENABLED),
	FLAG_ROW(PxDistanceJointFlag, eMIN_DISTANCE_ENABLED),
	FLAG_ROW(PxDistanceJointFlag, eSPRING_ENABLED),
};

struct DistanceJointProp
{
	enum Enum
	{
		MinDistance = JointProp::NUM_ELEMENTS,
		MaxDistance,
		Spring,
		Damping,
		Tolerance,
		JointFlags,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gDistanceJointProp[DistanceJointProp::NUM_ELEMENTS-JointProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(MinDistance, Float),
	PROPERTY_ROW(MaxDistance, Float),
	PROPERTY_ROW(Spring, Float),
	PROPERTY_ROW(Damping, Float),
	PROPERTY_ROW(Tolerance, Float),
	BITFLAG_PROPERTY_ROW(JointFlags, gDistanceJointFlags),
};

// ----------------------------- PxFixedJoint -------------------------------- //

struct FixedJointProp
{
	enum Enum
	{
		ProjectionLinearTolerance = JointProp::NUM_ELEMENTS,
		ProjectionAngularTolerance,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gFixedJointProp[FixedJointProp::NUM_ELEMENTS-JointProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(ProjectionLinearTolerance, Float),
	PROPERTY_ROW(ProjectionAngularTolerance, Float),
};

// ----------------------------- PxPrismaticJoint -------------------------------- //


static const PVD::NamedValueDefinition gPrismaticJointFlags[8] =
{
	FLAG_ROW(PxPrismaticJointFlag, eLIMIT_ENABLED),
};

struct PrismaticJointProp
{
	enum Enum
	{
		ProjectionLinearTolerance = JointProp::NUM_ELEMENTS,
		ProjectionAngularTolerance,
		Limit,
		JointFlags,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gPrismaticJointProp[PrismaticJointProp::NUM_ELEMENTS-JointProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(ProjectionLinearTolerance, Float),
	PROPERTY_ROW(ProjectionAngularTolerance, Float),
	PROPERTY_ROW(Limit, ObjectId),
	BITFLAG_PROPERTY_ROW(JointFlags, gPrismaticJointFlags),
};

// ----------------------------- PxRevoluteJoint -------------------------------- //

static const PVD::NamedValueDefinition gRevoluteJointFlags[6] =
{
	FLAG_ROW(PxRevoluteJointFlag, eLIMIT_ENABLED),
	FLAG_ROW(PxRevoluteJointFlag, eDRIVE_ENABLED),
	FLAG_ROW(PxRevoluteJointFlag, eDRIVE_FREESPIN),
};

struct RevoluteJointProp
{
	enum Enum
	{
		Limit = JointProp::NUM_ELEMENTS,
		DriveVelocity,
		DriveForceLimit,
		DriveGearRatio,
		ProjectionLinearTolerance,
		ProjectionAngularTolerance,
		JointFlags,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gRevoluteJointProp[RevoluteJointProp::NUM_ELEMENTS-JointProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(Limit, ObjectId),
	PROPERTY_ROW(DriveVelocity, Float),
	PROPERTY_ROW(DriveForceLimit, Float),
	PROPERTY_ROW(DriveGearRatio, Float),
	PROPERTY_ROW(ProjectionLinearTolerance, Float),
	PROPERTY_ROW(ProjectionAngularTolerance, Float),
	BITFLAG_PROPERTY_ROW(JointFlags, gRevoluteJointFlags),
};

// ----------------------------- PxSphericalJoint -------------------------------- //


static const PVD::NamedValueDefinition gSphericalJointFlags[8] =
{
	FLAG_ROW(PxSphericalJointFlag, eLIMIT_ENABLED),
};

struct SphericalJointProp
{
	enum Enum
	{
		LimitCone = JointProp::NUM_ELEMENTS,
		ProjectionLinearTolerance,
		JointFlags,
		NUM_ELEMENTS,
	};
};

static const PropertyRow gSphericalJointProp[SphericalJointProp::NUM_ELEMENTS-JointProp::NUM_ELEMENTS] =
{
	PROPERTY_ROW(LimitCone, ObjectId),
	PROPERTY_ROW(ProjectionLinearTolerance, Float),
	BITFLAG_PROPERTY_ROW(JointFlags, gSphericalJointFlags),
};

// ------------------------------------------------------------- //

} // namespace Pvd

#endif // PX_SUPPORT_VISUAL_DEBUGGER
#endif // EXT_PVD_CLASSDEFINITIONS_JOINTS_H

