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


#ifndef NP_FIXEDJOINTCONSTRAINT_H
#define NP_FIXEDJOINTCONSTRAINT_H

#include "ExtJoint.h"
#include "PxFixedJoint.h"
#include "deprecated/PxFixedJointDesc.h"

namespace Ext
{

	struct FixedJointData : public JointData
	{
		PxReal	projectionLinearTolerance;
		PxReal	projectionAngularTolerance;
	};

typedef Joint<PxFixedJoint, PxJointType::eFIXED> FixedJointT;

	class FixedJoint : public FixedJointT
	{
	public:
// PX_SERIALIZATION
									FixedJoint(PxRefResolver& v)	: FixedJointT(v)	{}
									DECLARE_SERIAL_CLASS(FixedJoint, FixedJointT)
		virtual		void			exportExtraData(PxSerialStream& stream);
		virtual		char*			importExtraData(char* address, PxU32& totalPadding);
		virtual		bool			resolvePointers(PxRefResolver&, void*);
		static		void			getMetaData(PxSerialStream& stream);
//~PX_SERIALIZATION
		virtual						~FixedJoint()
		{
			if(ownsMemory())
				PX_FREE(mData);
		}

		PxReal	getProjectionLinearTolerance() const;
		void	setProjectionLinearTolerance(PxReal tolerance);

		PxReal	getProjectionAngularTolerance() const;
		void	setProjectionAngularTolerance(PxReal tolerance);

		static ConstraintInternalInterface* create(const PxFixedJointDesc& desc)
		{
			return PX_NEW(FixedJoint)(desc);
		}

		FixedJoint(const PxFixedJointDesc& desc): FixedJointT(desc)
		 {
// PX_SERIALIZATION
			setType(PxSerialType::eUSER_FIXED_JOINT);
//~PX_SERIALIZATION
			FixedJointData* data = reinterpret_cast<FixedJointData*>(PX_ALLOC(sizeof(FixedJointData)));
			mData = data;
			data->projectionLinearTolerance = desc.projectionLinearTolerance;
			data->projectionAngularTolerance = desc.projectionAngularTolerance;

			initCommonData(*data,desc);
		 }

	private:
		void			finishDesc(PxConstraintDesc& desc);

		PX_FORCE_INLINE FixedJointData& data() const				
		{	
			return *static_cast<FixedJointData*>(mData);
		}
	};
} // namespace Ext

namespace Ext
{
	PxU32 FixedJointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 maxConstraints,
		const void* constantBlock,
		const PxTransform& bA2w,
		const PxTransform& bB2w);
}

#endif
