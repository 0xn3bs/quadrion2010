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


#if PX_SUPPORT_VISUAL_DEBUGGER

#include "PxProfileBase.h"
#include "PxVisualDebuggerExt.h"
#include "ExtVisualDebugger.h"
#include "ExtPvdClassDefinitions.h"

#include "ExtD6Joint.h"
#include "ExtFixedJoint.h"
#include "ExtSphericalJoint.h"
#include "ExtDistanceJoint.h"
#include "ExtSphericalJoint.h"
#include "ExtRevoluteJoint.h"
#include "ExtPrismaticJoint.h"

namespace Ext
{

	VisualDebugger::PvdNameSpace::PvdNameSpace(PVD::PvdDataStream& conn, const char* name)
		: mConnection(conn)
	{
		conn.pushNamespace();
		conn.setNamespace(name);
	}

	VisualDebugger::PvdNameSpace::~PvdNameSpace()
	{
		mConnection.popNamespace();
	}

	void VisualDebugger::updateDrive(PVD::PvdDataStream& pvdConnection, const PxD6JointDrive& drive, PxU64 id)
	{
		static const PxU32 numValues = Pvd::JointDriveProp::NUM_ELEMENTS;
		PVD::PvdCommLayerValue values[numValues];
		PVD::PvdCommLayerDatatype dataTypes[numValues];
		PxU32 propList[numValues];

		values[Pvd::JointDriveProp::Spring]			= drive.spring;
		values[Pvd::JointDriveProp::Damping]		= drive.damping;
		values[Pvd::JointDriveProp::ForceLimit]		= drive.forceLimit;
		values[Pvd::JointDriveProp::Flags]			= PVD::createBitflag(drive.flags);

		for(PxU32 i = 0; i < numValues; i++)
		{
			dataTypes[i] = values[i].getDatatype();
			propList[i] = i+1;
		}

		PvdNameSpace ns(pvdConnection, "physx3Ext");
		pvdConnection.beginPropertyBlock(Pvd::PvdClassKeys::JointDrive+1, propList, dataTypes, numValues);
		pvdConnection.sendPropertyBlock(PxU64(id), values);
		pvdConnection.endPropertyBlock();
	}

	void VisualDebugger::updateJointLimitParameters(const PxJointLimitParameters& param, PVD::PvdCommLayerValue* values)
	{
		values[Pvd::JointLimitParametersProp::Restitution]	= param.restitution;
		values[Pvd::JointLimitParametersProp::Spring]		= param.spring;
		values[Pvd::JointLimitParametersProp::Damping]		= param.damping;
		values[Pvd::JointLimitParametersProp::ContactDistance]	= param.contactDistance;
	}

	void VisualDebugger::updateJointLimit(PVD::PvdDataStream& pvdConnection, const PxJointLimit& limit, PxU64 id)
	{
		static const PxU32 numValues = Pvd::JointLimitProp::NUM_ELEMENTS;
		PVD::PvdCommLayerValue values[numValues];
		PVD::PvdCommLayerDatatype dataTypes[numValues];
		PxU32 propList[numValues];

		updateJointLimitParameters(limit, values);

		values[Pvd::JointLimitProp::Value]	= limit.value;
		
		for(PxU32 i = 0; i < numValues; i++)
		{
			dataTypes[i] = values[i].getDatatype();
			propList[i] = i+1;
		}

		PvdNameSpace ns(pvdConnection, "physx3Ext");
		pvdConnection.beginPropertyBlock(Pvd::PvdClassKeys::JointLimit+1, propList, dataTypes, numValues);
		pvdConnection.sendPropertyBlock(PxU64(id), values);
		pvdConnection.endPropertyBlock();
	}

	void VisualDebugger::updateJointLimitPair(PVD::PvdDataStream& pvdConnection, const PxJointLimitPair& limit, PxU64 id)
	{
		static const PxU32 numValues = Pvd::JointLimitPairProp::NUM_ELEMENTS;
		PVD::PvdCommLayerValue values[numValues];
		PVD::PvdCommLayerDatatype dataTypes[numValues];
		PxU32 propList[numValues];

		updateJointLimitParameters(limit, values);

		values[Pvd::JointLimitPairProp::Upper]	= limit.upper;
		values[Pvd::JointLimitPairProp::Lower]	= limit.lower;
		
		for(PxU32 i = 0; i < numValues; i++)
		{
			dataTypes[i] = values[i].getDatatype();
			propList[i] = i+1;
		}

		PvdNameSpace ns(pvdConnection, "physx3Ext");
		pvdConnection.beginPropertyBlock(Pvd::PvdClassKeys::JointLimitPair+1, propList, dataTypes, numValues);
		pvdConnection.sendPropertyBlock(PxU64(id), values);
		pvdConnection.endPropertyBlock();
	}

	void VisualDebugger::updateJointLimitCone(PVD::PvdDataStream& pvdConnection, const PxJointLimitCone& limit, PxU64 id)
	{
		static const PxU32 numValues = Pvd::JointLimitConeProp::NUM_ELEMENTS;
		PVD::PvdCommLayerValue values[numValues];
		PVD::PvdCommLayerDatatype dataTypes[numValues];
		PxU32 propList[numValues];

		updateJointLimitParameters(limit, values);

		values[Pvd::JointLimitConeProp::Y_Angle]	= limit.yAngle;
		values[Pvd::JointLimitConeProp::Z_Angle]	= limit.zAngle;
		
		for(PxU32 i = 0; i < numValues; i++)
		{
			dataTypes[i] = values[i].getDatatype();
			propList[i] = i+1;
		}

		PvdNameSpace ns(pvdConnection, "physx3Ext");
		pvdConnection.beginPropertyBlock(Pvd::PvdClassKeys::JointLimitCone+1, propList, dataTypes, numValues);
		pvdConnection.sendPropertyBlock(PxU64(id), values);
		pvdConnection.endPropertyBlock();
	}

	void VisualDebugger::releasePvdInstance(PVD::PvdDataStream& pvdConnection, const PxJoint* joint)
	{
		if(!pvdConnection.isConnected())
			return;

		PvdNameSpace ns(pvdConnection, "physx3Ext");
		pvdConnection.destroyInstance(PX_PROFILE_POINTER_TO_U64(joint));
		// release additional instances of aggregates structs:
		for(PxU32 i = 1; i <= 10; i++)
			pvdConnection.destroyInstance(PX_PROFILE_POINTER_TO_U64(joint)+i);
	}

	bool VisualDebugger::sendClassDescriptions(PVD::PvdDataStream& pvdConnection)
	{
		PVD::PvdCommLayerError error;
		PvdNameSpace ns(pvdConnection, "physx3Ext");

		// register all classes, see ExtPvdClassDefinitions.h
		for(PxU32 i = 0; i < Pvd::PvdClassKeys::NUM_ELEMENTS; i++)
		{
			const Pvd::ClassRow& classRow = Pvd::gClassKeyTable[i];
			PxU32 classKey = classRow.classKey;
			if ( !classRow.physxNamespace )
			{
				error = pvdConnection.pushNamespace();
				error = pvdConnection.setNamespace("");
			}
			
			error = pvdConnection.createClass(classRow.name, classKey);
			PX_ASSERT(error == PVD::PvdCommLayerError::None);
			if(classRow.parentId != (PxU16)-1)
			{
				error = pvdConnection.deriveClass(classRow.parentId+1, classKey);
				PX_ASSERT(error == PVD::PvdCommLayerError::None);
			}

			// register properties
			PxU32 numProperties = classRow.propertyTableSize;
			const Pvd::PropertyRow* propertyTable = classRow.propertyTable;
			for(PxU32 p = 0; p < numProperties; p++)
			{
				const Pvd::PropertyRow& prop = propertyTable[p];

				PxU32 propertyKey = classRow.propertyOffset + p+1;
				if (prop.dataType != (PxU32) -1)
					error = pvdConnection.defineProperty(classKey, prop.name, NULL, prop.dataType, propertyKey);
				else
					error = pvdConnection.defineArrayProperty(classKey, prop.name, prop.arrayClass, propertyKey);

				if(prop.dataType == PVD::PvdCommLayerDatatype::Bitflag)
					error = pvdConnection.defineBitflagNames(classKey, propertyKey, prop.table, prop.size);
				else if(prop.dataType == PVD::PvdCommLayerDatatype::EnumerationValue)
					error = pvdConnection.defineEnumerationNames(classKey, propertyKey, prop.table, prop.size);
					
				PX_ASSERT(error == PVD::PvdCommLayerError::None);
			}
			if ( !classRow.physxNamespace )
				error = pvdConnection.popNamespace();
		}
		return true;
	}
}

#else

#include "CmPhysXCommon.h"
#include "PxVisualDebuggerExt.h"

namespace PVD
{
	class PvdDataStream;
}

#endif // PX_SUPPORT_VISUAL_DEBUGGER



