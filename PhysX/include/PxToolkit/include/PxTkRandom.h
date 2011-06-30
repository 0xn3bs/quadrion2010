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

#ifndef PX_TOOLKIT_RANDOM_H
#define PX_TOOLKIT_RANDOM_H

#include "common/PxPhysXCommon.h"

	namespace PxToolkit
	{

	class BasicRandom
	{
		public:
									BasicRandom(PxU32 seed=0)	: mRnd(seed)	{}
									~BasicRandom()								{}

		PX_FORCE_INLINE	void		setSeed(PxU32 seed)			{ mRnd = seed;											}
		PX_FORCE_INLINE	PxU32		getCurrentValue()	const	{ return mRnd;											}
						PxU32		randomize()					{ mRnd = mRnd * 2147001325 + 715136305; return mRnd;	}

		PX_FORCE_INLINE	PxU32		rand()						{ return randomize() & 0xffff;							}

						PxF32		rand(PxF32 a, PxF32 b)
									{
										const PxF32 r = (PxF32)rand()/((PxF32)0x7fff+1);
										return r*(b-a) + a;
									}

						PxI32		rand(PxI32 a, PxI32 b)
									{
										return a + (PxI32)(rand()%(b-a));
									}

						PxF32		randomFloat()
									{
										return (PxF32(randomize() & 0xffff)/65535.0f) - 0.5f;
									}

						void		unitRandomPt(PxVec3& v);
						void		unitRandomQuat(PxQuat& v);
		private:
						PxU32		mRnd;
	};

	}

#endif