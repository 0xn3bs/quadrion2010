/////////////////////////////////////////////////////////////////////////////////////
//
// 3DSMODEL.H
//
// .3DS File parser written for Quadrion Engine by Shawn Simonson 
//
// Contains chunk types and load routines for parsing an Autodesk 3D Studio Max .3ds
// model object.
//
// All contents herein should be abstracted from c3DSModel. This file contains no
// client side interaction points
//
/////////////////////////////////////////////////////////////////////////////////////


#ifndef __Q3DSMODEL_H_
#define __Q3DSMODEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "qmath.h"
#include "qrender.h"
#include "qvertexbuffer.h"
#include "qindexbuffer.h"
#include "qresource.h"
#include "qmodelobject.h"

#pragma pack(push)
#pragma pack(1)

#ifdef QRENDER_EXPORTS
#define Q3DSMODELEXPORT_API		__declspec(dllexport)
#else
#define Q3DSMODELEXPORT_API		__declspec(dllimport)
#endif

// basic 3ds chunk type //
typedef Q3DSMODELEXPORT_API struct
{
	unsigned short id;			// id of chunk
	unsigned long  length;		// length in bytes of chunk
}chunk_3ds;

// 3ds material chunk type //
typedef Q3DSMODELEXPORT_API struct
{
	char			name[32];		// name of material
	float			ambient[4];		// ambient color
	float			diffuse[4];		// difffuse color
	float			specular[4];		// specular color
	float			emissive[4];		// emissive color
	float			shininess;		// phong model exponent
	char			texture[32];	// texture name
}chunk_material3ds;

// 3ds group chunk type //
typedef Q3DSMODELEXPORT_API struct
{
	char		name[32];		// group name	
	int			mat;			// material index
	int			start;			// starting index
	int			size;			// length in bytes of group
	long		*tris;			// poly list
}chunk_group3ds;

// 3ds mesh type //
typedef Q3DSMODELEXPORT_API struct
{
	char			name[32];				// name of mesh
	int				vertCount;				// vertexCount
	int				texCoordCount;			// texture coordinate count
	int				triCount;				// triangle count
	int				groupCount;				// number of chunk_group3ds's
	float			(*verts)[3];			// vertex list
	float			(*norms)[3];			// normal list
	float			(*texCoords)[2];		// texture coordinate list
	float			(*tangentSpace)[9];		// tangent space vectors
	long			(*tris)[3];				// poly list
	chunk_group3ds  *groups;				// group list
	long			*smooth;				// smoothing group list
	float			axis[3][3];				// local axis definition
	float			position[3];			// world space pos
	float			min[3];					// bb mins
	float			max[3];					// bb maxs
	float			center[3];				// center locally
}chunk_mesh3ds;

// main 3ds data type //
typedef Q3DSMODELEXPORT_API struct
{
	int					 materialCount;		// num of materials
	int					 meshCount;			// num of meshes
	int					 vertCount;			// num of verts
	int					 triCount;			// triangle count
	chunk_material3ds*	 materials;			// material list
	chunk_mesh3ds*		 meshes;			// mesh list
	float				 min[3];				// bb mins
	float				 max[3];				// bb max
	float				 center[3];			// model center
	float				 masterScale;		// master scaling
}chunk_data3ds;

#pragma pack(pop)

bool Q3DSMODELEXPORT_API read3DSFile(char* fName, chunk_data3ds* out);
void Q3DSMODELEXPORT_API free3DSData(chunk_data3ds* dat);
void Q3DSMODELEXPORT_API calculate3DSNormals(chunk_data3ds* dat);
void Q3DSMODELEXPORT_API calculate3DSTangentSpace(chunk_data3ds* dat);
void Q3DSMODELEXPORT_API calculate3DSBoundingBox(chunk_data3ds* dat);

///////////////////////////////////////////////////
// sModelTexture
// holds the render id of the texture as well as 
// the texture's file name so its searchable
// Also serves as a material type for a mesh
struct Q3DSMODELEXPORT_API s3DSMaterial
{
	std::string		fileName;			// texture file name
	int				textureRef;			// local texture handle
	int				normalmapRef;
	
	// values for diffuse and spec //
	vec4f	ambient;					// ambient color
	vec4f	emissive;					// emissive color
	vec4f	diffuse;					// diffuse color
	vec4f	specular;					// specular color
	float	shininess;					// phong exponent
};

/////////////////////////////////////////////////
// s3DSVertexFormat
// vertex format packet for typical 3DS model object
#pragma pack(push)
#pragma pack(8)

struct Q3DSMODELEXPORT_API s3DSVertexFormat
{
	float x, y, z;						// world space position
	float nx, ny, nz;					// normal coords (already normalized)
	float tx, ty, tz;					// tangent vector (already normalized)
	float u, v;							// texture coords base
};

#pragma pack(pop)

///////////////////////////////////////////////////
// s3DSVBOIBO
// Contains render handles for the vertex buffer
// and index buffer of a given mesh.
// also contains a low level of detail mesh handle
// as well as a medium level of detail mesh handle
struct Q3DSMODELEXPORT_API s3DSVBOIBO
{
	s3DSVBOIBO()
	{
		vboRef = QRENDER_INVALID_HANDLE;
		iboRef = QRENDER_INVALID_HANDLE;
		lowLODMesh = QRENDER_INVALID_HANDLE;
		medLODMesh = QRENDER_INVALID_HANDLE;
	}

	int		vboRef;					// vertex buffer object handle
	int		iboRef;					// index buffer object handle
	int		instanceRef;			
				
	int		lowLODMesh;				// low level of detail mesh handle
	int		medLODMesh;				// medium level of detail mesh handle
};

/////////////////////////////////////////////////////////////////////////////////////////
//
// SMS- 11/2006
//
// c3DSModel
// Managable interface to the routines above
// Class that manages the creation and destruction of the raw data of the .3DS model
// Also manages renderer resources such as texture creation.
//
// This class should NOT be used as a client side point of interaction.
// This class should be abstracted per need.
//
////////////////////////////////////////////////////////////////////////////////////////
class Q3DSMODELEXPORT_API c3DSModel : public CModelObject
{
	public:

		c3DSModel(const unsigned int handle, const std::string& name, const std::string& path = "./");
		~c3DSModel();
		
		// load model with "models/mymodel.3ds" file name structure
		// loadLODModel true if you want to load level of detail models from the original as well
		bool LoadModel( bool loadNormalmaps = false );	
		
		// explicit destructor //
		void killModel();
	
	
		// Render's all model's meshes. This method binds no shaders. Shader state is handled  //
		// by the client. Be sure to call BindDiffuseTexture and BindNormalmapTexture before invoking //
		// such that the textures are bound to the correct samplers //
		void RenderModel();
		

		
		// Get mesh's orientation matrix from mesh handle //
		void						getMeshOrientationMat( mat4& m, const int mesh );
		
		// Get mesh's relative center in model space from mesh handle //
		vec3f						getMeshCenter( const int mesh );
		
		// Get mesh's position in world space from mesh handle //
		vec3f						getMeshPosition( const int mesh );
		
		// Get meshs' bounding box from mesh handle //
		void						getMeshBoundingBox( const int mesh, vec3f& mins, vec3f& maxs );
		
		// Get model's overall bounding box //
		void		GetAABB(vec3f& mins, vec3f& maxs); 
		void		GetBoundingSphere(vec3f& center, float& rad);
		
		// Get models overall position //																				
		inline vec3f			getModelCenter() { return vec3f(mdlData.center[0], mdlData.center[1], mdlData.center[2]); }
		inline chunk_data3ds*	getModelData() { return &mdlData; }
		void					GetModelCenter(vec3f& center) { center.set(mdlData.center[0], mdlData.center[1], mdlData.center[2]); }
		
		// Get number of meshes in model //
		const inline int		getMeshCount() { return mdlData.meshCount; }
		
		// Get model's file name in maps/mymodel.3ds" format //
		const inline std::string				getFileName() { return fileName; }
		
		
	
	protected:
		
		std::vector<s3DSMaterial>	textureHandleList;			// list of local texture references 
		std::vector<s3DSVBOIBO>		meshRenderHandles;			// list of mesh render handles
			
		std::string					fileName;					// object's file name
	
		chunk_data3ds				mdlData;					// main 3ds chunk

		

	private:
	
		
		void makeVertexNormals();				// Process per-vertex normals from face normals
};

#endif /*__Q3DSMODEL_H_*/
