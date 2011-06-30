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



#ifndef EXT_PVD_CLASSDEFINITIONS_H
#define EXT_PVD_CLASSDEFINITIONS_H

#if PX_SUPPORT_VISUAL_DEBUGGER
namespace Pvd
{
// ----------------------------- ALL CLASSES -------------------------------- //

struct PvdClassKeys
{
	enum Enum
	{
		JointDrive,
		JointLimitParameters,
		JointLimit,
		JointLimitPair,
		JointLimitCone,
		Joint,
		D6Joint,
		DistanceJoint,
		FixedJoint,
		PrismaticJoint,
		RevoluteJoint,
		SphericalJoint,
		NUM_ELEMENTS,
	};
};

}

#include "ExtPvdClassDefinitionStructs.h"
#include "ExtPvdClassDefinitionsJoints.h"

namespace Pvd
{

static const ClassRow gClassKeyTable[PvdClassKeys::NUM_ELEMENTS] =
{
	CLASS_ROW(JointDrive),
	CLASS_ROW(JointLimitParameters),
	DERIVED_CLASS_ROW(JointLimit, JointLimitParameters),
	DERIVED_CLASS_ROW(JointLimitPair, JointLimitParameters),
	DERIVED_CLASS_ROW(JointLimitCone, JointLimitParameters),
	CLASS_ROW(Joint),
	DERIVED_CLASS_ROW(D6Joint, Joint),
	DERIVED_CLASS_ROW(DistanceJoint, Joint),
	DERIVED_CLASS_ROW(FixedJoint, Joint),
	DERIVED_CLASS_ROW(PrismaticJoint, Joint),
	DERIVED_CLASS_ROW(RevoluteJoint, Joint),
	DERIVED_CLASS_ROW(SphericalJoint, Joint),
};

// ------------------------------------------------------------- //

} // namespace Pvd

#endif // PX_SUPPORT_VISUAL_DEBUGGER
#endif // EXT_PVD_CLASSDEFINITIONS_H

