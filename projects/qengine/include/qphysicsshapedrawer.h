#ifndef _qPhysicsShapeDrawer_h
#define _qPhysicsShapeDrawer_h

#ifdef QRENDER_EXPORTS
	#define QPHYSICSSHAPEDRAWEREXPORT_API		__declspec(dllexport)
#else
	#define QPHYSICSSHAPEDRAWEREXPORT_API		__declspec(dllimport)
#endif

#include <vector>

#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btVector3.h"

#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "BulletCollision/CollisionShapes/btTriangleCallback.h"

#include "qmath.h"
#include "qrender.h"

#include "qphysicsmesh.h"
#include "qmodelobject.h"

class btCollisionShape;
class btConcaveShape;

struct QPHYSICSSHAPEDRAWEREXPORT_API qPSDVertex
{
	vec3f pos;					// world space position
	vec3f norm;					// normal coords (already normalized)
};

struct QPHYSICSSHAPEDRAWEREXPORT_API qPDSVertexVBO
{
	float px, py, pz;
	float nx, ny, nz;
};

class qPhysicsModelObject : public CModelObject
{
public:
	qPhysicsModelObject(const unsigned int handle, const std::string& name, const std::string& path = "./");
	/*{
		tex.SetFileName( "none" );
		tex.SetFilePath( "./" );
		unsigned int color = 0x000000ff;
		color |= ( unsigned char( 0.3f * 255 ) << 24 );
		color |= ( unsigned char( 0.8f * 255 ) << 16 );
		color |= ( unsigned char( 0.3f * 255 ) << 8 );
		tex.LoadFromColor( color );

		unsigned int flags = QTEXTURE_FILTER_TRILINEAR;
		tex_handel = g_pRender->AddTextureObject( tex, flags );
	}*/

	~qPhysicsModelObject();//{}

	void RenderModel();
	/*{

	}*/

private:
	CQuadrionTextureFile tex;
	CQuadrionTextureFile norm_tex;
	int tex_handel;
	int norm_tex_handel;
	SQuadrionVertexDescriptor v_desc;
};

/*struct QPHYSICSSHAPEDRAWEREXPORT_API qDrawMaterial
{
	int				textureRef;			// local texture handle
	
	// values for diffuse and spec //
	vec4f	ambient;					// ambient color
	vec4f	emissive;					// emissive color
	vec4f	diffuse;					// diffuse color
	vec4f	specular;					// specular color
	float	shininess;					// phong exponent
};*/


CQuadrionTextureFile QPHYSICSSHAPEDRAWEREXPORT_API qMakeShapeMaterial(vec4f _color);


struct QPHYSICSSHAPEDRAWEREXPORT_API qShapeCache
{
	struct Edge { btVector3 n[2]; int v[2]; };
	qShapeCache(btConvexShape* s) : shapeHull(s){}
	btShapeHull shapeHull;
	btAlignedObjectArray<Edge> edges;
};

struct QPHYSICSSHAPEDRAWEREXPORT_API qConcaveShapeCache
{
	qPhysicsMeshVBOIBO vertexBuffer;
	int numFaces;
};




class QPHYSICSSHAPEDRAWEREXPORT_API qPhysicsShapeDrawer
{
public:
	qPhysicsShapeDrawer();
	virtual ~qPhysicsShapeDrawer();

	void lost();
	void destroy();
	
	void renderShape(const btCollisionShape *shape, btTransform &trans);
	void renderConcaveShapes(mat4 world, size_t index);

	void addConcaveShape(btConcaveShape *shape);

	void resetStats()		{ mNumVertices = mNumFaces = mNumObjects = 0; }
	DWORD getNumVertices()	{ return mNumVertices; }
	DWORD getNumFaces()		{ return mNumFaces; }
	DWORD getNumObjects()	{ return mNumObjects; }

protected:
	qShapeCache*						Cache(btConvexShape*);

private:

	btAlignedObjectArray<qShapeCache*>	mShapeCaches;
	std::vector<qConcaveShapeCache*>	mConcaveShapeCaches;

	DWORD								mNumVertices;
	DWORD								mNumFaces;
	DWORD								mNumObjects;

	int									mDefaultMaterial;
	int									mEffect;

	qPhysicsModelObject*				box;
};




/*
 *	qDrawCallback
 *
 *	Bullet Physics traingle call back. This is used to
 *	process how a triangle is represented.
 *
 */

class QPHYSICSSHAPEDRAWEREXPORT_API qDrawCallback : public btTriangleCallback
{
public:
	qDrawCallback()
	{
		this->n_verts = 0;
		this->n_tris = 0;
	}

	~qDrawCallback()
	{
		this->vertices.clear();
	}

	virtual void processTriangle(btVector3* triangle,int partId, int triangleIndex)
	{	
		qPSDVertex tempvertices[3] = 
		{
			{ vec3f( triangle[0].getX(), triangle[0].getY(), triangle[0].getZ() ), vec3f(0.0f, 0.0f, 0.0f) },
			{ vec3f( triangle[1].getX(), triangle[1].getY(), triangle[1].getZ() ), vec3f(0.0f, 0.0f, 0.0f) },
			{ vec3f( triangle[2].getX(), triangle[2].getY(), triangle[2].getZ() ), vec3f(0.0f, 0.0f, 0.0f) },
		};

		// Generate normal
		vec3f vec1 = tempvertices[2].pos - tempvertices[0].pos;
		vec3f vec2 = tempvertices[0].pos - tempvertices[1].pos;
		vec3f normal = vec1.crossProd(vec2);
		normal.normalize();

		tempvertices[0].norm = normal;
		tempvertices[1].norm = normal;
		tempvertices[2].norm = normal;
		
		vertices.push_back( tempvertices[0] );
		vertices.push_back( tempvertices[1] );
		vertices.push_back( tempvertices[2] );
	
		this->n_verts +=3;
		this->n_tris++;
	}

	int n_verts;
	int n_tris;
	std::vector<qPSDVertex> vertices;
};


#endif