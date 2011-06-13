/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// QVERTEXBUFFER.H
//
// Author: SMS
// Date: 01/2009
//
// This file contains the new vertex buffer object interface code for the new renderer.
// Vertex Buffer Objects (or VBOs) allow the programmer to batch upload arrays
// of vertices into specific memory locations for access in reading or writing.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////



#include "qresource.h"
#include "qmath.h"

#ifndef __QVERTEXBUFFER_H_
#define __QVERTEXBUFFER_H_



#ifdef QRENDER_EXPORTS
	#define QVERTEXBUFFEREXPORT_API		__declspec(dllexport)
#else
	#define QVERTEXBUFFEREXPORT_API		__declspec(dllimport)
#endif


#ifdef _DEBUG
	#define D3D_DEBUG_INFO
#endif

#include <string>
#include <vector>
#include <d3d9.h>
#include <d3dx9.h>



const unsigned int		QVERTEXBUFFER_MAXSTREAMS	= 4;
const unsigned int		QVERTEXBUFFER_MAXINSTANCES  = 512;

enum QVERTEXBUFFEREXPORT_API EQuadrionVertexAttribUsage
{
	QVERTEXFORMAT_USAGE_POSITION		= 0,
	QVERTEXFORMAT_USAGE_NORMAL			= 1,
	QVERTEXFORMAT_USAGE_BINORMAL		= 2,
	QVERTEXFORMAT_USAGE_TANGENT			= 3,
	QVERTEXFORMAT_USAGE_TEXCOORD		= 4,
	QVERTEXFORMAT_USAGE_COLOR			= 5,
	QVERTEXFORMAT_USAGE_BLENDWEIGHT		= 6,
	QVERTEXFORMAT_USAGE_INDEXWEIGHT		= 7,
	QVERTEXFORMAT_USAGE_FOG				= 8,
	QVERTEXFORMAT_USAGE_GENERIC			= 9,
	QVERTEXFORMAT_USAGE_END				= 10,
};

enum QVERTEXBUFFEREXPORT_API EQuadrionVertexAttribSize
{
	QVERTEXFORMAT_SIZE_FLOAT1			= 0,
	QVERTEXFORMAT_SIZE_FLOAT2			= 1,
	QVERTEXFORMAT_SIZE_FLOAT3			= 2,
	QVERTEXFORMAT_SIZE_FLOAT4			= 3,
	QVERTEXFORMAT_SIZE_COLOR			= 4,
};


const unsigned int		QVERTEXBUFFER_MEMORY_DYNAMIC		= 0x00010000;
const unsigned int		QVERTEXBUFFER_MEMORY_STREAM			= 0x00020000;
const unsigned int		QVERTEXBUFFER_MEMORY_STATIC			= 0x00040000;
const unsigned int		QVERTEXBUFFER_MEMORY_SYSTEM			= 0x00080000;





//////////////////////////////////////////////////////////////////////////
// 
// SQuadrionVertexDescriptor
//
// Struct which describes the vertex format in the vertex buffer to
// be created
//
// Must have a final usage description of QVERTEXFORMAT_USAGE_END to
// signify that this is the end of the description
//
////////////////////////////////////////////////////////////////////////////
struct QVERTEXBUFFEREXPORT_API SQuadrionVertexDescriptor
{
	EQuadrionVertexAttribUsage		usage[16];
	EQuadrionVertexAttribSize		size[16];
	unsigned int					pool;				// One of QRENDER_VERTEX_BUFFER_DYNAMIC, QRENDER_VERTEX_BUFFER_STREAM, QRENDER_VERTEX_BUFFER_STATIC
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CQuadrionVertexBuffer
//
// VBO object class allows the programmer the ability to create and interact with
// vertex buffer objects. 
//
// This is a resource class, so creation, unloading, and obtaining the VBO interfaces
// are all handled via CQuadrionRender
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class QVERTEXBUFFEREXPORT_API CQuadrionVertexBuffer : public CQuadrionResource
{
	public:
		
		CQuadrionVertexBuffer(const unsigned int handle, const std::string& name, const std::string& path = "./");
		
		// Constructor w/ void*
		// Creates vertex buffer and associates with the render object point to by "pRender"
		// The Vertex buffer MUST be associated with a render device, otherwise all calls will fail
		CQuadrionVertexBuffer(const void* pRender, const unsigned int handle, const std::string& name, const std::string& path = "./");
		
		~CQuadrionVertexBuffer();
		

		
		
		// SetVertexStreams -- Allows the user to customize the vertex stream layout before vertex buffer creation
		// Params:
		// stream0- List of EQuadrionVertexAttributeUsage elements which will be packed for Stream 0
		// stream1- See Stream0 
		// stream2- See Stream0
		// stream3- See Stream0
		bool			SetVertexStreams(const EQuadrionVertexAttribUsage* stream0,
										 const EQuadrionVertexAttribUsage* stream1,
										 const EQuadrionVertexAttribUsage* stream2,
										 const EQuadrionVertexAttribUsage* stream3);
		
		
		// BindBuffer -- This binds the vertex buffer object as the current one. All subsequent render calls will reference this vertex buffer.
		//				 As such, BindBuffer must be called before any render calls that use this vertex buffer
		// Params:
		// streams- List of streams that the user wishes to activate:   EX:    unsigned int streams[2] = {0, 2};  
		//		    When passed, this will make streams 0 and 2 active
		// nStreams- The number of streams to be set. (The size of "streams")
		// nInstances- Defaults to no instancing. This parameter is used to instruct the vertex buffer as to how many instances to draw
		//			   from the instance buffer
		bool			BindBuffer(const unsigned int* streams, const int& nStreams, const int& nInstances = -1);
		bool			BindBuffer();
		
		// UnbindBuffer -- This unbinds the vertex buffer as the current one. This should be called when this vertex buffer is no longer
		//				   going to be used in subsequent draw calls
		bool			UnbindBuffer();
		
		// CreateInstanceBuffer -- 
		bool			CreateInstanceBuffer(mat4* pInstances, const int& nInstances);
		bool			UpdateInstanceBuffer(mat4* pInstances, const int& nInstances);
		
		
		// GetBufferSize -- This returns the size of the entire vertex buffer (all streams)(in bytes).
		int			GetBufferSize();
		
		// GetVertexCount -- This returns the number of vertices in this buffer //
		const inline unsigned int		GetVertexCount() { return m_nVertices; }
		
		// GetStreamSize -- This returns the entire size (in bytes) of a particular stream
		// Params:
		// stream-	The stream whose size to obtain. (0 - QVERTEXBUFFER_MAXSTREAMS)
		int			GetStreamSize(const unsigned int& stream);
		
		// GetStreamByAttribute -- This returns the stream that an attribute lies in
		// Params:
		// attrib- The attribute to query
		int			GetStreamByAttribute(const EQuadrionVertexAttribUsage& attrib);
		
		
		// UpdateBufferData -- This allows the user to update the data for the entire vertex buffer.
		//					   NOTE: This method will not work for vertex buffers that a streamed. The user must call
		//							 UpdateStreamData instead
		// Params:
		// dat- buffer of new data. Must be of valid size. IE: must be the exact size of the vertex buffer.
		//      To obtain this size, call GetBufferSize()
		bool		UpdateBufferData(const void* dat);
		bool		UpdateBufferData(const void* dat, const unsigned int& bytes);
		
		
		// CopyBufferData -- (FOR non-streamed VBOs) This allows the user to obtain vertex buffer data from where it resides
		//				     by packing the data into a client side buffer. The passed buffer must be allocated
		//					 to the same size as the vertex buffer. The size of the vertex buffer can be obtained by calling GetBufferSize
		// Params:
		// buf: Buffer in which data will be copied (copied from stream 0)
		bool		CopyBufferData(void* buf);
		
		
		// UpdateStreamData -- This allows the user to update the data for a particular stream
		//					   NOTE: This method will not work for vertex buffers that are not streamed. The user must call
		//							 UpdateBufferData instead
		// Params:
		// dat- buffer of new data. Must be of valid size. IE: must be the exact size of the vertex buffer for the specified stream.
		//      This size can be obtained by calling GetStreamSize
		// stream- The stream in which the data is to be updated
		bool		UpdateStreamData(const void* dat, const unsigned int& stream);
		
		// CreateVertexBuffer -- Creates a vertex buffer from local memory pool and uploads to client specified memory
		// params:
		// pVertices- list of vertex data
		// desc- filled out SQuadrionVertexDescriptor object which describes the layout of the data in "pVertices"
		// nVerts- number of vertices in "pVertices"
		// stream- should be "true" to hint the vertex buffer that the vertex buffer should be split into smaller streamed vertex buffers
		//         should be "false" to hint the vertex buffer that the vertex buffer should be loaded as-is 
		// 
		// return values:   true- if succeeded
		//					false- if failed
		bool			CreateVertexBuffer(const void* pVertices, const SQuadrionVertexDescriptor& desc, const int& nVerts, const bool& stream, const int& texOffset = -1);
		
		
		// HasInstanceBuffer -- Returns whether or not this vertex buffer has an instance buffer with it
		// params: none
		const inline bool		HasInstanceBuffer() { return m_bHasInstanceBuffer; }
		
		// TagAsInstanceBuffer -- Tag the buffer created as an instance buffer ONLY...this is different from CONTAINING an instance buffer
		// params: none
		const inline void		TagAsInstanceBuffer() { m_bIsInstanceBuffer = true; }
		
	
	protected:
	
		
		// DestroyVertexBuffer -- Releases all allocated assets of the vertex buffer and makes invalid
		void			DestroyVertexBuffer();
		
		// ChangeRenderDevice -- Changes the vertex buffer's render device association
		// params:
		// pRender-	pointer to new render device
		void			ChangeRenderDevice(const void* pRender);
	
	private:
	
		friend class					CQuadrionRender;
	
		// Private Usage //
		int								GetStream(const EQuadrionVertexAttribUsage& attrib);
		unsigned int					GetVertexStreamSize(const SQuadrionVertexDescriptor& desc, const int& stream);
		unsigned int					GetFinalVertexAttributeCount(const SQuadrionVertexDescriptor& desc);
	
		bool							CreateVertexFormat(const SQuadrionVertexDescriptor& desc, const bool& stream, const int& texOffset = -1);
		bool							FillVertexFormat(const SQuadrionVertexDescriptor& desc, const unsigned int& nAttribs, const bool& stream, 
													     D3DVERTEXELEMENT9* pVertexElements, const int& texOffset = -1);
		bool							FillVertexBufferNoStream(const SQuadrionVertexDescriptor& desc, const int& nVerts, const void* pVertices);
		bool							FillVertexBufferStream(const SQuadrionVertexDescriptor& desc, const int& nVerts, const void* pVertices);
		
		
			
		unsigned int					m_vertexSize[QVERTEXBUFFER_MAXSTREAMS];			// Vertex stream size in bytes
		unsigned int					m_memoryPool;									// Memory pool location
		unsigned int					m_nVertices;									// Number of vertices in this vertex buffer
		bool							m_bStreamed;									// Is this vertex buffer streamed?
		bool							m_bActiveStreams[QVERTEXBUFFER_MAXSTREAMS];		// Active stream table
		bool							m_bIsInstancing;
		bool							m_bHasInstanceBuffer;
		bool							m_bIsInstanceBuffer;
		
		LPDIRECT3DVERTEXDECLARATION9	m_pVertexDeclaration;		// Handle to vertex buffer
		LPDIRECT3DVERTEXBUFFER9			m_pVertexBuffer0;			// Buffer Stream 0
		LPDIRECT3DVERTEXBUFFER9			m_pVertexBuffer1;			// Buffer Stream 1
		LPDIRECT3DVERTEXBUFFER9			m_pVertexBuffer2;			// Buffer Stream 2
		LPDIRECT3DVERTEXBUFFER9			m_pVertexBuffer3;			// Buffer Stream 4
		LPDIRECT3DDEVICE9				m_pRenderDevice;	
		
		EQuadrionVertexAttribUsage		m_stream0[16];				// Custom Vertex Stream 0
		EQuadrionVertexAttribUsage		m_stream1[16];				// Custom Vertex Stream 1
		EQuadrionVertexAttribUsage		m_stream2[16];				// Custom Vertex Stream 2
		EQuadrionVertexAttribUsage		m_stream3[16];				// Custom Vertex Stream 3
		
		EQuadrionVertexAttribSize		m_streamSize0[16];			
		EQuadrionVertexAttribSize		m_streamSize1[16];			
		EQuadrionVertexAttribSize		m_streamSize2[16];			
		EQuadrionVertexAttribSize		m_streamSize3[16];			
};	



class QVERTEXBUFFEREXPORT_API CQuadrionInstancedVertexBuffer : public CQuadrionResource
{
	public:
	
		CQuadrionInstancedVertexBuffer(const unsigned int handle, const std::string& name, const std::string& path = "./");
		CQuadrionInstancedVertexBuffer(const void* pRender, const unsigned int handle, const std::string& name, const std::string& path = "./");
		~CQuadrionInstancedVertexBuffer();
	
		
		bool		CreateGeometryBuffer(const void* pVertices, const SQuadrionVertexDescriptor& desc, const int& nVerts);
		bool		CreateInstanceBuffer(const void* pVertices, const int& nInstances);
		bool		UpdateInstanceBuffer(const void* pVertices, const int& nInstances);
		
		bool		BindBuffer(const unsigned int& nInstances);
		bool		UnbindBuffer();
		
		void		ChangeRenderDevice(const void* pRender);
	
	protected:
	
	private:
	
		friend class			CQuadrionRender;
	
		LPDIRECT3DDEVICE9				m_pRenderDevice;
		LPDIRECT3DVERTEXDECLARATION9	m_pVertexDeclaration;
		LPDIRECT3DVERTEXBUFFER9			m_pGeomBuffer;
		LPDIRECT3DVERTEXBUFFER9			m_pInstanceBuffer;	
		
		unsigned int					m_vertexSize;
		unsigned int					m_nVertices;
};


#endif