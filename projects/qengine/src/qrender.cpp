// qrender.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "qrender.h"
#include "qTMS.h"
#include <fstream>

 
CQuadrionRender* g_pRender = new CQuadrionRender;

// Geometry only vertex format //
typedef struct
{
	D3DXVECTOR3 pos;
} SGeometryVertex;


// Colored non-transformed vertex format //
typedef struct
{
	float x, y, z;				// world space position
	unsigned int color;			// Color in ARGB format
} SColoredVertex;

// Textured non-transformed vertex format //
typedef struct
{
	D3DXVECTOR3 pos;					// world space position
	D3DXVECTOR2 texCoords;				// Tex coords
} STexturedVertex;

// Textured vertex with normal //
typedef struct
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 norm;
	D3DXVECTOR2 texCoords;
} STexturedNormalVertex;

// Textured and transformed vertex format //
typedef struct
{
	D3DXVECTOR4 pos;				// Screen space position
	D3DXVECTOR2 texCoords;			// Tex coords
} STransformedTexturedVertex;

// Colored and transformed vertex format //
typedef struct
{
	D3DXVECTOR4    pos;				// Screen space position
	unsigned int   color;			// Color in ARGB format
} STransformedColoredVertex;


// Colored, transformed, textured vertex format //
typedef struct
{
	D3DXVECTOR4		pos;
	unsigned int	color;
	D3DXVECTOR2		texCoords;
} STransformedTexturedColoredVertex;




// FVF Vertex format definitions //
static unsigned long GeometryFVF							= (D3DFVF_XYZ);
static unsigned long ColoredVertexFVF					    = (D3DFVF_XYZ | D3DFVF_DIFFUSE);
static unsigned long TexturedVertexFVF						= (D3DFVF_XYZ | D3DFVF_TEX1);
static unsigned long TexturedNormalVertexFVF			    = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );
static unsigned long TransformedTexturedVertexFVF			= (D3DFVF_XYZRHW | D3DFVF_TEX1);
static unsigned long TransformedTexturedColoredVertexFVF    = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
static unsigned long TransformedColoredVertexFVF            = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE);


// Backbuffer formats //
static D3DFORMAT bufferFormats[] = 
{
	D3DFMT_A2R10G10B10,
	D3DFMT_A8R8G8B8,
	D3DFMT_X8R8G8B8,
};

// Multisample formats for HW-FSAA // 
static D3DMULTISAMPLE_TYPE	multisampleFormats[] = 
{
	D3DMULTISAMPLE_NONE,
	D3DMULTISAMPLE_2_SAMPLES,
	D3DMULTISAMPLE_3_SAMPLES,
	D3DMULTISAMPLE_4_SAMPLES,
	D3DMULTISAMPLE_5_SAMPLES,
	D3DMULTISAMPLE_6_SAMPLES,
	D3DMULTISAMPLE_7_SAMPLES,
	D3DMULTISAMPLE_8_SAMPLES,
	D3DMULTISAMPLE_9_SAMPLES,
	D3DMULTISAMPLE_10_SAMPLES,
	D3DMULTISAMPLE_11_SAMPLES,
	D3DMULTISAMPLE_12_SAMPLES,
	D3DMULTISAMPLE_13_SAMPLES,
	D3DMULTISAMPLE_14_SAMPLES,
	D3DMULTISAMPLE_15_SAMPLES,
	D3DMULTISAMPLE_16_SAMPLES,
};


// Coverage Sampled Anti-Aliased formats for HW CSAA //
static D3DMULTISAMPLE_TYPE csaaFormats[] = 
{
	D3DMULTISAMPLE_NONE,
	D3DMULTISAMPLE_4_SAMPLES,
	D3DMULTISAMPLE_8_SAMPLES,
	D3DMULTISAMPLE_4_SAMPLES,
	D3DMULTISAMPLE_8_SAMPLES,
	D3DMULTISAMPLE_16_SAMPLES,
};


// Coverage Sampled Anti-Aliased qualities for HW CSAA //
static unsigned int csaaQualities[] = 
{
	0,
	2,
	0,
	4,
	2,
	0,
};


static unsigned int csaaTestQualities[] = 
{
	0,
	5,
	9,
	5,
	9,
	0,
};

// Depth-Stencil buffer formats //
static D3DFORMAT backbufferFormats[] = 
{
	D3DFMT_D32,
	D3DFMT_D16,
	D3DFMT_D24S8,
	D3DFMT_D24X8,
};


// Vertex format usage bindings //
static D3DDECLUSAGE vfUsages[] = 
{
	D3DDECLUSAGE_POSITION,
	D3DDECLUSAGE_BLENDWEIGHT,
	D3DDECLUSAGE_BLENDINDICES,
	D3DDECLUSAGE_NORMAL,
	D3DDECLUSAGE_PSIZE,
	D3DDECLUSAGE_TEXCOORD,
	D3DDECLUSAGE_TANGENT,
	D3DDECLUSAGE_BINORMAL,
	D3DDECLUSAGE_COLOR,
	D3DDECLUSAGE_FOG,	
};


// Vertex format type bindings //
static D3DDECLTYPE vfTypes[] = 
{
	D3DDECLTYPE_FLOAT1,
	D3DDECLTYPE_FLOAT2,
	D3DDECLTYPE_FLOAT3,
	D3DDECLTYPE_FLOAT4,
	D3DDECLTYPE_D3DCOLOR,
	D3DDECLTYPE_UBYTE4,
	D3DDECLTYPE_SHORT2,
	D3DDECLTYPE_SHORT4,
	D3DDECLTYPE_UBYTE4N,
	D3DDECLTYPE_USHORT2N,
	D3DDECLTYPE_USHORT4N,
	D3DDECLTYPE_FLOAT16_2,
	D3DDECLTYPE_FLOAT16_4,
	D3DDECLTYPE_UNUSED,
};

// Vertex format sizes //
// Indexed indentically to vfTypes[] //
static unsigned int vfSizes[] = 
{
	sizeof(float),
	sizeof(float) + sizeof(float),
	sizeof(float) + sizeof(float) + sizeof(float),
	sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float),
	4,
	sizeof(short) * 2,
	sizeof(short) * 4,
	sizeof(unsigned char) * 4,
	sizeof(unsigned short) * 2,
	sizeof(unsigned short) * 4,
	4, 
	8,
	0,
};


// Make into UINT RGBA format (OpenGL) //
QRENDEREXPORT_API unsigned int QRENDER_MAKE_RGBA(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
{
	return ((unsigned int)((a << 24) | (b << 16) | (g << 8) | (r)));	
}

// Make into UINT BGRA format (IRIX) //
QRENDEREXPORT_API unsigned int QRENDER_MAKE_BGRA(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
{
	return ((unsigned int)((b << 24) | (g << 16) | (r << 8) | (a)));
}

// Make into UINT ARGB format (Direct3D) //
QRENDEREXPORT_API unsigned int QRENDER_MAKE_ARGB(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
{
	return ((unsigned int)((a << 24) | (r << 16) | (g << 8) | (b)));
}




//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

CQuadrionRender::CQuadrionRender()
{
	m_pD3DObj = NULL;
	m_pD3DDev = NULL;
	m_pFramebuffer = NULL;
	m_pDepthStencilBuffer = NULL;
	m_adapterFormat = D3DFORMAT(0);
	m_displayWidth = 1024;
	m_displayHeight = 768;

	m_currentDepthMode = QRENDER_ZBUFFER_LEQUAL;
	m_currentViewport.X = 0;
	m_currentViewport.Y = 0;
	m_currentViewport.Width = m_displayWidth;
	m_currentViewport.Height = m_displayHeight;
	m_previousViewport.X = 0;
	m_previousViewport.Y = 0;
	m_previousViewport.Width = m_displayWidth;
	m_previousViewport.Height = m_displayHeight;
	m_currentViewport.MinZ = 0;
	m_currentViewport.MaxZ = 0;
	m_previousViewport.MinZ = 0;
	m_previousViewport.MaxZ = 0;
	
	
	m_pOcclusionQuery = NULL;
	m_pD3DObj = NULL;
	m_pD3DDev = NULL;
	m_pFramebuffer = NULL;
	m_pDepthStencilBuffer = NULL;
	m_bStencilTestEnabled = false;

	m_currentEffect = QRENDER_INVALID_HANDLE;
	m_effectResources = NULL;
	m_vertexBufferResources = NULL;
	m_indexBufferResources = NULL;
	m_textureObjectResources = NULL;
	m_renderTargetResources = NULL;
	m_depthStencilTargetResources = NULL;
	m_instancedVertexBufferResources = NULL;
	m_vertexNameRef = 0;
	m_instancedVertexNameRef = 0;
	m_indexNameRef = 0;
	m_textureNameRef = 0;
	m_renderTargetNameRef = 0;
	m_depthStencilNameRef = 0;
	m_currentStencilFunc = 0;
	m_clipPlaneMask = 0;
	m_currentFSAA = 0;
	m_currentMSAA = 0;
	m_curFVF	  = 0;
	
	m_lineVertexBuffer = NULL;
	
	m_pConeObject = NULL;
	m_pSphereObject = NULL;
	
	m_nonMSAADepthStencil = QRENDER_INVALID_HANDLE;
	
	m_currentDepthStencilTarget = -1;
	memset(m_currentTextures, -1, sizeof(int) * QRENDER_MAX_TEXTURE_SAMPLERS);
	memset(m_currentSamplers, 0, sizeof(SQuadrionTextureSampler) * QRENDER_MAX_TEXTURE_SAMPLERS);
	memset(m_currentRenderTargets, -1, sizeof(int) * QRENDER_MAX_RENDER_TARGETS);
	memset(m_currentTextureTypes, -1, sizeof(int) * QRENDER_MAX_TEXTURE_SAMPLERS);

}

CQuadrionRender::~CQuadrionRender()
{
	Release();	
}








/////////////////////////////////////////////////////////////////////////////////////
// initRender
// hWindow- window handle
// width- window width
// height- window height
//
// Initialize the direct3D object and device and bind it to either a fullscreen
// or windowed mode window
//
// MUST be called prior to any use of the CQuadrionRenderer object in a 
// one time initialization
//
/////////////////////////////////////////////////////////////////////////////////////
bool CQuadrionRender::Initialize(const HWND& hWindow, const SQuadrionRenderInitializer& init)
{
	m_displayWidth = init.displayWidth;
	m_displayHeight = init.displayHeight;
	
	m_currentViewport.X = 0;
	m_currentViewport.Y = 0;
	m_currentViewport.Width = m_displayWidth;
	m_currentViewport.Height = m_displayHeight;
	m_previousViewport.X = 0;
	m_previousViewport.Y = 0;
	m_previousViewport.Width = m_displayWidth;
	m_previousViewport.Height = m_displayHeight;
	m_hWnd = hWindow;
	
	// Create D3D Object //
	if(NULL == (m_pD3DObj = Direct3DCreate9(D3D_SDK_VERSION)))
	{
		MessageBoxA(NULL, "Could not create Direct3D Object", "Render initialization error!", MB_OK | MB_ICONERROR);
		return false;
	}
	
	// Check device capabilities //
	ZeroMemory(&m_caps, sizeof(m_caps));
	if(FAILED(m_pD3DObj->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &m_caps)))
	{
		MessageBoxA(NULL, "Could not obtain device capabilities", "Render initialization error!", MB_OK | MB_ICONERROR);
		return false;
	}
	
	// Initialize adapter format //
	if(init.bufferFormat < 0 || init.bufferFormat > 2)
	{
		MessageBoxA(NULL, "Invalid backbuffer format specified", "Render initialization error!", MB_OK | MB_ICONERROR);
		return false;
	}
	D3DFORMAT adapterFormat = bufferFormats[init.bufferFormat];
	m_adapterFormat = adapterFormat;
	
	// Always hardware vertex processing //
	DWORD behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	
	// Setup adapter //
	unsigned int adapterToUse;
	D3DDEVTYPE deviceType;
	
	if(!init.debugGraphics)
	{
		adapterToUse = D3DADAPTER_DEFAULT;
		deviceType = D3DDEVTYPE_HAL;
	}
	
	// Use nvidia debug device //
	else
	{
		for(unsigned int adapter = 0; adapter < m_pD3DObj->GetAdapterCount(); ++adapter)
		{
			D3DADAPTER_IDENTIFIER9 identifier;
			HRESULT res;
			
			res = m_pD3DObj->GetAdapterIdentifier(adapter, 0, &identifier);
			if(strstr(identifier.Description, "PerfHUD") != 0)
			{
				adapterToUse = adapter;
				deviceType = D3DDEVTYPE_REF;
				break;
			}
		}	
	}
	
	// Check device vendor //
	D3DADAPTER_IDENTIFIER9 deviceID;
	if( FAILED( m_pD3DObj->GetAdapterIdentifier( adapterToUse, 0, &deviceID ) ) )
		return false;
	
	DWORD vid = deviceID.VendorId;
	switch( vid )
	{
		case 0x000010DE:
			m_deviceCaps.vendor = QRENDER_DEVICE_NVIDIA;
			break;
			
		case 0x00001002:
			m_deviceCaps.vendor = QRENDER_DEVICE_ATI;
			break;
		
		default:
			break;
	}
	
	
	// Set presentation params //
	GetMultisample( init, m_adapterFormat, adapterToUse );
	m_deviceCaps.maxFSAA = m_currentMSAA;
	D3DPRESENT_PARAMETERS pp;
	ZeroMemory(&pp, sizeof(pp));
	pp.BackBufferFormat = m_adapterFormat;
	pp.MultiSampleType = csaaFormats[m_currentMSAA];
	
	if( m_deviceCaps.vendor == QRENDER_DEVICE_NVIDIA )
		pp.MultiSampleQuality = csaaQualities[m_currentMSAA];
	else
		pp.MultiSampleQuality = 0;
		
	pp.BackBufferWidth = init.displayWidth;
	pp.BackBufferHeight = init.displayHeight;
	pp.BackBufferCount = init.bufferCount;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.Windowed = init.windowed;
	pp.EnableAutoDepthStencil = true;
	pp.AutoDepthStencilFormat = backbufferFormats[init.backbufferFormat];
	pp.hDeviceWindow = hWindow;
	
	// Adjust presentation interval for vsync //
	if(init.vsync)
		pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	else
		pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		
		
	
	
	// Query Desired FSAA //
//	unsigned int desiredFSAA = init.fsaa;
//	DWORD	qualLevels;
//	if(desiredFSAA > 15)
//		desiredFSAA = 15;
//	if(desiredFSAA > 0)
//	{
//		bool supported = false;
//		while(!supported)
//		{
//			if(FAILED(m_pD3DObj->CheckDeviceMultiSampleType(adapterToUse, deviceType, adapterFormat, pp.Windowed, \
//				      multisampleFormats[desiredFSAA], &qualLevels)))
//			{
//				--desiredFSAA;
//			}
//			
//			else
//				supported = true;
//		}
//		
//		pp.MultiSampleType = multisampleFormats[desiredFSAA];
//	}
//	
//	m_currentFSAA = desiredFSAA;
	
	// Check that device configuration is valid before creation //
	if(FAILED(m_pD3DObj->CheckDeviceType(adapterToUse, deviceType, adapterFormat, adapterFormat, pp.Windowed)))
	{
		MessageBoxA(NULL, "Device configuration is not valid", "Renderer initialization failure!", MB_OK | MB_ICONERROR);
		return false;
	}
	
	// Acquire Direct3D Device //
	if(FAILED(m_pD3DObj->CreateDevice(adapterToUse, deviceType, hWindow, behaviorFlags, &pp, &m_pD3DDev)))
	{
		MessageBoxA(NULL, "Failed to create Direct 3D Device", "Renderer initialization failure!", MB_OK | MB_ICONERROR);
		return false;
	}
	
	// Enable FSAA if desired //
	if( m_currentFSAA > 0 || m_currentMSAA > 0 )
		m_pD3DDev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, true );
	
	// Acquire swap chain //
	if(FAILED(m_pD3DDev->GetRenderTarget(0, &m_pFramebuffer)))
	{
		MessageBoxA(NULL, "Failed to obtain swap surface", "Renderer initialization failure!", MB_OK | MB_ICONERROR);
		return false;
	}
	
	// Acquire depth/stencil surface //
	if(FAILED(m_pD3DDev->GetDepthStencilSurface(&m_pDepthStencilBuffer)))
	{
		MessageBoxA(NULL, "Failed to obtain depth/stencil buffer", "Renderer initialization failure!", MB_OK | MB_ICONERROR);
		return false;
	}

	
	// Create generic sphere shape //
	if(FAILED(D3DXCreateSphere(m_pD3DDev, 1.0, 16, 14, &m_pSphereObject, NULL)))
		return false;
	
	DWORD* adjacency = (DWORD*)malloc(sizeof(DWORD) * m_pSphereObject->GetNumFaces() * 3);
	if(FAILED(m_pSphereObject->GenerateAdjacency(0.001f, adjacency)))
	{
		free(adjacency); adjacency = NULL;
		return false;
	}

	DWORD* newadj = (DWORD*)malloc(sizeof(DWORD) * m_pSphereObject->GetNumFaces() * 3);
	LPD3DXBUFFER remap;
	if(FAILED(m_pSphereObject->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, adjacency, newadj, NULL, &remap)))
		return false;

	free(adjacency); adjacency = NULL;
	free(newadj); newadj = NULL;
	
	// Create generic cone shape //
	if(FAILED(D3DXCreateCylinder(m_pD3DDev, 1.0F, 5.0F, 1.0F, 6, 4, &m_pConeObject, NULL)))
		return false;
	
	adjacency = (DWORD*)malloc(sizeof(DWORD) * m_pConeObject->GetNumFaces() * 3);
	if(FAILED(m_pConeObject->GenerateAdjacency(0.001F, adjacency)))
	{
		free(adjacency); adjacency = NULL;
		return false;
	}
	
	newadj = (DWORD*)malloc(sizeof(DWORD) * m_pConeObject->GetNumFaces() * 3);
	LPD3DXBUFFER remap2;
	if(FAILED(m_pConeObject->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, adjacency, newadj, NULL, &remap2)))
		return false;
	
	free(adjacency); adjacency = NULL;
	free(newadj); newadj = NULL;
	
	
	
	
	// Set Default render state //
	m_pD3DDev->SetRenderState(D3DRS_LIGHTING, false);
	m_pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	m_pD3DDev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	m_pD3DDev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	m_pD3DDev->SetViewport(&m_currentViewport);
	
	CheckDeviceCapabilities(adapterFormat);
	CheckDisplayModes(adapterToUse);
	
//	currentVertexFormat = QRENDER_INVALID_HANDLE;
	
	m_effectResources = new CQuadrionResourceManager<CQuadrionEffect>;
	m_vertexBufferResources = new CQuadrionResourceManager<CQuadrionVertexBuffer>;
	m_instancedVertexBufferResources = new CQuadrionResourceManager<CQuadrionInstancedVertexBuffer>;
	m_indexBufferResources = new CQuadrionResourceManager<CQuadrionIndexBuffer>;
	m_textureObjectResources = new CQuadrionResourceManager<CQuadrionTextureObject>;
	m_renderTargetResources = new CQuadrionResourceManager<CQuadrionRenderTarget>;
	m_depthStencilTargetResources = new CQuadrionResourceManager<CQuadrionDepthStencilTarget>;
	
	m_pD3DDev->CreateVertexBuffer(2 * sizeof(SColoredVertex), D3DUSAGE_DYNAMIC, ColoredVertexFVF, D3DPOOL_DEFAULT, &m_lineVertexBuffer, NULL);
	
	if( m_currentMSAA > 0 )
		m_nonMSAADepthStencil = AddDepthStencilTarget( init.displayWidth, init.displayHeight, 24, 8 );


	// Load the noshader texture (has to be loaded before all others)
	unsigned int textureFlags = 0;
	textureFlags |= QTEXTURE_FILTER_TRILINEAR;
	m_noShaderHandle = AddTextureObject(textureFlags, "textures/noshader.jpg", "./");
	
	// Obtain a new D3D Query object for occlusion queries //
	m_pD3DDev->CreateQuery(D3DQUERYTYPE_OCCLUSION, &m_pOcclusionQuery);
	
	return true;
}


int	CQuadrionRender::InitializeNewSwapChain(const HWND& hWindow, const SQuadrionRenderInitializer& init)
{
	if(!m_pD3DDev)
		return QRENDER_INVALID_HANDLE;
	
	D3DPRESENT_PARAMETERS pp;
	ZeroMemory(&pp, sizeof(pp));
	
	pp.BackBufferFormat = m_adapterFormat;
	pp.MultiSampleType = csaaFormats[m_currentMSAA];
	
	if( m_deviceCaps.vendor == QRENDER_DEVICE_NVIDIA )
		pp.MultiSampleQuality = csaaQualities[m_currentMSAA];
	else
		pp.MultiSampleQuality = 0;
		
	pp.BackBufferWidth = init.displayWidth;
	pp.BackBufferHeight = init.displayHeight;
	pp.BackBufferCount = init.bufferCount;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.Windowed = init.windowed;
	pp.EnableAutoDepthStencil = true;
	pp.AutoDepthStencilFormat = backbufferFormats[init.backbufferFormat];
	pp.hDeviceWindow = hWindow;
	
	// Adjust presentation interval for vsync //
	if(init.vsync)
		pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	else
		pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		
	LPDIRECT3DSWAPCHAIN9 newChain;
	if(FAILED(m_pD3DDev->CreateAdditionalSwapChain(&pp, &newChain)))
		return QRENDER_INVALID_HANDLE;
	
	LPDIRECT3DSURFACE9 newDepthStencil;
	if(FAILED(m_pD3DDev->CreateDepthStencilSurface(pp.BackBufferWidth, pp.BackBufferHeight, pp.AutoDepthStencilFormat, pp.MultiSampleType, 
												   pp.MultiSampleQuality, TRUE, &newDepthStencil, NULL)))
		return QRENDER_INVALID_HANDLE;
	
	m_swapChains.push_back(newChain);
	m_swapDepthStencils.push_back(newDepthStencil);
	return m_swapChains.size() - 1;
}


bool CQuadrionRender::BindSwapChain(const int& id)
{
	if(id == QRENDER_INVALID_HANDLE || id > m_swapChains.size())
		return false;
		
	else if(id == QRENDER_DEFAULT)
	{
		m_pD3DDev->SetRenderTarget(0, m_pFramebuffer);
		m_pD3DDev->SetDepthStencilSurface(m_pDepthStencilBuffer);
	}
	
	else
	{
		LPDIRECT3DSURFACE9 backbuffer = NULL;
		if(FAILED(m_swapChains[id]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer)))
			return false;
		
		if(FAILED(m_pD3DDev->SetRenderTarget(0, backbuffer)))
			return false;
		
		backbuffer->Release();
		
		if(FAILED(m_pD3DDev->SetDepthStencilSurface(m_swapDepthStencils[id])))
			return false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Release
// Cleanup renderer
// MUST be called on application exit 
void CQuadrionRender::Release()
{
	memset(m_currentTextures, -1, sizeof(int) * QRENDER_MAX_TEXTURE_SAMPLERS);
	memset(m_currentRenderTargets, -1, sizeof(int) * QRENDER_MAX_RENDER_TARGETS);
	memset(m_currentSamplers, 0, sizeof(SQuadrionTextureSampler) * QRENDER_MAX_TEXTURE_SAMPLERS);
	m_textureNameRef = 0;
	m_depthStencilNameRef = 0;
	m_renderTargetNameRef = 0;
	m_depthStencilNameRef = 0;
	m_currentDepthStencilTarget = -1;
	m_currentEffect = QRENDER_INVALID_HANDLE;
	m_bStencilTestEnabled = false;
	m_currentStencilFunc = 0;
	m_instancedVertexNameRef = 0;
	m_curFVF				 = 0;


	if(m_pSphereObject)
	{
		m_pSphereObject->Release();
		m_pSphereObject = NULL;
	}

	if(m_effectResources)
	{
		delete m_effectResources;
		m_effectResources = NULL;
	}
	
	if(m_vertexBufferResources)
	{
		delete m_vertexBufferResources;
		m_vertexBufferResources = NULL;
	}
	
	if(m_indexBufferResources)
	{
		delete m_indexBufferResources;
		m_indexBufferResources = NULL;
	}
	
	if(m_textureObjectResources)
	{
		delete m_textureObjectResources;
		m_textureObjectResources = NULL;
	}
	
	if(m_renderTargetResources)
	{
		delete m_renderTargetResources;
		m_renderTargetResources = NULL;
	}
	
	if(m_depthStencilTargetResources)
	{
		delete m_depthStencilTargetResources;
		m_depthStencilTargetResources = NULL;
	}
	
	if(m_instancedVertexBufferResources)
	{
		delete m_instancedVertexBufferResources;
		m_instancedVertexBufferResources = NULL;
	}
	
	if(m_pOcclusionQuery)
	{
		m_pOcclusionQuery->Release();
		m_pOcclusionQuery = NULL;
	}
	

	unsigned int i;
	m_userFVFs.clear();

	if(m_pD3DDev)
	{
		m_pD3DDev->Release();
		m_pD3DDev = NULL;
	}
	
	if(m_pD3DObj)
	{
		m_pD3DObj->Release();
		m_pD3DObj = NULL;
	}
	
	if(m_pFramebuffer)
	{
		m_pFramebuffer->Release();
		m_pFramebuffer = NULL;
	}
	
	if(m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = NULL;
	}

	size_t len;
	len = m_spriteList.size();
	for(i = 0; i < len; ++i)
	{
		m_spriteList[i]->Release();
		m_spriteList[i] = NULL;
	}
	
	len = m_fontList.size();
	for(i = 0; i < len; ++i)
	{
		m_fontList[i]->Release();
		m_fontList[i] = NULL;
	}
	
	if(m_lineVertexBuffer)
	{
		m_lineVertexBuffer->Release();
		m_lineVertexBuffer = NULL;
	}
	
	for(int i = 0; i < m_swapChains.size(); ++i)
	{
		if(m_swapChains[i])
		{
			m_swapChains[i]->Release();
			m_swapChains[i] = NULL;
		}
	}
	
	for(int i = 0; i < m_swapDepthStencils.size(); ++i)
	{
		if(m_swapDepthStencils[i])
		{
			m_swapDepthStencils[i]->Release();
			m_swapDepthStencils[i] = NULL;
		}
	}
}



////////////////////////////////////////////////////////////////////////////////////
// GetMultisample
// Check init params and adapter format for the best possible multisample format
unsigned int CQuadrionRender::GetMultisample(const SQuadrionRenderInitializer& init, const D3DFORMAT& adapterFormat, const unsigned int& adapter)
{
	if( init.csaa == 0 )
	{
		m_currentMSAA = 0;
		return 0;
	}

	// Check FSAA down from desired setting rather than up from none //
	// We'll look for the best quality closest to what the user has selected //
	DWORD qual;
	for( unsigned int i = init.csaa; i >= 0; --i )
	{
		if( SUCCEEDED( m_pD3DObj->CheckDeviceMultiSampleType( adapter, D3DDEVTYPE_HAL, adapterFormat, init.windowed, csaaFormats[i], &qual ) ) )
		{
			if(m_deviceCaps.vendor == QRENDER_DEVICE_NVIDIA)
			{
				if( qual >= csaaTestQualities[i] )
				{
					m_currentMSAA = i;
					return i;
				}
			}
			
			m_currentMSAA = i;
			return i;
		}
	}
	
	m_currentMSAA = 0;
	return 0;
}



////////////////////////////////////////////////////////////////////////////
// Set vertex and pixel profile format from string
void CQuadrionRender::SetShaderProfiles(LPCSTR v, LPCSTR p)
{
	if(strcmp(v, "vs_1_0") == 0)
		m_deviceCaps.greatestVertexProfile = QRENDER_VERTEX_SHADER_1_0;
	else if(strcmp(v, "vs_1_1") == 0)
		m_deviceCaps.greatestVertexProfile = QRENDER_VERTEX_SHADER_1_1;
	else if(strcmp(v, "vs_1_2") == 0)
		m_deviceCaps.greatestVertexProfile = QRENDER_VERTEX_SHADER_1_2;
	else if(strcmp(v, "vs_2_0") == 0)
		m_deviceCaps.greatestVertexProfile = QRENDER_VERTEX_SHADER_2_0;
	else if(strcmp(v, "vs_3_0") == 0)
		m_deviceCaps.greatestVertexProfile = QRENDER_VERTEX_SHADER_3_0;
	else
		m_deviceCaps.greatestVertexProfile = QRENDER_VERTEX_SHADER_UNKNOWN;
	
	if(strcmp(p, "ps_1_0") == 0)
		m_deviceCaps.greatestPixelProfile = QRENDER_PIXEL_SHADER_1_0;
	else if(strcmp(p, "ps_2_0") == 0)
		m_deviceCaps.greatestPixelProfile = QRENDER_PIXEL_SHADER_2_0;
	else if(strcmp(p, "ps_3_0") == 0)
		m_deviceCaps.greatestPixelProfile = QRENDER_PIXEL_SHADER_3_0;
	else
		m_deviceCaps.greatestPixelProfile = QRENDER_PIXEL_SHADER_UNKNOWN;
}


///////////////////////////////////////////////////////////////////////////////
// CheckDeviceCapabilities
// Looks through the caps structure for meaningful information about the device
void CQuadrionRender::CheckDeviceCapabilities(const D3DFORMAT& adapterFormat)
{
	// Check for vertex/pixel shader support //
	LPCSTR gVP = D3DXGetVertexShaderProfile(m_pD3DDev);
	LPCSTR gPP = D3DXGetPixelShaderProfile(m_pD3DDev);
	
	SetShaderProfiles(gVP, gPP);
	
	// Check for texture compression
	if(m_pD3DObj->CheckDeviceFormat(m_caps.AdapterOrdinal, m_caps.DeviceType, adapterFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT1) == D3D_OK)
		m_deviceCaps.supportsS3TC = true;
	else
		m_deviceCaps.supportsS3TC = false;
	
	// Check for post pixel blending //
	if(SUCCEEDED(m_pD3DObj->CheckDeviceFormat(m_caps.AdapterOrdinal, m_caps.DeviceType, adapterFormat, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
										 D3DRTYPE_SURFACE, adapterFormat)))
	{
		m_deviceCaps.supportsPostPixelBlending = true;
	}
	
	else
	{
		m_deviceCaps.supportsPostPixelBlending = false;
	}
	
	m_deviceCaps.maxLights = m_caps.MaxActiveLights;
	m_deviceCaps.maxAnisotropy = m_caps.MaxAnisotropy;
	

	m_deviceCaps.maxSimultaneousMRT = m_caps.NumSimultaneousRTs;
	if(SUCCEEDED(m_pD3DObj->CheckDeviceFormat(m_caps.AdapterOrdinal, m_caps.DeviceType, adapterFormat, D3DUSAGE_RENDERTARGET | D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS, D3DRTYPE_SURFACE, adapterFormat)))
		m_deviceCaps.supportsIndependentMRTBitDepth = true;
	else
		m_deviceCaps.supportsIndependentMRTBitDepth = false;
	
	GetGreatestLuminanceFormat(adapterFormat);
	GetGreatestFloatFormat(adapterFormat);
}


void CQuadrionRender::CheckDisplayModes(const unsigned int& adapter)
{
	unsigned int nModes;
	nModes = m_pD3DObj->GetAdapterModeCount(adapter, D3DFMT_X8R8G8B8); 	
	
	D3DDISPLAYMODE dispModes;
	SResolution res;
	bool found = false;
	for(unsigned int i = 0; i < nModes; ++i)
	{
		if(m_pD3DObj->EnumAdapterModes(adapter, D3DFMT_X8R8G8B8, i, &dispModes) == D3D_OK)
		{
			if(m_deviceCaps.supportedResolutions.size() > 0)
			{
				for(unsigned int j = 0; j < m_deviceCaps.supportedResolutions.size(); ++j)
				{
					if((dispModes.Width == m_deviceCaps.supportedResolutions[j].m_width) && (dispModes.Height == m_deviceCaps.supportedResolutions[j].m_height))
					{
						found = true;
						continue;
					}
				}     
				
				if(!found)
				{
					res.m_width = dispModes.Width;
					res.m_height = dispModes.Height;
					m_deviceCaps.supportedResolutions.push_back(res);
				}
				
				found = false;
			}
			
			else
			{
				res.m_width = dispModes.Width;
				res.m_height = dispModes.Height;
				
				m_deviceCaps.supportedResolutions.push_back(res);
			}
		}
	}
}



void CQuadrionRender::GetGreatestLuminanceFormat(const D3DFORMAT& adapterFormat)
{
	m_deviceCaps.supportsFloatingPointLuminance = true;

	if(FAILED(m_pD3DObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, m_caps.DeviceType, adapterFormat, D3DUSAGE_RENDERTARGET,
												D3DRTYPE_TEXTURE, D3DFMT_R32F))) 
	{
		if( FAILED( m_pD3DObj->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_RENDERTARGET,
														D3DRTYPE_TEXTURE, D3DFMT_R16F ) ) )
		{
			if(FAILED(m_pD3DObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_RENDERTARGET,
														D3DRTYPE_TEXTURE, D3DFMT_A8L8)))
			{
				m_deviceCaps.greatestLuminanceFormat = QTEXTURE_FORMAT_NONE;
				m_deviceCaps.supportsFloatingPointLuminance = false;
			}
			else
			{
				m_deviceCaps.greatestLuminanceFormat = QTEXTURE_FORMAT_RGBA8;
				m_deviceCaps.supportsFloatingPointLuminance = false;
			}

		}
		
		else
			m_deviceCaps.greatestLuminanceFormat = QTEXTURE_FORMAT_I16F;
	}
	
	else
		m_deviceCaps.greatestLuminanceFormat = QTEXTURE_FORMAT_I32F;
}

void CQuadrionRender::GetGreatestFloatFormat(const D3DFORMAT& adapterFormat)
{
	m_deviceCaps.supportsFloatingPointTargets = true;

	if(FAILED(m_pD3DObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_RENDERTARGET |	
										D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F)))
	{
		if(FAILED(m_pD3DObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
											D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F)))
		{
			if(FAILED(m_pD3DObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
												D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16)))
			{
				m_deviceCaps.greatestFloatTextureFormat = QTEXTURE_FORMAT_RGBA8;
				m_deviceCaps.supportsFloatingPointTargets = false;
			}
			
			else
			{
				m_deviceCaps.greatestFloatTextureFormat = QTEXTURE_FORMAT_RGBA16;
				m_deviceCaps.supportsFloatingPointTargets = false;
			}
		}
		
		else
			m_deviceCaps.greatestFloatTextureFormat = QTEXTURE_FORMAT_RGBA16F;
	}	
	
	else
		m_deviceCaps.greatestFloatTextureFormat = QTEXTURE_FORMAT_RGBA32F;	
}





////////////////////////////////////////////////////////
// BeginRendering
// Call prior to any rendering calls
bool CQuadrionRender::BeginRendering()
{
	if(FAILED(m_pD3DDev->BeginScene()))
		return false;
	
	return true;
}


/////////////////////////////////////////////////////////
// EndRendering
// Call when through with any rendering calls 
// Does a back to front buffer swap
bool CQuadrionRender::EndRendering(const int& swapChain)
{
	if(swapChain == QRENDER_DEFAULT)
	{
		if(FAILED(m_pD3DDev->EndScene()))
			return false; 
		if(FAILED(m_pD3DDev->Present(NULL, NULL, NULL, NULL)))
			return false; 
	}

	else
	{
		if(FAILED(m_pD3DDev->EndScene()))
			return false;
		if(FAILED(m_swapChains[swapChain]->Present(NULL, NULL, NULL, NULL, 0)))
			return false;
	}
	
	
	memset(m_currentSamplers, 0, sizeof(SQuadrionTextureSampler) * QRENDER_MAX_TEXTURE_SAMPLERS);
	memset(m_currentTextures, -1, sizeof(int) * QRENDER_MAX_TEXTURE_SAMPLERS);
	memset(m_currentRenderTargets, -1, sizeof(int) * QRENDER_MAX_RENDER_TARGETS);
	m_currentDepthStencilTarget = -1;
	EvictRenderTargets();
	
	return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
// SetViewport
// x- upper left corner x-coordinate in screen space
// y- upper left corner y-coordinate in screen space
// width- width in pixels of viewport
// height- height in pixels of viewport
//
// Makes the viewport specified by the parameters the current one
// Returns false if failed
bool CQuadrionRender::SetViewport(const unsigned int& x, const unsigned int& y, const unsigned int& width, const unsigned int& height)
{
	if(width <= 0 || height <= 0)
		return false;

	D3DVIEWPORT9 tmp;
	memset(&tmp, 0, sizeof(D3DVIEWPORT9));
	tmp.X = x;
	tmp.Y = y;
	tmp.Width = width;
	tmp.Height = height;

	if(FAILED(m_pD3DDev->SetViewport(&tmp)))
		return false;
		
	memcpy(&m_previousViewport, &m_currentViewport, sizeof(D3DVIEWPORT9));
	memcpy(&m_currentViewport, &tmp, sizeof(D3DVIEWPORT9));
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// GetCurrentViewport
//
// Packs the parameters with the current viewport information
void CQuadrionRender::GetCurrentViewport(unsigned int& x, unsigned int& y, unsigned int& width, unsigned int& height)
{
// 	D3DVIEWPORT9 tmp;
// 	memset(&tmp, 0, sizeof(D3DVIEWPORT9));
// 	m_pD3DDev->GetViewport(&tmp);
// 	
// 	x = tmp.X;
// 	y = tmp.Y;
// 	width = tmp.Width;
// 	height = tmp.Height;
	
	x = m_currentViewport.X;
	y = m_currentViewport.Y;
	width = m_currentViewport.Width;
	height = m_currentViewport.Height;
}


///////////////////////////////////////////////////////////////////
// ResetViewport
// 
// Resets the viewport to the one last valid viewport
bool CQuadrionRender::ResetViewport()
{
	if(m_previousViewport.Width <= 0 || m_previousViewport.Height <= 0)
		return false;

	memcpy(&m_currentViewport, &m_previousViewport, sizeof(D3DVIEWPORT9));
	memset(&m_previousViewport, 0, sizeof(D3DVIEWPORT9));
	
	if(FAILED(m_pD3DDev->SetViewport(&m_currentViewport)))
		return false;
	
	return true;
}


////////////////////////////////////////////////////////////////////////
// ChangeCullMode
// mode- one of the QRENDER_CULL_ flags
//
// Changes the current cull mode to the one specified by mode
bool CQuadrionRender::ChangeCullMode(const unsigned int& mode)
{
	if(mode == m_currentCullMode)
		return true;
	
	if(mode == QRENDER_CULL_DEFAULT)
	{
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW)))
			return false;
		
		m_currentCullMode = QRENDER_CULL_CCW;
		return true;
	}
	
	else
	{
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_CULLMODE, (mode == QRENDER_CULL_NONE) ? D3DCULL_NONE : (mode == QRENDER_CULL_CCW) ? D3DCULL_CCW : D3DCULL_CW)))
			return false;
		
		m_currentCullMode = mode;
		return true;
	}
}


////////////////////////////////////////////////////////////////////////////////
// ChangeDepthMode
// mode- one of the QRENDER_ZBUFFER_ flags
//
// CHanges the current z-buffering state to the one specified by mode
bool CQuadrionRender::ChangeDepthMode(const unsigned int& mode)
{
//	if(mode == m_currentDepthMode)
//		return true;
	
	if(mode == QRENDER_ZBUFFER_ENABLEWRITE && !m_bZWriteEnabled)
	{
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, true)))
			return false;
		
		m_bZWriteEnabled = true;
		return true;
	}	
	
	else if(mode == QRENDER_ZBUFFER_DISABLEWRITE && m_bZWriteEnabled)
	{
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, false)))
			return false;
		
		m_bZWriteEnabled = false;
		return true;
	}
	
	else if(mode == QRENDER_ZBUFFER_DEFAULT)
	{
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, false)))
			return false;
		
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL)))
			return false;
		
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE)))
			return false;
		
		m_bZWriteEnabled = false;
		m_currentDepthMode = QRENDER_ZBUFFER_DEFAULT;
	}
	
	else if(mode == QRENDER_ZBUFFER_DISABLE)
	{
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ZWRITEENABLE, false)))
			return false;
		
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE)))
			return false;
		
		m_bZWriteEnabled = false;
		m_currentDepthMode = QRENDER_ZBUFFER_DISABLE;
	}
	
	else
	{
		if(mode == m_currentDepthMode)
			return false;
	
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE)))
			return false;
		
		D3DCMPFUNC zfunc = D3DCMP_NEVER;
		switch(mode)
		{
			case QRENDER_ZBUFFER_NEVER:
				zfunc = D3DCMP_NEVER;
				break;
			
			case QRENDER_ZBUFFER_LESS:
				zfunc = D3DCMP_LESS;
				break;
			
			case QRENDER_ZBUFFER_LEQUAL:
				zfunc = D3DCMP_LESSEQUAL;
				break;
			
			case QRENDER_ZBUFFER_EQUAL:
				zfunc = D3DCMP_EQUAL;
				break;
			
			case QRENDER_ZBUFFER_GEQUAL:
				zfunc = D3DCMP_GREATEREQUAL;
				break;
			
			case QRENDER_ZBUFFER_GREATER:
				zfunc = D3DCMP_GREATER;
				break;
			
			case QRENDER_ZBUFFER_NOTEQUAL:
				zfunc = D3DCMP_NOTEQUAL;
				break;
			
			case QRENDER_ZBUFFER_ALWAYS:
				zfunc = D3DCMP_ALWAYS;
				break;
				
			default:
				break;
		}
		
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ZFUNC, zfunc)))
			return false;
		
		m_currentDepthMode = mode;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////
// ChangeDepthBias
bool CQuadrionRender::ChangeDepthBias(const float bias)
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_DEPTHBIAS, F2DW(bias))))
		return false;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// ChangeSlopeBias
bool CQuadrionRender::ChangeSlopeBias(const float bias)
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(bias))))
		return false;
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// EnableAlphaBlending
// Enables alpha blending
bool CQuadrionRender::EnableAlphaBlending()
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true)))
		return false;
	
///*temp*/	m_pD3DDev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	
	m_bAlphaBlendingEnabled = true;
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// DisableAlphaBlending
// Disables alpha blending
bool CQuadrionRender::DisableAlphaBlending()
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, false)))
		return false;
	
	m_bAlphaBlendingEnabled = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////////////
// ChangeAlphaBlendMode
// src- source blend mode. One of QRENDER_ALPHABLEND_
// dest- destination blend mode. One of QRENDER_ALPHABLEND_
//
// Changes the current alpha blending mode to that described by dest & src
bool CQuadrionRender::ChangeAlphaBlendMode(const unsigned int& src, const unsigned int& dest)
{
	if(src == m_currentSourceAlpha && dest == m_currentDestAlpha)
		return true;

	D3DBLEND d, s;
	
	switch(src)
	{
		case 0x00200001:
			s = D3DBLEND_ZERO;
			break;
		
		case 0x00200002:
			s = D3DBLEND_ONE;
			break;
		
		case 0x00200003:
			s = D3DBLEND_SRCCOLOR;
			break;
		
		case 0x00200004:
			s = D3DBLEND_INVSRCCOLOR;
			break;
		
		case 0x00200005:
			s = D3DBLEND_INVSRCALPHA;
			break;
		
		case 0x00200006:
			s = D3DBLEND_SRCALPHA;
			break;
		
		case 0x00200007:
			s = D3DBLEND_DESTALPHA;
			break;
		
		case 0x00200008:
			s = D3DBLEND_DESTCOLOR;
			break;
	}		
	
	switch(dest)
	{
		case 0x00200001:
			d = D3DBLEND_ZERO;
			break;
		
		case 0x00200002:
			d = D3DBLEND_ONE;
			break;
		
		case 0x00200003:
			d = D3DBLEND_SRCCOLOR;
			break;
		
		case 0x00200004:
			d = D3DBLEND_INVSRCCOLOR;
			break;
		
		case 0x00200005:
			d = D3DBLEND_INVSRCALPHA;
			break;
		
		case 0x00200006:
			d = D3DBLEND_SRCALPHA;
			break;

		case 0x00200007:
			s = D3DBLEND_DESTALPHA;
			break;

		case 0x00200008:
			d = D3DBLEND_DESTCOLOR;
			break;	
	}
	
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_SRCBLEND, s)))
		return false;
	
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_DESTBLEND, d)))
		return false;
	
	m_currentSourceAlpha = src;
	m_currentDestAlpha = dest;
	
	return true;
}


//////////////////////////////////////////////////////////////////////////
// EnableStencilTest
bool CQuadrionRender::EnableStencilTest()
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_STENCILENABLE, true)))
		return false;
	
	m_bStencilTestEnabled = true;
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// DisableStencilTest
bool CQuadrionRender::DisableStencilTest()
{
	if(m_bStencilTestEnabled)
		if(FAILED(m_pD3DDev->SetRenderState(D3DRS_STENCILENABLE, false)))
			return false;
	
	m_bStencilTestEnabled = false;
	return false;
}


bool CQuadrionRender::SetStencilTest(const unsigned int& test)
{
	if(test == m_currentStencilFunc || !m_bStencilTestEnabled)
		return false;
	
	D3DCMPFUNC cmpfunc = D3DCMP_NEVER;
	switch(test)
	{
		case QRENDER_STENCIL_NEVER:
			cmpfunc = D3DCMP_NEVER;
			break;
		
		case QRENDER_STENCIL_LESS:
			cmpfunc = D3DCMP_LESS;
			break;
		
		case QRENDER_STENCIL_LEQUAL:
			cmpfunc = D3DCMP_LESSEQUAL;
			break;
		
		case QRENDER_STENCIL_EQUAL:
			cmpfunc = D3DCMP_EQUAL;
			break;
		
		case QRENDER_STENCIL_GEQUAL:
			cmpfunc = D3DCMP_GREATEREQUAL;
			break;
		
		case QRENDER_STENCIL_GREATER:
			cmpfunc = D3DCMP_GREATER;
			break;
		
		case QRENDER_STENCIL_ALWAYS:
			cmpfunc = D3DCMP_ALWAYS;
			break;
		
		case QRENDER_STENCIL_NOTEQUAL:
			cmpfunc = D3DCMP_NOTEQUAL;
			break;
		
		default:
			return false;
	}
	
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_STENCILFUNC, cmpfunc)))
		return false;
	
	m_currentStencilFunc = test;
	return true;
}

bool CQuadrionRender::SetStencilRef(const int& ref)
{
	if(!m_bStencilTestEnabled)
		return false;
	
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_STENCILREF, ref)))
		return false;
	
	return true;
}


bool CQuadrionRender::SetStencilMask(const int& mask)
{
	if(!m_bStencilTestEnabled)
		return false;
	
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_STENCILMASK, mask)))
		return false;
	
	return true;
}

bool CQuadrionRender::SetStencilAction(const unsigned int& stencilevent, const unsigned int& action)
{
	if(!m_bStencilTestEnabled)
		return false;
	
	D3DRENDERSTATETYPE rs = D3DRS_FORCE_DWORD;
	D3DSTENCILOP op = D3DSTENCILOP_FORCE_DWORD;
	
	switch(stencilevent)
	{
		case QRENDER_STENCIL_FAIL:
			rs = D3DRS_STENCILFAIL;
			break;
		
		case QRENDER_STENCIL_ZFAIL:
			rs = D3DRS_STENCILZFAIL;
			break;
		
		case QRENDER_STENCIL_PASS:
			rs = D3DRS_STENCILPASS;
			break;
		
		case QRENDER_STENCIL_WRITEMASK:
			rs = D3DRS_STENCILWRITEMASK;
			break;
		
		default:
			return false;
	}
	
	switch(action)
	{
		case QRENDER_STENCIL_KEEP:
			op = D3DSTENCILOP_KEEP;
			break;
		
		case QRENDER_STENCIL_ZERO:
			op = D3DSTENCILOP_ZERO;
			break;
		
		case QRENDER_STENCIL_REPLACE:
			op = D3DSTENCILOP_REPLACE;
			break;
		
		case QRENDER_STENCIL_INCREMENT_SATURATE:
			op = D3DSTENCILOP_INCRSAT;
			break;
		
		case QRENDER_STENCIL_DECREMENT_SATURATE:
			op = D3DSTENCILOP_DECRSAT;
			break;
		
		case QRENDER_STENCIL_INVERT:
			op = D3DSTENCILOP_INVERT;
			break;
		
		case QRENDER_STENCIL_INCREMENT:
			op = D3DSTENCILOP_INCR;
			break;
		
		case QRENDER_STENCIL_DECREMENT:
			op = D3DSTENCILOP_DECR;
			break;
		
		default:
			return false;
	}
	
	if(FAILED(m_pD3DDev->SetRenderState(rs, op)))
		return false;
	
	return true;
}


void CQuadrionRender::SetStencilDefaults()
{
	m_pD3DDev->SetRenderState(D3DRS_STENCILENABLE, false);
	m_pD3DDev->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	m_pD3DDev->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	m_pD3DDev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
	m_pD3DDev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	m_pD3DDev->SetRenderState(D3DRS_STENCILREF, 0);
	m_pD3DDev->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
	m_pD3DDev->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
}


////////////////////////////////////////////////////////////////////////
// EnableScissorTest
bool CQuadrionRender::EnableScissorTest()
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, true)))
		return false;
	
	m_bScissorTestEnabled = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
// DisableScissorTest
bool CQuadrionRender::DisableScissorTest()
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, false)))
		return false;
	
	m_bScissorTestEnabled = false;
	return true;
}


//////////////////////////////////////////////////////////////////
// SetScissorRect
// rect- RECT object describing screenspace area for scissor test
//
// Set scissor testing area in screenspace 
bool CQuadrionRender::SetScissorRect(const RECT& rect)
{
	if(FAILED(m_pD3DDev->SetScissorRect(&rect)))
		return false;
	
	return true;
}



/////////////////////////////////////////////////////////////////////////////////
// RenderIndexedList
// mode- one of QRENDER_PRIM_ rendering modes
// startIndex- index into index list of the starting index
// baseVertex- index of base vertex
// nVertices- number of vertices in list
// nIndices- number of indices in list
//
// Renders an indexed primitive from the currently bound vertex format, vertex buffer
// and index buffer
bool CQuadrionRender::RenderIndexedList(const unsigned int& mode, const unsigned int& startIndex, unsigned int baseVertex,			\
										unsigned int nVertices, unsigned int nIndices, unsigned int minVertexIndex)
{
//	if(nVertices <= 0 || baseVertex < 0)
//	{
//		nVertices = unsigned int(vertexBufferList[currentVertexBuffer[0]]->bufferSize / vertexFormatList[currentVertexFormat]->vertexSize[0]);
//		baseVertex = 0;
//	}		
	
//	if(nIndices <= 0)
//	{
//		nIndices = indexBufferList[currentIndexBuffer]->indexCount - startIndex;
//	}
	
	unsigned int triCount;
	D3DPRIMITIVETYPE primMode;
	
	if(mode == QRENDER_PRIM_TRIANGLES) triCount = nIndices / 3;
	if(mode == QRENDER_PRIM_TRIANGLE_STRIP || mode == QRENDER_PRIM_TRIANGLE_FAN) triCount = nIndices - 2;
	(mode == QRENDER_PRIM_TRIANGLES) ? primMode = D3DPT_TRIANGLELIST : (mode == QRENDER_PRIM_TRIANGLE_STRIP) ? primMode = D3DPT_TRIANGLESTRIP : \
									   primMode = D3DPT_TRIANGLEFAN;
	
	if( mode == QRENDER_PRIM_LINES || mode == QRENDER_PRIM_LINE_STRIP )
	{
		primMode = D3DPT_LINESTRIP;
		triCount = nIndices - 2;
	}
	
	if(FAILED(m_pD3DDev->DrawIndexedPrimitive(primMode, baseVertex, minVertexIndex, nVertices, startIndex, triCount)))
		return false;
	
	return true;
}

bool CQuadrionRender::RenderIndexedList(const unsigned int& mode, const void* vPtr, unsigned int nVertices,       \
										const void* iPtr, unsigned int indexSize, unsigned int nIndices, unsigned int minVertexIndex)
{	
	unsigned int primCount = 0;
	unsigned int vertexSize = m_userFVFs[m_curFVF].size;
	D3DPRIMITIVETYPE primMode;
	D3DFORMAT idxFormat = (indexSize == 2) ? D3DFMT_INDEX16 : D3DFMT_INDEX32;
	
	
	if(mode == QRENDER_PRIM_TRIANGLES) primCount = nIndices / 3;
	if(mode == QRENDER_PRIM_LINES) primCount = nIndices - 1;
	if(mode == QRENDER_PRIM_TRIANGLE_STRIP || mode == QRENDER_PRIM_TRIANGLE_FAN) primCount = nIndices - 2;
	(mode == QRENDER_PRIM_TRIANGLES) ? primMode = D3DPT_TRIANGLELIST : (mode == QRENDER_PRIM_TRIANGLE_STRIP) ? primMode = D3DPT_TRIANGLESTRIP : \
									   (mode == QRENDER_PRIM_TRIANGLE_FAN) ? primMode = D3DPT_TRIANGLEFAN : primMode = D3DPT_LINESTRIP;
	
	if(FAILED(m_pD3DDev->DrawIndexedPrimitiveUP(primMode, minVertexIndex, nVertices, primCount, iPtr, idxFormat, vPtr, vertexSize)))
		return false;
	
	return true;
}


bool CQuadrionRender::RenderLine(const vec3f& a, const vec3f& b, const unsigned int& color)
{
	SColoredVertex vlist[2];

	vlist[0].x = a.x;
	vlist[0].y = a.y;
	vlist[0].z = a.z;
	vlist[0].color = color;
	vlist[1].x = b.x;
	vlist[1].y = b.y;
	vlist[1].z = b.z;
	vlist[1].color = color;
	
	if(FAILED(m_pD3DDev->SetFVF(ColoredVertexFVF)))
		return false;

	if(FAILED(m_pD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, vlist, sizeof(SColoredVertex))))
		return false;
	
	return true;
}


bool CQuadrionRender::RenderLine(const vec3f& a, const vec3f& b, unsigned int startColor, unsigned int endColor)
{
	SColoredVertex vList[2];
	
	vList[0].x = a.x;
	vList[0].y = a.y;
	vList[0].z = a.z;
	vList[0].color = startColor;
	vList[1].x = b.x;
	vList[1].y = b.y;
	vList[1].z = b.z;
	vList[1].color = endColor;
	
	if(FAILED(m_pD3DDev->SetFVF(ColoredVertexFVF)))
		return false;
	
	if(FAILED(m_pD3DDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, vList, sizeof(SColoredVertex))))
		return false;
	
	return true;
}


bool CQuadrionRender::RenderTriangle(const vec3f& a, const vec3f& b, const vec3f& c, unsigned int color)
{
	SColoredVertex vList[3];
	
	memcpy(&vList[0], &a, sizeof(vec3f));
	memcpy(&vList[1], &b, sizeof(vec3f));
	memcpy(&vList[2], &c, sizeof(vec3f));
	
	vList[0].color = color;
	vList[1].color = color;
	vList[2].color = color;
	
	if(FAILED(m_pD3DDev->SetFVF(ColoredVertexFVF)))
		return false;
	
	if(FAILED(m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vList, sizeof(SColoredVertex))))
		return false;
	
	return true;
}


static unsigned short g_boxList[36] = {0, 2, 1, 0, 3, 2, 3, 7, 6, 3, 6, 2, 7, 4, 5, 7, 5, 6, 4, 0, 1, 4, 1, 5, 4, 7, 3, 4, 3, 0, 1, 2, 6, 1, 6, 5};

bool CQuadrionRender::RenderBox(const vec3f& min, const vec3f& max, unsigned int color)
{
	SColoredVertex vList[8];
	
	vList[0].x = min.x; vList[0].y = min.y; vList[0].z = min.z;
	vList[1].x = min.x; vList[1].y = max.y; vList[1].z = min.z;
	vList[2].x = max.x; vList[2].y = max.y; vList[2].z = min.z;
	vList[3].x = max.x; vList[3].y = min.y; vList[3].z = min.z;
	
	vList[4].x = min.x; vList[4].y = min.y; vList[4].z = max.z;
	vList[5].x = min.x; vList[5].y = max.y; vList[5].z = max.z;
	vList[6].x = max.x; vList[6].y = max.y; vList[6].z = max.z;
	vList[7].x = max.x; vList[7].y = min.y; vList[7].z = max.z;
	
	for(int i = 0; i < 8; ++i)
		vList[i].color = color;
	
	if(FAILED(m_pD3DDev->SetFVF(ColoredVertexFVF)))
		return false;
	
	if(FAILED(m_pD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 12, g_boxList, D3DFMT_INDEX16, vList, sizeof(SColoredVertex))))
		return false;
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
// RenderLineStrip
// start- starting index of line strip
// nLines- number of lines in line strip to render
//
// Renders a line list from the currently bound vertex buffer
bool CQuadrionRender::RenderLineStrip(const unsigned int& start, const unsigned int& nLines)
{
	if(FAILED(m_pD3DDev->DrawPrimitive(D3DPT_LINESTRIP, start, nLines)))
		return false;
	
	return true;	
}



////////////////////////////////////////////////////////////////////////////////////////
// RenderTriangleStrip
// start- starting index of triangle strip
// nTris- number of triangles in strip
//
// Renders a triangle strip from the currently bound vertex buffer
bool CQuadrionRender::RenderTriangleStrip(const unsigned int& start, const unsigned int& nTris)
{
	if(FAILED(m_pD3DDev->DrawPrimitive(D3DPT_TRIANGLELIST, start, nTris)))
		return false;
	
	return true;	
}


void CQuadrionRender::RenderSphere()
{
	m_pSphereObject->DrawSubset(0);
}

void CQuadrionRender::RenderCone()
{
	m_pConeObject->DrawSubset(0);
}

int CQuadrionRender::MakeVertexFormat(const unsigned int& vFlags)
{
	unsigned int size = 12;
	SUserFVF uFVF;
	if(!(vFlags & QRENDER_VERTEX_POSITION))
		return QRENDER_INVALID_HANDLE;
	
	unsigned long fvf = D3DFVF_XYZ;
	if(vFlags & QRENDER_VERTEX_NORMAL) 
	{
		fvf |= D3DFVF_NORMAL;
		size += 12;
	}
	
	if(vFlags & QRENDER_VERTEX_COLOR)  
	{
		fvf |= D3DFVF_DIFFUSE;
		size += 4;
	}
	
	int nTexCoords = 0;
	if(vFlags & QRENDER_VERTEX_TEXCOORD0) ++nTexCoords;
	if(vFlags & QRENDER_VERTEX_TEXCOORD1) ++nTexCoords;
	if(vFlags & QRENDER_VERTEX_TEXCOORD2) ++nTexCoords;
	if(vFlags & QRENDER_VERTEX_TEXCOORD3) ++nTexCoords;
	size += nTexCoords * 8;
	
	if(nTexCoords == 1) fvf |= D3DFVF_TEX1;
	else if(nTexCoords == 2) fvf |= D3DFVF_TEX2;
	else if(nTexCoords == 3) fvf |= D3DFVF_TEX3;
	else if(nTexCoords == 4) fvf |= D3DFVF_TEX4;
	else fvf = fvf;
	
	uFVF.fvf = fvf;
	uFVF.size = size;
	m_userFVFs.push_back(uFVF);
	return m_userFVFs.size() - 1;
}


bool CQuadrionRender::BindVertexFormat(const int& handle)
{
	if(!QRENDER_IS_VALID(handle))
		return false;
	
	if(QRENDER_IS_DEFAULT(handle))
	{
		m_pD3DDev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		return true;
	}
	
	m_pD3DDev->SetFVF(m_userFVFs[handle].fvf);
	m_curFVF = handle;
	
	return true;
}



bool CQuadrionRender::RenderScreenSpaceQuad(const cTextureRect& texCoords, const vec2f& upperLeft, const vec2f& widthHeight)
{
	// Adjust by half pixel //
	float widthOffset, heightOffset, ulxOffset, ulyOffset;
	ulxOffset    = upperLeft.x - 0.5F;
	ulyOffset    = upperLeft.y - 0.5F;
	widthOffset  = ulxOffset + widthHeight.x;
	heightOffset = ulyOffset + widthHeight.y;
	
	
	// Create quad //
	STransformedTexturedVertex quad[4];
	quad[0].pos = D3DXVECTOR4(ulxOffset, ulyOffset, 0.5F, 1.0F);
	quad[0].texCoords = D3DXVECTOR2(texCoords.leftU, texCoords.topV);
	quad[1].pos = D3DXVECTOR4(widthOffset, ulyOffset, 0.5F, 1.0F);
	quad[1].texCoords = D3DXVECTOR2(texCoords.rightU, texCoords.topV);
	quad[2].pos = D3DXVECTOR4(ulxOffset, heightOffset, 0.5F, 1.0F);
	quad[2].texCoords = D3DXVECTOR2(texCoords.leftU, texCoords.bottomV);
	quad[3].pos = D3DXVECTOR4(widthOffset, heightOffset, 0.5F, 1.0F);
	quad[3].texCoords = D3DXVECTOR2(texCoords.rightU, texCoords.bottomV);
	
	// Set Flexible Vertex Format //
	if(FAILED(m_pD3DDev->SetFVF(TransformedTexturedVertexFVF)))
		return false;
	
	// Render quad //
	if(FAILED(m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(STransformedTexturedVertex))))
		return false;
	
	return true;
}


bool CQuadrionRender::RenderScreenSpaceQuad(const unsigned int color, const vec2f& upperLeft, const vec2f& widthHeight)
{
	// Adjust by half pixel //
	float widthOffset, heightOffset, ulxOffset, ulyOffset;
	ulxOffset    = upperLeft.x - 0.5F;
	ulyOffset    = upperLeft.y - 0.5F;
	widthOffset  = ulxOffset + (widthHeight.x);// - 0.5F);
	heightOffset = ulyOffset + (widthHeight.y);// - 0.5F);

	
	// Create quad //
	STransformedColoredVertex quad[4];
	quad[0].pos = D3DXVECTOR4(ulxOffset, ulyOffset, 0.5F, 1.0F);
	quad[0].color = color;
	quad[1].pos = D3DXVECTOR4(widthOffset, ulyOffset, 0.5F, 1.0F);
	quad[1].color = color;
	quad[2].pos = D3DXVECTOR4(ulxOffset, heightOffset, 0.5F, 1.0F);
	quad[2].color = color;
	quad[3].pos = D3DXVECTOR4(widthOffset, heightOffset, 0.5F, 1.0F);
	quad[3].color = color;
	
	// Set Flexible Vertex Format //
	if(FAILED(m_pD3DDev->SetFVF(TransformedColoredVertexFVF)))
		return false;
	
	// Render quad //
	if(FAILED(m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(STransformedColoredVertex))))
		return false;
	
	return true;		
}

bool CQuadrionRender::RenderScreenSpaceQuad( const unsigned int color, const cTextureRect& texCoords, const vec2f& upperLeft, const vec2f& widthHeight )
{
	float widthOffset, heightOffset, ulxOffset, ulyOffset;
	ulxOffset    = upperLeft.x - 0.5F;
	ulyOffset    = upperLeft.y - 0.5F;
	widthOffset  = ulxOffset + (widthHeight.x);// - 0.5F);
	heightOffset = ulyOffset + (widthHeight.y);// - 0.5F);

	
	// Create quad //
	STransformedTexturedColoredVertex quad[4];
	quad[0].pos = D3DXVECTOR4(ulxOffset, ulyOffset, 0.5F, 1.0F);
	quad[0].color = color;
	quad[0].texCoords = D3DXVECTOR2( texCoords.leftU, texCoords.topV );
	quad[1].pos = D3DXVECTOR4(widthOffset, ulyOffset, 0.5F, 1.0F);
	quad[1].color = color;
	quad[1].texCoords = D3DXVECTOR2( texCoords.rightU, texCoords.topV );
	quad[2].pos = D3DXVECTOR4(ulxOffset, heightOffset, 0.5F, 1.0F);
	quad[2].color = color;
	quad[2].texCoords = D3DXVECTOR2( texCoords.leftU, texCoords.bottomV );
	quad[3].pos = D3DXVECTOR4(widthOffset, heightOffset, 0.5F, 1.0F);
	quad[3].color = color;
	quad[3].texCoords = D3DXVECTOR2( texCoords.rightU, texCoords.bottomV );
	
	// Set Flexible Vertex Format //
	if(FAILED(m_pD3DDev->SetFVF(TransformedTexturedColoredVertexFVF)))
		return false;
	
	// Render quad //
	if(FAILED(m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(STransformedTexturedColoredVertex))))
		return false;
	
	return true;			
}

bool CQuadrionRender::RenderQuad( const STexturedQuad& quad )
{
	STexturedVertex verts[4];
	verts[0].pos		= D3DXVECTOR3( quad.pos[0].x, quad.pos[0].y, quad.pos[0].z );
	verts[0].texCoords  = D3DXVECTOR2( quad.texcoords[0].x, quad.texcoords[0].y );
	verts[1].pos		= D3DXVECTOR3( quad.pos[1].x, quad.pos[1].y, quad.pos[1].z );
	verts[1].texCoords  = D3DXVECTOR2( quad.texcoords[1].x, quad.texcoords[1].y );
	verts[2].pos		= D3DXVECTOR3( quad.pos[2].x, quad.pos[2].y, quad.pos[2].z );
	verts[2].texCoords  = D3DXVECTOR2( quad.texcoords[2].x, quad.texcoords[2].y );
	verts[3].pos		= D3DXVECTOR3( quad.pos[3].x, quad.pos[3].y, quad.pos[3].z );
	verts[3].texCoords  = D3DXVECTOR2( quad.texcoords[3].x, quad.texcoords[3].y );
	
	if( FAILED( m_pD3DDev->SetFVF( TexturedVertexFVF ) ) )
		return false;
	
	if( FAILED( m_pD3DDev->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, verts, sizeof( STexturedVertex ) ) ) )
		return false;
	
	return true;
}

bool CQuadrionRender::RenderQuad( const STexturedNormalQuad& quad )
{
	STexturedNormalVertex verts[4];
	verts[0].pos		= D3DXVECTOR3( quad.pos[0].x, quad.pos[0].y, quad.pos[0].z );
	verts[0].texCoords  = D3DXVECTOR2( quad.texcoords[0].x, quad.texcoords[0].y );
	verts[0].norm		= D3DXVECTOR3( quad.norm[0].x, quad.norm[0].y, quad.norm[0].z );
	verts[1].pos		= D3DXVECTOR3( quad.pos[1].x, quad.pos[1].y, quad.pos[1].z );
	verts[1].texCoords  = D3DXVECTOR2( quad.texcoords[1].x, quad.texcoords[1].y );
	verts[1].norm		= D3DXVECTOR3( quad.norm[1].x, quad.norm[1].y, quad.norm[1].z );
	verts[2].pos		= D3DXVECTOR3( quad.pos[2].x, quad.pos[2].y, quad.pos[2].z );
	verts[2].texCoords  = D3DXVECTOR2( quad.texcoords[2].x, quad.texcoords[2].y );
	verts[2].norm		= D3DXVECTOR3( quad.norm[2].x, quad.norm[2].y, quad.norm[2].z );
	verts[3].pos		= D3DXVECTOR3( quad.pos[3].x, quad.pos[3].y, quad.pos[3].z );
	verts[3].texCoords  = D3DXVECTOR2( quad.texcoords[3].x, quad.texcoords[3].y );
	verts[3].norm		= D3DXVECTOR3( quad.norm[3].x, quad.norm[3].y, quad.norm[3].z );
	
	if( FAILED( m_pD3DDev->SetFVF( TexturedNormalVertexFVF ) ) )
		return false;

	if( FAILED( m_pD3DDev->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, verts, sizeof( STexturedNormalVertex ) ) ) )
		return false;

	return true;
}

bool CQuadrionRender::RenderFrustum(const vec2f& nearDims, const float& nearDist, const vec2f& farDims, const float& farDist,
									const vec3f& position, const vec3f& direction, const vec3f& strafe, const vec3f& up)
{
	SGeometryVertex verts[8];
	vec3f nc = position + direction * nearDist;
	vec3f fc = position + direction * farDist;
	
	vec3f nyvec(up * nearDims.y * 0.5F);
	vec3f nxvec(strafe * nearDims.x * 0.5F);
	vec3f fyvec(up * farDims.y * 0.5F);
	vec3f fxvec(strafe * farDims.x * 0.5F);
	
	vec3f n0 = nc - nyvec - nxvec;
	vec3f n1 = nc + nyvec - nxvec;
	vec3f n2 = nc + nyvec + nxvec;
	vec3f n3 = nc - nyvec + nxvec;
	
	vec3f f0 = fc - fyvec - fxvec;
	vec3f f1 = fc + fyvec - fxvec;
	vec3f f2 = fc + fyvec + fxvec;
	vec3f f3 = fc - fyvec + fxvec;
/*

	m_pD3DDev->SetFVF(GeometryFVF);
	verts[0].pos	= D3DXVECTOR3(&n0.x);
	verts[1].pos	= D3DXVECTOR3(&n2.x);
	verts[2].pos	= D3DXVECTOR3(&n1.x);
	verts[3].pos	= D3DXVECTOR3(&n0.x);
	verts[4].pos	= D3DXVECTOR3(&n3.x);
	verts[5].pos	= D3DXVECTOR3(&n2.x);

	verts[6].pos	= D3DXVECTOR3(&f0.x);
	verts[7].pos	= D3DXVECTOR3(&f2.x);
	verts[8].pos	= D3DXVECTOR3(&f1.x);
	verts[9].pos	= D3DXVECTOR3(&f0.x);
	verts[10].pos	= D3DXVECTOR3(&f3.x);
	verts[11].pos	= D3DXVECTOR3(&f2.x);

	verts[12].pos	= D3DXVECTOR3(&n3.x);
	verts[13].pos	= D3DXVECTOR3(&f3.x);
	verts[14].pos	= D3DXVECTOR3(&f2.x);
	verts[15].pos	= D3DXVECTOR3(&n3.x);
	verts[16].pos	= D3DXVECTOR3(&f2.x);
	verts[17].pos	= D3DXVECTOR3(&n2.x);

	verts[18].pos	= D3DXVECTOR3(&f0.x);
	verts[19].pos	= D3DXVECTOR3(&n0.x);
	verts[20].pos	= D3DXVECTOR3(&n1.x);
	verts[21].pos	= D3DXVECTOR3(&f0.x);
	verts[22].pos	= D3DXVECTOR3(&n1.x);
	verts[23].pos	= D3DXVECTOR3(&f1.x);

	verts[24].pos	= D3DXVECTOR3(&n2.x);
	verts[25].pos	= D3DXVECTOR3(&f2.x);
	verts[26].pos	= D3DXVECTOR3(&f1.x);
	verts[27].pos	= D3DXVECTOR3(&n2.x);
	verts[28].pos	= D3DXVECTOR3(&f1.x);
	verts[29].pos	= D3DXVECTOR3(&n1.x);
	
	verts[30].pos	= D3DXVECTOR3(&n0.x);
	verts[31].pos	= D3DXVECTOR3(&f0.x);
	verts[32].pos	= D3DXVECTOR3(&f3.x);
	verts[33].pos	= D3DXVECTOR3(&n0.x);
	verts[34].pos	= D3DXVECTOR3(&f3.x);
	verts[35].pos	= D3DXVECTOR3(&n3.x);

	m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, verts, sizeof(SGeometryVertex));


	return true;
*/
	verts[0].pos	= D3DXVECTOR3(&n0.x);
	verts[1].pos	= D3DXVECTOR3(&n1.x);
	verts[2].pos	= D3DXVECTOR3(&n2.x);
	verts[3].pos	= D3DXVECTOR3(&n3.x);
	verts[4].pos	= D3DXVECTOR3(&f0.x);
	verts[5].pos	= D3DXVECTOR3(&f1.x);
	verts[6].pos	= D3DXVECTOR3(&f2.x);
	verts[7].pos	= D3DXVECTOR3(&f3.x);

	if(FAILED(m_pD3DDev->SetFVF(GeometryFVF)))
		return false;

	unsigned int indices[36] = {0,3,2,0,2,1,7,4,5,7,5,6,3,7,6,3,6,2,4,0,1,4,1,5,2,6,5,2,5,1,0,4,7,0,7,3};

	if(FAILED(m_pD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 12, indices, D3DFMT_INDEX32, verts, sizeof(SGeometryVertex))))
		return false;
 
}

bool CQuadrionRender::SetMatrix(const unsigned int& type, const mat4& m)
{
	D3DXMATRIX M(m);
	D3DXMATRIX ID;
	HRESULT hr;
	
	switch(type)
	{
		case 0x00000001:
			hr = m_pD3DDev->SetTransform(D3DTS_WORLD, &M);
			break;
		
		case 0x00000002:	
			D3DXMatrixIdentity(&ID);
			hr = m_pD3DDev->SetTransform(D3DTS_WORLD, &ID);
			hr = m_pD3DDev->SetTransform(D3DTS_VIEW, &M);
			break;

		case 0x00000004:
			hr = m_pD3DDev->SetTransform(D3DTS_VIEW, &M);
			break;
		
		case 0x00000008:
			hr = m_pD3DDev->SetTransform(D3DTS_PROJECTION, &M);
			break;
		
		default:
			return false;
	}		
	
	if(hr != D3D_OK)
		return false;
	
	return true;
}


bool CQuadrionRender::MulMatrix(const unsigned int& type, const mat4& m)
{
	D3DXMATRIX M(m);
	D3DXMATRIX ID;
	HRESULT hr;
	
	switch(type)
	{
		case 0x00000001:
			hr = m_pD3DDev->MultiplyTransform(D3DTS_WORLD, &M);
			break;
		
		case 0x00000002:
			D3DXMatrixIdentity(&ID);
			hr = m_pD3DDev->SetTransform(D3DTS_WORLD, &ID);
			hr = m_pD3DDev->MultiplyTransform(D3DTS_VIEW, &M);
			break;
		
		case 0x00000004:
			hr = m_pD3DDev->MultiplyTransform(D3DTS_VIEW, &M);
			break;
		
		case 0x00000008:
			hr = m_pD3DDev->MultiplyTransform(D3DTS_PROJECTION, &M);
			break;
		
		default:
			return false;
	}
	
	if(hr != D3D_OK)
		return false;
	
	return true;
}


bool CQuadrionRender::GetMatrix(const unsigned int& type, mat4& m)
{
	D3DXMATRIX rm, view, proj, world;
	HRESULT hr;
	
	switch(type)
	{
		case 0x00000001:
			hr = m_pD3DDev->GetTransform(D3DTS_WORLD, &rm);
			break;
		
		case 0x00000002:
			hr = m_pD3DDev->GetTransform(D3DTS_WORLD, &world);
			if(hr == D3D_OK)
			{
				hr = m_pD3DDev->GetTransform(D3DTS_VIEW, &view);
				if(hr == D3D_OK)
					D3DXMatrixMultiply(&rm, &world, &view);
			}
			break;
		
		case 0x00000004:
			hr = m_pD3DDev->GetTransform(D3DTS_VIEW, &rm);
			break;
		
		case 0x00000008:
			hr = m_pD3DDev->GetTransform(D3DTS_PROJECTION, &rm);
			break;
		
		case 0x00000010:
			hr = m_pD3DDev->GetTransform(D3DTS_VIEW, &view);
			if(hr == D3D_OK)
			{
				hr = m_pD3DDev->GetTransform(D3DTS_PROJECTION, &proj);
				if(hr == D3D_OK)
					D3DXMatrixMultiply(&rm, &view, &proj);
			}
			break;
		
		case 0x00000020:
			hr = m_pD3DDev->GetTransform(D3DTS_WORLD, &world);
			if(hr == D3D_OK)
			{
				hr = m_pD3DDev->GetTransform(D3DTS_VIEW, &view);
				if(hr == D3D_OK)
				{
					hr = m_pD3DDev->GetTransform(D3DTS_PROJECTION, &proj);
					if(hr == D3D_OK)
					{
						D3DXMatrixMultiply(&rm, &view, &proj);
						D3DXMatrixMultiply(&rm, &world, &rm);
					}
				}
			}
			break;
			
		default:
			return false;
			
	}
	
	D3DXMatrixTranspose(&rm, &rm);
	m[0] = rm.m[0][0];		m[4] = rm.m[0][1];		m[8] = rm.m[0][2];		m[12] = rm.m[0][3];
	m[1] = rm.m[1][0];		m[5] = rm.m[1][1];		m[9] = rm.m[1][2];		m[13] = rm.m[1][3];
	m[2] = rm.m[2][0];		m[6] = rm.m[2][1];		m[10] = rm.m[2][2];		m[14] = rm.m[2][3];
	m[3] = rm.m[3][0];		m[7] = rm.m[3][1];		m[11] = rm.m[3][2];		m[15] = rm.m[3][3];
	
	return true;
}



int	CQuadrionRender::CreateFont(const unsigned int& height, const unsigned int& width, const unsigned int& weight, const bool& italic, LPCSTR fontName)
{
	LPD3DXFONT newFont;
	if(FAILED(D3DXCreateFont(m_pD3DDev, height, width, weight, 0, italic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,  	
						     DEFAULT_PITCH | FF_DONTCARE, _QT(fontName), &newFont)))
	{
		return QRENDER_INVALID_HANDLE;
	}
	
	else
	{
		m_fontList.push_back(newFont);
		return m_fontList.size() - 1;
	}
}

int	CQuadrionRender::CreateSprite()
{
	LPD3DXSPRITE newSprite;
	if(FAILED(D3DXCreateSprite(m_pD3DDev, &newSprite)))
		return QRENDER_INVALID_HANDLE;
	
	else
	{
		m_spriteList.push_back(newSprite);
		return m_spriteList.size() - 1;
	}
}

bool CQuadrionRender::RenderText(const int sprite, const int font, LPCSTR text, int offs, RECT* rect, int var, unsigned int color)
{
	if(font == QRENDER_INVALID_HANDLE)
		return false;
	
	D3DCOLOR fontColor = color;
	
	if(sprite != QRENDER_INVALID_HANDLE)
	{
		if(FAILED(m_spriteList[sprite]->Begin(D3DXSPRITE_ALPHABLEND)))
			return false;
		if(m_fontList[font]->DrawText(m_spriteList[sprite], _QT(text), offs, rect, DT_NOCLIP, fontColor) == 0)
		{
			m_spriteList[sprite]->End();
			return false;
		}
		if(FAILED(m_spriteList[sprite]->End()))
			return false;
	}
	
	else
	{
		if(strlen(text) <= 0)
			return false;
		if(m_fontList[font]->DrawText(NULL, _QT(text), offs, rect, var, fontColor) == 0)
			return false;
	}
	
	return true;
}


bool CQuadrionRender::EnableFSAA()
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, true)))
		return false;
	
	return true;
}

bool CQuadrionRender::DisableFSAA()
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, false)))
		return false;
	
	return true;
}



int CQuadrionRender::AddEffect(const std::string& name, const std::string& path)
{
	std::string		newName;	// Holds the new filename with the new extension

	if(name.empty() || path.empty())
		return -1;

	newName = name;

	// Attempt to load the provided format of the effect
	unsigned int handle = m_effectResources->AddResource(name, path);

	CQuadrionEffect* effect = m_effectResources->GetResource(handle);
	if(effect->GetRefCount() > 1)
		return handle;

	effect->ChangeRenderDevice(this);
	if(effect->CreateEffect(newName, path))
		return handle;
	else
	{
		int				periodLoc;	// Holds the location of the '.' in the filename
		std::string		cleanName;	// Holds the filename without an extension
		std::string		extension;	// Holds the file's extention

		// Find the '.'
		periodLoc = name.find(".");
		cleanName = name.substr(0, periodLoc);
		extension = name.substr(periodLoc + 1, (name.size() - periodLoc) - 1);

		// If it failed to load, attempt to load it's alternative format
		if(extension == "fxo")
			newName = cleanName + ".fx";
		else if(extension == "fx")
			newName = cleanName + ".fxo";
		else
			return AddEffect(cleanName + ".fx", path);

		// Attempt to load the file with the new extension
		handle = m_effectResources->AddResource(newName, path);
		effect = m_effectResources->GetResource(handle);
		if(effect->GetRefCount() > 1)
			return handle;

		effect->ChangeRenderDevice(this);
		if(effect->CreateEffect(newName, path))
			return handle;
		else
			return -1;		// If it's still a bad handle, return an error
	}
}

void CQuadrionRender::UnloadEffect(const std::string& name, const std::string& path)
{
	if(name.empty())
		return;
	
	CQuadrionEffect* fx = m_effectResources->GetResource(name, path);
	unsigned int handle = fx->GetHandle();
	m_effectResources->RemoveResource(handle);
}


CQuadrionEffect* CQuadrionRender::GetEffect(const std::string& name, const std::string& path)
{
	if(name.empty() || path.empty())
		return NULL;
	
	return m_effectResources->GetResource(name, path);
}



int CQuadrionRender::AddVertexBuffer(const std::string& name, const void* pVerts, const SQuadrionVertexDescriptor& desc, const int& nVerts, const bool& stream)
{
	std::string newname;
	if(name.empty())
	{
		CHAR p[4];
		itoa(m_vertexNameRef, p, 10);
		newname = p;
		++m_vertexNameRef;
	}
	
	else
		newname = name;
	
	unsigned int handle = m_vertexBufferResources->AddResource(newname);
	if(handle >= 0)
	{
		CQuadrionVertexBuffer* vb = m_vertexBufferResources->GetResource(handle);
		if(vb->GetRefCount() > 1)
			return handle;
		vb->ChangeRenderDevice(this);
		if(vb->CreateVertexBuffer(pVerts, desc, nVerts, stream))
			return handle;
	}
	
	return -1;
}

int CQuadrionRender::AddVertexBuffer(const std::string& name)
{
	std::string newname;
	if(name.compare(".") == 0)
	{
		CHAR p[4];
		itoa(m_vertexNameRef, p, 10);
		newname = p;
		++m_vertexNameRef;
	}
	
	else
		newname = name;
	
	unsigned int handle = m_vertexBufferResources->AddResource(newname);
	if(handle >= 0)
	{
		CQuadrionVertexBuffer* vbo = m_vertexBufferResources->GetResource(handle);
		if(vbo->GetRefCount() > 1)
			return handle;
		vbo->ChangeRenderDevice(this);
		return handle;
	}
	
	return -1;
}


int	CQuadrionRender::AddInstancedVertexBuffer(const std::string& name)
{
	std::string newname;
	if(name.compare(".") == 0)
	{
		CHAR p[4];
		itoa(m_instancedVertexNameRef, p, 10);
		newname = p;
		++m_vertexNameRef;
	}
	
	else
		newname = name;
	
	unsigned int handle = m_instancedVertexBufferResources->AddResource(newname);
	if(handle >= 0)
	{
		CQuadrionInstancedVertexBuffer* ivbo = m_instancedVertexBufferResources->GetResource(handle);
		if(ivbo->GetRefCount() > 1)
			return handle;
		
		ivbo->ChangeRenderDevice(this);
		return handle;
	}
	
	return -1;
}

int	CQuadrionRender::AddIndexBuffer(const std::string& name)
{
	std::string newname;
	if(name.compare(".") == 0)
	{
		CHAR p[4];
		itoa(m_indexNameRef, p, 10);
		newname = p;
		++m_indexNameRef;
	}
	
	else
		newname = name;
	
	unsigned int handle = m_indexBufferResources->AddResource(newname);
	if(handle >= 0)
	{
		CQuadrionIndexBuffer* ibo = m_indexBufferResources->GetResource(handle);
		if(ibo->GetRefCount() > 1)
			return handle;
		ibo->ChangeRenderDevice(this);
		return handle;
	}
	
	return -1;
}

int	CQuadrionRender::StreamTextureObject(unsigned int& flags, const std::string& name, const std::string& path)
{
	if(name.empty())
		return -1;
	
	unsigned int handle = m_textureObjectResources->AddResource(name, path);
	if(handle >= 0)
	{
		CQuadrionTextureObject* tex = m_textureObjectResources->GetResource(handle);
		std::ifstream	inp;
		std::string newPath;

		if(tex->GetRefCount() > 1 && (flags == tex->GetFlags()))
			return handle;
		else if(tex->GetRefCount() > 1 && (flags != tex->GetFlags()))
		{
			CQuadrionTextureFile *newtex = new CQuadrionTextureFile();
			//newtex->LoadTexture(name.c_str(), path.c_str());

			
			
			std::string newname = name;
			char p[4];
			itoa(m_textureNameRef, p, 10);
			newname.append(p);
			++m_textureNameRef;
			newtex->SetFileName(newname);
			newtex->SetFilePath(path);

			handle = m_textureObjectResources->AddResource(newname, path);

			cTMS::Instance()->AddTask(NULL, TMS_TYPE_NORMALMAPLOAD, (void*)newtex, flags, name);
			
			//return AddTextureObject(newtex, flags);
		}
		else
		{
			// Check if the file exists
			newPath = path + name;

			// Check if file exists
			/*struct stat my_stat;
			if(!(stat(newPath.c_str(), &my_stat) == 0))
			{
				// File doesn't exist, return error
				inp.clear(std::ios::failbit);
				m_textureObjectResources->RemoveResource(handle);
				return -1;
			}*/

			//inp.open(newPath.c_str(), std::ifstream::in);
			//inp.close();

			/*if(inp.fail())
			{
				// File doesn't exist, return error
				inp.clear(std::ios::failbit);
				m_textureObjectResources->RemoveResource(handle);
				return -1;
			}*/

			// If the file was found, add it to the TMS list to be loaded
			tex->ChangeRenderDevice(this);
			cTMS::Instance()->AddTask(NULL, TMS_TYPE_TEXTURELOAD, (void*)tex, flags);
		}
	}

	return handle;
}

int CQuadrionRender::StreamTextureObject(CQuadrionTextureFile& tex, unsigned int& flags)
{
	std::string fileName = tex.GetFileName();
	std::string filePath = tex.GetFilePath();
	int extloc = fileName.find(".");

	if(extloc > 0)
	{
		fileName = fileName.substr(0, fileName.size() - extloc);
	}

	
	unsigned int handle = m_textureObjectResources->AddResource(fileName, filePath);
	if(handle >= 0)
	{
		CQuadrionTextureObject* texobj = m_textureObjectResources->GetResource(handle);
		
		texobj->ChangeRenderDevice(this);
		if(!texobj->CreateTexture(tex, flags))
		{
			m_textureObjectResources->RemoveResource(handle);
			return -1;
		}
	}
	
	return handle;
}


int	CQuadrionRender::AddTextureObject(unsigned int& flags, const std::string& name, const std::string& path)
{
	if(name.empty())
		return -1;
	
	unsigned int handle = m_textureObjectResources->AddResource(name, path);
	if(handle >= 0)
	{

		//////////////////////////////////////
		CQuadrionTextureObject* tex = m_textureObjectResources->GetResource(handle);
		std::ifstream	inp;
		std::string newPath;

		if(tex->GetRefCount() > 1 && (flags == tex->GetFlags()))
			return handle;
		else if(tex->GetRefCount() > 1 && (flags != tex->GetFlags()))
		{
			CQuadrionTextureFile *newtex = new CQuadrionTextureFile();

			// Append the refference count to the name
			std::string		originalName;
			std::string		newname = name;
			char			p[4];

			itoa(m_textureNameRef, p, 10);
			newname.append(p);
			++m_textureNameRef;
			newtex->SetFileName(newname);
			newtex->SetFilePath(path);

			// Attain a handle for the texture (with the ref count appended)
			handle = m_textureObjectResources->AddResource(newname, path);
			originalName = name;//newtex->GetFileName(); // LoadTexture changes the fileName, so we need to store the old one here temporarily
							
			if(!newtex->LoadTexture(originalName.c_str(), newtex->GetFilePath().c_str()))
			{
				newname = name;
				newname.append(".dds");

				if(!newtex->LoadTexture(newname.c_str(), newtex->GetFilePath().c_str()))
				{
					newname = name;
					newname.append(".tga");

					if(!newtex->LoadTexture(newname.c_str(), newtex->GetFilePath().c_str()))
					{
						newname = name;
						newname.append(".jpg");

						newtex->LoadTexture(newname.c_str(), newtex->GetFilePath().c_str());
					}
				}
			}

			// Set the filename back to the original so AddTextureObject will get a proper handle
			newname = newtex->GetFileName();
			newtex->SetFileName(originalName);

			int result;
			result = AddTextureObject(*newtex, flags);
			newtex->SetFileName(newname);	// Now set it back
			return result;
		}
		else
		{
			tex->ChangeRenderDevice(this);
			if(!tex->CreateTextureFromFile(flags))
			{
				m_textureObjectResources->RemoveResource(handle);
				return -1;
			}
		}


		//////////////////////////////////////////////


		/*CQuadrionTextureObject* tex = m_textureObjectResources->GetResource(handle);
		if(tex->GetRefCount() > 1 && (flags == tex->GetFlags()))
			return handle;
		else if(tex->GetRefCount() > 1 && (flags != tex->GetFlags()))
		{
			CQuadrionTextureFile newtex;
			newtex.LoadTexture(name.c_str(), path.c_str());
			
			std::string newname = name;
			char p[4];
			itoa(m_textureNameRef, p, 10);
			newname.append(p);
			++m_textureNameRef;
			newtex.SetFileName(newname);
			return AddTextureObject(newtex, flags);
		}
		
		else
		{
			tex->ChangeRenderDevice(this);
			if(!tex->CreateTextureFromFile(flags))
			{
				m_textureObjectResources->RemoveResource(handle);
				return -1;
			}
		}*/
	}
	
	return handle;
}

int	CQuadrionRender::AddTextureObject(const void* dat, unsigned int& flags)
{
	std::string name;
	CHAR p[4];
	unsigned int handle;
	
	itoa(m_textureNameRef, p, 10);
	name = p;
	++m_textureNameRef;
		
	handle = m_textureObjectResources->AddResource(name);


	if(handle >= 0)
	{
		CQuadrionTextureObject* tex = m_textureObjectResources->GetResource(handle);
		if(tex->GetRefCount() > 1)
			return handle;
		
		tex->ChangeRenderDevice(this);
		if(!tex->CreateTextureFromData(flags, dat))
		{
			m_textureObjectResources->RemoveResource(handle);
			return -1;
		}
	}
			
	return handle;
}

int CQuadrionRender::AddTextureObject(CQuadrionTextureFile& tex, unsigned int& flags)
{
	std::string fileName = tex.GetFileName();
	std::string filePath = tex.GetFilePath();

	int extloc = fileName.find(".");

	if(extloc > 0)
	{
		fileName = fileName.substr(0, fileName.size() - extloc);
	}


	if(filePath == "")
		filePath = "./";

	unsigned int handle = m_textureObjectResources->AddResource(fileName, filePath);
	if(handle >= 0)
	{
		CQuadrionTextureObject* texobj = m_textureObjectResources->GetResource(handle);

		texobj->ChangeRenderDevice(this);

		/*if(texobj->GetRefCount() > 1)
			return handle;
		
		texobj->ChangeRenderDevice(this);*/
		if(!texobj->CreateTexture(tex, flags))
		{
			m_textureObjectResources->RemoveResource(handle);
			return -1;
		}
	}
	
	return handle;
}
	
void CQuadrionRender::UnloadTextureObject(const std::string& name, const std::string& path)
{
	if(name.empty())
		return;
	
	CQuadrionTextureObject* tex = m_textureObjectResources->GetResource(name, path);
	unsigned int handle = tex->GetHandle();
	m_textureObjectResources->RemoveResource(handle);
}


void CQuadrionRender::EvictTextures()
{
	for(unsigned int i = 0; i < QRENDER_MAX_TEXTURE_SAMPLERS; ++i)
	{
		if(m_currentTextures[i] >= 0)
		{
			if(m_currentTextureTypes[i] == -1)
				continue;
				
			else if(m_currentTextureTypes[i] != 0)
			{
				CQuadrionTextureObject* tex = m_textureObjectResources->GetResource(m_currentTextures[i]);
				tex->UnbindTexture();
			}
			
			else
			{
				CQuadrionTextureObject* tex = m_renderTargetResources->GetResource(m_currentTextures[i]);
				tex->UnbindTexture();
			}
		}
	}
}

int CQuadrionRender::GetIncrementalTextureRefference()
{
	// Return the refference number, then increment the value
	return m_textureNameRef++;
}

CQuadrionTextureObject* CQuadrionRender::GetTextureObject(const std::string& name, const std::string& path)
{
	CQuadrionTextureObject *tex = m_textureObjectResources->GetResource(name, path);

	if(tex->IsLoaded())
		return tex;
	else
		return m_textureObjectResources->GetResource(m_noShaderHandle);
}


CQuadrionTextureObject* CQuadrionRender::GetTextureObject(const unsigned int& handle)
{
	CQuadrionTextureObject *tex = m_textureObjectResources->GetResource(handle);
	
	if(tex && tex->IsLoaded())
		return tex;
	else
		return m_textureObjectResources->GetResource(m_noShaderHandle);
}


int	CQuadrionRender::AddRenderTarget( unsigned int flags, const unsigned int& w, const unsigned int& h, const ETexturePixelFormat& fmt,
									  bool msaa )
{
	std::string name;
	CHAR p[4];
	itoa(m_renderTargetNameRef, p, 10);
	name = p;
	++m_renderTargetNameRef;	
	
	unsigned int handle = m_renderTargetResources->AddResource(name);
	if(handle >= 0)
	{
		CQuadrionRenderTarget* tex = m_renderTargetResources->GetResource(handle);
		if(tex->GetRefCount() > 1)
			return handle;
		
		tex->ChangeRenderDevice(this);
		if( !tex->CreateRenderTarget( flags, w, h, fmt, msaa ) )
		{
			m_renderTargetResources->RemoveResource(handle);
			return -1;
		}
	}
			
	return handle;
}


void CQuadrionRender::EvictRenderTargets()
{
	for(unsigned int i = 0; i < QRENDER_MAX_RENDER_TARGETS; ++i)
	{
		if(m_currentRenderTargets[i] >= 0)
		{
			m_pD3DDev->SetRenderTarget(i, NULL);
		}
	}

	memset(m_currentRenderTargets, -1, sizeof(int) * QRENDER_MAX_RENDER_TARGETS);
}


void CQuadrionRender::EvictRenderTarget(const unsigned int& slot)
{
	if(slot > QRENDER_MAX_RENDER_TARGETS || slot == 0)
		return;
	
	m_pD3DDev->SetRenderTarget(slot, NULL);
	
	m_currentRenderTargets[slot] = -1;
}




void CQuadrionRender::SetDefaultRenderTarget()
{
	if(m_currentRenderTargets[0] != QRENDER_DEFAULT)
	{
		m_pD3DDev->SetRenderTarget(0, m_pFramebuffer);
		m_currentRenderTargets[0] = QRENDER_DEFAULT;
	}
}


int CQuadrionRender::AddDepthStencilTarget( const unsigned int &w, const unsigned int& h, const unsigned int& depthBits, 
										    const unsigned int& stencilBits, bool msaa )
{
	std::string name;
	CHAR p[4];
	itoa(m_depthStencilNameRef, p, 10);
	name = p;
	++m_depthStencilNameRef;
	
	unsigned int handle = m_depthStencilTargetResources->AddResource(name);
	if(handle >= 0)
	{
		CQuadrionDepthStencilTarget* ds = m_depthStencilTargetResources->GetResource(handle);
		if(ds->GetRefCount() > 1)
			return handle;
		
		ds->ChangeRenderDevice(this);
		if( !ds->CreateDepthStencilTarget( w, h, depthBits, stencilBits, msaa ) )
		{
			m_depthStencilTargetResources->RemoveResource(handle);
			return -1;
		}
	}
			
	return handle;
}


void CQuadrionRender::EvictDepthStencilTarget()
{
	m_pD3DDev->SetDepthStencilSurface(NULL);
	m_currentDepthStencilTarget = -1;
}


void CQuadrionRender::SetDefaultDepthStencilTarget()
{
	if(m_currentDepthStencilTarget != QRENDER_DEFAULT)
	{
		m_pD3DDev->SetDepthStencilSurface(m_pDepthStencilBuffer);
		m_currentDepthStencilTarget = QRENDER_DEFAULT;
	}
}

bool CQuadrionRender::ClearBuffers(const unsigned int& targets, const unsigned int& color, const float& depth, const DWORD& stencil)
{
	DWORD flags = 0;
	if(targets & QRENDER_CLEAR_BACKBUFFER) flags |= D3DCLEAR_TARGET;
	if(targets & QRENDER_CLEAR_ZBUFFER) flags |= D3DCLEAR_ZBUFFER;
	if(targets & QRENDER_CLEAR_STENCILBUFFER) flags |= D3DCLEAR_STENCIL;
	
	if(FAILED(m_pD3DDev->Clear(0, NULL, flags, (D3DCOLOR)color, depth, stencil)))
		return false;
	
	return true;	
}

bool CQuadrionRender::SetFrustumClipPlanes( const float* planes )
{

	if( !planes )	
		return false;
	
	unsigned long curPlane = 0;
	unsigned int ptr = 0;	
	
	
	m_clipPlaneMask |= ( D3DCLIPPLANE0 | D3DCLIPPLANE1 | D3DCLIPPLANE2 | D3DCLIPPLANE3 | D3DCLIPPLANE4 | D3DCLIPPLANE5 );
	if( FAILED( m_pD3DDev->SetRenderState( D3DRS_CLIPPLANEENABLE, m_clipPlaneMask ) ) ) 
		return false;

	for( unsigned int i = 0; i < 6; ++i )
	{
		curPlane = i;		
		float plane[4];
		memcpy( plane, planes + ptr, sizeof(float) * 4 );
		if( FAILED( m_pD3DDev->SetClipPlane( curPlane, plane ) ) )
			return false;

		ptr += 4;
	}	

	return true;
}


void CQuadrionRender::DisableFrustumCulling()
{
	m_clipPlaneMask ^= D3DCLIPPLANE0;
	m_clipPlaneMask ^= D3DCLIPPLANE1;
	m_clipPlaneMask ^= D3DCLIPPLANE2;
	m_clipPlaneMask ^= D3DCLIPPLANE3;
	m_clipPlaneMask ^= D3DCLIPPLANE4;
	m_clipPlaneMask ^= D3DCLIPPLANE5;
	m_pD3DDev->SetRenderState( D3DRS_CLIPPLANEENABLE, m_clipPlaneMask );
}


void CQuadrionRender::GetCurrentMSAA( D3DMULTISAMPLE_TYPE &msType, unsigned int& qual )
{
	msType = csaaFormats[m_currentMSAA];
	
	if( m_deviceCaps.vendor == QRENDER_DEVICE_NVIDIA )
		qual   = csaaQualities[m_currentMSAA];
	
	else
		qual = 0;
}


bool CQuadrionRender::DisableColorWrites()
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_COLORWRITEENABLE, 0)))
		return false;
	
	return true;
}

bool CQuadrionRender::EnableColorWrites()
{
	if(FAILED(m_pD3DDev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE)))
		return false;
	
	return true;
}



void CQuadrionRender::IssueOcclusionQuery()
{
	m_pOcclusionQuery->Issue(D3DISSUE_BEGIN);
}

unsigned int CQuadrionRender::CheckOcclusionQuery()
{
	m_pOcclusionQuery->Issue(D3DISSUE_END);

	unsigned int visiblePixels = 0;
	while(m_pOcclusionQuery->GetData((void*)&visiblePixels, sizeof(unsigned int), D3DGETDATA_FLUSH) == FALSE);
	
	return visiblePixels;
}