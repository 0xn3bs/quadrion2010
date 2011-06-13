/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// QTEXTURE.H
//
// Written by Shawn Simonson 08/2008 for Quadrion Engine 
//
// This file contains texture object class, descriptors and related functions that pertain to 
// actual texture asset files such as .JPG, .DDS, and .TGA
//
// The class is essentially abstracted by CQuadrionRenderer for texture loading, however
// the class can be used to manage raw texture data assets such as lightmaps, or run-time
// generated height and normal maps
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "qresource.h"

 
#ifndef __QTEXTURE_H_
#define __QTEXTURE_H_

#ifdef QRENDER_EXPORTS
#define QTEXTUREEXPORT_API __declspec(dllexport)
#else
#define QTEXTUREEXPORT_API __declspec(dllimport)
#endif



#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <string>
#include <vector>
#include <d3d9.h>

#ifdef QRENDER_EXPORTS
#include "jpegdecoder.h"
#endif


const unsigned int			QTEXTURE_ALL_MIPMAPS			= 127;


const unsigned int			QTEXTURE_FILTER_NEAREST			= 0x00000001;
const unsigned int			QTEXTURE_FILTER_LINEAR			= 0x00000002;
const unsigned int			QTEXTURE_FILTER_BILINEAR		= 0x00000004;
const unsigned int			QTEXTURE_FILTER_TRILINEAR		= 0x00000008;
const unsigned int			QTEXTURE_FILTER_BILINEAR_ANISO  = 0x00000010;
const unsigned int			QTEXTURE_FILTER_TRILINEAR_ANISO = 0x00000020;

const unsigned int			QTEXTURE_CLAMP_S				= 0x00000040;
const unsigned int			QTEXTURE_CLAMP_T				= 0x00000080;
const unsigned int			QTEXTURE_CLAMP_R				= 0x00000100;
const unsigned int			QTEXTURE_WRAP_S					= 0x00000200;
const unsigned int			QTEXTURE_WRAP_T					= 0x00000400;
const unsigned int			QTEXTURE_WRAP_R					= 0x00000800;
const unsigned int			QTEXTURE_CLAMP					= (QTEXTURE_CLAMP_S | QTEXTURE_CLAMP_T | QTEXTURE_CLAMP_R);
const unsigned int			QTEXTURE_WRAP					= (QTEXTURE_WRAP_S | QTEXTURE_WRAP_T | QTEXTURE_WRAP_R);
const unsigned int			QTEXTURE_CUBEMAP				= 0x00001000;
const unsigned int			QTEXTURE_NORMALMAP				= 0x00002000;
const unsigned int			QTEXTURE_KEEPHEIGHT				= 0x00004000;
const unsigned int			QTEXTURE_NORMALHEIGHTMAP		= (QTEXTURE_NORMALMAP | QTEXTURE_KEEPHEIGHT);

const unsigned int			QTEXTURE_CUBEMAPFACE_POSITIVE_X		= 0;
const unsigned int			QTEXTURE_CUBEMAPFACE_NEGATIVE_X		= 1;
const unsigned int			QTEXTURE_CUBEMAPFACE_POSITIVE_Y		= 2;
const unsigned int			QTEXTURE_CUBEMAPFACE_NEGATIVE_Y		= 3;
const unsigned int			QTEXTURE_CUBEMAPFACE_POSITIVE_Z		= 4;
const unsigned int			QTEXTURE_CUBEMAPFACE_NEGATIVE_Z		= 5;


const unsigned int			QTEXTURE_DYNAMIC				= 0x00010000;
const unsigned int			QTEXTURE_STATIC					= 0x00020000;

const unsigned int			QDEPTHTARGET_LOCKABLE			= 0x00040000;



//QTEXTUREEXPORT_API unsigned int		QTEXTURE_FOURCC(unsigned char c0, unsigned char c1, UCHAR c2, UCHAR c3);



// Pixel format descriptor //
enum QTEXTUREEXPORT_API ETexturePixelFormat
{
	QTEXTURE_FORMAT_NONE   = 0,
	QTEXTURE_FORMAT_I8	   = 1,
	QTEXTURE_FORMAT_IA8	   = 2,
	QTEXTURE_FORMAT_RGB8    = 3,
	QTEXTURE_FORMAT_RGBA8   = 4,
	QTEXTURE_FORMAT_I16     = 5,
	QTEXTURE_FORMAT_IA16    = 6,
	QTEXTURE_FORMAT_RGB16   = 7,
	QTEXTURE_FORMAT_RGBA16  = 8,
	QTEXTURE_FORMAT_I16F    = 9,
	QTEXTURE_FORMAT_IA16F   = 10,
	QTEXTURE_FORMAT_RGB16F  = 11,
	QTEXTURE_FORMAT_RGBA16F = 12,
	QTEXTURE_FORMAT_I32F    = 13,
	QTEXTURE_FORMAT_IA32F   = 14,
	QTEXTURE_FORMAT_RGB32F  = 15,
	QTEXTURE_FORMAT_RGBA32F = 16,
	QTEXTURE_FORMAT_DEPTH16 = 17,
	QTEXTURE_FORMAT_DEPTH24 = 18,
	QTEXTURE_FORMAT_RGB332  = 19,
	QTEXTURE_FORMAT_DXT1    = 20,
	QTEXTURE_FORMAT_DXT3    = 21,
	QTEXTURE_FORMAT_DXT5    = 22,
	QTEXTURE_FORMAT_ATI2N   = 23,
	QTEXTURE_FORMAT_ATI1N   = 24,
	QTEXTURE_FORMAT_RE8		= 25,
};












///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// cQuadrionTextureFile
//
// This class represents the actual texture file asset. 
// cQuadrionTextureFile will open, load, release, and delete TGA, JPG, and DDS texture file formats.
// This class is also used to load texture assets into the renderer and ultimately onto the video board.
// Ultimately, this class is fully abstracted unless using it to manage raw texture assets such as lightmaps
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class QTEXTUREEXPORT_API CQuadrionTextureFile
{
	public:
	
		CQuadrionTextureFile();
		CQuadrionTextureFile(const CQuadrionTextureFile& tex);
		CQuadrionTextureFile& operator= (const CQuadrionTextureFile& tex);
		~CQuadrionTextureFile();
		
		// Load lightmap (raw RGB8 format) //
		bool		LoadLightmap(unsigned char* pix);
		
		// Load from buffer //
		bool		LoadFromColor( const unsigned int& color );
		
		// Load texture from filename //
		bool		LoadTexture(const char* fname, const char* pname = "./");
		
		// Swap 2 channels in texture //
		bool		SwapChannels(const unsigned int& ch0, const unsigned int& ch1, bool normalMap = false);
		
		// Generate mipmaps //
		bool		GenerateMipMaps(const unsigned int nMips = QTEXTURE_ALL_MIPMAPS);
		
		// Normalmap gen //
		bool		HeightToNormal(const bool useRGBA = TRUE, const bool keepHeight = FALSE, float sz = 1.0F, float mipScaleZ = 2.0F);
		
		// Add channel //
		bool		AddChannel(const float val = 0);
		
		bool		ConvertToGreyscale(const float rf = 0.30f, const float gf = 0.59f, const float bf = 0.11f);
		
		// Get pointer to pixel data at "level" mip level //
		unsigned char*			GetData(const unsigned int& level = 0) const;
		unsigned char*			GetGreyscaleData(const unsigned int& level = 0) const;
		unsigned char*			GetNormalmapData(const unsigned int& level = 0) const;
		
		
		// Get pixel format in ETexturePixelFormat format //
		ETexturePixelFormat GetPixelFormat();
		
		int					GetMipMapCount();
		
		// Returns size in bytes of texture with mipmaps //
		// first- first mip level (defaults to 0) 
		// nLevels- number of mip levels from "first" to query (defaults to all mip maps)
		// fmt- use to query for different formats from the original for size comparison (defaults to none)
		int					GetSizeWithMipMaps(const unsigned int first = 0, unsigned int nLevels = QTEXTURE_ALL_MIPMAPS, ETexturePixelFormat fmt = QTEXTURE_FORMAT_NONE);
		
		// Obtain width, height, and depth of original format //
		int					GetWidth();
		int					GetHeight();
		int					GetDepth();
		
		// Obtain width, height, and depth at a particlar mip level //
		int					GetWidth(const unsigned int& level);
		int					GetHeight(const unsigned int& level);
		int					GetDepth(const unsigned int& level);
		
		// Obtain total pixel count //
		// firstMip- first mip level to query from (defaults to 0)
		// nLevels- number of mip levels to query (defaults to all mipmaps)
		int					GetPixelCount(const int firstMip = 0, int nLevels = QTEXTURE_ALL_MIPMAPS) const;
		
		// Obtain file name with path and extension //
		std::string			GetFileName();
		const inline void	SetFileName(const std::string& s) { fileName = s; }
		std::string			GetFilePath() { return m_pathName; }
		const inline void	SetFilePath(const std::string& s) { m_pathName = s; }
		void				GetFileName(const std::string* fname);
		
		// Query image type //
		bool	Is3D();
		bool    Is2D();
		bool	IsCubemap();
		
		
		const inline bool	IsLoaded() { return m_bIsLoaded; }
		
		
	protected:
	
	private:
	
		friend class			CQuadrionTextureObject;
	
		bool			LoadTGA(LPCSTR fname);
		bool			LoadDDS(LPCSTR fname);
		bool			LoadJPG(LPCSTR fname);
		
		bool			m_bIsLoaded;
	
		std::vector<unsigned char*> pixels;				// Raw levels of pixmap data
		std::vector<unsigned char*> normalMap;	
		unsigned char* greyscale;		
		
		std::string				fileName;				// file name and extension
		std::string				m_pathName;				// Path name
		
		unsigned int			width;					// width in pixels
		unsigned int			height;					// height in pixels
		unsigned int			bpp;					// bits per pixel
		unsigned int			depth;					// depth in pixels
		unsigned int			nMipMaps;				// number of mipmaps associated with raw data ("pixels")
		
		ETexturePixelFormat		pixelFormat;			// pixel format descriptor
};



////////////////////////////////////////////////////////////////
// 
// SQuadrionTextureSampler
// Internal Usage only
//
/////////////////////////////////////////////////////////////////
struct SQuadrionTextureSampler
{
	D3DTEXTUREFILTERTYPE		minFilter;
	D3DTEXTUREFILTERTYPE		magFilter;
	D3DTEXTUREFILTERTYPE		mipFilter;
	D3DTEXTUREADDRESS			wrapS;
	D3DTEXTUREADDRESS			wrapT;
	D3DTEXTUREADDRESS			wrapR;
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CQuadrionTextureObject
// Texture object declaration
//
// The CQuadrionTextureObject class is an interface to the main render device. 
// All texture object interaction is done via the CQuadrionTextureObject which can be obtained through the
// global render device object.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class QTEXTUREEXPORT_API CQuadrionTextureObject : public CQuadrionResource
{
	public:
	
		CQuadrionTextureObject(const unsigned int handle, const std::string& name, const std::string& path = "./");
		virtual ~CQuadrionTextureObject();
		
		// BindTexture
		// Params: 
		//		textureUnit- The texture unit in which the texture will be bound (0 - QRENDER_MAX_TEXTURE_SAMPLERS)
		// BindTexture() binds the texture object to a specific texture sampler for use.
		// Default parameter is 0, so the user can invoke the function w/o parameters and assume the texture is bound to
		// the 0th texture sampling unit.
		virtual bool			BindTexture(const unsigned int& textureUnit = 0);
		
		// BindTexture
		// Params:
		//		paramName- The name of the texture object in a currently bound shader
		// Differs from BindTexture() above in that this method requires a shader to be bound. If one is not
		// the function simply returns false.
		bool			BindTexture(const std::string& paramName);
		
		// UnbindTexture
		// UnbindTexture() forcefully unbinds the texture from its current texture unit and clears
		// texture stage sampling states for the texture unit it was bound
		bool			UnbindTexture();		
		
		
		// GetWidth
		// Obtain the width in pixels of the texture surface
		const inline unsigned int	GetWidth() { return m_textureWidth; }
		
		// GetHeight
		// Obtain the height in pixels of the texture surface
		const inline unsigned int	GetHeight() { return m_textureHeight; }
		
		// GetFlags
		// Obtain texture creation flags
		const inline unsigned int	GetFlags() { return m_textureFlags; }
		
		// IsLoaded
		// Query whether the texture was successfully loaded and is ready to be bound
		const inline bool			IsLoaded() { return m_bIsLoaded; }
		
		// IsBound
		// Query whether the texture is currently bound to a texture sampling unit 
		const inline bool			IsBound() { return m_bIsBound; }

		bool						CreateTextureFromFile(unsigned int& flags);
		bool						CreateTextureFromData(unsigned int& flags, const void* dat);
		bool						CreateTexture(CQuadrionTextureFile& tex, unsigned int& flags);
		
		
		
	protected:
	
		void						ChangeRenderDevice(const void* pRender);
		
		unsigned int				m_textureFlags;				
		unsigned int				m_textureWidth;				// Texture surface width in pixels
		unsigned int				m_textureHeight;			// Texture surface height in pixels	
		int							m_assignedTextureUnit;		// Currently assigned texture unit
		unsigned long				m_usage;					// Texture usage flags
		bool						m_bIsLoaded;				// Is texture loaded
		bool						m_bIsBound;					// Is texture bound
		
		D3DFORMAT					m_pixelFormat;				// Texture pixel format
		LPDIRECT3DDEVICE9			m_pRenderDevice;			// Handle to CQuadrionRender's direct3D device
		LPDIRECT3DBASETEXTURE9		m_pTextureObject;			// Handle to texture surface
		
		friend class				CQuadrionRender;
		friend class				CQuadrionEffect;
		CQuadrionRender*			m_pQuadrionRender;			// Handle to the global CQuadrionRender object
		
		SQuadrionTextureSampler		m_textureSampler;			// Sampler descriptor
	
	private:
	
		
		
		
		
		unsigned int				m_anisotropy;				// Anisotropic level (0-16), 0 is off.
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CQuadrionRenderTarget
//
// The Quadrion Offscreen rendertarget class (or RTT for "Render to texture") defines a programmer interface for
// the creation and interaction with RTT's. RTT's are offscreen targets that can be rendered to and read from in subsequent
// effect passes. 
//
// This class is derived from CQuadrionTexture, so binding a render target as a texture to be read is accomplished with the
// Usual ::BindTexture function. 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class QTEXTUREEXPORT_API CQuadrionRenderTarget : public CQuadrionTextureObject
{
	public:
	
		CQuadrionRenderTarget(const unsigned int handle, const std::string& name, const std::string& path = "./");
		~CQuadrionRenderTarget();
		
		
		// BindRenderTarget
		// Params:
		//		rttSlot- The MRT output slot in which the render target will be placed (0 - SQuadrionDeviceCapabilities.MaxSimultaneousMRT)
		// Binds the render target object to an output MRT slot to be rendered to
		bool					BindRenderTarget(const unsigned int& rttSlot = -2);
		
		// BindCubemapFace
		// Params:
		//		cubemapFace- flag (QTEXTURE_CUBEMAPFACE_POSITIVE_X....) designating which face to make current for rendering
		//		rttSlot- The MRT output slot in which the target will be placed
		// Binds the render target object to an output MRT slot to be rendered to.
		bool					BindCubemapFace(const unsigned int cubemapFace, const unsigned int& rttSlot = -2);
		
		bool					BindTexture(const unsigned int& textureUnit = 0);

		
		// Clear
		// Params:
		//		color (optional)- D3D ARGB ordered color to fill the render target. You can use QRENDER_MAKE_ARGB for this. Default is black
		// Fills the entire render target with the color specified by "color"
		bool			Clear(const unsigned int& color = 0x00000000);
		
		
		// ClearCubemapFace
		// Params:
		//		cubemapFace- flag (QTEXTURE_CUBEMAPFACE_POSITIVE_X....) designating which face is to be cleared
		//		color (optional)- ARGB ordered color with which to fill the face
		bool			ClearCubemapFace(const int cubemapFace, unsigned int color = 0x00000000);
		
		// IsMultisampled
		// Query whether this target was created as a Multisampled surface //
		const inline bool				IsMultisampled() { return m_bHasMSAA; }
		
		// GetPixelFormat
		// Obtains the render target's internal pixel format 
		const inline ETexturePixelFormat	GetPixelFormat() { return m_internalPixelFormat; }
	
	protected:
	
	private:
	
		friend class			CQuadrionRender;

	
		LPDIRECT3DSURFACE9*		m_pSurfaceList;					// Handle to render target surface
		LPDIRECT3DBASETEXTURE9  m_pResolvedTexture;				// Handle to resolved texture 
		LPDIRECT3DSURFACE9		m_pResolvedSurface;				// Handle to resolved surface
		
	
		ETexturePixelFormat		m_internalPixelFormat;
	
		unsigned int			m_nSurfaces;					// Number of surfaces in m_pSurfaceList
		unsigned int			m_assignedRTTSlot;				// Currently assigned MRT slot
	
		bool					m_bHasMSAA;						// Target has MSAA ?
		bool					m_bIsResolved;					// Is the surface resolved?
	
		// Private Use //
		bool					CreateRenderTarget( unsigned int& flags, const unsigned int& w, const unsigned int& h, const ETexturePixelFormat& fmt, bool msaa = false );
		
		
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CQuadrionDepthStencilTarget
//
// The CQuadrionDepthStencilTarget object gives the programmer an interface through which they can define their own
// Depth/Stencil surfaces. 
//
// Since output formats must be compatible when using RTT, most RTT's are not compatible with default depth/stencil
// surface. This also affords the programmer efficient stencil buffers.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class QTEXTUREEXPORT_API CQuadrionDepthStencilTarget : public CQuadrionResource
{
	public:
		
		CQuadrionDepthStencilTarget(const unsigned int handle, const std::string& name, const std::string& path = "./");
		~CQuadrionDepthStencilTarget();
		
		// BindDepthStencilTarget
		// Binds the Depth/Stencil surface as the current one 
		// Depth stencil surfaces can be evicted with CQuadrionRender::EvictDepthStencilTarget or
		// CQuadrionRender::SetDefaultDepthStencilTarget
		bool					BindDepthStencilTarget();
		
		// GetWidth
		// Get width in pixels of depth/stencil surface
		const inline unsigned int		GetWidth() { return m_width; }
		
		// GetHeight
		// Get height in pixels of depth/stencil surface
		const inline unsigned int		GetHeight() { return m_height; }
		
		// GetDepthBits
		// Get bitdepth of the zbuffer
		const inline unsigned int		GetDepthBits() { return m_depthBits; }
		
		// GetStencilBits
		// Get bitdepth of the stencil buffer
		const inline unsigned int		GetStencilBits() { return m_stencilBits; }
		
		// IsMultisampled
		// Query whether this target was created as a Multisampled surface //
		const inline bool				IsMultisampled() { return m_bHasMSAA; }
	
	protected:
	
		// Private use //
		void				ChangeRenderDevice(const void* pRender);
		bool				CreateDepthStencilTarget( const unsigned int& w, const unsigned int& h, 
													  const unsigned int& depthBits, const unsigned int& stencilBits, bool msaa = false );
	
	private:
	
		unsigned int			m_width;						// Surface width (pixels)
		unsigned int			m_height;						// Surface height (pixels)
		unsigned int			m_depthBits;					// zbuffer bit depth (bits)
		unsigned int			m_stencilBits;					// stencil buffer bit depth (bits)
	
		friend class			CQuadrionRender;
		CQuadrionRender*		m_pQuadrionRender;				// handle to CQuadrionRender object
		
		LPDIRECT3DDEVICE9		m_pRenderDevice;				// handle to CQuadrionRender's LPDIRECT3DDEVICE9
		LPDIRECT3DSURFACE9		m_pDepthStencilSurface;			// handle to depth/stencil surface
		
		
		bool					m_bHasMSAA;						// Has MSAA?
};




#endif