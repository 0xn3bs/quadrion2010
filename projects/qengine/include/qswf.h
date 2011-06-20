#ifndef __SWF_H_
#define __SWF_H_

#ifdef QRENDER_EXPORTS
#define QSWFEXPORT_API		__declspec(dllexport)
#else
#define QSWFEXPORT_API		__declspec(dllimport)
#endif

#define SWF_COMPRESSED_MAGIC	"CWS"
#define SWF_UNCOMPRESSED_MAGIC	"FWS"

#include "qswf_types.h"

#include <map>

class QSWFEXPORT_API SWF_FILE
{
public:
	SWF_FILE();
	~SWF_FILE();

	int LoadSWF(const char* path);
	bool IsLoaded();

	void GetBytes(char* dataOut, const unsigned int numBytes = 1);
	void GetBits(char* dataOut, const unsigned int numBits);

	unsigned int GetBitOffset() { return m_bitOffset; }
	unsigned int GetByteOffset() { return m_byteOffset; }
	void SetByteOffset(const unsigned int offset);
	void SetBitOffset(const unsigned int offset);
private:
	char* m_pFileData;
	unsigned int m_byteOffset;
	unsigned int m_bitOffset;
	unsigned int m_uiFileSize;
	bool m_bLoaded;
};

//	For simplicity we'll always use the long record header
class QSWFEXPORT_API SWF_DEFINE_TAG
{
public:
	SWF_DEFINE_TAG(){};
	SWF_DEFINE_TAG(SWF_RECORD_HEADER& recordHeader);
	~SWF_DEFINE_TAG(){};
	virtual bool Load(SWF_FILE* pFile) = 0;

	uint16_t GetCharacterID(){return characterID;}
protected:
	SWF_RECORD_HEADER recordHeaderLong;
	uint16_t characterID;	
};

class QSWFEXPORT_API SWF_DEFINE_SHAPE : public SWF_DEFINE_TAG
{
public:
	SWF_DEFINE_SHAPE(){};
	SWF_DEFINE_SHAPE(SWF_RECORD_HEADER& recordHeader);
	~SWF_DEFINE_SHAPE(){};
	bool Load(SWF_FILE* pFile){return true;};

	SWF_RECT GetShapeBounds(){return *shapeBounds;}
	SWF_FILL_STYLE_ARRAY* GetFillStyles(){return &m_FillStyles;}

private:		
	void ParseShapeWithStyle(SWF_FILE* pFile);
	void ParseFillStyles(SWF_FILE* pFile);
	void ParseLineStyles(SWF_FILE* pFile);
	void ParseShapeRecords(SWF_FILE* pFile);

	SWF_RECT* shapeBounds;

	//	SHAPEWITHSTYLE
	SWF_FILL_STYLE_ARRAY m_FillStyles;
	SWF_LINE_STYLE_ARRAY m_LineStyles;
	uint8_t m_numFillBits;
	uint8_t m_numLineBits;
};

class QSWFEXPORT_API SWF
{
public:
	SWF();
	~SWF();
	int32_t LoadSWF(const char* path);

	static void GetRect(SWF_RECT* rect, SWF_FILE* pFile);
	SWF_HEADER GetHeader(){return *m_pHeader;}
	SWF_RGB GetBackgroundColor(){return m_BackgroundColor;}

	void Display();

private:
	int32_t LoadHeader(SWF_FILE* file);		
	int32_t LoadTag(SWF_FILE* file);
	int32_t LoadFileAttributesTag(SWF_FILE* file);
	int32_t LoadDefSceneAndFrameLabelTag(SWF_FILE* file);
	int32_t LoadDefineShapeTag(SWF_RECORD_HEADER* pRecordHeader);

	bool m_bIsEnd;

	SWF_FILE* m_pFile;
	SWF_HEADER* m_pHeader;
	SWF_FILE_ATTRIBUTES* m_pAttributes;
	SWF_DEFINE_SCENE_AND_FRAME_LABEL_DATA* m_pSceneAndFrameLabelData;
	SWF_RGB m_BackgroundColor;

	std::map<uint16_t, SWF_DEFINE_TAG*> m_Dictionary;
};

unsigned int QSWFEXPORT_API GetEncodedU32(SWF_FILE *file);

#endif /*__SWF_H_*/
