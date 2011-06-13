
#include "stdafx.h"
#include "qtexture.h"
#include "qrender.h"


const unsigned int DDPF_ALPHAPIXELS = 0x00000001;
const unsigned int DDPF_FOURCC      = 0x00000004;
const unsigned int DDPF_RGB         = 0x00000040;
const unsigned int DDSD_CAPS        = 0x00000001;
const unsigned int DDSD_HEIGHT      = 0x00000002;
const unsigned int DDSD_WIDTH       = 0x00000004;
const unsigned int DDSD_PITCH       = 0x00000008;
const unsigned int DDSD_PIXELFORMAT = 0x00001000;
const unsigned int DDSD_MIPMAPCOUNT = 0x00020000;
const unsigned int DDSD_LINEARSIZE  = 0x00080000;
const unsigned int DDSD_DEPTH       = 0x00800000;
const unsigned int DDSCAPS_COMPLEX  = 0x00000008;
const unsigned int DDSCAPS_TEXTURE  = 0x00001000;
const unsigned int DDSCAPS_MIPMAP   = 0x00400000;
const unsigned int DDSCAPS2_VOLUME  = 0x00200000;
const unsigned int DDSCAPS2_CUBEMAP = 0x00000200;
const unsigned int DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400;
const unsigned int DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800;
const unsigned int DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000;
const unsigned int DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000;
const unsigned int DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000;
const unsigned int DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000;
#define DDSCAPS2_CUBEMAP_ALL_FACES (DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX | DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY | DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ)
 



static D3DFORMAT g_textureFormats[] = 
{
	D3DFMT_UNKNOWN,					
	D3DFMT_L8,
	D3DFMT_A8L8,
	D3DFMT_X8R8G8B8,
	D3DFMT_A8R8G8B8,
	D3DFMT_L16,
	D3DFMT_G16R16,
	D3DFMT_A16B16G16R16,
	D3DFMT_A16B16G16R16,
	D3DFMT_R16F,
	D3DFMT_G32R32F,
	D3DFMT_A16B16G16R16F,
	D3DFMT_A16B16G16R16F,
	D3DFMT_R32F, 
	D3DFMT_R32F,
	D3DFMT_A32B32G32R32F,
	D3DFMT_A32B32G32R32F,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_DXT1,   
	D3DFMT_DXT3,
	D3DFMT_DXT4,
	(D3DFORMAT)0x32495441,
	(D3DFORMAT)0x31495441,
	D3DFMT_A8L8,
};


static D3DTEXTUREFILTERTYPE g_filters[] = 
{
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_LINEAR,
	D3DTEXF_LINEAR,
	D3DTEXF_ANISOTROPIC,
	D3DTEXF_ANISOTROPIC,	
};

static D3DTEXTUREFILTERTYPE g_mipFilters[] =
{
	D3DTEXF_NONE,
	D3DTEXF_LINEAR,
	D3DTEXF_LINEAR,
	D3DTEXF_LINEAR,
	D3DTEXF_ANISOTROPIC,
	D3DTEXF_ANISOTROPIC,	
};

static unsigned int QTEXTURE_FOURCC(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3)
{
	return ((unsigned int)(c0 | (c1 << 8) | (c2 << 16) | (c3 << 24)));
}

static bool QTEXTURE_IS_DEPTH_FORMAT(const ETexturePixelFormat& fmt)
{
	return ((fmt >= QTEXTURE_FORMAT_DEPTH16) && (fmt <= QTEXTURE_FORMAT_DEPTH24));
}

static bool QTEXTURE_IS_PLAIN_FORMAT(const ETexturePixelFormat& fmt)
{
	return (fmt <= QTEXTURE_FORMAT_RGBA32F);
}

static bool QTEXTURE_IS_COMPRESSED_FORMAT(const ETexturePixelFormat& fmt)
{
	return (fmt >= QTEXTURE_FORMAT_DXT1);
}

static unsigned int QTEXTURE_GET_BYTES_PER_BLOCK(const ETexturePixelFormat& fmt)
{
	return (fmt == QTEXTURE_FORMAT_DXT1 || fmt == QTEXTURE_FORMAT_ATI1N) ? 8 : 16;
}

static unsigned int QTEXTURE_GET_BYTES_PER_PIXEL(const ETexturePixelFormat& fmt)
{
	static const unsigned int bpp[] = {0, 1, 2, 3, 4, 2, 4, 6, 8, 2, 4, 6, 8, 4, 8, 12, 16, 24, 1};
	return bpp[fmt];
}

static unsigned int QTEXTURE_GET_BYTES_PER_CHANNEL(const ETexturePixelFormat& fmt)
{
	return(fmt <= QTEXTURE_FORMAT_RGBA8) ? 1 : (fmt <= QTEXTURE_FORMAT_RGBA16F) ? 2 : 4;
}

static unsigned int QTEXTURE_GET_CHANNEL_COUNT(const ETexturePixelFormat& fmt)
{
	static const unsigned int channels[] = { 0, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 1, 3, 3, 4, 4, 2, 1 };
	return channels[fmt]; 	
}





static int getImageSize(const ETexturePixelFormat& fmt, const unsigned int& w, const unsigned int& h, const unsigned int& d, const unsigned int& nLevels)
{
	unsigned int width = w;
	unsigned int height = h;
	unsigned int depth = d;
	unsigned int levels = nLevels;

	bool isCubemap = (d == 0);
	if(isCubemap) depth = 1;
	
	int size = 0;
	while(levels)
	{
		if(QTEXTURE_IS_COMPRESSED_FORMAT(fmt))
			size += ((width + 3) >> 2) * ((height + 3) >> 2) * depth;
		else
			size += width * height * depth;
			
		if(width == 1 && height == 1 && depth == 1)
			break;
		
		if(width > 1) width >>= 1;
		if(height > 1) height >>= 1;
		if(depth > 1) depth >>= 1;
		--levels;
	}
	
	if(QTEXTURE_IS_COMPRESSED_FORMAT(fmt)) 
		size *= QTEXTURE_GET_BYTES_PER_BLOCK(fmt);
	else
		size *= QTEXTURE_GET_BYTES_PER_PIXEL(fmt);
		
	if(isCubemap)
		size *= 6;
	
	return size;
}




template <typename TYPE>
static void swapChannel(TYPE* pixels, const int& nPixels, const int& nChannels, const int& ch0, const int& ch1)
{
	int pixelCount = nPixels;

	do
	{
		TYPE tmp = pixels[ch1];
		pixels[ch1] = pixels[ch0];
		pixels[ch0] = tmp;
		pixels += nChannels;
	}while(--pixelCount);
}


template <typename TYPE>
static void genMipMap(TYPE* src, TYPE* dest, const unsigned int w, const unsigned int h, const unsigned int d, const unsigned int c)
{
	unsigned int xOffset = (w < 2) ? 0 : c;
	unsigned int yOffset = (h < 2) ? 0 : c * w;
	unsigned int zOffset = (d < 2) ? 0 : c * w * h;
	
	for(unsigned int z = 0; z < d; z += 2)
	{
		for(unsigned int y = 0; y < h; y += 2)
		{
			for(unsigned int x = 0; x < w; x += 2)
			{
				for(unsigned int i = 0; i < c; ++i)
				{
					*dest++ = (src[0] + src[xOffset] + src[yOffset] + src[yOffset + xOffset] + src[zOffset] + 
							   src[zOffset + xOffset] + src[zOffset + yOffset] + src[zOffset + yOffset + xOffset]) / 8;
					src++;
				}
				src += xOffset;
			}
			src += yOffset;
		}
		src += zOffset;
	}
}




static bool isPowerOfTwo(const int val)
{
	return (val & -val) == val;
}



static int getMipMapCount2(const int& maxDimension)
{
	int i = 0; 
	int maxDim = maxDimension;
	while(maxDim > 0)
	{
		maxDim >>= 1;
		++i;
	}
	
	return i;
}





static bool HasMipMapFlags(const unsigned int& flags)
{
	if(flags & QTEXTURE_FILTER_BILINEAR || flags & QTEXTURE_FILTER_TRILINEAR ||
	   flags & QTEXTURE_FILTER_BILINEAR_ANISO || flags & QTEXTURE_FILTER_TRILINEAR_ANISO)
		return true;
	
	return false;
}


static bool IsPlainFormat(const ETexturePixelFormat& fmt)
{
	return (fmt <= QTEXTURE_FORMAT_RGBA32F);
}

static bool IsUnsignedFormat(const ETexturePixelFormat& fmt)
{
	return (fmt <= QTEXTURE_FORMAT_RGBA16);
}


static int GetChannelCount(const ETexturePixelFormat& fmt)
{
	static const unsigned int channels[] = { 0, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 1, 3, 3, 4, 4, 2, 1 };
	return channels[fmt];
}


static int GetFilterIndex(const unsigned int& flags)
{
	if(flags & QTEXTURE_FILTER_NEAREST) return 0;
	else if(flags & QTEXTURE_FILTER_LINEAR) return 1;
	else if(flags & QTEXTURE_FILTER_BILINEAR) return 2;
	else if(flags & QTEXTURE_FILTER_TRILINEAR) return 3;
	else if(flags & QTEXTURE_FILTER_BILINEAR_ANISO) return 4;
	else if(flags & QTEXTURE_FILTER_TRILINEAR_ANISO) return 5;
	else return -1;
}

template <typename DATA_TYPE>
inline DATA_TYPE clamp(const DATA_TYPE x, const float lower, const float upper)
{
	return max(min(x, DATA_TYPE(upper)), DATA_TYPE(lower));
}

template <typename DATA_TYPE>
inline DATA_TYPE saturate(DATA_TYPE x)
{
	return clamp((x), 0, 1);
}




#pragma pack(push, 1)

// Truevision Targa header format 
typedef struct
{
	unsigned char	descLen;
	unsigned char	cmapType;
	unsigned char	imgType;
	unsigned short	cmapStart;
	unsigned short  cmapEntries;
	unsigned char	cmapBits;
	unsigned short	xOffset;
	unsigned short	yOffset;
	unsigned short	width;
	unsigned short	height;
	unsigned char	bpp;
	unsigned char	attrib;
}sTGAHeader;


// Direct Draw Surface header format 
struct sDDSHeader
{
	unsigned int		dwMagic;
	unsigned int		dwSize;
	unsigned int		dwFlags;
	unsigned int		dwHeight;
	unsigned int		dwWidth;
	unsigned int		dwPitchOrLinearSize;
	unsigned int		dwDepth;
	unsigned int		dwMipMapCount;
	unsigned int		dwReserved[11];
	
	struct
	{
		unsigned int		dwSize;
		unsigned int		dwFlags;
		unsigned int		dwFourCC;
		unsigned int		dwRGBBitCount;
		unsigned int		dwRBitMask;
		unsigned int		dwGBitMask;
		unsigned int		dwBBitMask;
		unsigned int		dwRGBAlphaBitMask;
	}ddpfPixelFormat;
	
	struct
	{
		unsigned int		dwCaps1;
		unsigned int		dwCaps2;
		unsigned int		reserved[2];
	}ddsCaps;
	
	unsigned int		dwReserved2;
};



#pragma pack(pop)




CQuadrionTextureFile::CQuadrionTextureFile()
{
	pixelFormat = QTEXTURE_FORMAT_NONE;
	width = height = bpp = depth = 0;
	nMipMaps = 0;
	m_bIsLoaded = false;
	greyscale = NULL;
}


CQuadrionTextureFile::CQuadrionTextureFile(const CQuadrionTextureFile& tex)
{
	pixelFormat = tex.pixelFormat;
	width = tex.width;
	height = tex.height;
	bpp = tex.bpp;
	depth = tex.depth;
	nMipMaps = tex.nMipMaps;
	m_bIsLoaded = tex.m_bIsLoaded;
}


CQuadrionTextureFile& CQuadrionTextureFile::operator= (const CQuadrionTextureFile& tex)
{
	this->pixelFormat = tex.pixelFormat;
	this->width = tex.width;
	this->height = tex.height;
	this->depth = tex.depth;
	this->bpp = tex.bpp;
	this->nMipMaps = tex.nMipMaps;
	this->m_bIsLoaded = tex.m_bIsLoaded;

	return *this;
}

CQuadrionTextureFile::~CQuadrionTextureFile()
{	
	for(unsigned int i = 0; i < pixels.size(); ++i)
	{
		if(pixels[i])
		{
			delete[] pixels[i];
			pixels[i] = NULL;
		}
	}
	
	for(unsigned int i = 0; i < normalMap.size(); ++i)
	{
		if(normalMap[i])
		{
			delete[] normalMap[i];
			normalMap[i] = NULL;
		}
	}
	
	if(greyscale)
	{
		delete[] greyscale;
		greyscale = NULL;
	}

	pixelFormat = QTEXTURE_FORMAT_NONE;
	width = height = depth = bpp = 0;
	nMipMaps = 0;
	m_bIsLoaded = false;
}



////////////////////////////////////////////////////////////////////////
// loadLightmap
// pix- RGB8 format pixel data in 128x128 resolution
// 
// Used for loading in lightmaps from the QBSP file format map 
bool CQuadrionTextureFile::LoadLightmap(unsigned char* pix)
{	
	for(unsigned int i = 0; i < pixels.size(); ++i)
	{
		if(pixels[i])
		{
			delete[] pixels[i];
			pixels[i] = NULL;
		}
	}
	
	width = height = 128;
	bpp = 24;
	nMipMaps = 1;
	pixelFormat = QTEXTURE_FORMAT_RGB8;
	
	unsigned int pixelSize = bpp / 8;
	unsigned int size = pixelSize * width * height;
	
	depth = 1;
	
	unsigned int start = pixels.size();
	unsigned char* newPix = new unsigned char[size];
	if(!newPix)
		return false;
	memcpy(newPix, pix, size);
	pixels.push_back(newPix);
	
	fileName = "lightmap";
	m_bIsLoaded = true;
	
	return true;
}

bool CQuadrionTextureFile::LoadFromColor( const unsigned int& color )
{
	for(unsigned int i = 0; i < pixels.size(); ++i)
	{
		if(pixels[i])
		{
			delete[] pixels[i];
			pixels[i] = NULL;
		}
	}
	
	width = 2;
	height = 2;
	depth = 1;
	nMipMaps = 1;
	pixelFormat = QTEXTURE_FORMAT_RGBA8;
	unsigned int pixelSize = QTEXTURE_GET_BYTES_PER_PIXEL( QTEXTURE_FORMAT_RGBA8 );
	bpp = pixelSize * 8;
	unsigned int size = width * height * pixelSize;
	unsigned int start = pixels.size();
	unsigned char* newPix = new unsigned char[size];

	for( int i = 0; i < 4; ++i )
	{
		newPix[i * 4] = ( color & 0xff000000 ) >> 24;
		newPix[i * 4 + 1] = ( color & 0x00ff0000 ) >> 16;
		newPix[i * 4 + 2] = ( color & 0x0000ff00 ) >> 8;
		newPix[i * 4 + 3] = 0xff; 
	}
	pixels.push_back( newPix );
	
	m_bIsLoaded = true;

	return true;
}



//////////////////////////////////////////////////////////////////////
// loadTexture
// fname- file name with path and extension
//
// Load a texture file from filename
bool CQuadrionTextureFile::LoadTexture(const char* fname, const char* pname)
{
	const char* ext = strrchr(fname, '.');
	if(!ext)
		return false;
		
	m_pathName = pname;
	std::string full = m_pathName + fname;
	
	++ext;
	if(stricmp(ext, "dds") == 0)
		return LoadDDS(full.c_str());
	
	else if(stricmp(ext, "tga") == 0)
		return LoadTGA(full.c_str());
	
	else if(stricmp(ext, "jpg") == 0)
		return LoadJPG(full.c_str());
	
	else 
		return false;
}



///////////////////////////////////////////////////////////////////////////////
// loadTGA
// Load .TGA texture from file name
bool CQuadrionTextureFile::LoadTGA(const char* fname)
{
	for(unsigned int i = 0; i < pixels.size(); ++i)
	{
		if(pixels[i])
		{
			delete[] pixels[i];
			pixels[i] = NULL;
		}
	}
	
	sTGAHeader header;
	FILE* file;
	if((file = fopen(fname, "rb")) == NULL)
		return false;
	
	fread(&header, sizeof(header), 1, file);
	if(header.descLen)
		fseek(file, header.descLen, SEEK_CUR);
	
	width = header.width;
	height = header.height;
	bpp = header.bpp;
	nMipMaps = 1;
	unsigned int pixelSize = bpp / 8;
	unsigned int size = width * height * pixelSize;
	depth = 1;
	
	unsigned char* pix = new unsigned char[size];
	if(!pix)
		return false;
	fread(pix, size, 1, file);
	fclose(file);
	
	unsigned char* dest, *src = pix + width * (height - 1) * pixelSize;
	unsigned char* newPix;
	unsigned int x, y;
	
	switch(bpp)
	{
		case 8:
			pixelFormat = QTEXTURE_FORMAT_I8;
			dest = newPix = new unsigned char[width * height];
			for(y = 0; y < height; ++y)
			{
				memcpy(dest, src, width);
				dest += width;
				src -= width;
			}
			break;
		
		case 16:
			pixelFormat = QTEXTURE_FORMAT_RGBA8;
			dest = newPix = new unsigned char[width * height * 4];
			for(y = 0; y < height; ++y)
			{
				for(x = 0; x < width; ++x)
				{
					unsigned short tmpPix = *((unsigned char*)src);
					dest[0] = ((tmpPix >> 10) & 0x1F) << 3;
					dest[1] = ((tmpPix >> 5) & 0x1F) << 3;
					dest[2] = ((tmpPix) & 0x1F) << 3;
					dest[3] = ((tmpPix >> 15) ? 0xFF : 0);
					dest += 4;
					src += 4;
				}
				
				src -= 4 * width;
			}
			break;
		
		case 24:
			pixelFormat = QTEXTURE_FORMAT_RGB8;
			dest = newPix = new unsigned char[width * height * 3];
			for(y = 0; y < height; ++y)
			{
				for(x = 0; x < width; ++x)
				{
					*dest++ = src[2];
					*dest++ = src[1];
					*dest++ = src[0];
					src += 3;
				}
				
				src -= 6 * width;
			}
			break;
		
		case 32:
			pixelFormat = QTEXTURE_FORMAT_RGBA8;
			dest = newPix = new unsigned char[width * height * 4];
			for(y = 0; y < height; ++y)
			{
				for(x = 0; x < width; ++x)
				{
					*dest++ = src[2];			
					*dest++ = src[1];			
					*dest++ = src[0];			
					*dest++ = src[3];			
					src += 4;
				}
				
				src -= 8 * width;
			}
			break;
	}
	
	if(header.attrib & 0x20)
	{
		if(QTEXTURE_IS_COMPRESSED_FORMAT(pixelFormat))
		{
			delete[] newPix;
			newPix = NULL;
			return false;
		}
		
		unsigned int lineWidth = QTEXTURE_GET_BYTES_PER_PIXEL(pixelFormat) * width;
		unsigned char* newPixels = new unsigned char[lineWidth * height];
		if(!newPixels)
			return false;
		
		for(unsigned int i = 0; i < height; ++i)
		{
			memcpy(newPixels + i * lineWidth, newPix + (height - 1 - i) * lineWidth, lineWidth);
		}
		
		pixels.push_back(newPixels);
		delete[] newPix;
		newPix = NULL;
	}
	
	else
		pixels.push_back(newPix);
	
	delete[] pix;
	
	fileName = fname;
	m_bIsLoaded = true;
	return true;
}



////////////////////////////////////////////////////////////////////////
// loadDDS
// Load .DDS texture from filename
bool CQuadrionTextureFile::LoadDDS(const char* fname)
{
	for(unsigned int i = 0; i < pixels.size(); ++i)
	{
		if(pixels[i])
		{
			delete[] pixels[i];
			pixels[i] = NULL;
		}
	}

	depth = 0;
	nMipMaps = 0;
	
	sDDSHeader header;
	FILE* file;
	if((file = fopen(fname, "rb")) == NULL)
		return false;
	
	fread(&header, sizeof(header), 1, file);
	if(header.dwMagic != QTEXTURE_FOURCC('D', 'D', 'S', ' '))
	{
		fclose(file);
		return false;
	}
	
	width = header.dwWidth;
	height = header.dwHeight;
	depth = header.dwDepth;
	nMipMaps = header.dwMipMapCount;
	
	if(header.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP)
		depth = 0;
	
	else
	{
		if(depth == 0)
			depth = 1;
	}
	
	if(nMipMaps <= 0)
		nMipMaps = 1;
		
	switch(header.ddpfPixelFormat.dwFourCC)
	{
		case 0x31545844:
			pixelFormat = QTEXTURE_FORMAT_DXT1;
			break;
		
		case 0x33545844:
			pixelFormat = QTEXTURE_FORMAT_DXT3;
			break;
		
		case 0x35545844:
			pixelFormat = QTEXTURE_FORMAT_DXT5;
			break;
		
		case 0x32495441:
			pixelFormat = QTEXTURE_FORMAT_ATI2N;
			break;
		
		case 0x31495441:
			pixelFormat = QTEXTURE_FORMAT_ATI1N;
			break;
		
		case 34:
			pixelFormat = QTEXTURE_FORMAT_IA16;
			break;
		
		case 36:
			pixelFormat = QTEXTURE_FORMAT_RGBA16;
			break;
		
		case 113:
			pixelFormat = QTEXTURE_FORMAT_RGBA16F;
			break;
		
		case 114:
			pixelFormat = QTEXTURE_FORMAT_I32F;
			break;
		
		case 115:
			pixelFormat = QTEXTURE_FORMAT_IA32F;
			break;
		
		case 116:
			pixelFormat = QTEXTURE_FORMAT_RGBA32F;
			break;
		
		default:
			switch(header.ddpfPixelFormat.dwRGBBitCount)
			{
				case 8:
					if(header.ddpfPixelFormat.dwRBitMask == 0xE0)
						pixelFormat = QTEXTURE_FORMAT_RGB332;
					else
						pixelFormat = QTEXTURE_FORMAT_I16;
					break;
					
				case 16:
					if(header.ddpfPixelFormat.dwRGBAlphaBitMask)
						pixelFormat = QTEXTURE_FORMAT_IA8;
					else
						pixelFormat = QTEXTURE_FORMAT_I16;
					break;
				
				case 24:
					pixelFormat = QTEXTURE_FORMAT_RGB8;
					break;
				
				case 32:
					pixelFormat = QTEXTURE_FORMAT_RGBA8;
					break;
				
				default:
					fclose(file);
					return false;
			}
	}
	
	int size = GetSizeWithMipMaps(0, nMipMaps);
	unsigned char* newPix = new unsigned char[size];
	
	if(IsCubemap())
	{
		for(unsigned int face = 0; face < 6; ++face)
		{
			for(unsigned int mipLevel = 0; mipLevel < nMipMaps; ++mipLevel)
			{
				int faceSize = GetSizeWithMipMaps(mipLevel, 1) / 6;
				unsigned char* src = GetData(mipLevel) + face * faceSize;
				fread(src, 1, faceSize, file);
			}
		}
	}
	
	else
		fread(newPix, 1, size, file);
	
	if((pixelFormat == QTEXTURE_FORMAT_RGB8 || pixelFormat == QTEXTURE_FORMAT_RGBA8) && header.ddpfPixelFormat.dwBBitMask == 0xFF)
	{
		int nChannels = QTEXTURE_GET_CHANNEL_COUNT(pixelFormat);
		swapChannel(newPix, size / nChannels, nChannels, 0, 2);
	}
	
	fclose(file);
	
	pixels.push_back(newPix);
	fileName = fname;
	m_bIsLoaded = true;
	return true;
}




//////////////////////////////////////////////////////////////////////////////
// loadJPG
// Load .JPG texture from file name
bool CQuadrionTextureFile::LoadJPG(const char* fname)
{	
	for(unsigned int i = 0; i < pixels.size(); ++i)
	{
		if(pixels[i])
		{
			delete[] pixels[i];
			pixels[i] = NULL;
		}
	}

	
	jpeg_decoder_file_stream jpegStream;
	if(!jpegStream.open(fname))
		return false;
	
	int fileSize = jpegStream.get_size();
	size_t bytesRead;
	bool eof;
	unsigned char* fileBuf = new unsigned char[fileSize];
	if(!fileBuf)
		return false;
	
	bytesRead = jpegStream.read(fileBuf, fileSize, &eof);
	jpegStream.reset();
	
	jpeg_decoder jpegDecode(&jpegStream, true);
	if(jpegDecode.begin() != JPGD_OKAY)
	{
		delete[] fileBuf;
		fileBuf = NULL;
		return false;
	}
	
	width = jpegDecode.get_width();
	height = jpegDecode.get_height();
	bpp = jpegDecode.get_bytes_per_pixel() * 8;
	depth = 1;
	nMipMaps = 1;
	
	unsigned int lineLen = 0;
	int y, hr;
	
	unsigned char* newPix = new unsigned char[width * height * (bpp / 8)];
	if(!newPix)
	{
		delete[] fileBuf;
		fileBuf = NULL;
		return false;
	}

	unsigned char* dest = newPix;
	
	switch(bpp)
	{
		case 8:
			pixelFormat = QTEXTURE_FORMAT_I8;
			for(y = 0; y < height; ++y)
			{
				hr = jpegDecode.decode((void**)&dest, &lineLen);
				if(hr == JPGD_FAILED || hr == JPGD_DECODE_ERROR)
				{
					delete[] newPix;
					newPix = NULL;
					dest = NULL;
					delete[] fileBuf;
					fileBuf = NULL;
					return false;
				}
				memcpy(newPix + (y + width), dest, lineLen);
				lineLen = 0;
			}
			break;
		
		case 24:
			pixelFormat = QTEXTURE_FORMAT_RGB8;
			for(y = 0; y < height; ++y)
			{
				hr = jpegDecode.decode((void**)&dest, &lineLen);
				if(hr == JPGD_FAILED || hr == JPGD_DECODE_ERROR)
				{
					delete[] newPix;
					delete[] fileBuf;
					newPix = NULL;
					fileBuf = NULL;
					dest = NULL;
					return false;
				}	
				memcpy(newPix + (y * width * 3), dest, lineLen);
			}
			break;
		
		case 32:
			pixelFormat = QTEXTURE_FORMAT_RGBA8;
			for(y = 0; y < height; ++y)
			{
				hr = jpegDecode.decode((void**)&dest, &lineLen);
				if(hr == JPGD_FAILED || hr == JPGD_DECODE_ERROR)
				{
					delete[] newPix;
					delete[] fileBuf;
					newPix = NULL;
					fileBuf = NULL;
					dest = NULL;
					return false;
				}
				
				memcpy(newPix + (y * width * 4), dest, lineLen);
			}
			break;
	}
	
	delete[] fileBuf;
	fileBuf = NULL;
	
	
	pixels.push_back(newPix);
	jpegDecode.~jpeg_decoder();
	fileName = fname;
	m_bIsLoaded = true;
	
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getSizeWithMipMaps
// Obtain image size in bytes from first mip to nLevels mips 
int CQuadrionTextureFile::GetSizeWithMipMaps(const unsigned int first, unsigned int nLevels, ETexturePixelFormat fmt)
{
	if(fmt == QTEXTURE_FORMAT_NONE)
		fmt = pixelFormat;
	
	return getImageSize(fmt, GetWidth(first), GetHeight(first), (depth > 0) * GetDepth(first), nLevels);	
}



////////////////////////////////////////////////////////////
// getWidth
// Get base level image width
int CQuadrionTextureFile::GetWidth()
{
	return width;
}



///////////////////////////////////////////////////////////
// getHeight
// Get base level image height
int CQuadrionTextureFile::GetHeight()
{
	return height;
}



////////////////////////////////////////////////////////
// getDepth
// Get base level image depth
int CQuadrionTextureFile::GetDepth()
{
	return depth;
}


///////////////////////////////////////////////////////////
// getWidth
// get width at mip level
int CQuadrionTextureFile::GetWidth(const unsigned int& level)
{
	int a = width >> level;
	return (a == 0) ? 1 : a;	
}


////////////////////////////////////////////////////////////
// getHeight
// Get height at mip level
int CQuadrionTextureFile::GetHeight(const unsigned int& level)
{
	int a = height >> level;
	return (a == 0) ? 1 : a;
}


//////////////////////////////////////////////////////////
// getDepth
// Get depth at mip level
int CQuadrionTextureFile::GetDepth(const unsigned int& level)
{
	int a = depth >> level;
	return (a == 0) ? 1 : a;
}



///////////////////////////////////////////////////////////
// isCubemap
// Is the texture a cubemap?
bool CQuadrionTextureFile::IsCubemap()
{
	return (depth == 0);
}


//////////////////////////////////////////////////////////////////////////////
// getData
// Get pixel surface pointer at a certain mip level
unsigned char* CQuadrionTextureFile::GetData(const unsigned int& level) const
{
	int mipMappedSize;
	int w, h, d;
	w = width >> 0;
	if(w == 0) w = 1;
	h = height >> 0;
	if(h == 0) h = 1;
	d = depth >> 0;
	if(d == 0) d = 1;
	
	unsigned int highest = pixels.size() - 1;
	
	mipMappedSize = getImageSize(pixelFormat, w, h, (depth > 0) * d, level);
	return(level < nMipMaps) ? (unsigned char*)(pixels[highest]) + mipMappedSize : NULL;			
}

/////////////////////////////////////////////////////////////////////////////////
// GetHeightmapData
// Get heightmap data pointer at a certain mip level
unsigned char* CQuadrionTextureFile::GetGreyscaleData(const unsigned int& level) const
{
	int mipMappedSize;
	int w, h, d;
	w = width >> 0;
	if(w == 0) w = 1;
	h = height >> 0;
	if(h == 0) h = 1;
	d = depth >> 0;
	if(d == 0) d = 1;
	
	mipMappedSize = getImageSize(QTEXTURE_FORMAT_I8, w, h, (depth > 0) * d, level);
	return(level < nMipMaps) ? (unsigned char*)(greyscale) + mipMappedSize : NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////
// GetNormalmapData
// Get normal map data pointer at a certain mip level
unsigned char* CQuadrionTextureFile::GetNormalmapData(const unsigned int& level) const
{
	int mipMappedSize;
	int w, h, d;
	w = width >> 0;
	if(w == 0) w = 1;
	h = height >> 0;
	if(h == 0) h = 1;
	d = depth >> 0;
	if(d == 0) d = 1;
	
	mipMappedSize = getImageSize(pixelFormat, w, h, (depth > 0) * d, level);
	return(level < nMipMaps) ? (unsigned char*)(normalMap[normalMap.size() - 1]) + mipMappedSize : NULL;	
}


////////////////////////////////////////////////////////////////////////////////
// swapChannels
// Swap channels from ch0 to ch1 in existing texture
bool CQuadrionTextureFile::SwapChannels(const unsigned int& ch0, const unsigned int& ch1, bool useNormalmap)
{
	if(!QTEXTURE_IS_PLAIN_FORMAT(pixelFormat))
		return false;
	
	unsigned int nPixels = GetPixelCount(0, nMipMaps);
	unsigned int nChannels = QTEXTURE_GET_CHANNEL_COUNT(pixelFormat);
	unsigned int largest = (useNormalmap) ? normalMap.size() - 1 : pixels.size() - 1;
	
	if(pixelFormat <= QTEXTURE_FORMAT_RGBA8)
	{
		if(!useNormalmap)
			swapChannel((unsigned char*)pixels[largest], nPixels, nChannels, ch0, ch1);
		else
		{
			swapChannel((unsigned char*)normalMap[largest], nPixels, nChannels, ch0, ch1);
			return true;
		}
	}
	else if(pixelFormat <= QTEXTURE_FORMAT_RGBA16F)
		swapChannel((unsigned short*)pixels[largest], nPixels, nChannels, ch0, ch1);
	else
		swapChannel((float*)pixels[largest], nPixels, nChannels, ch0, ch1);
	
	return true;
}



/////////////////////////////////////////////////////////////////////////////////
// getPixelCount
// Get total pixel count in image from firstMip to nLevels mips
int CQuadrionTextureFile::GetPixelCount(const int firstMip, int nLevels) const
{
	int w = width >> firstMip;
	if(w == 0) w = 1;
	
	int h = height >> firstMip;
	if(h == 0) h = 1;

	int d = depth >> firstMip;
	if(d == 0) d = 1;
	
	w <<= 1;
	h <<= 1;
	d <<= 1;

	int size = 0;
	
	while(nLevels && (w != 1 || h != 1 || d != 1))
	{
		if(w > 1) w >>= 1;
		if(h > 1) h >>= 1;
		if(d > 1) d >>= 1;
		
		size += w * h * d;
		--nLevels;
	}
	
	if(depth == 0)
		size *= 6;
	
	return size;
}




bool CQuadrionTextureFile::GenerateMipMaps(const unsigned int nMips)
{
	if(!QTEXTURE_IS_PLAIN_FORMAT(pixelFormat))
		return false;
	if(!isPowerOfTwo(width) || !isPowerOfTwo(height))
		return false;
	if(width != height)
		return false;
	
	nMipMaps = min(getMipMapCount2(max(max(width, height), depth)), nMips);
	int mipMappedSize = GetSizeWithMipMaps();
	

	unsigned char* newPix = new unsigned char[mipMappedSize];
	if(!newPix)
		return false;

	unsigned int largest = pixels.size() - 1;
	int nChannels = QTEXTURE_GET_CHANNEL_COUNT(pixelFormat);
	int channelSize = QTEXTURE_GET_BYTES_PER_CHANNEL(pixelFormat);
	memcpy(newPix, pixels[largest], width * height * depth * nChannels * channelSize);
	unsigned char* dest, *src = newPix;

	
	if(depth == 0)
	{
		int dim = width;
		for(int i = 1; i < nMipMaps; ++i)
		{
			int sliceSize = dim * dim * nChannels * channelSize;
			dest = src + 6 * sliceSize;
			
			for(unsigned int s = 0; s < 6; ++s)
			{
				if(channelSize == 1)
					genMipMap(src + s * sliceSize, dest + s * sliceSize / 4, dim, dim, 1, nChannels);
				else if(channelSize == 2)
					genMipMap((unsigned short*)(src + s * sliceSize), (unsigned short*)(dest + s * sliceSize / 4), dim, dim, 1, nChannels);
				else
					genMipMap((float*)(src + s * sliceSize), (float*)(dest + s * sliceSize / 4), dim, dim, 1, nChannels);
			}
			
			src = dest;
			dim >>= 1;
		}
	}
	
	else
	{
		int w = width;
		int h = height;
		int d = depth;
		
		for(int i = 1; i < nMipMaps; ++i)
		{
			dest = src + w * h * d * nChannels * channelSize;
			if(channelSize == 1)
				genMipMap(src, dest, w, h, d, nChannels);
			else if(channelSize == 2)
				genMipMap((unsigned short*)src, (unsigned short*)dest, w, h, d, nChannels);
			else
				genMipMap((float*)src, (float*)dest, w, h, d, nChannels);
			
			src = dest;
			if(w > 1) w >>= 1;
			if(h > 1) h >>= 1;
			if(d > 1) d >>= 1;
		}
	}
	
	pixels.push_back(newPix);
	return true;
}


std::string CQuadrionTextureFile::GetFileName()
{
	return fileName;
}

void CQuadrionTextureFile::GetFileName(const std::string* fName)
{
	fName->copy((char*)fileName.c_str(), fileName.size());
}

int CQuadrionTextureFile::GetMipMapCount()
{
	return nMipMaps;
}

bool CQuadrionTextureFile::ConvertToGreyscale(const float rf, const float gf, const float bf)
{
	if (pixelFormat < QTEXTURE_FORMAT_RGB8 || pixelFormat > QTEXTURE_FORMAT_RGBA8) 
		return false;

	int nChannels = (pixelFormat == QTEXTURE_FORMAT_RGB8)? 3 : 4;
	int nPixels = GetPixelCount(0, nMipMaps);

	unsigned char* src = pixels[pixels.size() - 1];
	unsigned char* newPixels = new unsigned char[nPixels];
	unsigned char *dest = newPixels;
	do
	{
		*dest++ = (unsigned char) (src[0] * rf + src[1] * gf + src[2] * bf);
		src += nChannels;
	} while (--nPixels);
	
//	pixelFormat = QTEXTURE_FORMAT_I8;
	greyscale = newPixels;
	return true;
}

bool CQuadrionTextureFile::HeightToNormal(const bool useRGBA, const bool keepHeight, float sz, float mipScaleZ)
{
	unsigned int xMask = 0, yMask = 0, zMask = 0, hMask = 0;
	unsigned int xShift = 0, yShift = 0, zShift = 0, hShift = 0, hFlip = 0;
//	switch (pixelFormat)
//	{
//		case QTEXTURE_FORMAT_RGBA8:
			xMask = yMask = zMask = hMask = 0xFF;
			yShift = 8;
			zShift = 16;
			hShift = 24;
//			break;
			
//		default:
//			return false;
//	}

	if (pixelFormat == QTEXTURE_FORMAT_RGB8 || pixelFormat == QTEXTURE_FORMAT_RGBA8) 
	{	
		if(!ConvertToGreyscale())
			return false;
	}
	
	pixelFormat = QTEXTURE_FORMAT_RGBA8;


	const float sobelX[5][5] = {
		1,  2,  0,  -2, -1,
		4,  8,  0,  -8, -4,
		6, 12,  0, -12, -6,
		4,  8,  0,  -8, -4,
		1,  2,  0,  -2, -1,
	};
	const float sobelY[5][5] = {
		 1,  4,   6,  4,  1,
		 2,  8,  12,  8,  2,
		 0,  0,   0,  0,  0,
		-2, -8, -12, -8, -2,
		-1, -4,  -6, -4, -1,
	};

	bool useShort = (QTEXTURE_GET_BYTES_PER_PIXEL(pixelFormat) == 2);

	float xyBias = float(IsUnsignedFormat(pixelFormat));
	float zBias  = xyBias;
	float xFactor = 0.5f * xMask;
	float yFactor = 0.5f * yMask;
	float zFactor = 0.5f * zMask;


	// Size of the z component
	sz *= 128.0f / max(width, height);

	unsigned char* newPixels = new unsigned char[GetSizeWithMipMaps(0, nMipMaps, pixelFormat)];

	union 
	{
		unsigned int* iDest;
		unsigned short* sDest;
	};
	
	iDest = (unsigned int*) newPixels;

	for(int mipMap = 0; mipMap < nMipMaps; mipMap++)
	{
		unsigned char* src = GetGreyscaleData(mipMap);

		int w = GetWidth(mipMap);
		int h = GetHeight(mipMap);

		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++)
			{
				// Apply a 5x5 Sobel filter
				float sX = 0;
				float sY = 0;
				for (int dy = 0; dy < 5; dy++)
				{
					int fy = (y + dy - 2 + h) % h;
					for (int dx = 0; dx < 5; dx++)
					{
						int fx = (x + dx - 2 + w) % w;
						sX += sobelX[dy][dx] * src[fy * w + fx];
						sY += sobelY[dy][dx] * src[fy * w + fx];
					}
				}
				// Construct the components
				sX *= 1.0f / (48 * 255);
				sY *= 1.0f / (48 * 255);

				// Normalize and store
				float invLen = 1.0f / sqrtf(sX * sX + sY * sY + sz * sz);
				float rX = xFactor * (sX * invLen + xyBias);
				float rY = yFactor * (sY * invLen + xyBias);
				float rZ = zFactor * (sz * invLen +  zBias);

				unsigned int result = 0;
				result |= (int(rX) & xMask) << xShift;
				result |= (int(rY) & yMask) << yShift;
				result |= (int(rZ) & zMask) << zShift;
				result |= ((src[y * w + x] ^ hFlip) & hMask) << hShift;

				if (useShort)
					*sDest++ = result;
				else 
					*iDest++ = result;
			}
		}
		sz *= mipScaleZ;
	}

//	format = destFormat;
//	delete pixels;
//	pixels = newPixels;
	normalMap.push_back(newPixels);

//	if (useCompression) compress(FORMAT_ATI2N);

	return true;
}


ETexturePixelFormat CQuadrionTextureFile::GetPixelFormat()
{
	return pixelFormat;
}


bool CQuadrionTextureFile::AddChannel(const float val)
{
	unsigned int nChannels = QTEXTURE_GET_CHANNEL_COUNT(pixelFormat);
	if(QTEXTURE_IS_COMPRESSED_FORMAT(pixelFormat) || nChannels == 4)
		return false;
	
	unsigned int value;
	if(pixelFormat <= QTEXTURE_FORMAT_RGBA8)
		value = (unsigned int)(255.0F * saturate(val));
	else if(pixelFormat <= QTEXTURE_FORMAT_RGBA16)
		value = (unsigned int)(65535.0F * saturate(val));
	else
		value = *(unsigned int*)&val;
	
	int nPixels = GetPixelCount(0, nMipMaps);
	int bpc = QTEXTURE_GET_BYTES_PER_CHANNEL(pixelFormat);
	int cs = bpc * nChannels;
	unsigned int largest = pixels.size() - 1;
	unsigned char* newPix = new unsigned char[nPixels * (nChannels + 1) * bpc];
	unsigned char* dest = newPix;

	unsigned char* src = pixels[largest];
	for(unsigned int i = 0; i < nPixels; ++i)
	{
		memcpy(dest, src, cs);
		dest += cs;
		src += cs;
		memcpy(dest, &value, bpc);
		dest += bpc;
	}
	
	pixels.push_back(newPix);
	pixelFormat = (ETexturePixelFormat)(pixelFormat + 1);
	
	return true;
}


bool CQuadrionTextureFile::Is2D()
{
	return(height > 1 && depth == 1);
}


bool CQuadrionTextureFile::Is3D()
{
	return (depth > 1);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

CQuadrionTextureObject::CQuadrionTextureObject(const unsigned int handle, const std::string& name, const std::string& path) : CQuadrionResource(handle, name, path)
{
	m_pRenderDevice = NULL;
	m_pTextureObject = NULL;
	
	m_textureWidth = 0;
	m_textureHeight = 0;
	m_anisotropy = 0;
	m_usage = 0;
	m_assignedTextureUnit = -1;
	m_bIsLoaded = false;
	m_bIsBound = false;
	m_pixelFormat = D3DFMT_UNKNOWN;
	m_textureFlags = 0;
	
	memset(&m_textureSampler, 0, sizeof(SQuadrionTextureSampler));
}

CQuadrionTextureObject::~CQuadrionTextureObject()
{
//	UnbindTexture();
	m_pRenderDevice = NULL;
	
	if(m_pTextureObject)
	{
		m_pTextureObject->Release();
		m_pTextureObject = NULL;
	}
	

	
	memset(&m_textureSampler, 0, sizeof(SQuadrionTextureSampler));
	
	m_textureWidth = 0;
	m_textureHeight = 0;
	m_anisotropy = 0;
	m_pixelFormat = D3DFMT_UNKNOWN;
	m_usage = 0;
	m_assignedTextureUnit = -1;
	m_pQuadrionRender = NULL;
	m_bIsLoaded = false;
	m_bIsBound = false;
	m_textureFlags = 0;
}



void CQuadrionTextureObject::ChangeRenderDevice(const void* pRender)
{
	m_pQuadrionRender = (CQuadrionRender*)pRender;
	m_pRenderDevice = m_pQuadrionRender->m_pD3DDev;
}


bool CQuadrionTextureObject::CreateTextureFromFile(unsigned int& flags)
{
	// Get full path and attempt to load texture //
	std::string fullpath = GetFilename();
	CQuadrionTextureFile tex;
	std::string baseFileName(fullpath);
	int extLoc = 0;

	// Search the last 5 characters for a '.', which means an extensions was already provided
	extLoc = fullpath.substr(fullpath.size() - 5, fullpath.size()).find(".");

	if(extLoc > 0)
	{
		if(!tex.LoadTexture(fullpath.c_str()))
			return false;
	}
	else
	{	
		baseFileName.append(".tga");

		if(!tex.LoadTexture(baseFileName.c_str()))
		{
			baseFileName = fullpath;
			baseFileName.append(".jpg");

			if(!tex.LoadTexture(baseFileName.c_str()))
			{
				baseFileName.assign("textures/noshader.tga");

				if(!tex.LoadTexture(baseFileName.c_str()))
					return false;
			}
		}
	}
	
	m_textureFlags = flags;
	
	// Look for mip flags and generate mip maps //
	bool mipGenerationFailed = false;
	if(tex.GetMipMapCount() <= 1 && HasMipMapFlags(flags))
	{
		if(!tex.GenerateMipMaps())
			mipGenerationFailed = true;
	}
	
	// Look for normal/heightmap generation flags //
	if((tex.GetHeight() > 0 && tex.GetDepth() == 1) && (flags & QTEXTURE_NORMALMAP))
	{
		if(!tex.HeightToNormal(true, (flags & QTEXTURE_KEEPHEIGHT) != 0))
			return false;
	}
	
	// Look for cubemap flags and clamp ranges //
	if(tex.IsCubemap())
		flags |= (QTEXTURE_CLAMP_S | QTEXTURE_CLAMP_T);
	
	ETexturePixelFormat fmt = tex.GetPixelFormat();
	if(IsPlainFormat(fmt) && GetChannelCount(fmt) == 3)
	{
		if(!tex.AddChannel(1))
			return false;
		
		fmt = tex.GetPixelFormat();
	}
	
	m_pixelFormat = g_textureFormats[fmt];
	m_textureWidth = tex.GetWidth();
	m_textureHeight = tex.GetHeight();
	
	m_usage = 0;
	if(flags & QTEXTURE_DYNAMIC)
		m_usage |= D3DUSAGE_DYNAMIC;
	if(mipGenerationFailed)
		m_usage |= D3DUSAGE_AUTOGENMIPMAP;
		
	D3DPOOL pool = D3DPOOL_MANAGED;

	LPDIRECT3DTEXTURE9 texture;
	LPDIRECT3DCUBETEXTURE9 cubeTexture;
	LPDIRECT3DVOLUMETEXTURE9 volumeTexture;
	
	if(tex.IsCubemap())
	{
		if(FAILED(m_pRenderDevice->CreateCubeTexture(m_textureWidth, tex.GetMipMapCount(), m_usage, m_pixelFormat, pool, &cubeTexture, NULL)))
			return false;

		m_pTextureObject = cubeTexture;
	}
	
	else if(tex.Is3D())
	{
		if(FAILED(m_pRenderDevice->CreateVolumeTexture(m_textureWidth, m_textureHeight, tex.GetDepth(), tex.GetMipMapCount(), m_usage, m_pixelFormat, pool, &volumeTexture, NULL)))
			return false;

		m_pTextureObject = volumeTexture;
	}
	
	else
	{
		if(FAILED(m_pRenderDevice->CreateTexture(m_textureWidth, m_textureHeight, tex.GetMipMapCount(), m_usage, m_pixelFormat, pool, &texture, NULL)))
			return false;
		
		m_pTextureObject = texture;
	}
	
	if(m_pixelFormat == D3DFMT_A8R8G8B8)
	{
		if(!tex.SwapChannels(0, 2, flags & QTEXTURE_NORMALMAP))
			return false;
	}
	
	unsigned char* src;
	int mipLevel = 0;
	
	while((src = (flags & QTEXTURE_NORMALMAP) ? tex.GetNormalmapData(mipLevel) : tex.GetData(mipLevel)) != NULL)
	{
		int size = tex.GetSizeWithMipMaps(mipLevel, 1);
		if(tex.Is3D())
		{
			D3DLOCKED_BOX box;
			if(SUCCEEDED(volumeTexture->LockBox(mipLevel, &box, NULL, 0)))
			{
				memcpy(box.pBits, src, size);
				volumeTexture->UnlockBox(mipLevel);
			}
		}
		
		else if(tex.IsCubemap())
		{
			size /= 6;
			D3DLOCKED_RECT rect;
			for(unsigned int i = 0; i < 6; ++i)
			{
				if(SUCCEEDED(cubeTexture->LockRect((D3DCUBEMAP_FACES)i, mipLevel, &rect, NULL, 0)))
				{
					memcpy(rect.pBits, src, size);
					cubeTexture->UnlockRect((D3DCUBEMAP_FACES)i, mipLevel);
				}
				
				src += size;
			}
		}
		
		else
		{
			D3DLOCKED_RECT rect;
			if(SUCCEEDED(texture->LockRect(mipLevel, &rect, NULL, 0)))
			{
				memcpy(rect.pBits, src, size);
				texture->UnlockRect(mipLevel);
			}
		}
		
		++mipLevel;
	}
	
	int filterIndex = GetFilterIndex(flags);
	if(filterIndex < 0) 
		return false;
		
	m_textureSampler.minFilter = g_filters[filterIndex];
	m_textureSampler.magFilter = (filterIndex > 0) ? D3DTEXF_LINEAR : D3DTEXF_POINT;
	m_textureSampler.mipFilter = g_mipFilters[filterIndex];
	m_textureSampler.wrapS = (flags & QTEXTURE_CLAMP_S) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	m_textureSampler.wrapT = (flags & QTEXTURE_CLAMP_T) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	m_textureSampler.wrapR = (flags & QTEXTURE_CLAMP_R) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	
	if(m_textureSampler.minFilter == D3DTEXF_ANISOTROPIC || m_textureSampler.mipFilter == D3DTEXF_ANISOTROPIC)
		m_anisotropy = m_pQuadrionRender->m_deviceCaps.maxAnisotropy;
	else
		m_anisotropy = 0;
	
	m_bIsLoaded = true;
	
	return true;
}


bool CQuadrionTextureObject::CreateTexture(CQuadrionTextureFile& tex, unsigned int& flags)
{
	// Get full path and attempt to load texture //
	std::string fullpath = GetFilename();
//	CQuadrionTextureFile tex;
	if(!tex.IsLoaded())
		return false;
	
	// Look for mip flags and generate mip maps //
	bool mipGenerationFailed = false;
	if(tex.GetMipMapCount() <= 1 && HasMipMapFlags(flags))
	{
		if(!tex.GenerateMipMaps())
			mipGenerationFailed = true;
	}
	
	// Look for normal/heightmap generation flags //
	if((tex.GetHeight() > 0 && tex.GetDepth() == 1) && (flags & QTEXTURE_NORMALMAP))
	{
		if(!tex.HeightToNormal(true, (flags & QTEXTURE_KEEPHEIGHT) != 0))
			return false;
	}
	
	// Look for cubemap flags and clamp ranges //
	if(tex.IsCubemap())
		flags |= (QTEXTURE_CLAMP_S | QTEXTURE_CLAMP_T);
	
	ETexturePixelFormat fmt = tex.GetPixelFormat();
	if(IsPlainFormat(fmt) && GetChannelCount(fmt) == 3)
	{
		if(!tex.AddChannel(1))
			return false;
		
		fmt = tex.GetPixelFormat();
	}
	
	m_pixelFormat = g_textureFormats[fmt];
	m_textureWidth = tex.GetWidth();
	m_textureHeight = tex.GetHeight();
	
	m_usage = 0;
	if(flags & QTEXTURE_DYNAMIC)
		m_usage |= D3DUSAGE_DYNAMIC;
	if(mipGenerationFailed)
		m_usage |= D3DUSAGE_AUTOGENMIPMAP;
		
	D3DPOOL pool = D3DPOOL_MANAGED;

	LPDIRECT3DTEXTURE9 texture;
	LPDIRECT3DCUBETEXTURE9 cubeTexture;
	LPDIRECT3DVOLUMETEXTURE9 volumeTexture;
	
	if(tex.IsCubemap())
	{
		if(FAILED(m_pRenderDevice->CreateCubeTexture(m_textureWidth, tex.GetMipMapCount(), m_usage, m_pixelFormat, pool, &cubeTexture, NULL)))
			return false;

		m_pTextureObject = cubeTexture;
	}
	
	else if(tex.Is3D())
	{
		if(FAILED(m_pRenderDevice->CreateVolumeTexture(m_textureWidth, m_textureHeight, tex.GetDepth(), tex.GetMipMapCount(), m_usage, m_pixelFormat, pool, &volumeTexture, NULL)))
			return false;

		m_pTextureObject = volumeTexture;
	}
	
	else
	{
		if(FAILED(m_pRenderDevice->CreateTexture(m_textureWidth, m_textureHeight, tex.GetMipMapCount(), m_usage, m_pixelFormat, pool, &texture, NULL)))
			return false;
		
		m_pTextureObject = texture;
	}
	
	if(m_pixelFormat == D3DFMT_A8R8G8B8)
	{
		if(!tex.SwapChannels(0, 2, flags & QTEXTURE_NORMALMAP))
			return false;
	}
	
	unsigned char* src;
	int mipLevel = 0;
	
	while((src = (flags & QTEXTURE_NORMALMAP) ? tex.GetNormalmapData(mipLevel) : tex.GetData(mipLevel)) != NULL)
	{
		int size = tex.GetSizeWithMipMaps(mipLevel, 1);
		if(tex.Is3D())
		{
			D3DLOCKED_BOX box;
			if(SUCCEEDED(volumeTexture->LockBox(mipLevel, &box, NULL, 0)))
			{
				memcpy(box.pBits, src, size);
				volumeTexture->UnlockBox(mipLevel);
			}
		}
		
		else if(tex.IsCubemap())
		{
			size /= 6;
			D3DLOCKED_RECT rect;
			for(unsigned int i = 0; i < 6; ++i)
			{
				if(SUCCEEDED(cubeTexture->LockRect((D3DCUBEMAP_FACES)i, mipLevel, &rect, NULL, 0)))
				{
					memcpy(rect.pBits, src, size);
					cubeTexture->UnlockRect((D3DCUBEMAP_FACES)i, mipLevel);
				}
				
				src += size;
			}
		}
		
		else
		{
			D3DLOCKED_RECT rect;
			if(SUCCEEDED(texture->LockRect(mipLevel, &rect, NULL, 0)))
			{
				memcpy(rect.pBits, src, size);
				texture->UnlockRect(mipLevel);
			}
		}
		
		++mipLevel;
	}
	
	int filterIndex = GetFilterIndex(flags);
	if(filterIndex < 0) 
		return false;
		
	m_textureSampler.minFilter = g_filters[filterIndex];
	m_textureSampler.magFilter = (filterIndex > 0) ? D3DTEXF_LINEAR : D3DTEXF_POINT;
	m_textureSampler.mipFilter = g_mipFilters[filterIndex];
	m_textureSampler.wrapS = (flags & QTEXTURE_CLAMP_S) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	m_textureSampler.wrapT = (flags & QTEXTURE_CLAMP_T) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	m_textureSampler.wrapR = (flags & QTEXTURE_CLAMP_R) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	
	if(m_textureSampler.minFilter == D3DTEXF_ANISOTROPIC || m_textureSampler.mipFilter == D3DTEXF_ANISOTROPIC)
		m_anisotropy = m_pQuadrionRender->m_deviceCaps.maxAnisotropy;
	else
		m_anisotropy = 0;
	
	m_bIsLoaded = true;
	
	return true;
}

bool CQuadrionTextureObject::CreateTextureFromData(unsigned int& flags, const void* dat)
{
	// Get full path and attempt to load texture //
	CQuadrionTextureFile tex;
	if(!tex.LoadLightmap((unsigned char*)dat))
		return false;
	
	// Look for mip flags and generate mip maps //
	bool mipGenerationFailed = false;
	if(tex.GetMipMapCount() <= 1 && HasMipMapFlags(flags))
	{
		if(!tex.GenerateMipMaps())
			mipGenerationFailed = true;
	}
	
	// Look for normal/heightmap generation flags //
	if((tex.GetHeight() > 0 && tex.GetDepth() == 1) && (flags & QTEXTURE_NORMALMAP))
	{
		if(!tex.HeightToNormal(true, (flags & QTEXTURE_KEEPHEIGHT) != 0))
			return false;
	}
	
	// Look for cubemap flags and clamp ranges //
	if(tex.IsCubemap())
		flags |= (QTEXTURE_CLAMP_S | QTEXTURE_CLAMP_T);
	
	ETexturePixelFormat fmt = tex.GetPixelFormat();
	if(IsPlainFormat(fmt) && GetChannelCount(fmt) == 3)
	{
		if(!tex.AddChannel(1))
			return false;
		
		fmt = tex.GetPixelFormat();
	}
	
	m_pixelFormat = g_textureFormats[fmt];
	m_textureWidth = tex.GetWidth();
	m_textureHeight = tex.GetHeight();
	
	m_usage = 0;
	if(flags & QTEXTURE_DYNAMIC)
		m_usage |= D3DUSAGE_DYNAMIC;
	if(mipGenerationFailed)
		m_usage |= D3DUSAGE_AUTOGENMIPMAP;
		
	D3DPOOL pool = D3DPOOL_MANAGED;

	LPDIRECT3DTEXTURE9 texture;
	LPDIRECT3DCUBETEXTURE9 cubeTexture;
	LPDIRECT3DVOLUMETEXTURE9 volumeTexture;
	
	if(tex.IsCubemap())
	{
		if(FAILED(m_pRenderDevice->CreateCubeTexture(m_textureWidth, tex.GetMipMapCount(), m_usage, m_pixelFormat, pool, &cubeTexture, NULL)))
			return false;

		m_pTextureObject = cubeTexture;
	}
	
	else if(tex.Is3D())
	{
		if(FAILED(m_pRenderDevice->CreateVolumeTexture(m_textureWidth, m_textureHeight, tex.GetDepth(), tex.GetMipMapCount(), m_usage, m_pixelFormat, pool, &volumeTexture, NULL)))
			return false;

		m_pTextureObject = volumeTexture;
	}
	
	else
	{
		if(FAILED(m_pRenderDevice->CreateTexture(m_textureWidth, m_textureHeight, tex.GetMipMapCount(), m_usage, m_pixelFormat, pool, &texture, NULL)))
			return false;
		
		m_pTextureObject = texture;
	}
	
	if(m_pixelFormat == D3DFMT_A8R8G8B8)
	{
		if(!tex.SwapChannels(0, 2))
			return false;
	}
	
	unsigned char* src;
	int mipLevel = 0;
	
	while((src = tex.GetData(mipLevel)) != NULL)
	{
		int size = tex.GetSizeWithMipMaps(mipLevel, 1);
		if(tex.Is3D())
		{
			D3DLOCKED_BOX box;
			if(SUCCEEDED(volumeTexture->LockBox(mipLevel, &box, NULL, 0)))
			{
				memcpy(box.pBits, src, size);
				volumeTexture->UnlockBox(mipLevel);
			}
		}
		
		else if(tex.IsCubemap())
		{
			size /= 6;
			D3DLOCKED_RECT rect;
			for(unsigned int i = 0; i < 6; ++i)
			{
				if(SUCCEEDED(cubeTexture->LockRect((D3DCUBEMAP_FACES)i, mipLevel, &rect, NULL, 0)))
				{
					memcpy(rect.pBits, src, size);
					cubeTexture->UnlockRect((D3DCUBEMAP_FACES)i, mipLevel);
				}
				
				src += size;
			}
		}
		
		else
		{
			D3DLOCKED_RECT rect;
			if(SUCCEEDED(texture->LockRect(mipLevel, &rect, NULL, 0)))
			{
				memcpy(rect.pBits, src, size);
				texture->UnlockRect(mipLevel);
			}
		}
		
		++mipLevel;
	}
	
	int filterIndex = GetFilterIndex(flags);
	if(filterIndex < 0) 
		return false;
		
	m_textureSampler.minFilter = g_filters[filterIndex];
	m_textureSampler.magFilter = (filterIndex > 0) ? D3DTEXF_LINEAR : D3DTEXF_POINT;
	m_textureSampler.mipFilter = g_mipFilters[filterIndex];
	m_textureSampler.wrapS = (flags & QTEXTURE_CLAMP_S) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	m_textureSampler.wrapT = (flags & QTEXTURE_CLAMP_T) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	m_textureSampler.wrapR = (flags & QTEXTURE_CLAMP_R) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	
	if(m_textureSampler.minFilter == D3DTEXF_ANISOTROPIC || m_textureSampler.mipFilter == D3DTEXF_ANISOTROPIC)
		m_anisotropy = m_pQuadrionRender->m_deviceCaps.maxAnisotropy;
	else
		m_anisotropy = 0;
	
	m_bIsLoaded = true;
	
	return true;	
}


bool CQuadrionTextureObject::BindTexture(const unsigned int& textureUnit)
{
	if(textureUnit > QRENDER_MAX_TEXTURE_SAMPLERS || (!IsLoaded()))
		return false;
		
	bool minAniso, mipAniso;
	minAniso = mipAniso = false;
	
	if(m_pQuadrionRender->m_currentTextures[textureUnit] != GetHandle())
	{
		if(FAILED(m_pRenderDevice->SetTexture(textureUnit, m_pTextureObject)))
			return false;
		
		if(m_textureSampler.minFilter > D3DTEXF_LINEAR)
		{	
			m_textureSampler.minFilter = D3DTEXF_LINEAR;
			minAniso = true;
		}
		
		if(m_textureSampler.mipFilter > D3DTEXF_LINEAR)
		{
			m_textureSampler.mipFilter = D3DTEXF_LINEAR; 
			mipAniso = true;
		}
		
		if(minAniso || mipAniso)
		{
			if(FAILED(m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_MAXANISOTROPY, m_anisotropy)))
				return false;
		}
		
		if(m_pQuadrionRender->m_currentSamplers[textureUnit].minFilter != m_textureSampler.minFilter)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_MINFILTER, m_textureSampler.minFilter);
			m_pQuadrionRender->m_currentSamplers[textureUnit].minFilter = m_textureSampler.minFilter;
		}

		if(m_pQuadrionRender->m_currentSamplers[textureUnit].magFilter != m_textureSampler.magFilter)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_MAGFILTER, m_textureSampler.magFilter);
			m_pQuadrionRender->m_currentSamplers[textureUnit].magFilter = m_textureSampler.magFilter;
		}
		
		if(m_pQuadrionRender->m_currentSamplers[textureUnit].mipFilter != m_textureSampler.mipFilter)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_MIPFILTER, m_textureSampler.mipFilter);
			m_pQuadrionRender->m_currentSamplers[textureUnit].mipFilter = m_textureSampler.mipFilter;
		}

		if(m_pQuadrionRender->m_currentSamplers[textureUnit].wrapS != m_textureSampler.wrapS)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_ADDRESSU, m_textureSampler.wrapS);
			m_pQuadrionRender->m_currentSamplers[textureUnit].wrapS = m_textureSampler.wrapS;
		}

		if(m_pQuadrionRender->m_currentSamplers[textureUnit].wrapT != m_textureSampler.wrapT)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_ADDRESSV, m_textureSampler.wrapT);
			m_pQuadrionRender->m_currentSamplers[textureUnit].wrapT = m_textureSampler.wrapT;
		}

		if(m_pQuadrionRender->m_currentSamplers[textureUnit].wrapR != m_textureSampler.wrapR)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_ADDRESSW, m_textureSampler.wrapR);
			m_pQuadrionRender->m_currentSamplers[textureUnit].wrapR = m_textureSampler.wrapR;
		}
		
		if(minAniso)
		{
			m_textureSampler.minFilter = D3DTEXF_ANISOTROPIC;
			m_pQuadrionRender->m_currentSamplers[textureUnit].minFilter = D3DTEXF_ANISOTROPIC;
			minAniso = false;
		}
		
		if(mipAniso)
		{
			m_textureSampler.mipFilter = D3DTEXF_ANISOTROPIC;
			m_pQuadrionRender->m_currentSamplers[textureUnit].mipFilter = D3DTEXF_ANISOTROPIC;
			mipAniso = false;
		}
	}
	
	m_assignedTextureUnit = textureUnit;
	m_bIsBound = true;
	m_pQuadrionRender->m_currentTextures[textureUnit] = GetHandle();
	m_pQuadrionRender->m_currentTextureTypes[textureUnit] = 1;

	return true;
}

bool CQuadrionTextureObject::BindTexture(const std::string& paramName)
{
	if(m_pQuadrionRender->m_currentEffect == QRENDER_INVALID_HANDLE)
		return false;
	
	CQuadrionEffect* effect = m_pQuadrionRender->GetEffect(m_pQuadrionRender->m_currentEffect);
	effect->UploadTexture(paramName, this);
	
	return true;
}

bool CQuadrionTextureObject::UnbindTexture()
{
	if(m_assignedTextureUnit >= 0 && IsLoaded())
	{
		if(FAILED(m_pRenderDevice->SetTexture(m_assignedTextureUnit, NULL)))
			return false;
	}

	else
		return false;
	
	m_pQuadrionRender->m_currentTextures[m_assignedTextureUnit] = -1;
	m_pQuadrionRender->m_currentTextureTypes[m_assignedTextureUnit] = -1;
	memset((void*)&m_pQuadrionRender->m_currentSamplers[m_assignedTextureUnit], 0, sizeof(SQuadrionTextureSampler));
	m_assignedTextureUnit = -1;
	m_bIsBound = false;
	return true;
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

CQuadrionRenderTarget::CQuadrionRenderTarget(const unsigned int handle, const std::string& name, const std::string& path) : CQuadrionTextureObject(handle, name, path)
{
	m_nSurfaces = 0;
	m_pSurfaceList = NULL;
	m_assignedRTTSlot = 0;
	
	m_bHasMSAA = false;
	m_bIsResolved = false;
	m_internalPixelFormat = QTEXTURE_FORMAT_NONE;
}

CQuadrionRenderTarget::~CQuadrionRenderTarget()
{
	m_pRenderDevice = NULL;
	
	if(m_pTextureObject)
	{
		m_pTextureObject->Release();
		m_pTextureObject = NULL;
	}
	
	if(m_pSurfaceList)
	{
		for(unsigned int i = 0; i < m_nSurfaces; ++i)
		{
			if(m_pSurfaceList[i])
			{
				m_pSurfaceList[i]->Release();
//				m_pSurfaceList[i] = NULL;
			}
		}
		
		delete[] m_pSurfaceList;
		m_pSurfaceList = NULL;
	}
	

	
	memset(&m_textureSampler, 0, sizeof(SQuadrionTextureSampler));
	
	m_textureWidth = 0;
	m_textureHeight = 0;
	m_pixelFormat = D3DFMT_UNKNOWN;
	m_usage = 0;
	m_assignedTextureUnit = 0;
	m_bIsLoaded = false;	
	m_pQuadrionRender = NULL;
	m_nSurfaces = 0;
	m_assignedRTTSlot = 0;
	m_bHasMSAA = false;
	m_bIsResolved = false;
	m_internalPixelFormat = QTEXTURE_FORMAT_NONE;;
}

bool CQuadrionRenderTarget::BindTexture( const unsigned int& textureUnit )
{
	if(textureUnit > QRENDER_MAX_TEXTURE_SAMPLERS || (!IsLoaded()))
		return false;
		
	bool minAniso, mipAniso;
	minAniso = mipAniso = false;
	
	if(m_pQuadrionRender->m_currentTextures[textureUnit] != GetHandle())
	{
	
		if( !m_bHasMSAA )
		{
			if(FAILED(m_pRenderDevice->SetTexture(textureUnit, m_pTextureObject)))
				return false;
		}
		
		else
		{
			if( !m_bIsResolved )
			{
				((LPDIRECT3DTEXTURE9)m_pResolvedTexture)->GetSurfaceLevel( 0, &m_pResolvedSurface );
				
				if( FAILED( m_pRenderDevice->StretchRect( m_pSurfaceList[0], NULL, m_pResolvedSurface, NULL, D3DTEXF_LINEAR ) ) ) 
					return false;
					
				m_bIsResolved = true;
			}
			
			if( FAILED( m_pRenderDevice->SetTexture( textureUnit, m_pResolvedTexture ) ) )
				return false;
		}
		
		if(m_textureSampler.minFilter > D3DTEXF_LINEAR)
		{	
			m_textureSampler.minFilter = D3DTEXF_LINEAR;
			minAniso = true;
		}
		
		if(m_textureSampler.mipFilter > D3DTEXF_LINEAR)
		{
			m_textureSampler.mipFilter = D3DTEXF_LINEAR; 
			mipAniso = true;
		}
		
//		if(minAniso || mipAniso)
//		{
//			if(FAILED(m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_MAXANISOTROPY, m_anisotropy)))
//				return false;
//		}
		
		if(m_pQuadrionRender->m_currentSamplers[textureUnit].minFilter != m_textureSampler.minFilter)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_MINFILTER, m_textureSampler.minFilter);
			m_pQuadrionRender->m_currentSamplers[textureUnit].minFilter = m_textureSampler.minFilter;
		}

		if(m_pQuadrionRender->m_currentSamplers[textureUnit].magFilter != m_textureSampler.magFilter)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_MAGFILTER, m_textureSampler.magFilter);
			m_pQuadrionRender->m_currentSamplers[textureUnit].magFilter = m_textureSampler.magFilter;
		}
		
		if(m_pQuadrionRender->m_currentSamplers[textureUnit].mipFilter != m_textureSampler.mipFilter)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_MIPFILTER, m_textureSampler.mipFilter);
			m_pQuadrionRender->m_currentSamplers[textureUnit].mipFilter = m_textureSampler.mipFilter;
		}

		if(m_pQuadrionRender->m_currentSamplers[textureUnit].wrapS != m_textureSampler.wrapS)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_ADDRESSU, m_textureSampler.wrapS);
			m_pQuadrionRender->m_currentSamplers[textureUnit].wrapS = m_textureSampler.wrapS;
		}

		if(m_pQuadrionRender->m_currentSamplers[textureUnit].wrapT != m_textureSampler.wrapT)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_ADDRESSV, m_textureSampler.wrapT);
			m_pQuadrionRender->m_currentSamplers[textureUnit].wrapT = m_textureSampler.wrapT;
		}

		if(m_pQuadrionRender->m_currentSamplers[textureUnit].wrapR != m_textureSampler.wrapR)
		{
			m_pRenderDevice->SetSamplerState(textureUnit, D3DSAMP_ADDRESSW, m_textureSampler.wrapR);
			m_pQuadrionRender->m_currentSamplers[textureUnit].wrapR = m_textureSampler.wrapR;
		}
		
//		if(minAniso)
//		{
//			m_textureSampler.minFilter = D3DTEXF_ANISOTROPIC;
//			m_pQuadrionRender->m_currentSamplers[textureUnit].minFilter = D3DTEXF_ANISOTROPIC;
//			minAniso = false;
//		}
		
//		if(mipAniso)
//		{
//			m_textureSampler.mipFilter = D3DTEXF_ANISOTROPIC;
//			m_pQuadrionRender->m_currentSamplers[textureUnit].mipFilter = D3DTEXF_ANISOTROPIC;
//			mipAniso = false;
//		}
	}
	
	m_assignedTextureUnit = textureUnit;
	m_bIsBound = true;
	m_pQuadrionRender->m_currentTextures[textureUnit] = GetHandle();
	m_pQuadrionRender->m_currentTextureTypes[textureUnit] = 0;

	return true;	
}


bool CQuadrionRenderTarget::CreateRenderTarget( unsigned int& flags, const unsigned int& w, const unsigned int& h, const ETexturePixelFormat& fmt,
											    bool msaa )
{
	m_usage = D3DUSAGE_RENDERTARGET;
	D3DPOOL pool = D3DPOOL_DEFAULT;
	bool hasMipFlags = HasMipMapFlags(flags);
	m_pixelFormat = g_textureFormats[fmt];


	if(hasMipFlags)	
		m_usage |= D3DUSAGE_AUTOGENMIPMAP;
	
	if(flags & QTEXTURE_CUBEMAP)
	{
		if(w != h)
			return false;
		
		if(FAILED(m_pRenderDevice->CreateCubeTexture(w, hasMipFlags ? 0 : 1, m_usage, m_pixelFormat, pool, (LPDIRECT3DCUBETEXTURE9*)&m_pTextureObject, NULL)))
			return false;
		
		m_nSurfaces = 6;
		m_textureHeight = w;
		if(!m_pSurfaceList)
			m_pSurfaceList = new LPDIRECT3DSURFACE9[6];
		
		for(unsigned int i = 0; i < 6; ++i)
		{
			((LPDIRECT3DCUBETEXTURE9)m_pTextureObject)->GetCubeMapSurface((D3DCUBEMAP_FACES)i, 0, &m_pSurfaceList[i]);
		}
	}
	
	else
	{
		m_nSurfaces = 1;
		if(FAILED(m_pRenderDevice->CreateTexture(w, h, hasMipFlags ? 0 : 1, m_usage, m_pixelFormat, pool, (LPDIRECT3DTEXTURE9*)&m_pTextureObject, NULL)))
			return false;
		
		if(!m_pSurfaceList)
			m_pSurfaceList = new LPDIRECT3DSURFACE9;
		
		((LPDIRECT3DTEXTURE9)m_pTextureObject)->GetSurfaceLevel(0, m_pSurfaceList);
		
		// IF we have MSAA or CSAA specified we must create a render target from this surface //
		if( msaa )
		{
			D3DMULTISAMPLE_TYPE ms;
			unsigned int qual;
			m_pQuadrionRender->GetCurrentMSAA( ms, qual );
			
			if( ms != D3DMULTISAMPLE_NONE )
			{		
				if( FAILED( m_pRenderDevice->CreateRenderTarget( w, h, m_pixelFormat, ms, qual, FALSE, m_pSurfaceList, NULL ) ) )
					return false;
			
				if( FAILED( m_pRenderDevice->CreateTexture( w, h, hasMipFlags ? 0 : 1, D3DUSAGE_RENDERTARGET, m_pixelFormat, pool, (LPDIRECT3DTEXTURE9*)&m_pResolvedTexture, NULL ) ) )
					return false;
				
				m_bHasMSAA = true;
			}
			
			else
				m_bHasMSAA = false;
		}
		
		m_textureHeight = h;
	}
	
	int filterIndex = GetFilterIndex(flags);
	if(filterIndex < 0) 
		return false;
	
	m_textureSampler.minFilter = g_filters[filterIndex];
	m_textureSampler.magFilter = (filterIndex > 0) ? D3DTEXF_LINEAR : D3DTEXF_POINT;
	m_textureSampler.mipFilter = g_mipFilters[filterIndex];
	m_textureSampler.wrapS = (flags & QTEXTURE_CLAMP_S) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	m_textureSampler.wrapT = (flags & QTEXTURE_CLAMP_T) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	m_textureSampler.wrapR = (flags & QTEXTURE_CLAMP_R) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	
	m_textureWidth = w;
	m_bIsLoaded = true;
	m_internalPixelFormat = fmt;
	
	return true;
}


bool CQuadrionRenderTarget::BindRenderTarget(const unsigned int& rttSlot)
{
	unsigned int slot = 0;
	if(rttSlot == -2)
	{
		for(unsigned int i = 0; i < QRENDER_MAX_RENDER_TARGETS; ++i)
		{
			if(m_pQuadrionRender->m_currentRenderTargets[i] >= 0)
			{
				++slot;
				continue;
			}
			
			else
				break;
		}
		
		if(slot >= QRENDER_MAX_RENDER_TARGETS)
			return false;
	}
	
	else
		slot = rttSlot;
	
	unsigned int handle = GetHandle();
	if(m_pQuadrionRender->m_currentRenderTargets[slot] != handle)	
	{
		if(FAILED(m_pRenderDevice->SetRenderTarget(slot, m_pSurfaceList[0])))
			return false;
	}
	
	m_pQuadrionRender->m_currentRenderTargets[slot] = handle;
	
	m_bIsResolved = false;
	
	return true;
}


bool CQuadrionRenderTarget::BindCubemapFace(const unsigned int cubemapFace, const unsigned int& rttSlot)
{
	if(cubemapFace < 0 || cubemapFace > 5)
		return false;

	unsigned int slot = 0;
	if(rttSlot == -2)
	{
		for(unsigned int i = 0; i < QRENDER_MAX_RENDER_TARGETS; ++i)
		{
			if(m_pQuadrionRender->m_currentRenderTargets[i] >= 0)
			{
				++slot;
				continue;
			}
			
			else 
				break;
		}
		
		if(slot >= QRENDER_MAX_RENDER_TARGETS)
			return false;
	}
	
	else 
		slot = rttSlot;
	
	if(FAILED(m_pRenderDevice->SetRenderTarget(slot, m_pSurfaceList[cubemapFace])))
		return false;
	
	m_bIsResolved = false;
	return true;
}


bool CQuadrionRenderTarget::Clear(const unsigned int& color)
{
	if(!m_pSurfaceList[0])
		return false;

	if(FAILED(m_pRenderDevice->ColorFill(m_pSurfaceList[0], NULL, color)))
		return false;
	
	m_bIsResolved = false;
	return true;
}

bool CQuadrionRenderTarget::ClearCubemapFace(const int cubemapFace, unsigned int color)
{
	if(cubemapFace < 0 || cubemapFace > 5)
		return false;
	
	if(FAILED(m_pRenderDevice->ColorFill(m_pSurfaceList[cubemapFace], NULL, color)))
		return false;
	
	m_bIsResolved = false;
	return true;	
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

CQuadrionDepthStencilTarget::CQuadrionDepthStencilTarget(const unsigned int handle, const std::string& name, const std::string& path) : CQuadrionResource(handle, name, path)
{
	m_pRenderDevice = NULL;
	m_pQuadrionRender = NULL;
	m_pDepthStencilSurface = NULL;
	m_width = 0;
	m_height = 0;
	m_depthBits = 0;
	m_stencilBits = 0;
	m_bHasMSAA = false;
}

CQuadrionDepthStencilTarget::~CQuadrionDepthStencilTarget()
{
	m_pRenderDevice = NULL;
	m_pQuadrionRender = NULL;
	
	if(m_pDepthStencilSurface)
	{
		m_pDepthStencilSurface->Release();
		m_pDepthStencilSurface = NULL;
	}
	
	m_width = 0;
	m_height = 0;
	m_depthBits = 0;
	m_stencilBits = 0;
	
	m_bHasMSAA = false;
}


void CQuadrionDepthStencilTarget::ChangeRenderDevice(const void* pRender)
{
	m_pQuadrionRender = (CQuadrionRender*)pRender;
	m_pRenderDevice = m_pQuadrionRender->m_pD3DDev;
}


bool CQuadrionDepthStencilTarget::CreateDepthStencilTarget( const unsigned int& w, const unsigned int& h, 
															const unsigned int& depthBits, const unsigned int& stencilBits, bool msaa )
{
	D3DFORMAT fmt;
	D3DPOOL pool = D3DPOOL_DEFAULT;

//	if(flags & QDEPTHTARGET_LOCKABLE)
//	{
//		if(depthBits > 16 || stencilBits > 0)
//			return false;
//		
//		fmt = D3DFMT_D16_LOCKABLE;
//	}
	
	if(stencilBits > 0)
		fmt = D3DFMT_D24S8;
	else if(depthBits > 16 && depthBits < 32)
		fmt = D3DFMT_D24X8;
	else if(depthBits > 24 && depthBits <= 32)
		fmt = D3DFMT_D32;
	else
		fmt = D3DFMT_D16;
	

	D3DMULTISAMPLE_TYPE ms;
	unsigned int qual;
	if( msaa )
		m_pQuadrionRender->GetCurrentMSAA( ms, qual );
	else
	{
		ms = D3DMULTISAMPLE_NONE;
		qual = 0;
	}

	if(FAILED(m_pRenderDevice->CreateDepthStencilSurface(w, h, fmt, ms, qual, false, &m_pDepthStencilSurface, NULL)))
	{
		return false;
	}
	
	m_bHasMSAA = msaa;
	
	m_width = w;
	m_height = h;
	m_depthBits = depthBits;
	m_stencilBits = stencilBits;

	return true;
}


bool CQuadrionDepthStencilTarget::BindDepthStencilTarget()
{
	unsigned int handle = GetHandle();
	if(m_pQuadrionRender->m_currentDepthStencilTarget != handle)
	{
		if(FAILED(m_pRenderDevice->SetDepthStencilSurface(m_pDepthStencilSurface)))
			return false;
	}
	
	m_pQuadrionRender->m_currentDepthStencilTarget = handle;
	return true;
}