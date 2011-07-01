

#if defined (_DEBUG) || (DEBUG)
	#define D3D_DEBUG_INFO
#endif


#ifdef QRENDER_EXPORTS
#define QRENDEREXPORT_API __declspec(dllexport)
#else
#define QRENDEREXPORT_API __declspec(dllimport)
#endif



#define QRENDER_IS_VALID(x)		((x) >= 0)
#define QRENDER_IS_DEFAULT(x)	((x) == -2)
#define VERTEX_CACHE_SIZE		64


#include "qtexture.h"
#include "qmath.h"
#include "qeffect.h"
#include "qresource.h"
#include "qvertexbuffer.h"
#include "qindexbuffer.h"
#include "qtext.h"

#ifndef __QRENDER_H_
#define __QRENDER_H_



#include <windows.h>
#include <d3d9.h>	
#include <d3dx9.h>
#include <string>
#include <vector>


static inline DWORD F2DW(FLOAT f) { return *((DWORD*)&f); }  


 
// Pack pixel format //
QRENDEREXPORT_API unsigned int QRENDER_MAKE_RGBA(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
QRENDEREXPORT_API unsigned int QRENDER_MAKE_BGRA(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
QRENDEREXPORT_API unsigned int QRENDER_MAKE_ARGB(unsigned char a, unsigned char r, unsigned char g, unsigned char b);



// Object defines //
const int				QRENDER_INVALID_HANDLE		= -1;
const int				QRENDER_DEFAULT				= -2;



// Render and Object State //
const unsigned int		QRENDER_CLEAR_BACKBUFFER =			0x10000000;
const unsigned int		QRENDER_CLEAR_ZBUFFER =				0x20000000;
const unsigned int		QRENDER_CLEAR_STENCILBUFFER =		0x40000000;
const unsigned int		QRENDER_CLEAR_ACCUMBUFFER =			0x80000000;

const unsigned int		QRENDER_MATRIX_MODEL					= 0x00000001;
const unsigned int		QRENDER_MATRIX_MODELVIEW				= 0x00000002;
const unsigned int		QRENDER_MATRIX_VIEW						= 0x00000004;
const unsigned int		QRENDER_MATRIX_PROJECTION				= 0x00000008;
const unsigned int		QRENDER_MATRIX_VIEWPROJECTION			= 0x00000010;
const unsigned int		QRENDER_MATRIX_MODELVIEWPROJECTION		= 0x00000020;
const unsigned int		QRENDER_MATRIX_IDENTITY					= 0x00000011;
const unsigned int		QRENDER_MATRIX_TRANSPOSE				= 0x00000012;
const unsigned int		QRENDER_MATRIX_INVERSE					= 0x00000014;
const unsigned int		QRENDER_MATRIX_INVERSETRANSPOSE			= 0x00000018;

const unsigned int		QRENDER_DATA_STREAM					= 0x00000400;
const unsigned int		QRENDER_DATA_STATIC					= 0x00000800;
const unsigned int		QRENDER_DATA_DYNAMIC				= 0x00001000;


const unsigned int		QRENDER_PRIM_TRIANGLES					= 0x00008000;
const unsigned int		QRENDER_PRIM_TRIANGLE_FAN				= 0x00010000;
const unsigned int		QRENDER_PRIM_TRIANGLE_STRIP				= 0x00020000;
const unsigned int		QRENDER_PRIM_LINES						= 0x00040000;
const unsigned int		QRENDER_PRIM_LINE_STRIP					= 0x00080000;

const unsigned int		QRENDER_CULL_NONE					= 0x00080000;
const unsigned int		QRENDER_CULL_CCW					= 0x00100000;
const unsigned int		QRENDER_CULL_CW						= 0x00200000;
const unsigned int		QRENDER_CULL_DEFAULT				= 0x00400000;

const unsigned int		QRENDER_ZBUFFER_NEVER				= 0x01000001;
const unsigned int		QRENDER_ZBUFFER_LESS				= 0x01000002;
const unsigned int		QRENDER_ZBUFFER_LEQUAL				= 0x01000003;
const unsigned int		QRENDER_ZBUFFER_EQUAL				= 0x00100004;
const unsigned int		QRENDER_ZBUFFER_GEQUAL				= 0x00100005;
const unsigned int		QRENDER_ZBUFFER_GREATER				= 0x00100006;
const unsigned int		QRENDER_ZBUFFER_ALWAYS				= 0x00100007;
const unsigned int		QRENDER_ZBUFFER_NOTEQUAL			= 0x00100008;
const unsigned int		QRENDER_ZBUFFER_DISABLE				= 0x0010000A;
const unsigned int		QRENDER_ZBUFFER_DEFAULT				= 0x0010000B;
const unsigned int		QRENDER_ZBUFFER_ENABLEWRITE			= 0x0010000C;
const unsigned int		QRENDER_ZBUFFER_DISABLEWRITE		= 0x0010000D;

const unsigned int		QRENDER_STENCIL_NEVER				= QRENDER_ZBUFFER_NEVER;
const unsigned int		QRENDER_STENCIL_LESS				= QRENDER_ZBUFFER_LESS;
const unsigned int		QRENDER_STENCIL_LEQUAL				= QRENDER_ZBUFFER_LEQUAL;
const unsigned int		QRENDER_STENCIL_EQUAL				= QRENDER_ZBUFFER_EQUAL;
const unsigned int		QRENDER_STENCIL_GEQUAL				= QRENDER_ZBUFFER_GEQUAL;
const unsigned int		QRENDER_STENCIL_GREATER				= QRENDER_ZBUFFER_GREATER;
const unsigned int		QRENDER_STENCIL_ALWAYS				= QRENDER_ZBUFFER_ALWAYS;
const unsigned int		QRENDER_STENCIL_NOTEQUAL			= QRENDER_ZBUFFER_NOTEQUAL;

const unsigned int		QRENDER_STENCIL_KEEP				= 0x00400001;
const unsigned int		QRENDER_STENCIL_ZERO				= 0x00400002;
const unsigned int		QRENDER_STENCIL_REPLACE				= 0x00400004;
const unsigned int		QRENDER_STENCIL_INCREMENT_SATURATE  = 0x00400008;
const unsigned int		QRENDER_STENCIL_DECREMENT_SATURATE	= 0x00400010;
const unsigned int		QRENDER_STENCIL_INVERT				= 0x00400020;
const unsigned int		QRENDER_STENCIL_INCREMENT			= 0x00400040;
const unsigned int		QRENDER_STENCIL_DECREMENT			= 0x00400080;

const unsigned int		QRENDER_STENCIL_FAIL				= 0x00800001;
const unsigned int		QRENDER_STENCIL_ZFAIL				= 0x00800002;
const unsigned int		QRENDER_STENCIL_PASS				= 0x00800004;
const unsigned int		QRENDER_STENCIL_WRITEMASK			= 0x00800008;
	
const unsigned int		QRENDER_ALPHABLEND_ZERO				= 0x00200001;
const unsigned int		QRENDER_ALPHABLEND_ONE				= 0x00200002;
const unsigned int		QRENDER_ALPHABLEND_SRCCOLOR			= 0x00200003;
const unsigned int		QRENDER_ALPHABLEND_INVSRCCOLOR		= 0x00200004;
const unsigned int		QRENDER_ALPHABLEND_INVSRCALPHA		= 0x00200005;
const unsigned int		QRENDER_ALPHABLEND_SRCALPHA			= 0x00200006;
const unsigned int		QRENDER_ALPHABLEND_DESTALPHA		= 0x00200007;
const unsigned int		QRENDER_ALPHABLEND_DESTCOLOR		= 0x00200008;

const unsigned int		QRENDER_SCISSOR_ENABLE			= 0x00300001;
const unsigned int		QRENDER_SCISSOR_DISABLE			= 0x00300002;
const unsigned int		QRENDER_SCISSOR_DEFAULT			= 0x00300003;

const unsigned int		QRENDER_VERTEX_POSITION			= 0x00500001;
const unsigned int		QRENDER_VERTEX_NORMAL			= 0x00500002;
const unsigned int		QRENDER_VERTEX_COLOR			= 0x00500004;
const unsigned int		QRENDER_VERTEX_TEXCOORD0		= 0x00500008;
const unsigned int		QRENDER_VERTEX_TEXCOORD1		= 0x00500010;
const unsigned int		QRENDER_VERTEX_TEXCOORD2		= 0x00500020;
const unsigned int		QRENDER_VERTEX_TEXCOORD3		= 0x00500040;


const unsigned int			QRENDER_MAX_VERTEX_STREAMS		= 16;
const unsigned int			QRENDER_MAX_TEXTURE_SAMPLERS	= 8;
const unsigned int			QRENDER_MAX_RENDER_TARGETS		= 4;






enum QRENDEREXPORT_API	EQuadrionBufferFormat
{
	QRENDER_BUFFER_FORMAT_A2R10G10B10		= 0,
	QRENDER_BUFFER_FORMAT_A8R8G8B8			= 1,
	QRENDER_BUFFER_FORMAT_X8R8G8B8			= 2,
};

enum QRENDEREXPORT_API EQuadrionMultisample
{
	QRENDER_MULTISAMPLE_NONE	= 0,
	QRENDER_MULTISAMPLE_2		= 1,
	QRENDER_MULTISAMPLE_3		= 2,
	QRENDER_MULTISAMPLE_4		= 3,
	QRENDER_MULTISAMPLE_5		= 4,
	QRENDER_MULTISAMPLE_6		= 5,
	QRENDER_MULTISAMPLE_7		= 6,
	QRENDER_MULTISAMPLE_8		= 7,
	QRENDER_MULTISAMPLE_9		= 8,
	QRENDER_MULTISAMPLE_10		= 9,
	QRENDER_MULTISAMPLE_11		= 10,
	QRENDER_MULTISAMPLE_12		= 11,
	QRENDER_MULTISAMPLE_13		= 12,
	QRENDER_MULTISAMPLE_14		= 13,
	QRENDER_MULTISAMPLE_15		= 14,
	QRENDER_MULTISAMPLE_16		= 15,
};

enum QRENDEREXPORT_API EQuadrionCSAA
{
	QRENDER_CSAA_NONE			= 0,
	QRENDER_CSAA_8X				= 1,
	QRENDER_CSAA_8XQ			= 2,
	QRENDER_CSAA_16X			= 3,
	QRENDER_CSAA_16XQ			= 4,
};


// Backbuffer format enumerator
enum QRENDEREXPORT_API EQuadrionBackbufferFormat
{
	QRENDER_DEPTH_32			= 0,
	QRENDER_DEPTH_16			= 1,
	QRENDER_DEPTH_24_STENCIL_8  = 2,
	QRENDER_DEPTH_24_X_8		= 3,
};


// Vertex shader version enumerator
enum QRENDEREXPORT_API EQuadrionVertexShader
{
	QRENDER_VERTEX_SHADER_1_0			= 0,
	QRENDER_VERTEX_SHADER_1_1			= 1,
	QRENDER_VERTEX_SHADER_1_2			= 2,
	QRENDER_VERTEX_SHADER_2_0			= 3,
	QRENDER_VERTEX_SHADER_3_0			= 4,
	QRENDER_VERTEX_SHADER_UNKNOWN		= 5,
};


// Pixel Shader version enumerator
enum QRENDEREXPORT_API EQuadrionPixelShader
{
	QRENDER_PIXEL_SHADER_1_0			= 0,
	QRENDER_PIXEL_SHADER_2_0			= 1,
	QRENDER_PIXEL_SHADER_3_0			= 2,
	QRENDER_PIXEL_SHADER_UNKNOWN		= 3,	
};

// Adapter Vendor Identifier enumerator //
enum QRENDEREXPORT_API EQuadrionDeviceVendor
{
	QRENDER_DEVICE_NVIDIA			= 0,
	QRENDER_DEVICE_ATI				= 1,
	QRENDER_DEVICE_MATROX			= 2,
	QRENDER_DEVICE_INTEL			= 3,
};


// Resolution pairing definition
typedef struct
{
	unsigned int m_width;
	unsigned int m_height;
}SResolution;

typedef struct
{
	vec3f		pos[4];
	vec2f		texcoords[4];
}STexturedQuad;

typedef struct
{
	vec3f pos[4];
	vec2f texcoords[4];
	vec3f norm[4];
}STexturedNormalQuad;


struct SUserFVF
{
	unsigned long	fvf;		// FVF Code
	unsigned int	size;		// Size of Vertex in bytes	
};


/////////////////////////////////////////////////////////////////////////////
//
// SQuadrionRenderInitializer
// Struct which contains renderer initialization options 
//
/////////////////////////////////////////////////////////////////////////////
struct SQuadrionRenderInitializer
{
	unsigned int		 displayWidth;			// Display width
	unsigned int		 displayHeight;			// Display height
		
	EQuadrionBufferFormat	  bufferFormat;		 // Backbuffer format
	unsigned int			  bufferCount;		 // Number of backbuffers
	EQuadrionMultisample	  multisample;		 // Multisample type
	EQuadrionBackbufferFormat backbufferFormat;	 // Backbuffer format
		
	bool			windowed;			// In windowed mode?
		
	bool			vsync;				// Use vsync?
	bool			debugGraphics;		// Use nvPerfHUD to debug
	unsigned int	fsaa;				// hardware fullscene anti-aliasing mode
	unsigned int	csaa;				// hardware coverage sampled anti-aliasing mode
};

//////////////////////////////////////////////////////////////////////////////////
//
// SQuadrionDeviceCapabilities 
// Encapsulation of meaningful rendering device capabilities
// 
/////////////////////////////////////////////////////////////////////////////////
struct SQuadrionDeviceCapabilities
{

	EQuadrionVertexShader			greatestVertexProfile;		// Greatest vertex shader profile
	EQuadrionPixelShader			greatestPixelProfile;		// Greatest pixel shader profile
		
	ETexturePixelFormat				greatestLuminanceFormat;	// Greatest pixel format for luminance textures
	ETexturePixelFormat				greatestFloatTextureFormat;	// Greatest pixel format for floating point textures
	
	EQuadrionDeviceVendor			vendor;					// Device's vendor 
		
	std::vector<SResolution>		supportedResolutions;		// List of supported resolutions
		
	bool				supportsS3TC;					// Supports DXT/ATI texture compression
	bool				supportsPostPixelBlending;		// Supports post pixel shader surface blending
	bool				supportsFloatingPointTargets;	// Supports greater than RGBA16F float targets
	bool				supportsFloatingPointLuminance; // Supports floating point luminance textures
	bool				supportsIndependentMRTBitDepth; // Supports independent bit depths for simultaneous render targets
		
	DWORD				maxAnisotropy;				// max levels of anisotropy
	DWORD				maxLights;					// max simultaneous hw lights
	unsigned int		maxFSAA;					// max level of full scene anti-aliasing
	unsigned int		maxSimultaneousMRT;			// maximum simultaneous multiple render targets (1-4)
};





///////////////////////////////////////////////////////////////////////////////
// 
// CQuadrionRender
// 
// Rendering point of interaction
//
///////////////////////////////////////////////////////////////////////////////
class QRENDEREXPORT_API CQuadrionRender
{
	public:
		
		
		
		CQuadrionRender();
		~CQuadrionRender();
		
		// Initialize- Performs the initialization of the render object. 
		// This should only be done *ONE* time or in the event of a lost device 
		// Parameters:
		//		hWindow- Handle to parent window
		//		init- SQuadrionRenderInitializer object which specifies device creation characteristics
		bool		Initialize(const HWND& hWindow, const SQuadrionRenderInitializer& init);
		int			InitializeNewSwapChain(const HWND& hWindow, const SQuadrionRenderInitializer& init);
		bool		BindSwapChain(const int& id = QRENDER_DEFAULT);
		
		// Release - Shuts down the renderer and all of its assets. 
		void		Release();
		
		
		// BeginRendering - Prepares the renderer for the beginning of a frame render block
		// This should be called once per frame to encapsulate the entire rendering content for that frame
		bool	BeginRendering();
		
		// EndRendering - Must be called when the frame render is complete. This is a pair to ::BeginRendering
		// EndRendering MUST be called after BeginRendering.
		bool	EndRendering(const int& swapChain = QRENDER_DEFAULT);
		
		// SetMatrix- Upload a state matrix
		// Parameters:
		//		type- one of the QRENDER_MATRIX_ flags which tells the renderer which matrix to set
		//		m- a mat4 object of the new matrix value
		bool		SetMatrix(const unsigned int& type, const mat4& m);
		
		// MulMatrix - Premultiplies a state matrix by the matrix supplied
		// Parameters:
		//		type- one of the QRENDER_MATRIX_ flags which tells the renderer which matrix to mul
		//		m- a mat4 object of the value the matrix will be mul'd with
		bool		MulMatrix(const unsigned int& type, const mat4& m);
		
		// GetMatrix - Obtain a copy of a state matrix
		// Paramters:
		//		type- one of the QRENDER_MATRIX_ flags which tells the renderer which matrix to obtain
		//		m- A mat4 object in which the matrix will be copied
		bool		GetMatrix(const unsigned int& type, mat4& m);
		

		// Font and sprite mgmt //
		int			CreateFont(const unsigned int& height, const unsigned int& width, const unsigned int& weight, const bool& italic, const char* fontName);	
		int			CreateSprite();
		

		
		// Viewports //
		bool		SetViewport(const unsigned int& x, const unsigned int& y, const unsigned int& width, const unsigned int& height);
		void		GetCurrentViewport(unsigned int& x, unsigned int& y, unsigned int& width, unsigned int& height);
		bool		ResetViewport();
		
		
		// Cull mode //
		bool		ChangeCullMode(const unsigned int& mode);
		
		// Depth mode //
		bool		ChangeDepthMode(const unsigned int& mode);
		bool		ChangeDepthBias(const float bias);
		bool		ChangeSlopeBias(const float bias);

		// Alpha blending //
		bool		EnableAlphaBlending();
		bool		DisableAlphaBlending();
		bool		ChangeAlphaBlendMode(const unsigned int& src, const unsigned int& dest);
		
		// Scissor Test //
		bool		EnableScissorTest();
		bool		DisableScissorTest();
		bool		SetScissorRect(const RECT& rect);
		
		// Stencil buffer //
		bool		EnableStencilTest();
		bool		DisableStencilTest();
		bool		SetStencilTest(const unsigned int& test);
		bool		SetStencilRef(const int& ref);
		bool		SetStencilMask(const int& mask);
		bool		SetStencilWrite(const unsigned int& write);
		bool		SetStencilAction(const unsigned int& stencilevent, const unsigned int& action);
		void		SetStencilDefaults();
		
		// Color Writes //
		bool		DisableColorWrites();
		bool		EnableColorWrites();
		
		// Rendering //
		int			MakeVertexFormat(const unsigned int& vFlags);
		bool		BindVertexFormat(const int& handle);
		
		bool		RenderIndexedList(const unsigned int& mode, const void* vPtr, unsigned int nVertices,       \
									  const void* iPtr, unsigned int indexSize, unsigned int nIndices, unsigned int minVertexIndex = 0);
		
		bool		RenderIndexedList(const unsigned int& mode, const unsigned int& startIndex, unsigned int baseVertex, unsigned int nVertices,		\
									  unsigned int nIndices, unsigned int minVertexIndex = 0);
		
		bool		RenderLine(const vec3f& a, const vec3f& b, const unsigned int& color);
		bool		RenderLine(const vec3f& a, const vec3f& b, unsigned int startColor, unsigned int endColor);
		
		bool		RenderTriangle(const vec3f& a, const vec3f& b, const vec3f& c, unsigned int color);
		bool		RenderBox(const vec3f& min, const vec3f& max, unsigned int color);
		
		bool		RenderLineStrip(const unsigned int& start, const unsigned int& nLines);
		bool		RenderTriangleStrip(const unsigned int& start, const unsigned int& nTris);
		bool		RenderText(const int sprite, const int font, const char* text, int offs, RECT* rect, int var, unsigned int color);
		bool		RenderScreenSpaceQuad(const cTextureRect& texCoords, const vec2f& upperLeft, const vec2f& widthHeight);
		bool		RenderScreenSpaceQuad(const unsigned int color, const vec2f& upperLeft, const vec2f& widthHeight);
		bool		RenderScreenSpaceQuad( const unsigned int color, const cTextureRect& texCoords, const vec2f& upperLeft, const vec2f& widthHeight );
		bool		RenderQuad( const STexturedQuad& quad );
		bool		RenderQuad( const STexturedNormalQuad& quad );
		void		RenderSphere();
		void		RenderCone();
		bool		RenderFrustum(const vec2f& nearDims, const float& nearDist, const vec2f& farDims, const float& farDist,
								  const vec3f& position, const vec3f& direction, const vec3f& strafe, const vec3f& up);
		
		// Full scene anti-aliasing //
		bool		EnableFSAA();
		bool		DisableFSAA();
		
		
		
		// New Effect Interface //
		int							AddEffect(const std::string& name, const std::string& path = "./");
		inline CQuadrionEffect*		GetEffect(const unsigned int& handle) { return m_effectResources->GetResource(handle); }
		CQuadrionEffect*			GetEffect(const std::string& name, const std::string& path = "./");
		const inline void			UnloadEffect(const int& handle) { m_effectResources->RemoveResource(handle); }	
		void						UnloadEffect(const std::string& name, const std::string& path = "./");				
		// End new effect interface //
		// New Vertex Buffer Interface //
		int								AddVertexBuffer(const std::string& name = ".");
		int								AddInstancedVertexBuffer(const std::string& name = ".");
		int								AddVertexBuffer(const std::string& name, const void* pVerts, const SQuadrionVertexDescriptor& desc, const int& nVerts, const bool& stream = FALSE);
		inline CQuadrionVertexBuffer*	GetVertexBuffer(const unsigned int& handle) { return m_vertexBufferResources->GetResource(handle); }
		inline CQuadrionInstancedVertexBuffer* GetInstancedVertexBuffer(const unsigned int& handle) { return m_instancedVertexBufferResources->GetResource(handle); }
		const inline void				UnloadVertexBuffer(const int& handle) { m_vertexBufferResources->RemoveResource(handle); }
		const inline void				UnloadInstancedVertexBuffer(const int& handle) { m_instancedVertexBufferResources->RemoveResource(handle); }
		// End new vertex buffer interface //
		// New Index Buffer Interface //
		int								AddIndexBuffer(const std::string& name = ".");
		inline CQuadrionIndexBuffer*	GetIndexBuffer(const unsigned int& handle) { return m_indexBufferResources->GetResource(handle); }
		const inline void				UnloadIndexBuffer(const int& handle) { m_indexBufferResources->RemoveResource(handle); }
		// New Texture object interface //
		int								AddTextureObject(unsigned int& flags, const std::string& name, const std::string& path = "./");
		int								AddTextureObject(const void* dat, unsigned int& flags);
		int								AddTextureObject(CQuadrionTextureFile& tex, unsigned int& flags);
		int								StreamTextureObject(unsigned int& flags, const std::string& name, const std::string& path);
		int								StreamTextureObject(CQuadrionTextureFile& tex, unsigned int& flags);
		inline CQuadrionTextureObject*  GetTextureObject(const unsigned int& handle);
		CQuadrionTextureObject*			GetTextureObject(const std::string& name, const std::string& path = "./");
		void							UnloadTextureObject(const std::string& name, const std::string& path = "./");
		const inline void				UnloadTextureObject(const int& handle) { m_textureObjectResources->RemoveResource(handle); }
		void							EvictTextures();
		int								GetIncrementalTextureRefference();
		// New render to texture interface //
		int									AddRenderTarget( unsigned int flags, const unsigned int& w, const unsigned int& h, 
														     const ETexturePixelFormat& fmt, bool msaa = false );
		inline CQuadrionRenderTarget*		GetRenderTarget(const unsigned int& handle) { return m_renderTargetResources->GetResource(handle); }
		const inline void					UnloadRenderTarget(const int& handle) { m_renderTargetResources->RemoveResource(handle); }
		void								EvictRenderTargets();
		void								EvictRenderTarget(const unsigned int& slot);
		void								EvictDepthStencilTarget();
		void								SetDefaultRenderTarget();
		// New depth stencil target interface //
		int										AddDepthStencilTarget( const unsigned int &w, const unsigned int& h, const unsigned int& depthBits,		\
																	   const unsigned int& stencilBits, bool msaa = false );
		inline CQuadrionDepthStencilTarget*		GetDepthStencilTarget(const unsigned int& handle) { return m_depthStencilTargetResources->GetResource(handle); }
		const inline void						UnloadDepthStencilTarget(const unsigned int& handle) { m_depthStencilTargetResources->RemoveResource(handle); }
		void									SetDefaultDepthStencilTarget();
		
		// Buffer Clears //
		bool						ClearBuffers(const unsigned int& targets, const unsigned int& color, const float& depth, const DWORD& stencil = 0);


		// Set Clipping planes //
		void 						DisableFrustumCulling();
		bool						SetFrustumClipPlanes( const float* planes );
		
		// Occlusion Queries //
		void				IssueOcclusionQuery();
		unsigned int		CheckOcclusionQuery();
		
		
		// Device accessors //
		inline const SQuadrionDeviceCapabilities* GetDeviceCapabilities() { return &m_deviceCaps; }
		const inline unsigned int				  GetDisplayWidth() { return m_displayWidth; }
		const inline unsigned int				  GetDisplayHeight() { return m_displayHeight; }
		const inline unsigned int				  GetCurrentFSAA() { return m_currentFSAA; }
		const inline unsigned int				  GetCurrentMSAA() { return m_currentMSAA; }
		void									  GetCurrentMSAA( D3DMULTISAMPLE_TYPE &msType, unsigned int& qual );
		const inline int						  GetNonMSAADepthStencil() { return m_nonMSAADepthStencil; }
		const inline bool						  IsBackbufferMultisampled() { return (m_currentMSAA > 0); }
		inline HWND								  GetHWnd() { return m_hWnd; }
	
	protected:
	
		int								m_currentTextures[QRENDER_MAX_TEXTURE_SAMPLERS];
		int							m_currentTextureTypes[QRENDER_MAX_TEXTURE_SAMPLERS];
		int								m_currentRenderTargets[QRENDER_MAX_RENDER_TARGETS];
		int								m_currentDepthStencilTarget;
		int								m_currentEffect;
		SQuadrionTextureSampler			m_currentSamplers[QRENDER_MAX_TEXTURE_SAMPLERS];
		SQuadrionDeviceCapabilities		m_deviceCaps;			// Queriable device caps 
	
	private:
	
		friend class					CQuadrionEffect;
		friend class					CQuadrionVertexBuffer;
		friend class					CQuadrionIndexBuffer;
		friend class					CQuadrionTextureObject;
		friend class					CQuadrionRenderTarget;
		friend class					CQuadrionDepthStencilTarget;
		friend class					CQuadrionInstancedVertexBuffer;
		
		CQuadrionResourceManager<CQuadrionEffect>*					m_effectResources;
		CQuadrionResourceManager<CQuadrionVertexBuffer>*			m_vertexBufferResources;
		CQuadrionResourceManager<CQuadrionInstancedVertexBuffer>*	m_instancedVertexBufferResources;
		CQuadrionResourceManager<CQuadrionIndexBuffer>*				m_indexBufferResources;
		CQuadrionResourceManager<CQuadrionTextureObject>*			m_textureObjectResources;
		CQuadrionResourceManager<CQuadrionRenderTarget>*			m_renderTargetResources;
		CQuadrionResourceManager<CQuadrionDepthStencilTarget>*		m_depthStencilTargetResources;
		
		std::vector<LPDIRECT3DSWAPCHAIN9>			m_swapChains;
		std::vector<LPDIRECT3DSURFACE9>				m_swapDepthStencils;
		
		
		// For OpenGL style immediate mode rendering paradigm //
		std::vector <SUserFVF>				m_userFVFs;
		unsigned int						m_curFVF;


	
		int								m_vertexNameRef;
		int								m_instancedVertexNameRef;
		int								m_indexNameRef;
		int								m_textureNameRef;
		int								m_depthStencilNameRef;
		int								m_renderTargetNameRef;
		int								m_nonMSAADepthStencil;
		
	
	
	
		
		unsigned int		    GetMultisample(const SQuadrionRenderInitializer& init, const D3DFORMAT& adapterFormat, const unsigned int& adapter);
		void					CheckDeviceCapabilities(const D3DFORMAT& adapterFormat);
		void					CheckDisplayModes(const unsigned int& adapterFormat);
		void					GetGreatestLuminanceFormat(const D3DFORMAT& adapterFormat);
		void					GetGreatestFloatFormat(const D3DFORMAT& adapterFormat);
		void					SetShaderProfiles(const char* v, const char* p);
	
	
		LPD3DXMESH				m_pSphereObject;
		LPD3DXMESH				m_pConeObject;
		
		LPDIRECT3DQUERY9		m_pOcclusionQuery;
		LPDIRECT3D9				m_pD3DObj;			// Direct3D Object
		LPDIRECT3DDEVICE9		m_pD3DDev;			// Direct3D Device
		LPDIRECT3DSURFACE9		m_pFramebuffer;			// default framebuffer
		LPDIRECT3DSURFACE9		m_pDepthStencilBuffer;		// default depth stencil buffer suface
		D3DFORMAT				m_adapterFormat;		// graphics adapter format
		D3DVIEWPORT9			m_currentViewport;		// Current viewport
		D3DVIEWPORT9			m_previousViewport;		// Last viewport set
		D3DCAPS9				m_caps;					// Device capabilities
		LPDIRECT3DVERTEXBUFFER9 m_lineVertexBuffer;
		HWND					m_hWnd;
		
		unsigned int					m_displayWidth;			// Current screen width
		unsigned int					m_displayHeight;			// Current screen height
		
		unsigned int					m_currentCullMode;
		unsigned int					m_currentDepthMode;
		unsigned int					m_currentSourceAlpha;
		unsigned int					m_currentDestAlpha;
		unsigned int					m_currentStencilFunc;
		unsigned long					m_clipPlaneMask;
		unsigned int					m_currentFSAA;
		unsigned int					m_currentMSAA;
		
		bool					m_bZWriteEnabled;
		bool					m_bAlphaBlendingEnabled;
		bool					m_bScissorTestEnabled;
		bool					m_bStencilTestEnabled;
		

		std::vector<LPD3DXFONT>					m_fontList;					// List of D3DX Font objects
		std::vector<LPD3DXSPRITE>				m_spriteList;					// List of D3DX Sprite objects


		int						m_noShaderHandle;
};




extern QRENDEREXPORT_API CQuadrionRender* g_pRender;

#endif