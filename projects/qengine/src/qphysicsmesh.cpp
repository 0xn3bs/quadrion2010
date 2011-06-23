#include "Extras/ConvexDecomposition/ConvexBuilder.h"

#include "Extras/Serialize/BulletWorldImporter/btBulletWorldImporter.h"

#include "btBulletDynamicsCommon.h"

#include "LinearMath/btQuickprof.h"
#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btGeometryUtil.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"

#include "qscriptable.h"

#include "qmodelobject.h"
#include "q3dsmodel.h"

#include "qphysicsmesh.h"


///////////// qPhysicsMesh ////////////////

qPhysicsMesh::qPhysicsMesh()
{
	this->mdl_handle = NULL;
}

qPhysicsMesh::qPhysicsMesh(CModelObject *mdl)
{
	this->setModel(mdl);
}

qPhysicsMesh::qPhysicsMesh(btCollisionShape *_shape)
{
	this->mdl_handle = NULL;
	this->shape = _shape;
}

void qPhysicsMesh::setModel(CModelObject *mdl)
{
	this->mdl_handle = mdl;
	
}

void qPhysicsMesh::processMesh()
{
}

btCollisionShape *qPhysicsMesh::getCollisionShape()
{
	return this->shape;
}

bool qPhysicsMesh::save(const std::string& name, const std::string& path)
{
	std::string final = path + name;
	if(this->shape == NULL) return false;
	FILE* file = fopen(final.c_str(),"wb");
	if(!file) return false;

	int maxSerializeBufferSize = 1024*1024*5;
	btDefaultSerializer*	serializer = new btDefaultSerializer(maxSerializeBufferSize);
 
	serializer->startSerialization();
	this->shape->serializeSingleShape(serializer);
	serializer->registerNameForPointer(this->shape, name.c_str());
	serializer->finishSerialization();
 
	fwrite(serializer->getBufferPointer(),serializer->getCurrentBufferSize(),1, file);
	fclose(file);

	return true;
}

bool qPhysicsMesh::load(const std::string& name, const std::string& path)
{
	std::string final = path + name;
	FILE* file = fopen(final.c_str(),"wb");
	if(!file) return false;

	return true;
}

void qPhysicsMesh::intiVBOIBO()
{
};

///////////// qMeshDecomposition3DS ////////////////

qMeshDecomposition3DS::qMeshDecomposition3DS(c3DSModel *mdl)
{
}

btVector3	centroid=btVector3(0,0,0);
btVector3   convexDecompositionObjectOffset(10,0,0);

void qMeshDecomposition3DS::ConvexDecompResult(ConvexDecomposition::ConvexResult &result)
{

	btTriangleMesh* trimesh = new btTriangleMesh();
	m_trimeshes.push_back(trimesh);

	btVector3 localScaling(1.f,1.f,1.f);

	
	if (1)
	{
		//fprintf(mOutputFile,"## Hull Piece %d with %d vertices and %d triangles.\r\n", mHullCount, result.mHullVcount, result.mHullTcount );
		//fprintf(mOutputFile,"usemtl Material%i\r\n",mBaseCount);
		//fprintf(mOutputFile,"o Object%i\r\n",mBaseCount);

		for (unsigned int i=0; i<result.mHullVcount; i++)
		{
			const float *p = &result.mHullVertices[i*3];
			//fprintf(mOutputFile,"v %0.9f %0.9f %0.9f\r\n", p[0], p[1], p[2] );
		}

		centroid.setValue(0,0,0);

		btAlignedObjectArray<btVector3> vertices;
		if ( 1 )
		{
			//const unsigned int *src = result.mHullIndices;
			for (unsigned int i=0; i<result.mHullVcount; i++)
			{
				btVector3 vertex(result.mHullVertices[i*3],result.mHullVertices[i*3+1],result.mHullVertices[i*3+2]);
				vertex *= localScaling;
				centroid += vertex;
							
			}
		}
		
		centroid *= 1.f/(float(result.mHullVcount) );

		if ( 1 )
		{
			//const unsigned int *src = result.mHullIndices;
			for (unsigned int i=0; i<result.mHullVcount; i++)
			{
				btVector3 vertex(result.mHullVertices[i*3],result.mHullVertices[i*3+1],result.mHullVertices[i*3+2]);
				vertex *= localScaling;
				vertex -= centroid ;
				vertices.push_back(vertex);
			}
		}
					
		if ( 1 )
		{
			const unsigned int *src = result.mHullIndices;
			for (unsigned int i=0; i<result.mHullTcount; i++)
			{
				unsigned int index0 = *src++;
				unsigned int index1 = *src++;
				unsigned int index2 = *src++;

				btVector3 vertex0(result.mHullVertices[index0*3], result.mHullVertices[index0*3+1],result.mHullVertices[index0*3+2]);
				btVector3 vertex1(result.mHullVertices[index1*3], result.mHullVertices[index1*3+1],result.mHullVertices[index1*3+2]);
				btVector3 vertex2(result.mHullVertices[index2*3], result.mHullVertices[index2*3+1],result.mHullVertices[index2*3+2]);
				vertex0 *= localScaling;
				vertex1 *= localScaling;
				vertex2 *= localScaling;
							
				vertex0 -= centroid;
				vertex1 -= centroid;
				vertex2 -= centroid;

				trimesh->addTriangle(vertex0,vertex1,vertex2);

				index0+=mBaseCount;
				index1+=mBaseCount;
				index2+=mBaseCount;
							
				//fprintf(mOutputFile,"f %d %d %d\r\n", index0+1, index1+1, index2+1 );
			}
		}

		//	float mass = 1.f;
					

//this is a tools issue: due to collision margin, convex objects overlap, compensate for it here:
//#define SHRINK_OBJECT_INWARDS 1
#ifdef SHRINK_OBJECT_INWARDS

		float collisionMargin = 0.01f;
					
		btAlignedObjectArray<btVector3> planeEquations;
		btGeometryUtil::getPlaneEquationsFromVertices(vertices,planeEquations);

		btAlignedObjectArray<btVector3> shiftedPlaneEquations;
		for (int p=0;p<planeEquations.size();p++)
		{
			btVector3 plane = planeEquations[p];
			plane[3] += collisionMargin;
			shiftedPlaneEquations.push_back(plane);
		}
		btAlignedObjectArray<btVector3> shiftedVertices;
		btGeometryUtil::getVerticesFromPlaneEquations(shiftedPlaneEquations,shiftedVertices);	
		
		btConvexHullShape* convexShape = new btConvexHullShape(&(shiftedVertices[0].getX()),shiftedVertices.size());
					
#else //SHRINK_OBJECT_INWARDS
		btConvexHullShape* convexShape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
#endif 
		convexShape->setMargin(0.01f);
		m_convexShapes.push_back(convexShape);
		m_convexCentroids.push_back(centroid);
		m_collisionShapes.push_back(convexShape);
		mBaseCount+=result.mHullVcount; // advance the 'base index' counter.
	}
}

///////////// qPhysicsMesh3DS ////////////////

qPhysicsMesh3DS::qPhysicsMesh3DS()
{
	this->mdl_handle = NULL;
}

qPhysicsMesh3DS::qPhysicsMesh3DS(CModelObject *mdl)
{
	this->mdl_handle = (c3DSModel*)mdl;
}

void qPhysicsMesh3DS::setModel(CModelObject *mdl)
{
	this->mdl_handle = (c3DSModel*)mdl;
}

void qPhysicsMesh3DS::processMesh()
{
	unsigned int depth = 5;
    float cpercent     = 5;
	float ppercent     = 15;
	unsigned int maxv  = 16;
	float skinWidth    = 0.0;

	ConvexDecomposition::DecompDesc desc;
	chunk_data3ds *DATA = this->mdl_handle->getModelData();
	desc.mVcount		= DATA->meshes[0].vertCount;
	desc.mVertices		= (float*)(DATA->meshes[0].verts);
	desc.mTcount		= DATA->meshes[0].triCount;
	desc.mIndices		= (unsigned int*)(DATA->meshes[0].tris);
	desc.mDepth        = depth;
	desc.mCpercent     = cpercent;
	desc.mPpercent     = ppercent;
	desc.mMaxVertices  = maxv;
	desc.mSkinWidth    = skinWidth;

	qMeshDecomposition3DS decomp(this->mdl_handle);
	desc.mCallback = (ConvexDecomposition::ConvexDecompInterface*)&decomp;

	ConvexBuilder cb(desc.mCallback);
	cb.process(desc);

	this->shape = new btCompoundShape();

	btTransform trans;
	trans.setIdentity();

	for(int i = 0;i < decomp.m_convexShapes.size();i++)
	{
		//qDebug::Instance().print("hello");
		btVector3 centroid = decomp.m_convexCentroids[i];
		trans.setOrigin(centroid);
		btConvexHullShape* convexShape = decomp.m_convexShapes[i];
		((btCompoundShape*)shape)->addChildShape(trans,convexShape);

		//btRigidBody* body;
		//body = localCreateRigidBody( 1.0, trans,convexShape);
	}
}


/////////////  qPhysicsLoader /////////////

qPhysicsLoader::qPhysicsLoader(btDynamicsWorld *world)
{
	this->fileLoader = new btBulletWorldImporter(world);
}

bool qPhysicsLoader::loadFile(const std::string& file, const std::string& path)
{
	std::string final = path + file;
	return this->fileLoader->loadFile(final.c_str());
}

btCollisionShape *qPhysicsLoader::getCollisionShapeByName(const std::string& name)
{
	return this->fileLoader->getCollisionShapeByName(name.c_str());
}
