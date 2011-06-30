#include "qfont.h"
//#include "fps.h"

//#include "cGUI.h"

CFont::CFont()
{
	m_iTextureHandle = -1;
};

CFont::~CFont()
{
	for(int i = 0; i < m_vCharTexOffsets.size(); ++i)
	{
		if(m_vCharTexOffsets[i])
		{
			delete m_vCharTexOffsets[i];
			m_vCharTexOffsets[i] = NULL;
		}
	}

	m_vCharTexOffsets.clear();
};

bool CFont::LoadFont(const std::string& name, const std::string& path, bool monospaced, unsigned int charSize)
{
	CQuadrionTextureFile fontFile;

	// New resource manager prefers filename and path be passed separately
	// SMS
	if(!fontFile.LoadTexture(name.c_str(), path.c_str()))
		return false;

	unsigned int textureFlags = 0;
	textureFlags |= (QTEXTURE_FILTER_NEAREST | QTEXTURE_WRAP);

	unsigned char* data = fontFile.GetData();

	GenerateCharacterOffsets(fontFile, monospaced, charSize, data);
	m_iTextureHandle = g_pRender->AddTextureObject(fontFile, textureFlags);

	if(m_iTextureHandle < 0)
		return false;

	m_sName = std::string(name);
	m_sPath = std::string(path);

	return true;
}

void CFont::GenerateCharacterOffsets(CQuadrionTextureFile &fontFile, bool monospaced, unsigned int charSize, unsigned char* data)
{
	unsigned int iTexWidth	= fontFile.GetWidth();
	unsigned int iTexHeight	= fontFile.GetHeight();

	if(monospaced)
	{
		for(unsigned int y = 0; y < iTexHeight; y += charSize)
		{
			for(unsigned int x = 0; x < iTexWidth; x += charSize)
			{
				SCharacterOffset* pCharOffset = new SCharacterOffset;

				pCharOffset->u1 = (float)x / (float)iTexWidth;
				pCharOffset->u2 = ((float)x + charSize) / (float)iTexWidth;

				pCharOffset->v1 = (float)y / (float)iTexHeight;
				pCharOffset->v2 = ((float)y + charSize) / (float)iTexHeight;

				pCharOffset->xScale = (pCharOffset->u2 - pCharOffset->u1) * iTexWidth;
				pCharOffset->yScale = (pCharOffset->v2 - pCharOffset->v1) * iTexHeight;

				m_vCharTexOffsets.push_back(pCharOffset);
			}
		}
	}
	else
	{
		std::vector<std::vector<unsigned int>> pixels;
		pixels.resize(iTexHeight);

		for(unsigned int i = 0; i < iTexHeight; ++i)
		{
			for(unsigned int j = 0; j < iTexWidth; ++j)
			{
				unsigned int srcPixel = 0;
				memcpy(&srcPixel, &data[((iTexWidth * 4) * i) + (j * 4)], 4);
				unsigned int fPixel = 0;
				fPixel |= ((srcPixel>>24) & 0x000000ff);
				fPixel |= ((srcPixel<<8)  & 0x0000ff00);
				fPixel |= ((srcPixel<<16) & 0x00ff0000);
				fPixel |= ((srcPixel<<24) & 0xff000000);
				pixels[i].push_back(fPixel);
			}
		}
	
		for(unsigned int character = 0; character < iTexWidth; ++character)
		{
			float u1 = 1, u2 = -1, v1 = 1, v2 = -1;
			float u = -1, v = -1;
			SCharacterOffset* pCharOffset = GenerateCharOffset(charSize, character, iTexWidth, iTexHeight);
			for(unsigned int y = (unsigned int)pCharOffset->v1; y < pCharOffset->v2; ++y) // TODO: verify these casts.
			{
				for(unsigned int x = (unsigned int)pCharOffset->u1; x < pCharOffset->u2; ++x) // TODO: verify these casts.
				{
					unsigned char pix = pixels[y][x] & 0x000000ff;
					if(pix > 0)
					{
						u = (float)(x) / (float)iTexWidth;

						u1 = u1 >= u ? u : u1;
						u2 = u2 <= u ? (float)(x+1) / (float)iTexWidth : u2;
						v1 = v1 >= v ? v : v1;
						v2 = v2 <= v ? (float)(y+1) / (float)iTexHeight : v2;
					}
				}
			}

			//	If we failed to find a character within range, set UVs to monospaced.
			u1 = u1 == 1  ? pCharOffset->u1/iTexWidth + 7/(float)iTexWidth : u1;
			u2 = u2 == -1 ? pCharOffset->u2/iTexWidth - 7/(float)iTexWidth : u2;

			v1 = pCharOffset->v1/(float)iTexHeight, 
			v2 = pCharOffset->v2/(float)iTexHeight;

			pCharOffset->u1 = u1;
			pCharOffset->u2 = u2;
			pCharOffset->v1 = v1;
			pCharOffset->v2 = v2;

			pCharOffset->xScale = (pCharOffset->u2 - pCharOffset->u1) * iTexWidth;
			pCharOffset->yScale = (pCharOffset->v2 - pCharOffset->v1) * iTexHeight;

			m_vCharTexOffsets.push_back(pCharOffset);
		}
	}
}

SCharacterOffset* CFont::GenerateCharOffset(unsigned int charSize, unsigned char chr, unsigned int texWidth, unsigned int texHeight)
{
	unsigned int x = chr * charSize;
	unsigned int y = 0;

	while(x > texWidth - charSize)
	{
		x -= texWidth;
		y += charSize;
	}

	SCharacterOffset *res = new SCharacterOffset;
	res->u1 = (float)x;
	res->u2 = ((float)x + charSize);

	res->v1 = (float)y;
	res->v2 = ((float)y + charSize);

	res->xScale = (res->u2 - res->u1);
	res->yScale = (res->v2 - res->v1);

	return res;
}

std::vector<SCharacterOffset*>* CFont::GetCharOffsets()
{
	return &m_vCharTexOffsets;
}

SCharacterOffset* CFont::GetCharOffset(char character)
{
	return m_vCharTexOffsets[(unsigned int)character];
}

int CFont::GetFontTextureHandle()
{
	return m_iTextureHandle;
}

void CFont::WriteText(const std::string& str, vec2f pos, vec2f bounds, ETextAlignment alignment, UINT color)
{
	std::vector<SCharacterOffset*>* offsets = &m_vCharTexOffsets;
	CQuadrionTextureObject* tex = NULL;

	//tex = g_pRender->GetTextureObject(m_sPath + m_sName, _BASE_DIR);
	//tex->BindTexture(0);

	std::string fname(m_sPath + m_sName);

	int curWidth = 0;
	SCharacterOffset* charOffset;
	//gGUI->reSetGlobalTextureShift();
	texture_vertex_format vert[6];// = gGUI->getGlobalTextureShiftPtr();

	//tex = g_pRender->GetTextureObject(m_sPath + m_sName, _BASE_DIR);
	tex = g_pRender->GetTextureObject(m_iTextureHandle);
	tex->BindTexture(0);

	for(unsigned int i = 0; i < str.size(); ++i)
	{
		charOffset = m_vCharTexOffsets[str[i]];
		std::string texName;

		// Top Left
		vert[0].u = charOffset->u1;
		vert[0].v = charOffset->v1;

		// Bottom Left
		vert[1].u = charOffset->u1;
		vert[1].v = charOffset->v2;

		// Bottom Right
		vert[2].u = charOffset->u2;
		vert[2].v = charOffset->v2;

		// Bottom Right
		vert[3].u = charOffset->u2;
		vert[3].v = charOffset->v2;

		// Top Right
		vert[4].u = charOffset->u2;
		vert[4].v = charOffset->v1;

		// Top Left
		vert[5].u = charOffset->u1;
		vert[5].v = charOffset->v1;

		texName = m_sPath + m_sName;

		cTextureRect tc;
		tc.leftU = vert[0].u;
		tc.topV = vert[0].v;
		tc.rightU = vert[2].u;
		tc.bottomV = vert[2].v;
		
		vec2f ul( pos.x + curWidth, pos.y );
		vec2f wh( charOffset->xScale, charOffset->yScale );
		
		g_pRender->RenderScreenSpaceQuad( color, tc, ul, wh );


		curWidth += (int)( charOffset->xScale + 1 ); // TODO: verify these casts.
	}

//	gGUI->reSetGlobalTextureShift();	
	
	tex->UnbindTexture();
}

int CFont::GetStringWidth(std::string str)
{
	SCharacterOffset* charOffset;
	int curWidth = 0;

	for(unsigned int i = 0; i < str.size(); ++i)
	{
		charOffset = m_vCharTexOffsets[str[i]];
		curWidth += (int)( charOffset->xScale + 1 ); // TODO: verify these casts.
	}
	return curWidth;
}