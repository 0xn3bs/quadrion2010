#include "qvertexbuffer.h"
#include "qrender.h"


static D3DDECLUSAGE g_vertexFormatUsages[] = 
{
	D3DDECLUSAGE_POSITION,
	D3DDECLUSAGE_NORMAL,
	D3DDECLUSAGE_BINORMAL,
	D3DDECLUSAGE_TANGENT,
	D3DDECLUSAGE_TEXCOORD,
	D3DDECLUSAGE_COLOR,
	D3DDECLUSAGE_BLENDWEIGHT,
	D3DDECLUSAGE_BLENDINDICES,
	D3DDECLUSAGE_FOG,
	D3DDECLUSAGE(0), 
	D3DDECLUSAGE(-1),
};

static D3DDECLTYPE g_vertexFormatTypes[] = 
{
	D3DDECLTYPE_FLOAT1,
	D3DDECLTYPE_FLOAT2,
	D3DDECLTYPE_FLOAT3,
	D3DDECLTYPE_FLOAT4,
	D3DDECLTYPE_D3DCOLOR,
};

static unsigned int g_vertexFormatSizes[] = 
{
	4,
	8,
	12,
	16,
	4,
};


static void SwapVertexDeclarations(D3DVERTEXELEMENT9& l, D3DVERTEXELEMENT9& r)
{
	D3DVERTEXELEMENT9 tmp;
	memcpy(&tmp, &l, sizeof(D3DVERTEXELEMENT9));
	memcpy(&l, &r, sizeof(D3DVERTEXELEMENT9));
	memcpy(&r, &tmp, sizeof(D3DVERTEXELEMENT9));
}


static int VertexDecCompare(const void* a, const void* b)
{
	D3DVERTEXELEMENT9* l = (D3DVERTEXELEMENT9*)a;
	D3DVERTEXELEMENT9* r = (D3DVERTEXELEMENT9*)b;
	
	int ret;
	(l->Stream < r->Stream) ? ret = -1 : (l->Stream > r->Stream) ? ret = 1 : ret = 0;
	return ret;
}


static unsigned int	GetVertexAttributeCount(const SQuadrionVertexDescriptor& desc)
{
	unsigned int p = 0;
	while(!(desc.usage[p] == QVERTEXFORMAT_USAGE_END))
	{
		++p;
	} 
	
	return p;
}

static unsigned int GetVertexAttributeOffset(const SQuadrionVertexDescriptor& desc, const EQuadrionVertexAttribUsage& attrib, const unsigned int& usageIndex)
{
	unsigned int p = 0;
	unsigned int idx = 0;
	unsigned int tmp = 0;
	while(!(desc.usage[idx] == QVERTEXFORMAT_USAGE_END))
	{
		if(!(desc.usage[idx] == attrib))
			p += g_vertexFormatSizes[desc.size[idx]];
		
		else
		{
			if((attrib == QVERTEXFORMAT_USAGE_TEXCOORD) && tmp == usageIndex)
				return p;
			
			else if((attrib == QVERTEXFORMAT_USAGE_TEXCOORD) && tmp != usageIndex)
			{
				++tmp;
				++idx;
				p += g_vertexFormatSizes[desc.size[idx]];
				continue;
			}
			
			else
				return p;
		}
		
		++idx;
	}	
}


static unsigned int GetVertexDescriptorSize(const SQuadrionVertexDescriptor& desc)
{
	unsigned int p = 0;
	unsigned int size = 0;
	while(!(desc.usage[p] == QVERTEXFORMAT_USAGE_END))
	{
		size += g_vertexFormatSizes[desc.size[p]];
		++p;
	}
	
	return size;
}


CQuadrionVertexBuffer::CQuadrionVertexBuffer(const unsigned int handle, const std::string& name, const std::string& path) : CQuadrionResource(handle, name, path)
{
	m_pRenderDevice = NULL;
	m_pVertexDeclaration = NULL;
	m_pVertexBuffer0 = NULL;
	m_pVertexBuffer1 = NULL;
	m_pVertexBuffer2 = NULL;
	m_pVertexBuffer3 = NULL;
	
	m_memoryPool = 0;
	m_nVertices = 0;
	m_bStreamed = false;
	m_bIsInstancing = false;
	m_bHasInstanceBuffer = false;
	m_bIsInstanceBuffer = false;
	memset(m_bActiveStreams, 0, sizeof(bool) * QVERTEXBUFFER_MAXSTREAMS);
	
	m_stream0[0] = QVERTEXFORMAT_USAGE_POSITION;
	m_stream0[1] = QVERTEXFORMAT_USAGE_BLENDWEIGHT;
	m_stream0[2] = QVERTEXFORMAT_USAGE_INDEXWEIGHT;
	m_stream0[3] = QVERTEXFORMAT_USAGE_END;
	
	m_stream1[0] = QVERTEXFORMAT_USAGE_TEXCOORD;
	m_stream1[1] = QVERTEXFORMAT_USAGE_END;
	
	m_stream2[0] = QVERTEXFORMAT_USAGE_NORMAL;
	m_stream2[1] = QVERTEXFORMAT_USAGE_BINORMAL;
	m_stream2[2] = QVERTEXFORMAT_USAGE_TANGENT;
	m_stream2[3] = QVERTEXFORMAT_USAGE_END;
	
	m_stream3[0] = QVERTEXFORMAT_USAGE_COLOR;
	m_stream3[1] = QVERTEXFORMAT_USAGE_END;
	
	memset(m_streamSize0, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize1, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize2, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize3, 0, sizeof(EQuadrionVertexAttribSize) * 16);

	
	for(int i = 0; i < QVERTEXBUFFER_MAXSTREAMS; ++i)
	{
		m_vertexSize[i] = 0;
	}	
}

CQuadrionVertexBuffer::CQuadrionVertexBuffer(const void* pRender, const unsigned int handle, const std::string& name, const std::string& path) : CQuadrionResource(handle, name, path)
{
	CQuadrionRender* p = (CQuadrionRender*)pRender;
	m_pRenderDevice = p->m_pD3DDev;
	
	m_pVertexDeclaration = NULL;
	m_pVertexBuffer0 = NULL;
	m_pVertexBuffer1 = NULL;
	m_pVertexBuffer2 = NULL;
	m_pVertexBuffer3 = NULL;
	
	m_memoryPool = 0;
	m_nVertices = 0;
	m_bStreamed = false;
	m_bIsInstancing = false;
	m_bHasInstanceBuffer= false;
	m_bIsInstanceBuffer = false;
	memset(m_bActiveStreams, 0, sizeof(bool) * QVERTEXBUFFER_MAXSTREAMS);
	
	m_stream0[0] = QVERTEXFORMAT_USAGE_POSITION;
	m_stream0[1] = QVERTEXFORMAT_USAGE_BLENDWEIGHT;
	m_stream0[2] = QVERTEXFORMAT_USAGE_INDEXWEIGHT;
	m_stream0[3] = QVERTEXFORMAT_USAGE_END;
	
	m_stream1[0] = QVERTEXFORMAT_USAGE_TEXCOORD;
	m_stream1[1] = QVERTEXFORMAT_USAGE_END;
	
	m_stream2[0] = QVERTEXFORMAT_USAGE_NORMAL;
	m_stream2[1] = QVERTEXFORMAT_USAGE_BINORMAL;
	m_stream2[2] = QVERTEXFORMAT_USAGE_TANGENT;
	m_stream2[3] = QVERTEXFORMAT_USAGE_END;
	
	m_stream3[0] = QVERTEXFORMAT_USAGE_COLOR;
	m_stream3[1] = QVERTEXFORMAT_USAGE_END;
	
	memset(m_streamSize0, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize1, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize2, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize3, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	
	
	for(int i = 0; i < QVERTEXBUFFER_MAXSTREAMS; ++i)
	{
		m_vertexSize[i] = 0;
	}	
}





CQuadrionVertexBuffer::~CQuadrionVertexBuffer()
{
	DestroyVertexBuffer();	
}


void CQuadrionVertexBuffer::DestroyVertexBuffer()
{
	if(m_pVertexDeclaration)
	{
		m_pVertexDeclaration->Release();
		m_pVertexDeclaration = NULL;
	}
	
	if(m_pVertexBuffer0)
	{
		m_pVertexBuffer0->Release();
		m_pVertexBuffer0 = NULL;
	}
	
	if(m_pVertexBuffer1)
	{
		m_pVertexBuffer1->Release();
		m_pVertexBuffer1 = NULL;
	}
	
	if(m_pVertexBuffer2)
	{
		m_pVertexBuffer2->Release();
		m_pVertexBuffer2 = NULL;
	}
	
	if(m_pVertexBuffer3)
	{
		m_pVertexBuffer3->Release();
		m_pVertexBuffer3 = NULL;
	}
	
	memset(m_stream0, 0, sizeof(EQuadrionVertexAttribUsage) * 16);
	memset(m_stream1, 0, sizeof(EQuadrionVertexAttribUsage) * 16);
	memset(m_stream2, 0, sizeof(EQuadrionVertexAttribUsage) * 16);
	memset(m_stream3, 0, sizeof(EQuadrionVertexAttribUsage) * 16);
	
	memset(m_streamSize0, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize1, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize2, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize3, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	
	
	m_memoryPool = 0;
	m_nVertices = 0;
	m_bStreamed = false;
	m_bIsInstancing = false;
	m_bHasInstanceBuffer = false;
	m_bIsInstanceBuffer = false;
	memset(m_bActiveStreams, 0, sizeof(bool) * QVERTEXBUFFER_MAXSTREAMS);
}


bool CQuadrionVertexBuffer::FillVertexBufferNoStream(const SQuadrionVertexDescriptor& desc, const int& nVerts, const void* pVertices)
{
	unsigned int usage;
	D3DPOOL mem_pool;
	bool isDynamic = (desc.pool & QVERTEXBUFFER_MEMORY_DYNAMIC || desc.pool & QVERTEXBUFFER_MEMORY_STREAM) != 0;
	(isDynamic == true) ? usage = D3DUSAGE_DYNAMIC : usage = D3DUSAGE_WRITEONLY;
	mem_pool = D3DPOOL_DEFAULT;
	
	if(desc.pool & QVERTEXBUFFER_MEMORY_SYSTEM)
	{
		usage = 0;
		mem_pool = D3DPOOL_MANAGED;
		isDynamic = false;
	}
	
	unsigned int size = nVerts * m_vertexSize[0];
	if(FAILED(m_pRenderDevice->CreateVertexBuffer(size, usage, 0, mem_pool, &m_pVertexBuffer0, NULL)))
		return false;
	
	if(pVertices)
	{
		void* dest;
		if(SUCCEEDED(m_pVertexBuffer0->Lock(0, size, &dest, isDynamic ? D3DLOCK_DISCARD : 0)))
		{
			memcpy(dest, pVertices, size);
			m_pVertexBuffer0->Unlock();
		}
		
		else 
			return false;
	}
	
	else
		return false;

	return true;
}

bool CQuadrionVertexBuffer::FillVertexBufferStream(const SQuadrionVertexDescriptor& desc, const int& nVerts, const void* pVertices)
{
	unsigned int usage;
	D3DPOOL mem_pool;
	bool isDynamic = (desc.pool & QVERTEXBUFFER_MEMORY_DYNAMIC || desc.pool & QVERTEXBUFFER_MEMORY_STREAM) != 0;
	(isDynamic == true) ? usage = D3DUSAGE_DYNAMIC : usage = D3DUSAGE_WRITEONLY;
	if(desc.pool & QVERTEXBUFFER_MEMORY_DYNAMIC)
		usage |= D3DUSAGE_WRITEONLY;
	mem_pool = D3DPOOL_DEFAULT;
	
	if(desc.pool * QVERTEXBUFFER_MEMORY_SYSTEM)
	{
		usage = 0;
		mem_pool = D3DPOOL_MANAGED;
		isDynamic = false;
	}
	
	void* buf;
	unsigned int elementOffset = 0;
	unsigned int bufOffset = 0;
	unsigned int nAttribs = GetVertexAttributeCount(desc);
	unsigned int vertexSize = GetVertexDescriptorSize(desc);
	unsigned int nTexCoords = 0;
	for(int i = 0; i < QVERTEXBUFFER_MAXSTREAMS; ++i)
	{
		bool cont = false;
		bool populated = false;
		if(m_vertexSize[i] > 0)
		{
			buf = malloc(m_vertexSize[i] * nVerts);
			if(!buf)
				return false;
		}
		
		else
			continue;
		
		for(int j = 0; j < nAttribs; ++j)
		{
			if(GetStream(desc.usage[j]) == i)
			{
				elementOffset = GetVertexAttributeOffset(desc, desc.usage[j], nTexCoords);
				for(int k = 0; k < nVerts; ++k)
				{
					// Satisfy linker by casting to known size (char*) for byte offset //
					memcpy((char*)buf + (k * m_vertexSize[i] + bufOffset), (char*)pVertices + (k * vertexSize + elementOffset), g_vertexFormatSizes[desc.size[j]]);
				}

				if(desc.usage[j] == QVERTEXFORMAT_USAGE_TEXCOORD)
					++nTexCoords;
				
				bufOffset += g_vertexFormatSizes[desc.size[j]];
				populated = true;
			}
			
			else
			{
				if(j + 1 >= nAttribs && !populated)
					cont = true;
			}
		}
		
		if(cont)
		{
			free(buf);
			buf = NULL;
			continue;
		}
		
		switch(i)
		{
			case 0:
			{
				if(FAILED(m_pRenderDevice->CreateVertexBuffer(m_vertexSize[i] * nVerts, usage, 0, mem_pool, &m_pVertexBuffer0, NULL)))
					return false;
				void* dest;
				if(SUCCEEDED(m_pVertexBuffer0->Lock(0, m_vertexSize[i] * nVerts, &dest, isDynamic ? D3DLOCK_DISCARD : 0)))
				{
					memcpy(dest, buf, m_vertexSize[i] * nVerts);
					m_pVertexBuffer0->Unlock();
				}
				
				else
					return false;
				
				break;
			}
			
			case 1:
			{
				if(FAILED(m_pRenderDevice->CreateVertexBuffer(m_vertexSize[i] * nVerts, usage, 0, mem_pool, &m_pVertexBuffer1, NULL)))
					return false;
				void* dest;
				if(SUCCEEDED(m_pVertexBuffer1->Lock(0, m_vertexSize[i] * nVerts, &dest, isDynamic ? D3DLOCK_DISCARD : 0)))
				{
					memcpy(dest, buf, m_vertexSize[i] * nVerts);
					m_pVertexBuffer1->Unlock();
				}
				
				else
					return false;
				
				break;
			}
			
			case 2:
			{
				if(FAILED(m_pRenderDevice->CreateVertexBuffer(m_vertexSize[i] * nVerts, usage, 0, mem_pool, &m_pVertexBuffer2, NULL)))
					return false;
				void* dest;
				if(SUCCEEDED(m_pVertexBuffer2->Lock(0, m_vertexSize[i] * nVerts, &dest, isDynamic ? D3DLOCK_DISCARD : 0)))
				{
					memcpy(dest, buf, m_vertexSize[i] * nVerts);
					m_pVertexBuffer2->Unlock();
				}
				
				else
					return false;
				
				break;
			}
			
			case 3:
			{
				if(FAILED(m_pRenderDevice->CreateVertexBuffer(m_vertexSize[i] * nVerts, usage, 0, mem_pool, &m_pVertexBuffer3, NULL)))
					return false;
				void* dest;
				if(SUCCEEDED(m_pVertexBuffer3->Lock(0, m_vertexSize[i] * nVerts, &dest, isDynamic ? D3DLOCK_DISCARD : 0)))
				{
					memcpy(dest, buf, m_vertexSize[i] * nVerts);
					m_pVertexBuffer3->Unlock();
				}
				
				else
					return false;
				
				break;
			}
		}
		
		free(buf);
		buf = NULL;
		
		bufOffset = 0;
		nTexCoords = 0;
	}

	return true;
}


bool CQuadrionVertexBuffer::CreateVertexBuffer(const void* pVertices, const SQuadrionVertexDescriptor& desc, const int& nVerts, const bool& stream, const int& texOffset)
{
	if(!CreateVertexFormat(desc, stream, texOffset))
		return false;
	
	if(!stream)
	{
		if(!FillVertexBufferNoStream(desc, nVerts, pVertices))
			return false;
	}
	
	else
	{
		if(!FillVertexBufferStream(desc, nVerts, pVertices))
			return false;
	}
	
	m_memoryPool = desc.pool;
	m_nVertices = nVerts;
	m_bStreamed = stream;
	return true;	
}

bool CQuadrionVertexBuffer::CreateVertexFormat(const SQuadrionVertexDescriptor& desc, const bool& stream, const int& texOffset)
{
//	unsigned int nAttribs = GetVertexAttributeCount(desc);
	unsigned int nAttribs = GetFinalVertexAttributeCount(desc);
	if(nAttribs <= 0)
		return false;
	
	D3DVERTEXELEMENT9* pVertexElements = new D3DVERTEXELEMENT9[nAttribs + 1];
	nAttribs = GetVertexAttributeCount(desc);
	
	FillVertexFormat(desc, nAttribs, stream, pVertexElements, texOffset);
	
	if(stream && texOffset == -1)
		qsort(pVertexElements, nAttribs + 1, sizeof(D3DVERTEXELEMENT9), VertexDecCompare);
	
	if(FAILED(m_pRenderDevice->CreateVertexDeclaration(pVertexElements, &m_pVertexDeclaration)))
	{
		delete[] pVertexElements;
		pVertexElements = NULL;
		return false;
	}
	
	delete[] pVertexElements;
	pVertexElements = NULL;
		
	return true;
}


bool CQuadrionVertexBuffer::FillVertexFormat(const SQuadrionVertexDescriptor& desc, const unsigned int& nAttribs, const bool& stream, D3DVERTEXELEMENT9* pVertexElements,	
											 const int& texOffset)
{
	if(!pVertexElements)
		return false;


	
	unsigned int nTexCoords = (texOffset > 0) ? texOffset : 0;
	unsigned int vp = 0;
	int s0, s1, s2, s3;
	s0 = s1 = s2 = s3 = 0;
	int i;
	for(i = 0; i < nAttribs; ++i)
	{
		if(stream)
		{
			int vertexstream = GetStream(desc.usage[i]);
		
			if(vertexstream == -1)
				continue;
			else
				pVertexElements[vp].Stream = vertexstream;
		}
		else
			pVertexElements[vp].Stream = 0;
	
		pVertexElements[vp].Type = (D3DDECLTYPE)g_vertexFormatTypes[desc.size[i]];
		pVertexElements[vp].Method = D3DDECLMETHOD_DEFAULT;
		pVertexElements[vp].Usage = (D3DDECLUSAGE)g_vertexFormatUsages[desc.usage[i]];
		pVertexElements[vp].Offset = m_vertexSize[pVertexElements[vp].Stream];
		
		if(desc.usage[i] == QVERTEXFORMAT_USAGE_TEXCOORD)
			pVertexElements[vp].UsageIndex = nTexCoords++;
		else
			pVertexElements[vp].UsageIndex = 0;
		
		
		switch(pVertexElements[i].Stream)
		{
			case 0:
				m_streamSize0[s0] = desc.size[i];
				m_stream0[s0] = desc.usage[i];
				++s0;
				break;
			
			case 1:
				m_streamSize1[s1] = desc.size[i];
				m_stream1[s1] = desc.usage[i];
				++s1;
				break;
			
			case 2:
				m_streamSize2[s2] = desc.size[i];
				m_stream2[s2] = desc.usage[i];
				++s2;
				break;
			
			case 3:
				m_streamSize3[s3] = desc.size[i];
				m_stream3[s3] = desc.usage[i];
				++s3;
				break;
		}
		
		m_vertexSize[pVertexElements[vp].Stream] += g_vertexFormatSizes[desc.size[i]];
		++vp;
	}
	
	pVertexElements[vp].Method = 0;
	pVertexElements[vp].Offset = 0;
	pVertexElements[vp].Usage = 0;
	pVertexElements[vp].UsageIndex = 0;
	pVertexElements[vp].Stream = 0xFF;
	pVertexElements[vp].Type = D3DDECLTYPE_UNUSED;
	
	
	m_stream0[s0] = QVERTEXFORMAT_USAGE_END;
	m_stream1[s1] = QVERTEXFORMAT_USAGE_END;
	m_stream2[s2] = QVERTEXFORMAT_USAGE_END;
	m_stream3[s3] = QVERTEXFORMAT_USAGE_END;
	
	return true;
}


void CQuadrionVertexBuffer::ChangeRenderDevice(const void* pRender)
{
	CQuadrionRender* p = (CQuadrionRender*)pRender;
	m_pRenderDevice = p->m_pD3DDev;	
}

int CQuadrionVertexBuffer::GetStream(const EQuadrionVertexAttribUsage& attrib)
{
	unsigned int p = 0;
	for(int i = 0; i < 4; ++i)
	{
		if(i == 0)
		{
			while(!(m_stream0[p] == QVERTEXFORMAT_USAGE_END))
			{
				if(m_stream0[p] == attrib)
					return i;
				++p;
			}
		}
		
		else if(i == 1)
		{
			while(!(m_stream1[p] == QVERTEXFORMAT_USAGE_END))
			{
				if(m_stream1[p] == attrib)
					return i;
				++p;
			}
		}
		
		else if(i == 2)
		{
			while(!(m_stream2[p] == QVERTEXFORMAT_USAGE_END))
			{
				if(m_stream2[p] == attrib)
					return i;
				++p;
			}
		}
		
		else
		{
			while(!(m_stream3[p] == QVERTEXFORMAT_USAGE_END))
			{
				if(m_stream3[p] == attrib)
					return i;
				++p;
			}
		}
		
		p = 0;
	}
	
	return -1;
}


unsigned int CQuadrionVertexBuffer::GetVertexStreamSize(const SQuadrionVertexDescriptor& desc, const int& stream)
{
	unsigned int p = 0;
	unsigned int size = 0;
	while(!(desc.usage[p] == QVERTEXFORMAT_USAGE_END))
	{
		if(GetStream(desc.usage[p]) == stream)
			size += g_vertexFormatSizes[desc.size[p]];
		
		++p;
	}
	
	return size;
}

unsigned int CQuadrionVertexBuffer::GetFinalVertexAttributeCount(const SQuadrionVertexDescriptor& desc)
{
	unsigned int p = 0;
	unsigned int sptr = 0;
	unsigned int count = 0;
	while(!(desc.usage[p] == QVERTEXFORMAT_USAGE_END))
	{
		for(int i = 0; i < 4; ++i)
		{
			switch(i)
			{
				case 0:
				{
					while(!(m_stream0[sptr] == QVERTEXFORMAT_USAGE_END))
					{
						if(desc.usage[p] == m_stream0[sptr])
							++count;
						++sptr;
					}
					
					sptr = 0;
					break;
				}
				
				
				case 1:
				{
					while(!(m_stream1[sptr] == QVERTEXFORMAT_USAGE_END))
					{
						if(desc.usage[p] == m_stream1[sptr])
							++count;
						++sptr;
					}
					
					sptr = 0;
					break;
				}
				
				case 2:
				{
					while(!(m_stream2[sptr] == QVERTEXFORMAT_USAGE_END))
					{
						if(desc.usage[p] == m_stream2[sptr])
							++count;
						++sptr;
					}
					
					sptr = 0;
					break;
				}
				
				case 3:
				{
					while(!(m_stream3[sptr] == QVERTEXFORMAT_USAGE_END))
					{
						if(desc.usage[p] == m_stream3[sptr])
							++count;
						++sptr;
					}
					
					sptr = 0;
					break;
				}
			}
		}
		
		++p;
	} 
	
	return count;	
}

bool CQuadrionVertexBuffer::SetVertexStreams(const EQuadrionVertexAttribUsage* stream0,
										     const EQuadrionVertexAttribUsage* stream1,
										     const EQuadrionVertexAttribUsage* stream2,
										     const EQuadrionVertexAttribUsage* stream3)
{
	unsigned int p = 0;
	
	memset(m_stream0, 0, sizeof(EQuadrionVertexAttribUsage) * 16);
	memset(m_stream1, 0, sizeof(EQuadrionVertexAttribUsage) * 16);
	memset(m_stream2, 0, sizeof(EQuadrionVertexAttribUsage) * 16);
	memset(m_stream3, 0, sizeof(EQuadrionVertexAttribUsage) * 16);
	
	memset(m_streamSize0, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize1, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize2, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	memset(m_streamSize3, 0, sizeof(EQuadrionVertexAttribSize) * 16);
	
	
	for(int i = 0; i < QVERTEXBUFFER_MAXSTREAMS; ++i)
	{
		m_vertexSize[i] = 0;
	}
	
	if(m_pVertexDeclaration)
	{
		m_pVertexDeclaration->Release();
		m_pVertexDeclaration = NULL;
	}

	if(m_vertexSize[0] <= 0 && m_vertexSize[1] <= 0 && m_vertexSize[2] <= 0 && m_vertexSize[3] <= 0)
	{
		if(stream0)
		{
			while(stream0[p] != QVERTEXFORMAT_USAGE_END && p < 16)
			{
				m_stream0[p] = stream0[p];
				++p;
			}
		}
		m_stream0[p] = QVERTEXFORMAT_USAGE_END;
			
		p = 0;
			
		if(stream1)
		{
			while(stream1[p] != QVERTEXFORMAT_USAGE_END && p < 16)
			{
				m_stream1[p] = stream1[p];
				++p;
			}
		}
		m_stream1[p] = QVERTEXFORMAT_USAGE_END;
		
		p = 0;
		
		if(stream2)
		{
			while(stream2[p] != QVERTEXFORMAT_USAGE_END && p < 16)
			{
				m_stream2[p] = stream2[p];
				++p;
			}
		}
		m_stream2[p] = QVERTEXFORMAT_USAGE_END;
			
		p = 0;
			
		if(stream3)
		{
			while(stream3[p] != QVERTEXFORMAT_USAGE_END && p < 16)
			{
				m_stream3[p] = stream3[p];
				++p;
			}
		}
		m_stream3[p] = QVERTEXFORMAT_USAGE_END;
		
		return true;
	}
	
	return false;
}


bool CQuadrionVertexBuffer::BindBuffer(const unsigned int* streams, const int& nStreams, const int& nInstances)
{
	if(FAILED(m_pRenderDevice->SetVertexDeclaration(m_pVertexDeclaration)))
		return false;
	
	if(nInstances < 0)
	{	
		for(unsigned int i = 0; i < nStreams; ++i)
		{
			if(m_vertexSize[streams[i]] > 0)
			{
				switch(streams[i])
				{
					case 0:
						if(FAILED(m_pRenderDevice->SetStreamSource(0, m_pVertexBuffer0, 0, m_vertexSize[0])))
							return false;
						m_bActiveStreams[0] = true;
						break;
					
					case 1:
						if(FAILED(m_pRenderDevice->SetStreamSource(1, m_pVertexBuffer1, 0, m_vertexSize[1])))
							return false;
						m_bActiveStreams[1] = true;
						break;
					
					case 2:
						if(FAILED(m_pRenderDevice->SetStreamSource(2, m_pVertexBuffer2, 0, m_vertexSize[2])))
							return false;
						m_bActiveStreams[2] = true;
						break;
					
					case 3:
						if(FAILED(m_pRenderDevice->SetStreamSource(3, m_pVertexBuffer3, 0, m_vertexSize[3])))
							return false;
						m_bActiveStreams[3] = true;
						break;
					
					default:
						break;	
				}
			}
		}
	}
	
	else if(m_bHasInstanceBuffer && nInstances > 0)
	{
		if(m_vertexSize[0] <= 0 || m_vertexSize[1] <= 0)
			return false;
		
		// Bind Geometry Buffer //
		unsigned int num_instances = nInstances;
		if(FAILED(m_pRenderDevice->SetStreamSource(0, m_pVertexBuffer0, 0, m_vertexSize[0])))
			return false;
		if(FAILED(m_pRenderDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | num_instances))))
			return false;

		
		// Bind Instance Buffer //
		if(FAILED(m_pRenderDevice->SetStreamSource(1, m_pVertexBuffer1, 0, m_vertexSize[1])))
			return false;
		if(FAILED(m_pRenderDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1))))
			return false;
			
		m_bIsInstancing = true;
		m_bActiveStreams[0] = true;
		m_bActiveStreams[1] = true;
	}
	
	else
	{
		if(!m_bIsInstanceBuffer && nInstances > 0)
		{
			if(m_vertexSize[0] <= 0 || m_vertexSize[1] > 0 || m_vertexSize[2] > 0 || m_vertexSize[3] > 0)
				return false;
		
			unsigned int num_instances = nInstances;
			if(FAILED(m_pRenderDevice->SetStreamSource(0, m_pVertexBuffer0, 0, m_vertexSize[0])))
				return false;
			if(FAILED(m_pRenderDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | num_instances))))
				return false;
				
			m_bActiveStreams[0] = true;
		}
		
		else
		{
			if(m_vertexSize[0] > 0 || m_vertexSize[1] <= 0 || m_vertexSize[2] > 0 || m_vertexSize[3] > 0)
				return false;
			
			if(FAILED(m_pRenderDevice->SetStreamSource(1, m_pVertexBuffer1, 0, m_vertexSize[1])))
				return false;
			if(FAILED(m_pRenderDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1))))
				return false;
			
			m_bActiveStreams[1] = true;
		}
		
		m_bIsInstancing = true;
	}
	
	return true;
}


int CQuadrionVertexBuffer::GetBufferSize()
{
	int size = 0;
	for(unsigned int i = 0; i < QVERTEXBUFFER_MAXSTREAMS; ++i)
	{
		if(m_vertexSize[i] > 0)
			size += m_vertexSize[i] * m_nVertices;
	}
	
	return size;
}


int CQuadrionVertexBuffer::GetStreamSize(const unsigned int& stream)
{
	if(stream > QVERTEXBUFFER_MAXSTREAMS)
		return -1;
		
	unsigned int size = 0;
	return m_vertexSize[stream] * m_nVertices;
}


bool CQuadrionVertexBuffer::UpdateBufferData(const void* dat)
{
	if(m_bStreamed)
		return false;
		
	unsigned int sizeToLock = GetBufferSize();
	void* buf;
	
	if(!m_bIsInstanceBuffer)
	{
		
		if(SUCCEEDED(m_pVertexBuffer0->Lock(0, sizeToLock, &buf, 0)))
		{
			memcpy(buf, dat, sizeToLock);
		
			if(FAILED(m_pVertexBuffer0->Unlock()))
				return false;
		}
	
		else 
			return false;
	}
	
	else
	{
		if(SUCCEEDED(m_pVertexBuffer1->Lock(0, sizeToLock, &buf, 0)))
		{
			memcpy(buf, dat, sizeToLock);
			
			if(FAILED(m_pVertexBuffer1->Unlock()))
				return false;
		}
		
		else 
			return false;
	}
	return true;
}

bool CQuadrionVertexBuffer::UpdateBufferData(const void* dat, const unsigned int& bytes)
{
	if(m_bStreamed)
		return false;
		
	unsigned int bufSize = GetBufferSize();
	if(bytes > bufSize || bytes <= 0)
		return false;
	
	void* buf;
	if(!m_bIsInstanceBuffer)
	{
		if(SUCCEEDED(m_pVertexBuffer0->Lock(0, bytes, &buf, 0)))
		{
			memcpy(buf, dat, bytes);
			
			if(FAILED(m_pVertexBuffer0->Unlock()))
				return false;
		}
		
		else
			return false;
	}	
	
	else
		return false;
	
	return true;
}

bool CQuadrionVertexBuffer::UpdateStreamData(const void* dat, const unsigned int& stream)
{
	if(!m_bStreamed)
		return false;
		
	if(stream > QVERTEXBUFFER_MAXSTREAMS)	
		return false;
		
	unsigned int sizeToLock = GetStreamSize(stream);
	void* buf;
	switch(stream)
	{
		case 0:
			if(SUCCEEDED(m_pVertexBuffer0->Lock(0, sizeToLock, &buf, 0)))
			{
				memcpy(buf, dat, sizeToLock);
				
				if(FAILED(m_pVertexBuffer0->Unlock()))
					return false;
			}
			break;
		
		case 1:
			if(SUCCEEDED(m_pVertexBuffer1->Lock(0, sizeToLock, &buf, 0)))
			{
				memcpy(buf, dat, sizeToLock);
				
				if(FAILED(m_pVertexBuffer1->Unlock()))
					return false;
			}
			break;
			
		case 2:
			if(SUCCEEDED(m_pVertexBuffer2->Lock(0, sizeToLock, &buf, 0)))
			{
				memcpy(buf, dat, sizeToLock);
				
				if(FAILED(m_pVertexBuffer2->Unlock()))
					return false;
			}
			break;
			
		case 3:
			if(SUCCEEDED(m_pVertexBuffer3->Lock(0, sizeToLock, &buf, 0)))
			{
				memcpy(buf, dat, sizeToLock);
				
				if(FAILED(m_pVertexBuffer3->Unlock()))
					return false;
			}
			break;
			
		default:
			break;
	}
	
	return true;
}


bool CQuadrionVertexBuffer::CreateInstanceBuffer(mat4* pInstances, const int& nInstances)
{
	// The geometry buffer must not go outside of stream 0 //
	if(m_vertexSize[1] > 0 || m_vertexSize[2] > 0 || m_vertexSize[3] > 0 || nInstances < 0)
		return false;
		
	// If there are more than max instances get out //
	if(nInstances > QVERTEXBUFFER_MAXINSTANCES)
		return false;
		
	int size = m_vertexSize[1] = sizeof(mat4) * nInstances;
	
	// Create instance buffer in system memory //
	if(FAILED(m_pRenderDevice->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &m_pVertexBuffer1, NULL)))
		return false;
	
	// Copy instance data to buffer //
	void* buf;
	if(SUCCEEDED(m_pVertexBuffer1->Lock(0, size, &buf, 0)))
	{
		memcpy(buf, pInstances, size);
		if(FAILED(m_pVertexBuffer1->Unlock()))
			return false;
	}
	
	// Make a copy of existing vertex declaration //
	unsigned int nElements;
	m_pVertexDeclaration->GetDeclaration(NULL, &nElements);
	D3DVERTEXELEMENT9* decl = new D3DVERTEXELEMENT9[nElements + 4];
	m_pVertexDeclaration->GetDeclaration(decl, &nElements);
	unsigned int nTexCoords = 0;
	for(int i = 0; i < nElements; ++i)
	{
		if(decl[i].Usage == D3DDECLUSAGE_TEXCOORD)
			++nTexCoords;
	}
	
	// Pack the rest with instancing information into stream 1 //
	for(int i = nElements - 1; i < nElements + 3; ++i)
	{
		decl[i].Method = D3DDECLMETHOD_DEFAULT;
		decl[i].Offset = 0;
		decl[i].Stream = 1;
		decl[i].Type = D3DDECLTYPE_FLOAT4;
		decl[i].Usage = D3DDECLUSAGE_TEXCOORD;
		decl[i].UsageIndex = ++nTexCoords;
	}
	
	// Set end //
	decl[nElements + 3].Method = 0;
	decl[nElements + 3].Offset = 0;
	decl[nElements + 3].Usage = 0;
	decl[nElements + 3].UsageIndex = 0;
	decl[nElements + 3].Stream = 0xFF;
	decl[nElements + 3].Type = D3DDECLTYPE_UNUSED;
	
	// Release old Vertex declaration //
	m_pVertexDeclaration->Release();
	m_pVertexDeclaration = NULL;
	
	// Re-create new declaration //
	if(FAILED(m_pRenderDevice->CreateVertexDeclaration(decl, &m_pVertexDeclaration)))
		return false;
	
	m_bHasInstanceBuffer = true;
	
	return true;
}

bool CQuadrionVertexBuffer::UpdateInstanceBuffer(mat4* pInstances, const int& nInstances)
{
	if(nInstances > QVERTEXBUFFER_MAXINSTANCES)
		return false;
		
	int size = nInstances * sizeof(mat4);
	void* buf;
	if(SUCCEEDED(m_pVertexBuffer1->Lock(0, size, &buf, 0)))
	{
		memcpy(buf, pInstances, size);
		if(FAILED(m_pVertexBuffer1->Unlock()))
			return false;
	}
	
	return true;
}


bool CQuadrionVertexBuffer::UnbindBuffer()
{
	for(unsigned int i = 0; i < QVERTEXBUFFER_MAXSTREAMS; ++i)
	{
		if(m_bActiveStreams[i])
		{
			switch(i)
			{
				case 0:
					if(FAILED(m_pRenderDevice->SetStreamSource(0, NULL, 0, 0)))
						return false;
					break;
				
				case 1:
					if(FAILED(m_pRenderDevice->SetStreamSource(1, NULL, 0, 0)))
						return false;
					break;
				
				case 2:
					if(FAILED(m_pRenderDevice->SetStreamSource(2, NULL, 0, 0)))
						return false;
					break;
				
				case 3:
					if(FAILED(m_pRenderDevice->SetStreamSource(3, NULL, 0, 0)))
						return false;
					break;
				
				default:
					break;
			}
		}
	}
	
	if(m_bIsInstancing)
	{
		if(m_bHasInstanceBuffer)
		{
			m_pRenderDevice->SetStreamSourceFreq(0, 1);
			m_pRenderDevice->SetStreamSourceFreq(1, 1);
		}
		else
		{
			if(m_bIsInstanceBuffer)
				m_pRenderDevice->SetStreamSourceFreq(1, 1);
			else
				m_pRenderDevice->SetStreamSourceFreq(0, 1);
		}
		m_bIsInstancing = false;	
	}
	
	memset(m_bActiveStreams, 0, sizeof(bool) * QVERTEXBUFFER_MAXSTREAMS);
	return true;
}


bool CQuadrionVertexBuffer::BindBuffer()
{
	unsigned int streams[4] = {0, 1, 2, 3};
	return BindBuffer(streams, 4);
}

int	CQuadrionVertexBuffer::GetStreamByAttribute(const EQuadrionVertexAttribUsage& attrib)
{
	return GetStream(attrib);
}

bool CQuadrionVertexBuffer::CopyBufferData(void* buf)
{
	if(m_vertexSize[0] <= 0 || !buf)
		return false;
		
	void* tmp;
	if(SUCCEEDED(m_pVertexBuffer0->Lock(0, m_vertexSize[0] * m_nVertices, &tmp, 0)))
	{
		memcpy(buf, tmp, m_vertexSize[0] * m_nVertices);
		m_pVertexBuffer0->Unlock();
	}
	
	return true;	
}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

CQuadrionInstancedVertexBuffer::CQuadrionInstancedVertexBuffer(const unsigned int handle, const std::string& name, const std::string& path) 
							   : CQuadrionResource(handle, name, path)
{
	m_pRenderDevice = NULL;	
	m_pVertexDeclaration = NULL;
	m_pGeomBuffer = NULL;
	m_pInstanceBuffer = NULL;
	
	m_vertexSize = 0;
	m_nVertices = 0;
}

CQuadrionInstancedVertexBuffer::CQuadrionInstancedVertexBuffer(const void* pRender, const unsigned int handle, const std::string& name, const std::string& path)
							  : CQuadrionResource(handle, name, path)
{
	CQuadrionRender* p = (CQuadrionRender*)pRender;
	m_pRenderDevice = p->m_pD3DDev;	
	
	m_pVertexDeclaration = NULL;
	m_pGeomBuffer = NULL;
	m_pInstanceBuffer = NULL;
	m_vertexSize = 0;
	m_nVertices = 0;
}

CQuadrionInstancedVertexBuffer::~CQuadrionInstancedVertexBuffer()
{
	m_pRenderDevice = NULL;
	
	if(m_pVertexDeclaration)
	{
		m_pVertexDeclaration->Release();
		m_pVertexDeclaration = NULL;
	}
	
	if(m_pGeomBuffer)
	{
		m_pGeomBuffer->Release();
		m_pGeomBuffer = NULL;
	}
	
	if(m_pInstanceBuffer)
	{
		m_pInstanceBuffer->Release();
		m_pInstanceBuffer = NULL;
	}
	
	m_vertexSize = 0;
	m_nVertices = 0;
}




bool CQuadrionInstancedVertexBuffer::CreateGeometryBuffer(const void* pVertices, const SQuadrionVertexDescriptor& desc, const int& nVerts)
{
	unsigned int nAttribs = GetVertexAttributeCount(desc);
	if(nAttribs <= 0 || !pVertices)
		return false;
	
	D3DVERTEXELEMENT9* pVertexElements = new D3DVERTEXELEMENT9[nAttribs + 5];
	unsigned int nTexCoords = 0;
	unsigned int instancedSize = 0;
	for(int i = 0; i < nAttribs; ++i)
	{
		pVertexElements[i].Stream = 0;
	
		pVertexElements[i].Type = (D3DDECLTYPE)g_vertexFormatTypes[desc.size[i]];
		pVertexElements[i].Method = D3DDECLMETHOD_DEFAULT;
		pVertexElements[i].Usage = (D3DDECLUSAGE)g_vertexFormatUsages[desc.usage[i]];
		pVertexElements[i].Offset = m_vertexSize;
		
		if(desc.usage[i] == QVERTEXFORMAT_USAGE_TEXCOORD)
			pVertexElements[i].UsageIndex = nTexCoords++;
		else
			pVertexElements[i].UsageIndex = 0;
		
		m_vertexSize += g_vertexFormatSizes[desc.size[i]];
	}
	
	for(int i = nAttribs; i < nAttribs + 4; ++i)
	{
		pVertexElements[i].Method = D3DDECLMETHOD_DEFAULT;
		pVertexElements[i].Offset = instancedSize;
		pVertexElements[i].Stream = 1;
		pVertexElements[i].Type = D3DDECLTYPE_FLOAT4;
		pVertexElements[i].Usage = D3DDECLUSAGE_TEXCOORD;
		pVertexElements[i].UsageIndex = nTexCoords++;
		
		instancedSize += sizeof(float) * 4;
	}
	
	pVertexElements[nAttribs + 4].Method = 0;
	pVertexElements[nAttribs + 4].Offset = 0;
	pVertexElements[nAttribs + 4].Usage = 0;
	pVertexElements[nAttribs + 4].UsageIndex = 0;
	pVertexElements[nAttribs + 4].Stream = 0xFF;
	pVertexElements[nAttribs + 4].Type = D3DDECLTYPE_UNUSED;
	
	if(FAILED(m_pRenderDevice->CreateVertexDeclaration(pVertexElements, &m_pVertexDeclaration)))
		return false;
	
	
	delete[] pVertexElements;
	pVertexElements = NULL;
	
	unsigned int usage;
	D3DPOOL mem_pool;
	bool isDynamic = (desc.pool & QVERTEXBUFFER_MEMORY_DYNAMIC || desc.pool & QVERTEXBUFFER_MEMORY_STREAM) != 0;
	(isDynamic == true) ? usage = D3DUSAGE_DYNAMIC : usage = D3DUSAGE_WRITEONLY;
	mem_pool = D3DPOOL_DEFAULT;
	
	if(desc.pool & QVERTEXBUFFER_MEMORY_SYSTEM)
	{
		usage = 0;
		mem_pool = D3DPOOL_MANAGED;
		isDynamic = false;
	}
	
	unsigned int size = nVerts * m_vertexSize;
	if(FAILED(m_pRenderDevice->CreateVertexBuffer(size, usage, 0, mem_pool, &m_pGeomBuffer, NULL)))
		return false;
	
	void* dest;
	if(SUCCEEDED(m_pGeomBuffer->Lock(0, size, &dest, isDynamic ? D3DLOCK_DISCARD : 0)))
	{
		memcpy(dest, pVertices, size);
		m_pGeomBuffer->Unlock();
	}
		
	else 
		return false;


	m_nVertices = nVerts;
	return true;

}


bool CQuadrionInstancedVertexBuffer::CreateInstanceBuffer(const void* pVertices, const int& nInstances)
{
	unsigned int usage = 0;
	D3DPOOL mem_pool = D3DPOOL_MANAGED;
	bool isDynamic = false;
	usage = D3DUSAGE_WRITEONLY;

	unsigned int size = nInstances * sizeof(mat4);
	if(FAILED(m_pRenderDevice->CreateVertexBuffer(size, usage, 0, mem_pool, &m_pInstanceBuffer, NULL)))
		return false;
	
	void* dest;
	if(SUCCEEDED(m_pInstanceBuffer->Lock(0, size, &dest, isDynamic ? D3DLOCK_DISCARD : 0)))
	{
		memcpy(dest, pVertices, size);
		m_pInstanceBuffer->Unlock();
		
		return true;
	}
		
	return false;
}


bool CQuadrionInstancedVertexBuffer::UpdateInstanceBuffer(const void* pVertices, const int& nInstances)
{
	if(!m_pInstanceBuffer || !pVertices || nInstances < 0)
		return false;
	
	unsigned int size = sizeof(mat4) * nInstances;
	void* dest;
	if(SUCCEEDED(m_pInstanceBuffer->Lock(0, size, &dest, 0)))
	{
		memcpy(dest, pVertices, size);
		m_pInstanceBuffer->Unlock();
		
		return true;
	}
	
	return false;
}


bool CQuadrionInstancedVertexBuffer::BindBuffer(const unsigned int& nInstances)
{
	if(FAILED(m_pRenderDevice->SetVertexDeclaration(m_pVertexDeclaration)))
		return false;
		
	// Bind Geometry Buffer //
	if(FAILED(m_pRenderDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | nInstances))))
		return false;
	if(FAILED(m_pRenderDevice->SetStreamSource(0, m_pGeomBuffer, 0, m_vertexSize)))
		return false;


	
	// Bind Instance Buffer //
	if(FAILED(m_pRenderDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1))))
		return false;	
	if(FAILED(m_pRenderDevice->SetStreamSource(1, m_pInstanceBuffer, 0, sizeof(mat4))))
		return false;

	
	return true;
}


bool CQuadrionInstancedVertexBuffer::UnbindBuffer()
{
	m_pRenderDevice->SetStreamSource(0, NULL, 0, 0);
	m_pRenderDevice->SetStreamSource(1, NULL, 0, 0);
	m_pRenderDevice->SetStreamSourceFreq(0, 1);
	m_pRenderDevice->SetStreamSourceFreq(1, 1);
	
	return true;
}


void CQuadrionInstancedVertexBuffer::ChangeRenderDevice(const void* pRender)
{
	CQuadrionRender* p = (CQuadrionRender*)pRender;
	m_pRenderDevice = p->m_pD3DDev;		
}