#include "qindexbuffer.h"
#include "qrender.h"


CQuadrionIndexBuffer::CQuadrionIndexBuffer(const unsigned int handle, const std::string& name, const std::string& path) : CQuadrionResource(handle, name, path)
{
	m_pIndexBuffer = NULL;	
	m_bufferSize = 0;
	m_nIndices = 0;
}


CQuadrionIndexBuffer::CQuadrionIndexBuffer(const void* pRender, const unsigned int handle, const std::string& name, const std::string& path) : CQuadrionResource(handle, name, path)
{
	CQuadrionRender* p = (CQuadrionRender*)pRender;
	m_pRenderDevice = p->m_pD3DDev;

	m_pIndexBuffer = NULL;
	m_bufferSize = 0;
	m_nIndices = 0;
}


CQuadrionIndexBuffer::~CQuadrionIndexBuffer()
{
	DestroyIndexBuffer();
}


void CQuadrionIndexBuffer::DestroyIndexBuffer()
{
	if(m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = NULL;
	}
	
	m_bufferSize = 0;
	m_nIndices = 0;
}


void CQuadrionIndexBuffer::ChangeRenderDevice(const void* pRender)
{
	CQuadrionRender* p = (CQuadrionRender*)pRender;
	m_pRenderDevice = p->m_pD3DDev;
}


bool CQuadrionIndexBuffer::CreateIndexBuffer(const unsigned int& memFlags, const unsigned int& sizeFlags, const unsigned int& nIndices, const void* buf)
{
	unsigned int size, usage, bufSize;
	D3DFORMAT format;
	(sizeFlags & QINDEXBUFFER_SIZE_USHORT) ? size = 2 : size = 4;
	(size == 2) ? format = D3DFMT_INDEX16 : format = D3DFMT_INDEX32;
	(memFlags & QINDEXBUFFER_MEMORY_STREAM || memFlags & QINDEXBUFFER_MEMORY_DYNAMIC) ? usage = D3DUSAGE_DYNAMIC : usage = D3DUSAGE_WRITEONLY;
	bufSize = size * nIndices;
	
	bool isDynamic = (memFlags & QINDEXBUFFER_MEMORY_DYNAMIC || memFlags & QINDEXBUFFER_MEMORY_STREAM) != 0;
	
	if(FAILED(m_pRenderDevice->CreateIndexBuffer(bufSize, usage, format, D3DPOOL_DEFAULT, &m_pIndexBuffer, NULL)))
		return false;
	
	if(buf)
	{
		void* dest;
		if(SUCCEEDED(m_pIndexBuffer->Lock(0, bufSize, &dest, isDynamic ? D3DLOCK_DISCARD : 0)))
		{
			memcpy(dest, buf, bufSize);
			if(FAILED(m_pIndexBuffer->Unlock()))
				return false;
		}
		
		else 
			return false;
	}
	
	m_nIndices = nIndices;
	m_bufferSize = bufSize;
	
	return true;
}


bool CQuadrionIndexBuffer::BindBuffer()
{
	if(FAILED(m_pRenderDevice->SetIndices(m_pIndexBuffer)))
		return false;
	
	return true;
}

bool CQuadrionIndexBuffer::UnbindBuffer()
{
	if(FAILED(m_pRenderDevice->SetIndices(NULL)))
		return false;
	
	return true;
}

bool CQuadrionIndexBuffer::UpdateBufferData(const void* buf)
{
	void* dat;
	if(SUCCEEDED(m_pIndexBuffer->Lock(0, m_bufferSize, &dat, 0)))
	{
		memcpy(dat, buf, m_bufferSize);
		
		if(FAILED(m_pIndexBuffer->Unlock()))
			return false;
	}
	
	else 
		return false;
	
	return true;
}