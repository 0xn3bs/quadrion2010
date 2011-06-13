
#include "q3dsmodel.h"
#include "qindex_t.h"

#pragma pack(push)
#pragma pack(1)

typedef struct
{
	long mat;
	long verts[3];
	long index;
	long smooth;
}triangle_3ds;

#pragma pack(pop)

// callback declaration //
typedef void (*cb)(chunk_3ds*, void*);

//bool read3DSFile(char* fName);															
//void free3DSData(chunk_data3ds* dat);														
//void calculate3DSNormals(chunk_data3ds* dat);												
//void calculate3DSTangentSpace(chunk_data3ds* dat);										
//void calculate3DSBoundingBox(chunk_data3ds* dat);											
static void readString(char* str);															
static void readChunkArray(long length, void(*callback)(chunk_3ds*, void*), void* dat);		
static void inspectChunkArray(long length, void(*callback)(chunk_3ds*, void*), void* dat);	
static void readMain3DS(chunk_3ds* chunk, chunk_data3ds* out);								
static void inspectEdit3DS(chunk_3ds* chunk, chunk_data3ds* out);							
static void inspectEditObject(chunk_3ds* chunk, chunk_data3ds* out);						
static void readEdit3DS(chunk_3ds* chunk, chunk_data3ds* out);								
static void readEditObject(chunk_3ds* chunk, chunk_data3ds* out);							
static void readTriangleObject(chunk_3ds* chunk, chunk_mesh3ds* out);						
static void readVertexList(chunk_mesh3ds* out);												
static void readFaceList(chunk_mesh3ds* out, long length);									
static void inspectFaceSubs(chunk_3ds* chunk, chunk_mesh3ds* out);							
static void readFaceSubs(chunk_3ds* chunk, chunk_mesh3ds* out);								
static void readTexCoords(chunk_mesh3ds* out);												
static void readLocalAxis(chunk_mesh3ds* out);												
static void readMaterialGroup(chunk_group3ds* group);										
static void readMaterial(chunk_3ds* chunk, chunk_material3ds* mat);							
static void readTexture(chunk_3ds* chunk, chunk_material3ds* mat);							
static void readColor(chunk_3ds* chunk, float* color);										
static void readPercentage(chunk_3ds* chunk, float* val);									
//static short readChunkID();
//static void skipChunk();

static void removeDegenerates(chunk_mesh3ds* out);											
static void sortTriangles(chunk_mesh3ds* out);													
static void calculateMeshNormals(chunk_mesh3ds* dat);
static void calculateMeshTangentSpace(chunk_mesh3ds* dat);									
static void calculateMeshBoundingBox(chunk_mesh3ds* dat);									
static void smoothTangentSpace(chunk_mesh3ds* dat);													
static void tangentSpace(float* v1, float* v2, float* v3, float* t1, float* t2, float* t3, float* norm, float* tangentSpace);  

static FILE* g_pFile = NULL;
static char g_szString[64];

// static qsort compare func //
static int comparePos(float* a, float* b)
{
	for(int i = 0; i < 3; ++i)
	{
		if(a[i] > b[i])
			return 1;
		
		if(a[i] < b[i])
			return -1;
	}
	
	return 0;
}

// static qsort compare func //
static int compareLongs(long* a, long* b)
{
	if(*a > *b)
		return 1;
	if(*a < *b)
		return -1;
	return 0;
}

// static sort by mat func //
static int sortByMaterial(triangle_3ds* a, triangle_3ds* b)
{
	if(a->mat > b->mat)
		return 1;
	if(a->mat < b->mat)
		return -1;
	
	if(a->smooth > b->smooth)
		return 1;
	if(a->smooth < b->smooth)
		return -1;
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// read3DSFile
// This is the main point of contact for loading of 3DS data via the c3DS class
// returns a pointer to the main 3ds data chunk
bool read3DSFile(const char* fName, chunk_data3ds* out)
{
	chunk_3ds chunk;
	memset(out, 0, sizeof(chunk_data3ds));
	
	g_pFile = fopen(fName, "rb");
	
	if(g_pFile)
	{
		// if file exists proceed with reading the chunk list //
		fseek(g_pFile, 0, SEEK_SET);
		fread(&chunk, sizeof(chunk_3ds), 1, g_pFile);
		if(chunk.id == 0x4D4D)
			readChunkArray(chunk.length - 6, (cb)readMain3DS, out);
		
		fclose(g_pFile);
	}
	
	else
	{
		//std::string err("Failed to load 3DS File: ");
		//err.append(fName);
		//cErrorLog::Instance()->WriteError(err.c_str());
		return false;
	}
	
	// this essentially removes both duplicate triangles and those triangles that were generated
	// in error by the app, then sorts them via group/material //
	for(int i = 0; i < out->meshCount; ++i)
	{
		removeDegenerates(&out->meshes[i]);
		sortTriangles(&out->meshes[i]);
	}
	
	out->vertCount = 0;
	out->triCount = 0;
	
	for(int i = 0; i < out->meshCount; ++i)
	{
		out->vertCount += out->meshes[i].vertCount;
		out->triCount += out->meshes[i].triCount;
	}
	
	
	// calc normals, bb's, and tangent space out of new data object //
	calculate3DSBoundingBox(out);
	calculate3DSNormals(out);
	calculate3DSTangentSpace(out);
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// free3DSData
// frees up any allocated 3ds data, main point of contact for c3DS cleanup
void free3DSData(chunk_data3ds* dat)
{
	if(dat->meshes)
	{
		for(int i = 0; i < dat->meshCount; ++i)
		{	
			if(dat->meshes[i].verts)
			{
				free(dat->meshes[i].verts);
				dat->meshes[i].verts = NULL;
			}
			
			if(dat->meshes[i].tris)
			{
				free(dat->meshes[i].tris);
				dat->meshes[i].tris = NULL;
			}
			
			if(dat->meshes[i].norms)
			{
				free(dat->meshes[i].norms);
				dat->meshes[i].norms = NULL;
			}

			if(dat->meshes[i].tangentSpace)
			{
				free(dat->meshes[i].tangentSpace);
				dat->meshes[i].tangentSpace = NULL;
			}

			if(dat->meshes[i].texCoords)
			{
				free(dat->meshes[i].texCoords);
				dat->meshes[i].texCoords = NULL;
			}

			if(dat->meshes[i].smooth)
			{
				free(dat->meshes[i].smooth);
				dat->meshes[i].smooth = NULL;
			}
			
			if(dat->meshes[i].groups)
			{
				for(int j = 0; j < dat->meshes[i].groupCount; ++j)
				{
					free(dat->meshes[i].groups[j].tris);
					dat->meshes[i].groups[j].tris = NULL;
				}
				
				free(dat->meshes[i].groups);
				dat->meshes[i].groups = NULL;
			}
		}
		
		free(dat->meshes);
		dat->meshes = NULL;
	}
	
	free(dat->materials);
	dat->materials = NULL;
	memset(dat, 0, sizeof(chunk_data3ds));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate3DSNormals
// calculates normals per vertex for the data set
void calculate3DSNormals(chunk_data3ds* dat)
{
	for(int i = 0; i < dat->meshCount; ++i)
		calculateMeshNormals(&dat->meshes[i]);
}

////////////////////////////////////////////////////////////////////////////////////////
// calculate3DSTangentSpace
// calculates tangent space vectors for the entire 3ds data set 
void calculate3DSTangentSpace(chunk_data3ds* dat)
{
	for(int i = 0; i < dat->meshCount; ++i)
	{
		calculateMeshTangentSpace(&dat->meshes[i]);
		if(dat->meshes[i].tangentSpace)
			smoothTangentSpace(&dat->meshes[i]);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// calculate3DSBoundingBox
// Calculates bounding box for entire 3DS model data set
void calculate3DSBoundingBox(chunk_data3ds* dat)
{
	for(int i = 0; i < dat->meshCount; ++i)
	{
		calculateMeshBoundingBox(&dat->meshes[i]);
		
		if(i == 0)
		{
			memcpy(dat->max, dat->meshes[i].max, sizeof(float) * 3);
			memcpy(dat->min, dat->meshes[i].min, sizeof(float) * 3);
		}
		
		// check and swap max values conditionally //
		else
		{
			for(int j = 0; j < 3; ++j)
			{
				if(dat->meshes[i].max[j] > dat->max[j])
					dat->max[j] = dat->meshes[i].max[j];
				if(dat->meshes[i].min[j] < dat->min[j])
					dat->min[j] = dat->meshes[i].min[j];
			}
		}
	}
	
	// compute model space center //
	dat->center[0] = dat->min[0] + (dat->max[0] - dat->min[0]) * 0.5f;
	dat->center[1] = dat->min[1] + (dat->max[1] - dat->min[1]) * 0.5f;
	dat->center[2] = dat->min[2] + (dat->max[2] - dat->min[2]) * 0.5f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// readString
// reads a string up to a null terminator
void readString(char* str)
{
	int i = 0;
	do
	{
		fread(&str[i], sizeof(char), 1, g_pFile);
	}while(str[i++] != '\0');
}

////////////////////////////////////////////////////////////////////////////////////////////////
// readChunkArray
// reads the main chunk list for the 3ds data set
void readChunkArray(long length, void(*callback)(chunk_3ds*, void*), void* dat)
{
	chunk_3ds chunk;
	long start, pos;
	pos = 0;
	
	// eat one chunk at a time calling the appropriate callback to read it //
	do
	{
		start = ftell(g_pFile);
		fread(&chunk, sizeof(chunk_3ds), 1, g_pFile);
		
		callback(&chunk, dat);
		
		fseek(g_pFile, start + chunk.length, SEEK_SET);
		
		// move new pos up
		pos += chunk.length;
	}while(pos < length);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// inspectChunkArray
// Essentially the same as above but is called with a different callback
void inspectChunkArray(long length, void(*callback)(chunk_3ds*, void*), void* dat)
{
	chunk_3ds chunk;
	long start, chunkStart, pos;
	pos = 0;
	
	chunkStart = ftell(g_pFile);
	
	// eat a chunk at a time //
	do
	{
		start = ftell(g_pFile);
		fread(&chunk, sizeof(chunk_3ds), 1, g_pFile);
		
		callback(&chunk, dat);
		
		fseek(g_pFile, start + chunk.length, SEEK_SET);
		
		// update file pos ptr //
		pos += chunk.length;
	}while(pos < length);
	
	fseek(g_pFile, chunkStart, SEEK_SET);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// readMain3DS
// called from read3DSFile, and essentially reads chunk by chunk ensuring the appropriate
// callback funcs that read the chunks are called 
void readMain3DS(chunk_3ds* chunk, chunk_data3ds* out)
{
	// If its a valid chunk it will be tagged w/ 0x3D3D as an id
	if(chunk->id == 0x3D3D)
	{
		inspectChunkArray(chunk->length - 6, (cb)inspectEdit3DS, out);
	
		// allocate for mesh and material list //
		if(out->meshCount)
		{
			out->meshes = (chunk_mesh3ds*)malloc(sizeof(chunk_mesh3ds) * out->meshCount);
			memset(out->meshes, 0, sizeof(chunk_mesh3ds) * out->meshCount);
		}
		
		if(out->materialCount)
		{
			out->materials = (chunk_material3ds*)malloc(sizeof(chunk_material3ds) * out->materialCount);
			memset(out->materials, 0, sizeof(chunk_material3ds) * out->materialCount);
		}
		
		out->meshCount = 0;
		out->materialCount = 0;
		
		// actually read the chunk data //
		readChunkArray(chunk->length - 6, (cb)readEdit3DS, out);
		
		// set the materials per mesh, per group
		for(int i = 0; i < out->meshCount; ++i)
		{
			for(int j = 0; j < out->meshes[i].groupCount; ++j)
			{
				for(int k = 0; k < out->materialCount; ++k)
				{
					if(!strcmp(out->meshes[i].groups[j].name, out->materials[k].name))
					{
						out->meshes[i].groups[j].mat = k;
						break;
					}
				}
			}
		}
	}
	
	else
	{
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////
// inspectEdit3DS
// switches the editing of the chunk over to the appropriate callback
void inspectEdit3DS(chunk_3ds* chunk, chunk_data3ds* out)
{
	if(chunk->id == 0xAFFF)			// flag for material edit
		out->materialCount++;
	else if(chunk->id == 0x4000)	// flag for object edit
	{
		readString(g_szString);
		inspectChunkArray(chunk->length - 6 + strlen(g_szString), (cb)inspectEditObject, out);
	}
	else
	{
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// inspectEditObject
// same as inspectEdit3DS except that it operates on a singular piece of 3DS data
void inspectEditObject(chunk_3ds* chunk, chunk_data3ds* out)
{
	if(chunk->id == 0x4100)		// flag for object edit
		out->meshCount++;
	
	else
	{
		return;
	}
}

void readFloat( chunk_3ds* chunk, chunk_data3ds* out )
{
	long start, pos;
	pos = 0;
	
	do
	{
		start = ftell( g_pFile );
		fread( &out->masterScale, sizeof(float), 1, g_pFile );
		fseek( g_pFile, start + chunk->length, SEEK_SET );
		pos += chunk->length;
	} while( pos < chunk->length );
}

//////////////////////////////////////////////////////////////////////////////////////////
// readEdit3DS
// This is in turn called by readMain3DS after the inspect chunk function has been called
void readEdit3DS(chunk_3ds* chunk, chunk_data3ds* out)
{
	int start;

	switch(chunk->id)
	{
		case 0x0100:
			start = ftell( g_pFile );
			fread( &out->masterScale, sizeof(float), 1, g_pFile );
			fseek( g_pFile, start + chunk->length, SEEK_SET );
			break;
	
		case 0x4000:
			readString(g_szString);
			readChunkArray(chunk->length - 6 + strlen(g_szString), (cb)readEditObject, out);
			break;
		
		case 0xAFFF:
			readChunkArray(chunk->length - 6, (cb)readMaterial, &out->materials[out->materialCount++]);
			break;
		
		default:
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// readEditObject
// Edits the global string with the one in the chunk being read
void readEditObject(chunk_3ds* chunk, chunk_data3ds* out)
{
	if(chunk->id == 0x4100)				// read triangle object flag
	{
		strcpy(out->meshes[out->meshCount].name, g_szString);
		readChunkArray(chunk->length - 6, (cb)readTriangleObject, &out->meshes[out->meshCount++]);
	}		
	
	else
	{
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// readTriangleObject
// Coordinates the reads of a triangle list chunk with reading vertices, tex coords, normals, etc...
void readTriangleObject(chunk_3ds* chunk, chunk_mesh3ds* out)
{
	switch(chunk->id)
	{
		case 0x4110:		// vertex list
			readVertexList(out);
			break;
		
		case 0x4120:		// face list
			readFaceList(out, chunk->length - 6);
			break;
		
		case 0x4140:		// tex coord list
			readTexCoords(out);
			break;
		
		case 0x4160:		// local axis
			readLocalAxis(out);
			break;
		
		case 0x4170:
			break;
		default:
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// readVertexList
// Reads in raw vertex data chunk
void readVertexList(chunk_mesh3ds* out)
{
	unsigned short nVerts;
	fread(&nVerts, sizeof(unsigned short), 1, g_pFile);
	
	out->vertCount = nVerts;
	out->verts = (float(*)[3])malloc(sizeof(float) * 3 * out->vertCount);
	
	// read raw vertex data in //
	fread(out->verts, sizeof(float), 3 * out->vertCount, g_pFile);
}

/////////////////////////////////////////////////////////////////////////////////////////
// readFaceList
// Reads faces in by groups in the mesh
void readFaceList(chunk_mesh3ds* out, long length)
{
	long pos = 6;
	unsigned short nTris;
	
	fread(&nTris, sizeof(unsigned short), 1, g_pFile);
	pos += sizeof(unsigned short);
	
	out->triCount = nTris;
	out->tris = (long(*)[3])malloc(sizeof(long) * 3 * out->triCount);
	
	unsigned short verts[4];
	
	// construct a temp face from 4 verts //
	for(int i = 0; i < nTris; ++i)
	{
		// read the verts in sequence and construct the tri list //
		fread(verts, sizeof(short), 4, g_pFile);
		for(int j = 0; j < 3; ++j)
		{
			out->tris[i][j] = (long)verts[j];
		}
	}
	
	// update fp pos //
	pos += sizeof(unsigned short) * 4 * nTris;
	
	// if we've got more data then there are sub faces in groups //
	if(pos < length)
	{
		inspectChunkArray(pos - 6, (cb)inspectFaceSubs, out);
		
		if(out->groupCount)
		{
			out->groups = (chunk_group3ds*)malloc(sizeof(chunk_group3ds) * out->groupCount);
			memset(out->groups, 0, sizeof(chunk_group3ds) * out->groupCount);
			if(out->groups)
			{
				out->groupCount = 0;
				readChunkArray(pos - 6, (cb)readFaceSubs, out);
			}
		}
	}
}

//////////////////////////////////////////////////////////////
// inspectFaceSubs
// Inspects possibly degenerate faces 
void inspectFaceSubs(chunk_3ds* chunk, chunk_mesh3ds* out)
{
	if(chunk->id == 0x4130)
		out->groupCount++;
	
	else
	{
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
// readFaceSubs
void readFaceSubs(chunk_3ds* chunk, chunk_mesh3ds* out)
{
	switch(chunk->id)
	{
		case 0x4130:		// material group
			readMaterialGroup(&out->groups[out->groupCount++]);
			break;
		
		case 0x4150:		// smoothing group
			out->smooth = (long*)malloc(sizeof(long) * out->triCount);
			if(out->smooth)
				fread(out->smooth, sizeof(long), out->triCount, g_pFile);
			break;
		
		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// readTexCoords
void readTexCoords(chunk_mesh3ds* out)
{
	// alloc and read raw texture data //
	unsigned short nTexCoords;
	fread(&nTexCoords, sizeof(unsigned short), 1, g_pFile);
	out->texCoordCount = nTexCoords;
	
	out->texCoords = (float(*)[2])malloc(sizeof(float) * 2 * out->texCoordCount);
	fread(out->texCoords, sizeof(float), 2 * out->texCoordCount, g_pFile);
}

///////////////////////////////////////////////////////////////////////////////////
// readLocalAxis
void readLocalAxis(chunk_mesh3ds* out)
{
	fread(out->axis, sizeof(float), 9, g_pFile);
	fread(out->position, sizeof(float), 3, g_pFile);
}

//////////////////////////////////////////////////////////////////////////////////
// readMaterialGroup
void readMaterialGroup(chunk_group3ds* group)
{
	unsigned short nFaces;
	unsigned short face;
	
	readString(group->name);
	fread(&nFaces, sizeof(unsigned short), 1, g_pFile);
	
	group->tris = (long*)malloc(nFaces * sizeof(long));
	
	// group together tris that share material properties //
	if(group->tris)
	{
		memset(group->tris, 0, nFaces * sizeof(long));
		
		group->size = nFaces;
		group->mat = 0;
		
		// read in face indices that share this material //
		for(int i = 0; i < nFaces; ++i)
		{
			fread(&face, sizeof(unsigned short), 1, g_pFile);
			group->tris[i] = face;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// readMaterial
void readMaterial(chunk_3ds* chunk, chunk_material3ds* mat)
{
	switch(chunk->id)
	{
		case 0xA000:			// name
			readString(mat->name);
			break;
		
		case 0xA010:			// ambient
			readChunkArray(chunk->length - 6, (cb)readColor, mat->ambient);
			mat->ambient[3] = 1.0f;
			break;
		
		case 0xA020:			// diffuse
			readChunkArray(chunk->length - 6, (cb)readColor, mat->diffuse);
			mat->diffuse[3] = 1.0f;
			break;
		
		case 0xA030:			// specular
			readChunkArray(chunk->length - 6, (cb)readColor, mat->specular);
			mat->specular[3] = 1.0f;
			break;
		
		case 0xA040:			// shininess
			readChunkArray(chunk->length - 6, (cb)readPercentage, &mat->shininess);
			mat->shininess *= 140.0f;
			break;
		
		case 0xA080:			// emissive
			readChunkArray(chunk->length - 6, (cb)readColor, mat->emissive);
			mat->emissive[3] = 1.0f;
			break;
		
		case 0xA200:			// texture index
			readChunkArray(chunk->length - 6, (cb)readTexture, mat);
			break;
		
		default:
			return;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// readTexture
void readTexture(chunk_3ds* chunk, chunk_material3ds* mat)
{
	if(chunk->id == 0xA300)
		readString(mat->texture);
	else
	{
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// readColor
void readColor(chunk_3ds* chunk, float* val)
{
	unsigned char rgb[3];
	
	if(chunk->id == 0x0010)     // COLOR_F
		fread(val, sizeof(float), 3, g_pFile);
		
	// color is stored as a normalized char //
	else if(chunk->id == 0x0011)			
	{
		fread(rgb, sizeof(char), 3, g_pFile);
		val[0] = float(rgb[0]) / 256.0f;
		val[1] = float(rgb[1]) / 256.0f;
		val[2] = float(rgb[2]) / 256.0f;
	}
	
	else
	{
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// readPercentage
void readPercentage(chunk_3ds* chunk, float* val)
{
	short sPer;
	
	// short percentage
	if(chunk->id == 0x0030)
	{
		fread(&sPer, sizeof(short), 1, g_pFile);
		*val = (float)sPer;
	}
	
	// float percentage
	else if(chunk->id == 0x0031)
		fread(val, sizeof(float), 1, g_pFile);

	else
	{
		return;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// removeDegenerates
void removeDegenerates(chunk_mesh3ds* mesh)
{
	int i, j, k, l;
	long* found;
	
	for(i = 0; i < mesh->triCount; ++i)
	{
		if((mesh->tris[i][0] >= mesh->vertCount) ||
		   (mesh->tris[i][1] >= mesh->vertCount) ||
		   (mesh->tris[i][2] >= mesh->vertCount))
		{
			i = i;
		}
	}
	
	for(i = 0; i < mesh->groupCount; ++i)
	{	
		// sort triangle groups by size
		qsort(mesh->groups[i].tris, mesh->groups[i].size, sizeof(long), (sortFunc_t)compareLongs);
	}
	
	for(i = 0; i < mesh->triCount; ++i)
	{
		// this monolithic if statement basically checks for dupe triangles
		if(((mesh->tris[i][0] == mesh->tris[i][1]) ||
            (mesh->tris[i][0] == mesh->tris[i][2]) ||
            (mesh->tris[i][1] == mesh->tris[i][2])) ||

          ((mesh->verts[mesh->tris[i][0]][0] == mesh->verts[mesh->tris[i][1]][0]) &&
           (mesh->verts[mesh->tris[i][0]][1] == mesh->verts[mesh->tris[i][1]][1]) &&
           (mesh->verts[mesh->tris[i][0]][2] == mesh->verts[mesh->tris[i][1]][2])) ||

          ((mesh->verts[mesh->tris[i][0]][0] == mesh->verts[mesh->tris[i][2]][0]) &&
           (mesh->verts[mesh->tris[i][0]][1] == mesh->verts[mesh->tris[i][2]][1]) &&
           (mesh->verts[mesh->tris[i][0]][2] == mesh->verts[mesh->tris[i][2]][2])) ||

          ((mesh->verts[mesh->tris[i][1]][0] == mesh->verts[mesh->tris[i][2]][0]) &&
           (mesh->verts[mesh->tris[i][1]][1] == mesh->verts[mesh->tris[i][2]][1]) &&
           (mesh->verts[mesh->tris[i][1]][2] == mesh->verts[mesh->tris[i][2]][2])))
		{
			// if there were any found, scooch over and kick it out
			if(i != (mesh->triCount - 1))
				memmove(&mesh->tris[i], &mesh->tris[i + 1], sizeof(long) * 3 * (mesh->triCount - i - 1));
			
			for(j = 0; j < mesh->groupCount; ++j)
			{
				found = (long*)bsearch(&i, mesh->groups[j].tris, mesh->groups[j].size, sizeof(long), (sortFunc_t)compareLongs);
				
				if(found != NULL)
				{
					k = ((int)found - (int)mesh->groups[j].tris) / sizeof(long);
					
					if(k < mesh->groups[j].size - 1)
						memmove(&mesh->groups[j].tris[k], &mesh->groups[j].tris[k + 1], sizeof(long) * (mesh->groups[j].size - k -1));
					
					mesh->groups[j].size--;
					
					for(l = k; l < mesh->groups[j].size; ++l)
						mesh->groups[j].tris[l]--;
				}
			}
			
			mesh->triCount--;
			--i;
        }
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// sortTriangles
void sortTriangles(chunk_mesh3ds* dat)
{
	int result, next, nextNext, i, j;
	result = next = nextNext = 0;
	triangle_3ds* tris;
	triangle_3ds* temp;
	long* oldSmooth = NULL;
	int groupCount;
	
	if(dat->groupCount == 0)
	{
		dat->groups = (chunk_group3ds*)malloc(sizeof(chunk_group3ds));
		if(dat->groups)
		{
			dat->groupCount = 1;
			memset(&dat->groups[0], 0, sizeof(chunk_group3ds));
			dat->groups[0].mat = -1;
			dat->groups[0].start = 0;
			dat->groups[0].size = dat->triCount;
			dat->groups[0].tris = (long*)malloc(sizeof(long) * dat->triCount);
			
			for(i = 0; i < dat->groups[0].size; ++i)
			{
				dat->groups[0].tris[i] = i;
			}
		}
		
		if(dat->smooth == NULL)
			return;
	}
	
	tris = (triangle_3ds*)malloc(dat->triCount * sizeof(triangle_3ds));
	temp = (triangle_3ds*)malloc(dat->triCount * sizeof(triangle_3ds));
	
	for(i = 0; i < dat->triCount; ++i)
	{
		memcpy(tris[i].verts, dat->tris[i], sizeof(long) * 3);
		tris[i].mat = -1;
		tris[i].index = i;
		
		if(dat->smooth)
			tris[i].smooth = dat->smooth[i];
		else
			tris[i].smooth = 0;
	}
	
	for(i = 0; i < dat->groupCount; ++i)
	{
		for(j = 0; j < dat->groups[i].size; ++j)
		{
			tris[dat->groups[i].tris[j]].mat = dat->groups[i].mat;
		}
	}
	
	
	// sort tris by material //
	i = 0;
	while(i < dat->triCount)
	{
		result = 0;
		next = i + 1;
		while((next < dat->triCount) && (result == 0))
		{
			result = sortByMaterial(&tris[i], &tris[next]);
			++next;
		}
		
		if(next == dat->triCount)
			break;
		--next;
		
		if(result > 0)
		{
			result = 0;
			nextNext = next + 1;
			while((nextNext < dat->triCount) && (result == 0))
			{
				result = sortByMaterial(&tris[next], &tris[nextNext]);
				++nextNext;
			}
			
			if(result != 0)
				--nextNext;
			
			// copy less than portion off to a temp buffer //
			memcpy(temp, &tris[next], (nextNext - next) * sizeof(triangle_3ds));
			
			// slide the greater than portion ahead //
			memmove(&tris[(nextNext - next) + i], &tris[i], (next - i) * sizeof(triangle_3ds));
			
			// copy the less than portion back in from temp //
			memcpy(&tris[i], temp, (nextNext - next) * sizeof(triangle_3ds));
			
			i = 0;
		}
		
		else if(result < 0)
			i = next;
		else
			break;
	}
	
	groupCount = 1;
	dat->groups[0].mat = tris[0].mat;
	dat->groups[0].start = 0;
	dat->groups[0].size = 0;
	for(i = 0; i < dat->triCount; ++i)
	{
		memcpy(dat->tris[i], tris[i].verts, sizeof(long) * 3);
		if(dat->smooth)
			dat->smooth[i] = tris[i].smooth;
		
		if(dat->groups[groupCount - 1].mat != tris[i].mat)
		{
			dat->groups[groupCount].mat = tris[i].mat;
			dat->groups[groupCount].start = i;
			dat->groups[groupCount].size = 0;
			++groupCount;
		}
		
		dat->groups[groupCount - 1].size++;
	}
	
	if(tris)
	{
		free(tris);
		tris = NULL;
	}
	
	if(temp)
	{
		free(temp);
		temp = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////
// calculateMeshTangentSpace
void calculateMeshTangentSpace(chunk_mesh3ds* mesh)
{
	if(mesh->norms == NULL)
		calculateMeshNormals(mesh);
	
	if((mesh->norms == NULL) || (mesh->texCoords == NULL))
		return;
	
	if(mesh->tangentSpace != NULL)
	{	
		free(mesh->tangentSpace);
		mesh->tangentSpace = NULL;
	}
	
	
	mesh->tangentSpace = (float(*)[9])malloc(sizeof(float) * 9 * mesh->triCount * 3);

	
	memset(mesh->tangentSpace, 0, sizeof(float) * 9 * mesh->triCount * 3);
	for(int i = 0; i < mesh->triCount; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			tangentSpace(mesh->verts[mesh->tris[i][0]], mesh->verts[mesh->tris[i][1]], mesh->verts[mesh->tris[i][2]],
						 mesh->texCoords[mesh->tris[i][0]], mesh->texCoords[mesh->tris[i][1]], mesh->texCoords[mesh->tris[i][2]],
						 mesh->norms[i * 3 + j], mesh->tangentSpace[i * 3 + j]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
// calculateMeshBoundingBox
void calculateMeshBoundingBox(chunk_mesh3ds* mesh)
{
	for(int i = 0; i < mesh->vertCount; ++i)
	{
		if(i == 0)
		{
			memcpy(mesh->max, mesh->verts[i], sizeof(float) * 3);
			memcpy(mesh->min, mesh->verts[i], sizeof(float) * 3);
		}
		
		else
		{
			for(int j = 0; j < 3; ++j)
			{
				if(mesh->verts[i][j] > mesh->max[j])
					mesh->max[j] = mesh->verts[i][j];
				if(mesh->verts[i][j] < mesh->min[j])
					mesh->min[j] = mesh->verts[i][j];
			}
		}
	}
	
	mesh->center[0] = mesh->min[0] + (mesh->max[0] - mesh->min[0]) * 0.5f;
	mesh->center[1] = mesh->min[1] + (mesh->max[1] - mesh->min[1]) * 0.5f;
	mesh->center[2] = mesh->min[2] + (mesh->max[2] - mesh->min[2]) * 0.5f;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// smoothTangentSpace
void smoothTangentSpace(chunk_mesh3ds* mesh)
{
	int i, j, k;
	int tri, vert;
	int tanCount;
	int (*tanRefs)[50];
	float tanSpace[9];
	float tempBinorm[3];
	float dotTangent;
	float dotBinormal;
	float dotNormal;
	float (*newTanSpace)[9];
	
	newTanSpace = (float(*)[9])malloc(sizeof(float) * 9 * mesh->triCount * 3);
	tanRefs = (int(*)[50])malloc(sizeof(int) * 50 * mesh->vertCount);

	
	memset(tanRefs, 0, sizeof(int) * 50 * mesh->vertCount);
	
	for(i = 0; i < mesh->triCount; ++i)
	{
		for(j = 0; j < 3; ++j)
		{
			if(tanRefs[mesh->tris[i][j]][0] < 48)
			{
				tanRefs[mesh->tris[i][j]][0]++;
				tanRefs[mesh->tris[i][j]][tanRefs[mesh->tris[i][j]][0]] = i * 3 + j;
			}
		}
	}
	
	for(i = 0; i < mesh->triCount; ++i)
	{
		for(j = 0; j < 3; ++j)
		{
			memset(tanSpace, 0, sizeof(float) * 9);
			tanCount = 0;
			
			for(k = 0; k <= tanRefs[mesh->tris[i][j]][0]; ++k)
			{
				tri = tanRefs[mesh->tris[i][j]][k] / 3;
				vert = tanRefs[mesh->tris[i][j]][k] % 3;
				
				dotTangent = QMATH_VEC3F_DOTPROD(&mesh->tangentSpace[i * 3 + j][0], &mesh->tangentSpace[tri * 3 + vert][0]);
				dotBinormal = QMATH_VEC3F_DOTPROD(&mesh->tangentSpace[i * 3 + j][3], &mesh->tangentSpace[tri * 3 + vert][3]);
				dotNormal = QMATH_VEC3F_DOTPROD(&mesh->tangentSpace[i * 3 + j][6], &mesh->tangentSpace[tri * 3 + vert][6]);
				
				if((dotTangent > 0.85) && (dotBinormal > 0.85) && (dotNormal > 0.85))
				{
					QMATH_VEC3F_ADDCOPY(&tanSpace[0], &mesh->tangentSpace[tri * 3 + vert][0]);
					QMATH_VEC3F_ADDCOPY(&tanSpace[3], &mesh->tangentSpace[tri * 3 + vert][3]);
					QMATH_VEC3F_ADDCOPY(&tanSpace[6], &mesh->tangentSpace[tri * 3 + vert][6]);
					++tanCount;
				}
			}
			
			float iScale = 1.0f / (float)tanCount;
			tanSpace[0] *= iScale;
			tanSpace[1] *= iScale;
			tanSpace[2] *= iScale;
			tanSpace[3] *= iScale;
			tanSpace[4] *= iScale;
			tanSpace[5] *= iScale;
			tanSpace[6] *= iScale;
			tanSpace[7] *= iScale;
			tanSpace[8] *= iScale;
			QMATH_VEC3F_NORMALIZE(&tanSpace[0]);
			QMATH_VEC3F_NORMALIZE(&tanSpace[3]);
			QMATH_VEC3F_NORMALIZE(&tanSpace[6]);

			QMATH_VEC3F_CROSSPROD(&tanSpace[0], &tanSpace[6], tempBinorm);
			QMATH_VEC3F_NORMALIZE(tempBinorm);
			dotBinormal = QMATH_VEC3F_DOTPROD(&tanSpace[3], tempBinorm);
			if(dotBinormal < 0.0f)
			{
				tempBinorm[0] = -tempBinorm[0];
				tempBinorm[1] = -tempBinorm[1];
				tempBinorm[2] = -tempBinorm[2];
			}
			
			memcpy(newTanSpace[i * 3 + j], tanSpace, sizeof(float) * 9);
			memcpy(&newTanSpace[i * 3 + j][3], tempBinorm, sizeof(float) * 3);
		}
	}
	
	free(mesh->tangentSpace);
	mesh->tangentSpace = newTanSpace;
	
	if(tanRefs)
	{
		free(tanRefs);
		tanRefs = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// tangentSpace
void tangentSpace(float* vert1, float* vert2, float* vert3, float* t1, float* t2, float* t3, float* norm, float* tangentSpace)
{
	float a, b;
	float u1, u2;
	float v1, v2;
	float edge1[3];
	float edge2[3];
	float dot;
	float dvTmp[3];
	float duTmp[3];
	
	if((t1[0] == t2[0]) ||
	   (t1[1] == t2[1]) ||
	   (t1[0] == t3[0]) ||
	   (t1[1] == t3[1]))
	{
		tangentSpace[0] = 1.0f;
		tangentSpace[1] = 0.0f;
		tangentSpace[2] = 0.0f;
		
		tangentSpace[3] = 0.0f;
		tangentSpace[4] = 1.0f;
		tangentSpace[5] = 0.0f;
		
		tangentSpace[6] = 0.0f;
		tangentSpace[7] = 0.0f;
		tangentSpace[8] = 1.0f;
		
		return;
	}
	
	edge1[0] = vert2[0] - vert1[0];
	edge1[1] = vert2[1] - vert1[1];
	edge1[2] = vert2[2] - vert1[2];
	edge2[0] = vert3[0] - vert1[0];
	edge2[1] = vert3[1] - vert1[1];
	edge2[2] = vert3[2] - vert1[2];
	
	u1 = t2[0] - t1[0];
    u2 = t3[0] - t1[0];
    v1 = t2[1] - t1[1];
    v2 = t3[1] - t1[1];
    
    a = (u1 - v1 * u2 / v2);
    if(a != 0.0f)
		a = 1.0f / a;
	
	b = (u2 - v2 * u1 / v1);
	if(b != 0.0f)
		b = 1.0f / b;
	
	duTmp[0] = a * edge1[0] + b * edge2[0];
	duTmp[1] = a * edge1[1] + b * edge2[1];
	duTmp[2] = a * edge1[2] + b * edge2[2];
	QMATH_VEC3F_NORMALIZE(duTmp);
	
	a = (v1 - u1 * v2 / u2);
	if(a != 0.0f)
		a = 1.0f / a;
	
	b = (v2 - u2 * v1 / u1);
	if(b != 0.0f)
		b = 1.0f / b;
	
	dvTmp[0] = a * edge1[0] + b * edge2[0];
	dvTmp[1] = a * edge1[1] + b * edge2[1];
	dvTmp[2] = a * edge1[2] + b * edge2[2];
	QMATH_VEC3F_NORMALIZE(dvTmp);
	
	tangentSpace[6] = norm[0];
	tangentSpace[7] = norm[1];
	tangentSpace[8] = norm[2];
	dot = QMATH_VEC3F_DOTPROD(duTmp, &tangentSpace[6]);
	
	tangentSpace[0] = duTmp[0] - (dot * tangentSpace[6]);
	tangentSpace[1] = duTmp[1] - (dot * tangentSpace[7]);
	tangentSpace[2] = duTmp[2] - (dot * tangentSpace[8]);
	QMATH_VEC3F_NORMALIZE(tangentSpace);
	
	dot = QMATH_VEC3F_DOTPROD(dvTmp, &tangentSpace[6]);
	
	tangentSpace[3] = dvTmp[0] - (dot * tangentSpace[6]);
	tangentSpace[4] = dvTmp[1] - (dot * tangentSpace[7]);
	tangentSpace[5] = dvTmp[2] - (dot * tangentSpace[8]);
	QMATH_VEC3F_NORMALIZE(&tangentSpace[3]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// calculateMeshNormals
void calculateMeshNormals(chunk_mesh3ds* mesh)
{
	mesh->norms = ( float(*)[3] )malloc( sizeof( float ) * 3 * mesh->triCount * 3 );
	memset( mesh->norms, 0, sizeof( float ) * 3 * mesh->triCount * 3 );
	
	for( unsigned i = 0; i < mesh->triCount; ++i )
	{
		vec3f n, v1, v2, v3;
		int vertA, vertB, vertC;
		
		vertA = mesh->tris[i][0];
		vertB = mesh->tris[i][1];
		vertC = mesh->tris[i][2];
		
		v1.set( mesh->verts[vertA] );
		v2.set( mesh->verts[vertB] );
		v3.set( mesh->verts[vertC] );
		
		vec3f u, v;
		u = v2 - v1;
		v = v3 - v1;
		n = u.crossProd( v );
		n.normalize();
		
		mesh->norms[vertA][0] += n.x;
		mesh->norms[vertA][1] += n.y;
		mesh->norms[vertA][2] += n.z;
		mesh->norms[vertB][0] += n.x;
		mesh->norms[vertB][1] += n.y;
		mesh->norms[vertB][2] += n.z;
		mesh->norms[vertC][0] += n.x;
		mesh->norms[vertC][1] += n.y;
		mesh->norms[vertC][2] += n.z;
		
		QMATH_VEC3F_NORMALIZE( mesh->norms[vertA] );
		QMATH_VEC3F_NORMALIZE( mesh->norms[vertB] );
		QMATH_VEC3F_NORMALIZE( mesh->norms[vertC] );
	}
/*
	int i, j, k;
	int normCount;
	long (*triRefs)[50];
	float (*faceNorms)[3];
	float v1[3];
	float v2[3];
	
	mesh->norms = (float(*)[3])malloc(sizeof(float) * 3 * mesh->triCount * 3);
	memset(mesh->norms, 0, sizeof(float) * 3 * mesh->triCount * 3);
	
	if(mesh->smooth == NULL)
	{
		for(i = 0; i < mesh->triCount; ++i)
		{
			QMATH_VEC3F_SUB(mesh->verts[mesh->tris[i][1]], mesh->verts[mesh->tris[i][0]], v1);
			QMATH_VEC3F_SUB(mesh->verts[mesh->tris[i][2]], mesh->verts[mesh->tris[i][0]], v2);
			QMATH_VEC3F_CROSSPROD(v1, v2, mesh->norms[i * 3]);
			QMATH_VEC3F_NORMALIZE(mesh->norms[i * 3]);
			
			memcpy(mesh->norms[i * 3 + 1], mesh->norms[i * 3], sizeof(float) * 3);
			memcpy(mesh->norms[i * 3 + 2], mesh->norms[i * 3], sizeof(float) * 3);
		}
		
		return;
	}

	index_t vertIndex;
	int vert = 0;
	int result = 0;
	
	indexArray(&vertIndex, (char*)mesh->verts, sizeof(float) * 3, mesh->vertCount, (sortFunc_t)comparePos);
	
	triRefs = (long(*)[50])malloc(sizeof(long) * 50 * vertIndex.count);

	memset(triRefs, 0, sizeof(unsigned long) * 50 * vertIndex.count);
	
	for(i = 0; i < mesh->triCount; ++i)
	{
		for(j = 0; j < 3; ++j)
		{
			vert = indexFind(&vertIndex, mesh->verts[mesh->tris[i][j]], &result);
			if(triRefs[vert][0] < 48)
			{
				triRefs[vert][0]++;
				triRefs[vert][triRefs[vert][0]] = i;
			}
		}
	}
	
	faceNorms = (float(*)[3])malloc(sizeof(float) * 3 * mesh->triCount);

	
	if(triRefs != NULL)
	{
		memset(faceNorms, 0, sizeof(float) * 3 * mesh->triCount);
		
		for(i = 0; i < mesh->triCount; ++i)
		{
			QMATH_VEC3F_SUB(mesh->verts[mesh->tris[i][1]], mesh->verts[mesh->tris[i][0]], v1);
			QMATH_VEC3F_SUB(mesh->verts[mesh->tris[i][2]], mesh->verts[mesh->tris[i][0]], v2);
			QMATH_VEC3F_CROSSPROD(v1, v2, faceNorms[i]);
			QMATH_VEC3F_NORMALIZE(faceNorms[i]);
		}
		
		for(i = 0; i < mesh->triCount; ++i)
		{
			for(j = 0; j < 3; ++j)
			{
				vert = indexFind(&vertIndex, mesh->verts[mesh->tris[i][j]], &result);
				
				normCount = 0;
				for(k = 1; k <= triRefs[vert][0]; ++k)
				{
					if(mesh->smooth[i] == mesh->smooth[triRefs[vert][k]])
					{
						QMATH_VEC3F_ADDCOPY(mesh->norms[i * 3 + j], faceNorms[triRefs[vert][k]]);
						QMATH_VEC3F_NORMALIZE( mesh->norms[i * 3 + j] );
						++normCount;
					}
				}
				
//				float iScale = 1.0f / (float)normCount;
//				
//				QMATH_VEC3F_SCALE(mesh->norms[i * 3 + j], iScale);
//				QMATH_VEC3F_NORMALIZE(mesh->norms[i * 3 + j]);
			}
		}
	}
	
	indexFree(&vertIndex);
	
	if(triRefs)
	{
		free(triRefs);
		triRefs = NULL;
	}
	
	if(faceNorms)
	{
		free(faceNorms);
		faceNorms = NULL;
	}
*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

c3DSModel::~c3DSModel()
{
	for(int i = 0; i < textureHandleList.size(); ++i)
	{
		if(QRENDER_IS_VALID(textureHandleList[i].textureRef))
			g_pRender->UnloadTextureObject(textureHandleList[i].textureRef);
	}
	
	for(int i = 0; i < meshRenderHandles.size(); ++i)
	{
		if(QRENDER_IS_VALID(meshRenderHandles[i].iboRef))
			g_pRender->UnloadIndexBuffer(meshRenderHandles[i].iboRef);
	
		if(QRENDER_IS_VALID(meshRenderHandles[i].vboRef))
			g_pRender->UnloadInstancedVertexBuffer(meshRenderHandles[i].vboRef);
		
//		if( meshRenderHandles[i].lowLODMesh != -1 )
//			g_pRender->deleteOptimizedMeshObject( meshRenderHandles[i].lowLODMesh );
		
//		if( meshRenderHandles[i].medLODMesh != -1 )
//			g_pRender->deleteOptimizedMeshObject( meshRenderHandles[i].medLODMesh );
	}
	
//	if( vertexFormatHandle != QRENDER_INVALID_VERTEX_FORMAT )
//		g_pRender->deleteVertexFormat( vertexFormatHandle );
		

	free3DSData(&mdlData);
	
//	m_bIsLoaded = false;
}



c3DSModel::c3DSModel(const unsigned int handle, const std::string& name, const std::string& path) : CModelObject(handle, name, path)
{
	fileName = path + name;
	
	m_diffuseBindPoint = 0;
	m_normalmapBindPoint = 2;
//	m_bIsLoaded = false;
//	std::string file_path = path + name;
//	if(loadModel(file_path.c_str()))
//		is3DSLoaded = true;
}



//////////////////////////////////////////////////////////////////////////////////////
// loadModel
// Loads .3DS file from path (fName).
// Automatically handles texture resources by appending proper file extensions
// and obtaining handles from the renderer
bool c3DSModel::LoadModel( bool loadNormalmaps )
{
	if(!read3DSFile(fileName.c_str(), &mdlData))
		return false;
		
	
	textureHandleList.resize( mdlData.materialCount );
	
	for( int i = 0; i < mdlData.materialCount; ++i )
	{
		textureHandleList[i].ambient.set( mdlData.materials[i].ambient );
		textureHandleList[i].emissive.set( mdlData.materials[i].emissive );
		textureHandleList[i].diffuse.set( mdlData.materials[i].diffuse );
		textureHandleList[i].specular.set( mdlData.materials[i].specular );
		textureHandleList[i].shininess = mdlData.materials[i].shininess;
		
	
		// check for null texture name //
		if( !mdlData.materials[i].texture || strcmp( mdlData.materials[i].texture, "" ) == 0 )
		{
			CQuadrionTextureFile tex;
			tex.SetFileName( mdlData.materials[i].name );
			tex.SetFilePath( "./" );
			unsigned int color = 0x000000ff;
			color |= ( unsigned char( textureHandleList[i].diffuse.x * 255 ) << 24 );
			color |= ( unsigned char( textureHandleList[i].diffuse.y * 255 ) << 16 );
			color |= ( unsigned char( textureHandleList[i].diffuse.z * 255 ) << 8 );
			tex.LoadFromColor( color );
			
			unsigned int flags = QTEXTURE_FILTER_TRILINEAR;
			textureHandleList[i].textureRef = g_pRender->AddTextureObject( tex, flags );
			textureHandleList[i].normalmapRef = QRENDER_INVALID_HANDLE;
			textureHandleList[i].fileName.assign( mdlData.materials[i].name );
			continue;
//			textureHandleList[i].fileName.assign( "NULL" );
//			textureHandleList[i].textureRef = QRENDER_INVALID_HANDLE;
//			continue;
		}
	
		// check for duplicate textures //
		std::string strTex( mdlData.materials[i].texture );
		
		// now prepend texture/ to the file name //
//		std::string insert( baseDir("models/") );
//		strTex.insert( 0, insert );
//		textureHandleList[i].fileName.assign( strTex );
		strTex.insert(0, "Textures/");
		
		memset(mdlData.materials[i].texture, 0, sizeof(char) * 32);
		strcpy(mdlData.materials[i].texture, strTex.c_str());
		
		unsigned int tex_flags = QTEXTURE_FILTER_TRILINEAR;
		textureHandleList[i].textureRef = g_pRender->AddTextureObject( tex_flags, strTex.c_str(), m_filePath );
		if(QRENDER_IS_VALID(textureHandleList[i].textureRef))
			textureHandleList[i].fileName.assign(strTex);
			
		if( loadNormalmaps )
		{
			tex_flags |= QTEXTURE_NORMALHEIGHTMAP;
			textureHandleList[i].normalmapRef = g_pRender->AddTextureObject( tex_flags, strTex.c_str(), m_filePath );
			if( QRENDER_IS_VALID( textureHandleList[i].normalmapRef ) )
				m_bHasNormalmaps = true;
		}
	}
	
//	makeVertexNormals();
	
	meshRenderHandles.resize(mdlData.meshCount);

	SQuadrionVertexDescriptor v_desc;
	v_desc.pool = QVERTEXBUFFER_MEMORY_STATIC;
	v_desc.usage[0] = QVERTEXFORMAT_USAGE_POSITION;
	v_desc.size[0] = QVERTEXFORMAT_SIZE_FLOAT3;
	v_desc.usage[1] = QVERTEXFORMAT_USAGE_NORMAL;
	v_desc.size[1] = QVERTEXFORMAT_SIZE_FLOAT3;
	v_desc.usage[2] = QVERTEXFORMAT_USAGE_TANGENT;
	v_desc.size[2] = QVERTEXFORMAT_SIZE_FLOAT3;
	v_desc.usage[3] = QVERTEXFORMAT_USAGE_TEXCOORD;
	v_desc.size[3] = QVERTEXFORMAT_SIZE_FLOAT2;
	v_desc.usage[4] = QVERTEXFORMAT_USAGE_END;

	for(int i = 0; i < mdlData.meshCount; ++i)
	{
		if(mdlData.meshes[i].vertCount == 0 || mdlData.meshes[i].triCount == 0)
			continue;
	
		int num_indices = mdlData.meshes[i].triCount * 3;
		bool int_indices = false;
		unsigned short* mesh_indices = (unsigned short*)malloc(sizeof(unsigned short) * num_indices);
			
		s3DSVertexFormat* mverts = new s3DSVertexFormat[mdlData.meshes[i].vertCount];
		
		int idx = 0;
		for( int j = 0; j < mdlData.meshes[i].vertCount; ++j )
		{
			mverts[idx].x = mdlData.meshes[i].verts[j][0]; 
			mverts[idx].y = mdlData.meshes[i].verts[j][1]; 
			mverts[idx].z = mdlData.meshes[i].verts[j][2]; 
			
			vec3f norm(mdlData.meshes[i].norms[j]);
			norm.normalize();
			mverts[idx].nx = norm.x;
			mverts[idx].ny = norm.y;
			mverts[idx].nz = norm.z;
			
			if( mdlData.meshes[i].tangentSpace )
			{
				mverts[idx].tx = mdlData.meshes[i].tangentSpace[j][3]; 
				mverts[idx].ty = mdlData.meshes[i].tangentSpace[j][4]; 
				mverts[idx].tz = mdlData.meshes[i].tangentSpace[j][5]; 
			}
			
			else
			{
				mverts[idx].tx = 0.0f;
				mverts[idx].ty = 0.0f;
				mverts[idx].tz = 0.0f;
			}
				
			if( mdlData.meshes[i].texCoordCount <= 0 )
			{
				mverts[idx].u = 0.0f; 
				mverts[idx].v = 0.0f; 
			}
				
			else
			{
				mverts[idx].u = mdlData.meshes[i].texCoords[j][0]; 
				mverts[idx].v = 1.0f - mdlData.meshes[i].texCoords[j][1]; 
			}
			
			++idx;
		}
		
		for( int c = 0; c < mdlData.meshes[i].triCount; ++c )
		{
			mesh_indices[c * 3 + 0] = (unsigned short)mdlData.meshes[i].tris[c][0];
			mesh_indices[c * 3 + 1] = (unsigned short)mdlData.meshes[i].tris[c][1];
			mesh_indices[c * 3 + 2] = (unsigned short)mdlData.meshes[i].tris[c][2];
		}

		meshRenderHandles[i].vboRef = g_pRender->AddVertexBuffer();
		meshRenderHandles[i].iboRef = g_pRender->AddIndexBuffer();
		
		CQuadrionVertexBuffer* vb = g_pRender->GetVertexBuffer(meshRenderHandles[i].vboRef);
//		EQuadrionVertexAttribUsage geom[5] = {QVERTEXFORMAT_USAGE_POSITION, QVERTEXFORMAT_USAGE_NORMAL, QVERTEXFORMAT_USAGE_TANGENT, 
//											  QVERTEXFORMAT_USAGE_TEXCOORD, QVERTEXFORMAT_USAGE_END};
//		EQuadrionVertexAttribUsage instance[5] = {QVERTEXFORMAT_USAGE_TEXCOORD, QVERTEXFORMAT_USAGE_TEXCOORD, QVERTEXFORMAT_USAGE_TEXCOORD, 
//												  QVERTEXFORMAT_USAGE_TEXCOORD, QVERTEXFORMAT_USAGE_END};
//		vb->SetVertexStreams(geom, instance, NULL, NULL);
		vb->CreateVertexBuffer(mverts, v_desc, mdlData.meshes[i].vertCount, false);
		
		CQuadrionIndexBuffer* ib = g_pRender->GetIndexBuffer(meshRenderHandles[i].iboRef);
		ib->CreateIndexBuffer(QINDEXBUFFER_MEMORY_STATIC, QINDEXBUFFER_SIZE_USHORT, num_indices, mesh_indices);
		
//		meshRenderHandles[i].vboRef = g_pRender->addVertexBuffer( sizeof(s3DSVertexFormat) * mdlData.meshes[i].vertCount, mverts, QRENDER_DATA_STATIC );
//		meshRenderHandles[i].iboRef = g_pRender->addIndexBuffer( mdlData.meshes[i].triCount * 3, QRENDER_INDEX_USHORT, mindices, QRENDER_DATA_STATIC );
		
//		if( loadLODModel ) 
//		{
//			int mesh = g_pRender->addMeshObject( mdlData.meshes[i].triCount, mdlData.meshes[i].vertCount, vertexFormatHandle, SMS_MESH_DYNAMIC );
//			g_pRender->copyIndicesToMesh( mesh, (void*)&mindices[0] );
//			g_pRender->copyVerticesToMesh( mesh, (void*)&mverts[0].x );
//			int optimizedMesh = g_pRender->createOptimizedMeshFromMesh( mesh, 0.01F );
			
//			int nMediumFaces, nLowFaces;
//			nMediumFaces = (int) ( mdlData.meshes[i].triCount / 2 );
//			nLowFaces = (int) ( mdlData.meshes[i].triCount / 5 );
//			g_pRender->reduceOptimizedMeshFaces( optimizedMesh, nMediumFaces );
			
//			meshRenderHandles[i].medLODMesh = g_pRender->createMeshFromOptimizedMesh( optimizedMesh, SMS_MESH_DYNAMIC );
//			g_pRender->reduceOptimizedMeshFaces( optimizedMesh, nLowFaces );
//			meshRenderHandles[i].lowLODMesh = g_pRender->createMeshFromOptimizedMesh( optimizedMesh, SMS_MESH_DYNAMIC );
			
//			g_pRender->deleteMeshObject( mesh );
//			g_pRender->deleteOptimizedMeshObject( optimizedMesh );
//		}

		if(mverts) delete[] mverts; mverts = NULL;
		if(mesh_indices) free(mesh_indices); mesh_indices = NULL;
	}
	
	m_bIsLoaded = true;
	m_bIsRenderable = true;
	
	m_modelCenter.set(mdlData.center[0], mdlData.center[1], mdlData.center[2]);
	return true;
}

///////////////////////////////////////////////////////////////////////
// killModel
// Explicit destructor
void c3DSModel::killModel()
{
	c3DSModel::~c3DSModel();
}

////////////////////////////////////////////////////////////////////////
// getMeshOrientationMat
// Constructs a 4x4 matrix from the mesh (mesh)'s axis information
// and stores the result in matrix (m)
void c3DSModel::getMeshOrientationMat(mat4& m, const int mesh)
{
	m[0] = mdlData.meshes[mesh].axis[0][0];
	m[1] = mdlData.meshes[mesh].axis[1][0];
	m[2] = mdlData.meshes[mesh].axis[2][0];
	m[3] = 0.0f;
	
	m[4] = mdlData.meshes[mesh].axis[0][1];
	m[5] = mdlData.meshes[mesh].axis[1][1];
	m[6] = mdlData.meshes[mesh].axis[2][1];
	m[7] = 0.0f;
	
	m[8] = mdlData.meshes[mesh].axis[0][2];
	m[9] = mdlData.meshes[mesh].axis[1][2];
	m[10] = mdlData.meshes[mesh].axis[2][2];
	m[11] = 0.0f;
	
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
}

///////////////////////////////////////////////////////////////
// getMeshCenter
// Returns the model space center of the mesh (mesh)
vec3f c3DSModel::getMeshCenter(const int mesh)
{
	return vec3f(mdlData.meshes[mesh].center[0], mdlData.meshes[mesh].center[1], mdlData.meshes[mesh].center[2]);
}

///////////////////////////////////////////////////////////////
// getMeshPosition
// Returns the world space center of the mesh(mesh) in relation
// to the rest of the model
vec3f c3DSModel::getMeshPosition(const int mesh)
{
	return vec3f(mdlData.meshes[mesh].position[0], mdlData.meshes[mesh].position[1], mdlData.meshes[mesh].position[2]);
}

void c3DSModel::GetAABB(vec3f& mins, vec3f& maxs)
{
	mins.set(mdlData.min[0], mdlData.min[1], mdlData.min[2]); 
	maxs.set(mdlData.max[0], mdlData.max[1], mdlData.max[2]); 
}

void c3DSModel::GetBoundingSphere(vec3f& center, float& rad)
{
	vec3f mins(mdlData.min[0], mdlData.min[1], mdlData.min[2]);
	vec3f maxs(mdlData.max[0], mdlData.max[1], mdlData.max[2]);
	vec3f minsToMaxs = mins - maxs;
	float len = minsToMaxs.getLength();
	len *= 0.5F;
	minsToMaxs.normalize();
	
	center = mins + (minsToMaxs * len);
	rad = len;
}

/////////////////////////////////////////////////////////////
// makeVertexModels
// Routine that makes per-face normals into per-vertex normals
// Normals supplied by the model are OVERWRITTEN by this routine
// It will not make backups
void c3DSModel::makeVertexNormals()
{
/*
	for(int i = 0; i < mdlData.meshCount; ++i)
	{
		if(mdlData.meshes[i].vertCount > 0 && mdlData.meshes[i].triCount > 0)
		{
			UINT* pl = new UINT[mdlData.meshes[i].triCount * 3];
			vec3f* vl = new vec3f[mdlData.meshes[i].vertCount];
			for(int z = 0; z < mdlData.meshes[i].triCount; ++z)
			{
				pl[z * 3 + 0] = (UINT)mdlData.meshes[i].tris[z][0];
				pl[z * 3 + 1] = (UINT)mdlData.meshes[i].tris[z][1];
				pl[z * 3 + 2] = (UINT)mdlData.meshes[i].tris[z][2];
			}
			
			for(int z = 0; z < mdlData.meshes[i].vertCount; ++z)
			{
				vl[z].set(mdlData.meshes[i].verts[z][0], mdlData.meshes[i].verts[z][1], mdlData.meshes[i].verts[z][2]);
			}
			QMATH_CREATE_VERTEX_NORMALS(vl, mdlData.meshes[i].vertCount, pl, mdlData.meshes[i].triCount, (FLOAT*)&mdlData.meshes[i].norms[0]);
			delete[] pl;
			pl = NULL;
			delete[] vl;
			vl = NULL;
		}
	}
*/
	
	for( unsigned i = 0; i < mdlData.meshCount; ++i )
	{
		if( mdlData.meshes[i].vertCount > 0 && mdlData.meshes[i].triCount > 0 )
		{
			
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////
// getMeshBoundingBox
// Obtains the bounding box for the mesh "mesh", and packs them into 
// the vectors "mins" and "maxs"
void c3DSModel::getMeshBoundingBox( const int mesh, vec3f& mins, vec3f& maxs )
{
	mins.set(mdlData.meshes[mesh].min[0], mdlData.meshes[mesh].min[1], mdlData.meshes[mesh].min[2]);
	maxs.set(mdlData.meshes[mesh].max[0], mdlData.meshes[mesh].max[1], mdlData.meshes[mesh].max[2]);
}


void c3DSModel::RenderModel()
{	
	g_pRender->ChangeCullMode(QRENDER_CULL_CW);

	CQuadrionVertexBuffer* mesh_vbo = NULL;
	CQuadrionIndexBuffer* mesh_ibo = NULL;
	
	for(int i = 0; i < mdlData.meshCount; ++i)
	{
		chunk_mesh3ds cur_mesh = mdlData.meshes[i];
		if(mdlData.meshes[i].triCount < 1)
			continue;
	
		if( m_normalmapBindPoint >= 0 || m_diffuseBindPoint >= 0 )
		{
			int cur_bound_textures = 0;
			for(int j = 0; j < mdlData.meshes[i].groupCount; ++j)
			{
				chunk_group3ds cur_chunk = cur_mesh.groups[j];
				chunk_material3ds cur_mat = mdlData.materials[cur_mesh.groups[j].mat];
				
				std::string cur_tex = cur_mat.texture;
				int c;
				for(c = 0; c < textureHandleList.size(); ++c)
				{
					if(cur_tex.compare(textureHandleList[c].fileName) == 0)
						break;
				}
				
				if(c < textureHandleList.size())
				{
					CQuadrionTextureObject* tex_obj = NULL;
					CQuadrionTextureObject* nm_obj = NULL;
					if( m_diffuseBindPoint >= 0 )
					{
						tex_obj = g_pRender->GetTextureObject(textureHandleList[c].textureRef);
						if( tex_obj )
							tex_obj->BindTexture( m_diffuseBindPoint );
					}
					
					if( m_normalmapBindPoint >= 0 )
					{
						nm_obj = g_pRender->GetTextureObject( textureHandleList[c].normalmapRef );
						if( nm_obj )
							nm_obj->BindTexture( m_normalmapBindPoint );
					}
				}
				
				else
				{
					c = 0;
					for( c = 0; c < textureHandleList.size(); ++c )
					{
						if( strcmp( cur_mat.name, textureHandleList[c].fileName.c_str() ) == 0 )
						{
							if( QRENDER_IS_VALID( textureHandleList[c].textureRef ) )
							{
								if( m_diffuseBindPoint >= 0 )
								{
									CQuadrionTextureObject* tex_obj = g_pRender->GetTextureObject( textureHandleList[c].textureRef );
									if( tex_obj )
										tex_obj->BindTexture( m_diffuseBindPoint );
								}
								
								if( m_normalmapBindPoint >= 0 )
								{
									CQuadrionTextureObject* nm_obj = g_pRender->GetTextureObject( textureHandleList[c].normalmapRef );
									if( nm_obj )
										nm_obj->BindTexture( m_normalmapBindPoint );
								}
							}
						}
					}
				}
			}
		}
				
		mesh_vbo = g_pRender->GetVertexBuffer(meshRenderHandles[i].vboRef);
		mesh_vbo->BindBuffer();
		mesh_ibo = g_pRender->GetIndexBuffer(meshRenderHandles[i].iboRef);
		mesh_ibo->BindBuffer();
		
		g_pRender->RenderIndexedList(QRENDER_PRIM_TRIANGLES, 0, 0, cur_mesh.vertCount, cur_mesh.triCount * 3);
		
		mesh_vbo->UnbindBuffer();
		mesh_ibo->UnbindBuffer();
//		g_pRender->EvictTextures();
	}	
	
	g_pRender->EvictTextures();
	g_pRender->ChangeCullMode(QRENDER_CULL_DEFAULT);
}


