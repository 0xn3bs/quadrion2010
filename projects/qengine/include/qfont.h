#ifndef __QFONT_H_
#define __QFONT_H_

#ifdef QRENDER_EXPORTS
	#define QFONTEXPORT_API  __declspec(dllexport)
#else
	#define QFONTEXPORT_API  __declspec(dllimport)
#endif

#include <vector>

#include "qrender.h"
#include "qtexture.h"

struct QFONTEXPORT_API texture_vertex_format
{
 float x, y, z;
 float u, v;
};

struct QFONTEXPORT_API SCharacterOffset
{
	float u1, v1, u2, v2;
	float xScale, yScale;
};

enum QFONTEXPORT_API ETextAlignment
{
	FONT_ALIGN_LEFT		= 0x000000001,
	FONT_ALIGN_RIGHT	= 0x000000002,
	FONT_ALIGN_CENTER	= 0x000000004,
	FONT_ALIGN_TOP		= 0x000000010,
	FONT_ALIGN_BOTTOM	= 0x000000020,
	FONT_ALIGN_MIDDLE	= 0x000000040
};

class QFONTEXPORT_API CFont
{
private:
	int m_iTextureHandle;
	std::vector<SCharacterOffset*> m_vCharTexOffsets;
	std::string m_sName;
	std::string m_sPath;
	
	SCharacterOffset* GenerateCharOffset(unsigned int charSize, unsigned char chr, unsigned int texWidth, unsigned int texHeight);

public:
	CFont();
	~CFont();

	bool LoadFont(const std::string& name, const std::string& path, bool monospaced, unsigned int charSize = 16);

	void GenerateCharacterOffsets(CQuadrionTextureFile &fontFile, bool monospaced, unsigned int charSize, unsigned char* data = NULL);

	std::vector<SCharacterOffset*>* GetCharOffsets();
	SCharacterOffset* GetCharOffset(char character);

	int GetFontTextureHandle();

	void WriteText(const std::string& str, vec2f pos, vec2f bounds, ETextAlignment alignment, UINT color);
	int GetStringWidth(std::string str);
};

#endif
