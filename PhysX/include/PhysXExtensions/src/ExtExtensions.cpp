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

#include "PxExtensionsAPI.h"
#include "PsFoundation.h"
#include "CmPhysXCommon.h"
#include "CmMetaData.h"
#include "PxGaussMapLimit.h"
#include "ExtDistanceJoint.h"
#include "ExtD6Joint.h"
#include "ExtFixedJoint.h"
#include "ExtPrismaticJoint.h"
#include "ExtRevoluteJoint.h"
#include "ExtSphericalJoint.h"

#include <stdio.h>

#if PX_SUPPORT_VISUAL_DEBUGGER
#include "PvdConnectionManager.h"
#include "ExtVisualDebugger.h"
#include "ExtVisualDebuggerTemplates.h"
#endif

using namespace Ps;

#if PX_SUPPORT_VISUAL_DEBUGGER
struct JointConnectionHandler : public PVD::PvdConnectionHandler
{
	virtual void onPvdConnected( PVD::PvdConnection* inFactory )
	{
		using namespace PVD;
		//register the joint classes.
		PvdDataStream* connection = inFactory->createDataStream();
		connection->addRef();
		Ext::VisualDebugger::sendClassDescriptions( *connection );
		connection->flush();
		connection->release();
	}
	virtual void onPvdDisconnected( PVD::PvdConnection*)
	{
	}
};

static JointConnectionHandler gPvdHandler;
#endif

bool PxInitExtensions(PxPhysics& physics)
{
	Ps::Foundation::setInstance(static_cast<Ps::Foundation*>(&physics.getFoundation()));

	physics.registerClass(PxSerialType::eUSER_SPHERICAL_JOINT,	Ext::SphericalJoint::createInstance);
	physics.registerClass(PxSerialType::eUSER_REVOLUTE_JOINT,	Ext::RevoluteJoint::createInstance);
	physics.registerClass(PxSerialType::eUSER_DISTANCE_JOINT,	Ext::DistanceJoint::createInstance);
	physics.registerClass(PxSerialType::eUSER_D6_JOINT,			Ext::D6Joint::createInstance);
	physics.registerClass(PxSerialType::eUSER_PRISMATIC_JOINT,	Ext::PrismaticJoint::createInstance);
	physics.registerClass(PxSerialType::eUSER_FIXED_JOINT,		Ext::FixedJoint::createInstance);
#if PX_SUPPORT_VISUAL_DEBUGGER
	if ( physics.getPvdConnectionManager() != NULL )
		physics.getPvdConnectionManager()->addHandler( &gPvdHandler );
#endif
	return true;
}

void PxCloseExtensions(void)
{
}

void PxRegisterExtJointMetaData(PxSerialStream& stream);
bool PxDumpMetaData(const char* metaFile, const PxPhysics& sdk)
{
	PX_ASSERT(metaFile);
	if(!metaFile)
		return false;

	class MetaDataStream : public PxSerialStream
	{
		public:
		virtual		void		storeBuffer(const void* buffer, PxU32 size)
		{
			PX_ASSERT(size==sizeof(Cm::MetaDataEntry));
			const Cm::MetaDataEntry* entry = (const Cm::MetaDataEntry*)buffer;
			metaData.pushBack(*entry);
		}
		virtual		PxU32		getTotalStoredSize()	{ return 0; }

		Array<Cm::MetaDataEntry> metaData;
	}s;

	sdk.getMetaData(s);
	PxRegisterExtJointMetaData(s);

	physx::shdfnd3::Array<char>	stringTable;
	struct Local
	{
		static PxU32 addToStringTable(physx::shdfnd3::Array<char>& stringTable, const char* str)
		{
			if(!str)
				return 0xffffffff;

			PxU32 length = stringTable.size();
			const char* table = stringTable.begin();
			const char* start = table;
			while(length)
			{
				if(strcmp(table, str)==0)
					return PxU32(table - start);

				const char* saved = table;
				while(*table++);
				length -= PxU32(table - saved);
				PX_ASSERT(length>=0);
			}

			const PxU32 offset = stringTable.size();

			while(*str)
				stringTable.pushBack(*str++);
			stringTable.pushBack(0);
			return offset;
		}
	};


	PxU32 nb = s.metaData.size();
	Cm::MetaDataEntry* entries = s.metaData.begin();
	for(PxU32 i=0;i<nb;i++)
	{
		entries[i].mType = (const char*)Local::addToStringTable(stringTable, entries[i].mType);
		entries[i].mName = (const char*)Local::addToStringTable(stringTable, entries[i].mName);
	}

	PxU32 platformTag = 0;
#ifdef PX_X64
	platformTag = 'PC64';
	const PxU32 gaussMapLimit = PxGetGaussMapVertexLimitForPlatform(PxPlatform::ePC);
	const PxU32 tiledHeightFieldSamples = 0;
#endif
#if defined(PX_X86) || defined(__CYGWIN__)
	platformTag = 'PC32';
	const PxU32 gaussMapLimit = PxGetGaussMapVertexLimitForPlatform(PxPlatform::ePC);
	const PxU32 tiledHeightFieldSamples = 0;
#endif
#ifdef PX_X360
	platformTag = 'XBOX';
	const PxU32 gaussMapLimit = PxGetGaussMapVertexLimitForPlatform(PxPlatform::eXENON);
	const PxU32 tiledHeightFieldSamples = 0;
#endif
#ifdef PX_PS3
	platformTag = 'PS_3';
	const PxU32 gaussMapLimit = PxGetGaussMapVertexLimitForPlatform(PxPlatform::ePLAYSTATION3);
	const PxU32 tiledHeightFieldSamples = 1;
#endif
#ifdef PX_ARM
	platformTag = 'ARM ';
	const PxU32 gaussMapLimit = PxGetGaussMapVertexLimitForPlatform(PxPlatform::eARM);
	const PxU32 tiledHeightFieldSamples = 1;
#endif

	FILE* fp = fopen(metaFile, "wb");
	if(!fp)
		return false;

	const PxU32 header = 'META';
	const PxU32 version = 1;
	const PxU32 ptrSize = sizeof(void*);
	fwrite(&header, 1, 4, fp);
	fwrite(&version, 1, 4, fp);
	fwrite(&ptrSize, 1, 4, fp);
	fwrite(&platformTag, 1, 4, fp);
	fwrite(&gaussMapLimit, 1, 4, fp);
	fwrite(&tiledHeightFieldSamples, 1, 4, fp);

	fwrite(&nb, 1, 4, fp);
	fwrite(entries, 1, nb*sizeof(Cm::MetaDataEntry), fp);

	PxU32 length = stringTable.size();
	const char* table = stringTable.begin();
	fwrite(&length, 1, 4, fp);
	fwrite(table, 1, length, fp);

	fclose(fp);

	return true;
}


#include "PxConvexMesh.h"
#include "PxTriangleMesh.h"
#include "PxHeightField.h"
#include "PxDeformableMesh.h"
#include "PxMaterial.h"
void PxCollectForExportSDK(const PxPhysics& sdk, PxCollection& collection)
{
	// Collect convexes
	{
		const PxU32 nbConvexes = sdk.getNbConvexMeshes();
		PxConvexMesh** convexes = new PxConvexMesh*[nbConvexes];
		const PxU32 nb = sdk.getConvexMeshes(convexes, nbConvexes);
		PX_ASSERT(nb==nbConvexes);
		for(PxU32 i=0;i<nbConvexes;i++)
			convexes[i]->collectForExport(collection);
		delete [] convexes;
	}

	// Collect triangle meshes
	{
		const PxU32 nbMeshes = sdk.getNbTriangleMeshes();
		PxTriangleMesh** meshes = new PxTriangleMesh*[nbMeshes];
		const PxU32 nb = sdk.getTriangleMeshes(meshes, nbMeshes);
		PX_ASSERT(nb==nbMeshes);
		for(PxU32 i=0;i<nbMeshes;i++)
			meshes[i]->collectForExport(collection);
		delete [] meshes;
	}

	// Collect heightfields
	{
		const PxU32 nbHeightFields = sdk.getNbHeightFields();
		PxHeightField** heightFields = new PxHeightField*[nbHeightFields];
		const PxU32 nb = sdk.getHeightFields(heightFields, nbHeightFields);
		PX_ASSERT(nb==nbHeightFields);
		for(PxU32 i=0;i<nbHeightFields;i++)
			heightFields[i]->collectForExport(collection);
		delete [] heightFields;
	}

#if PX_USE_DEFORMABLE_API
	// Collect deformables
	{
		const PxU32 nbDeformables = sdk.getNbDeformableMeshes();
		PxDeformableMesh** deformables = new PxDeformableMesh*[nbDeformables];
		const PxU32 nb = sdk.getDeformableMeshes(deformables, nbDeformables);
		PX_ASSERT(nb==nbDeformables);
		for(PxU32 i=0;i<nbDeformables;i++)
			deformables[i]->collectForExport(collection);
		delete [] deformables;
	}
#endif

	// Collect materials
	{
		const PxU32 nbMaterials = sdk.getNbMaterials();
		PxMaterial** materials = new PxMaterial*[nbMaterials];
		const PxU32 nb = sdk.getMaterials(materials, nbMaterials);
		PX_ASSERT(nb==nbMaterials);
		for(PxU32 i=0;i<nbMaterials;i++)
			materials[i]->collectForExport(collection);
		delete [] materials;
	}
}

#include "PxScene.h"
#include "PxAttachment.h"
#include "PxArticulation.h"
#include "PxAggregate.h"
void PxCollectForExportScene(const PxScene& scene, PxCollection& collection)
{
	// Collect actors
	{
		const PxActorTypeSelectionFlags selectionFlags = PxActorTypeSelectionFlag::eRIGID_STATIC
														|PxActorTypeSelectionFlag::eRIGID_DYNAMIC
														|PxActorTypeSelectionFlag::eDEFORMABLE
														|PxActorTypeSelectionFlag::ePARTICLE_SYSTEM
														|PxActorTypeSelectionFlag::ePARTICLE_FLUID;

		const PxU32 nbActors = scene.getNbActors(selectionFlags);
		PxActor** actors = new PxActor*[nbActors];
		const PxU32 nb = scene.getActors(selectionFlags, actors, nbActors);
		PX_ASSERT(nb==nbActors);
		for(PxU32 i=0;i<nbActors;i++)
			actors[i]->collectForExport(collection);
		delete [] actors;
	}

	// Collect attachments
	{
		const PxU32 nbActors = scene.getNbAttachments();
		PxAttachment** actors = new PxAttachment*[nbActors];
		const PxU32 nb = scene.getAttachments(actors, nbActors);
		PX_ASSERT(nb==nbActors);
		for(PxU32 i=0;i<nbActors;i++)
			actors[i]->collectForExport(collection);
		delete [] actors;
	}

	// Collect constraints
	{
		const PxU32 nbActors = scene.getNbConstraints();
		PxConstraint** actors = new PxConstraint*[nbActors];
		const PxU32 nb = scene.getConstraints(actors, nbActors);
		PX_ASSERT(nb==nbActors);
		for(PxU32 i=0;i<nbActors;i++)
			actors[i]->collectForExport(collection);
		delete [] actors;
	}

	// Collect articulations
	{
		const PxU32 nbActors = scene.getNbArticulations();
		PxArticulation** actors = new PxArticulation*[nbActors];
		const PxU32 nb = scene.getArticulations(actors, nbActors);
		PX_ASSERT(nb==nbActors);
		for(PxU32 i=0;i<nbActors;i++)
			actors[i]->collectForExport(collection);
		delete [] actors;
	}

	// Collect aggregates
	{
		const PxU32 nbActors = scene.getNbAggregates();
		PxAggregate** actors = new PxAggregate*[nbActors];
		const PxU32 nb = scene.getAggregates(actors, nbActors);
		PX_ASSERT(nb==nbActors);
		for(PxU32 i=0;i<nbActors;i++)
			actors[i]->collectForExport(collection);
		delete [] actors;
	}
}