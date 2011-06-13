/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// QINDEXBUFFER.H
//
// Author: SMS
// Date: 01/2009
//
// This file contains the new vertex buffer object interface code for the new renderer.
// Index Buffer Objects (or IBOs) allow the programmer to batch upload arrays
// of indices into specific memory locations for access in reading or writing.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////



#include "qresource.h"


#ifndef __QINDEXBUFFER_H_
#define __QINDEXBUFFER_H_



#ifdef QRENDER_EXPORTS
	#define QINDEXBUFFEREXPORT_API     __declspec(dllexport)
#else
	#define QINDEXBUFFEREXPORT_API	   __declspec(dllimport)
#endif

#include <d3d9.h>



const unsigned int		QINDEXBUFFER_MEMORY_DYNAMIC			= 0x00010000;
const unsigned int		QINDEXBUFFER_MEMORY_STREAM			= 0x00020000;
const unsigned int		QINDEXBUFFER_MEMORY_STATIC			= 0x00040000;

const unsigned int		QINDEXBUFFER_SIZE_UINT				= 0x00000001;
const unsigned int		QINDEXBUFFER_SIZE_USHORT			= 0x00000002;



class QINDEXBUFFEREXPORT_API CQuadrionIndexBuffer : public CQuadrionResource
{
	public:
	
		CQuadrionIndexBuffer(const unsigned int handle, const std::string& name, const std::string& path = "./");
		CQuadrionIndexBuffer(const void* pRender, const unsigned int handle, const std::string& name, const std::string& path = "./");
		~CQuadrionIndexBuffer();
		
		// CreateIndexBuffer -- Creates buffer of USHORT or unsigned int indices and upload it into local or video memory
		// Params:
		// memFlags: One of QINDEXBUFFER_MEMORY_STATIC, QINDEXBUFFER_MEMORY_DYNAMIC, or QINDEXBUFFER_MEMORY_STREAM
		// sizeFlags: One of QINDEXBUFFER_SIZE_unsigned int or QINDEXBUFFER_SIZE_USHORT
		// nIndices: number of indices in "buf"
		// buf: Buffer of index data to upload	
		bool		CreateIndexBuffer(const unsigned int& memFlags, const unsigned int& sizeFlags, const unsigned int& nIndices, const void* buf);
		
		// BindBuffer -- Binds this index buffer as the current IBO. If there is an IBO bound, a VBO MUST be bound for rendering to succeed
		bool		BindBuffer();
		
		// UnbindBuffer -- Unbinds this index buffer as the current IBO
		bool		UnbindBuffer();
		
		// UpdateBufferData -- Upload and update this index buffer with new data. "buf" must be allocated with the
		//					   exact number of bytes as the index buffer. The size of the index buffer in bytes can be
		//					   obtained with GetBufferSize 
		// Params:
		// buf: buffer of new index data to upload
		bool		UpdateBufferData(const void* buf);
		
		
		// GetBufferSize -- Obtains this IBOs size in bytes
		const inline int		GetBufferSize() { return m_bufferSize; }
	
	protected:
	
		void		DestroyIndexBuffer();
	
		void		ChangeRenderDevice(const void* pRender);
	
	private:
	
		friend class					CQuadrionRender;
		
		unsigned int							m_bufferSize;				// Buffer size in bytes
		unsigned int							m_nIndices;					// Number of indices in buffer
	
		LPDIRECT3DDEVICE9				m_pRenderDevice;			// Pointer to current render device
		LPDIRECT3DINDEXBUFFER9			m_pIndexBuffer;				// Pointer to IBO handle
};

#endif