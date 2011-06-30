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


#include "PxParticleExt.h"
#include "particles/PxParticleReadData.h"
#include "CmPhysXCommon.h"
#include "PsArray.h"
#include "PsHash.h"
#include "PsBitUtils.h"
#include "PxBounds3.h"
#include "PsMathUtils.h"
#include "PsIntrinsics.h"

//----------------------------------------------------------------------------//

const static PxU32 sInvalidIndex = 0xffffffff;

//----------------------------------------------------------------------------//

struct CellCoords
{
	//! Set grid cell coordinates based on a point in space and a scaling factor
	PX_INLINE void set(const PxVec3& realVec, PxReal scale)
	{
		x = static_cast<PxI16>(Ps::floor(realVec.x * scale));
		y = static_cast<PxI16>(Ps::floor(realVec.y * scale));
		z = static_cast<PxI16>(Ps::floor(realVec.z * scale));
	}

	PX_INLINE bool operator==(const CellCoords& v) const
	{
		return ((x == v.x) && (y == v.y) && (z == v.z));
	}

	PxI16 x;
	PxI16 y;
	PxI16 z;
};

//----------------------------------------------------------------------------//

struct Cell
{
	CellCoords coords;
	PxBounds3 aabb;
	PxU32 start;
	PxU32 size;
};

//----------------------------------------------------------------------------//

PxU32 PX_INLINE hashFunction(const CellCoords& coords)
{
	PxU32 mix = static_cast<PxU32>(coords.x) + 101 * static_cast<PxU32>(coords.y) + 7919 * static_cast<PxU32>(coords.z);
	PxU32 hash = Ps::hash(mix);
	return hash;
}

//----------------------------------------------------------------------------//

PxU32 PX_INLINE getEntry(const CellCoords& coords, const PxU32 hashSize, const Cell* cells)
{
	PxU32 hash = hashFunction(coords);
	PxU32 index = hash & (hashSize - 1);
	for (;;)
	{
		const Cell& cell = cells[index];
		if (cell.size == sInvalidIndex || cell.coords == coords)
			break;
		index = (index + 1) & (hashSize - 1);
	}
	return index;
}

//----------------------------------------------------------------------------//

PxU32 PxParticleExt::buildBoundsHash(PxU32* sortedParticleIndices,
									 ParticleBounds* particleBounds,
									 const PxStrideIterator<const PxVec3>& positionBuffer,
									 const PxU32* validParticleBitmap,
									 const PxU32 validParticleRange,
									 const PxU32 hashSize,
									 const PxU32 maxBounds,
									 const PxReal gridSpacing)
{
	// test if hash size is a multiple of 2
	PX_ASSERT((((hashSize - 1) ^ hashSize) + 1) == (2 * hashSize));
	PX_ASSERT(maxBounds <= hashSize);

	PxReal cellSizeInv = 1.0f / gridSpacing;

	Ps::Array<PxU32> particleToCellMap PX_DEBUG_EXP("buildBoundsHashCellMap"); 	
	particleToCellMap.resize(validParticleRange);

	// initialize cells
	Ps::Array<Cell> cells PX_DEBUG_EXP("buildBoundsCells");
	cells.resize(hashSize);
	Ps::memSet(cells.begin(), sInvalidIndex, sizeof(Cell) * hashSize);

	// count number of particles per cell
	PxU32 entryCounter = 0;

	if (validParticleRange > 0)
	{
		for (PxU32 w = 0; w <= (validParticleRange-1) >> 5; w++)
			for (PxU32 b = validParticleBitmap[w]; b; b &= b-1)
			{
				PxU32 index = (w<<5|Ps::lowestSetBit(b));
				const PxVec3& position = positionBuffer[index];

				PxU32& cellIndex = particleToCellMap[index];
				cellIndex = sInvalidIndex;	// initialize to invalid in case we reach maxBounds	
				if (entryCounter < maxBounds)
				{
					CellCoords particleCoords;
					particleCoords.set(position, cellSizeInv);
					cellIndex = getEntry(particleCoords, hashSize, cells.begin());
					PX_ASSERT(cellIndex != sInvalidIndex);

					Cell& cell = cells[cellIndex];
					if (cell.size == sInvalidIndex)
					{
						// this is the first particle in this cell
						cell.coords = particleCoords;
						cell.aabb = PxBounds3(position, position);
						cell.size = 1;
						++entryCounter;
					}
					else
					{
						// the cell is already occupied
						cell.aabb.include(position);
						++cell.size;
					}
				}
			}
	}


	// accumulate start indices from cell size histogram and write to the user's particleBounds buffer
	PxU32 numBounds = 0;
	for (PxU32 i = 0, counter = 0; i < cells.size(); i++)
	{
		Cell& cell = cells[i];
		if (cell.size != sInvalidIndex)
		{
			cell.start = counter;
			counter += cell.size;
			
			PxParticleExt::ParticleBounds& cellBounds = particleBounds[numBounds++];
			cellBounds.bounds = cell.aabb;
			cellBounds.firstParticle = cell.start;
			cellBounds.numParticles = cell.size;
			
			cell.size = 0;
		}
	}

	// sort output particle indices by cell
	if (validParticleRange > 0)
	{
		for (PxU32 w = 0; w <= (validParticleRange-1) >> 5; w++)
			for (PxU32 b = validParticleBitmap[w]; b; b &= b-1)
			{
				PxU32 index = (w<<5|Ps::lowestSetBit(b));
				PxU32 cellIndex = particleToCellMap[index];
				if (cellIndex != sInvalidIndex)
				{
					Cell& cell = cells[cellIndex];
					PX_ASSERT(cell.start != sInvalidIndex && cell.size != sInvalidIndex);
					sortedParticleIndices[cell.start + cell.size] = index;
					++cell.size;
				}
			}
	}
	
	return numBounds;
}

//----------------------------------------------------------------------------//
