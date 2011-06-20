#ifndef __SWF_TYPES_H_
#define __SWF_TYPES_H_

#include <cstdint>
#include <vector>

#ifdef QRENDER_EXPORTS
#define QSWFTYPESEXPORT_API		__declspec(dllexport)
#else
#define QSWFTYPESEXPORT_API		__declspec(dllimport)
#endif

enum SWF_TAGS
{
	TAG_END = 0,
	TAG_SHOW_FRAME,
	TAG_DEFINE_SHAPE,
	TAG_PLACE_OBJECT = 4,
	TAG_REMOVE_OBJECT,
	TAG_DEFINE_BITS,
	TAG_DEFINE_BUTTON,
	TAG_JPEG_TABLES,
	TAG_SET_BACKGROUND_COLOR,
	TAG_DEFINE_FONT,
	TAG_DEFINE_TEXT,
	TAG_DO_ACTION,
	TAG_DEFINE_FONT_INFO,
	TAG_DEFINE_SOUND,
	TAG_START_SOUND,
	TAG_DEFINE_BUTTON_SOUND,
	TAG_SOUND_STREAM_HEAD,
	TAG_SOUND_STREAM_BLOCK,
	TAG_DEFINE_BITS_LOSSLESS,
	TAG_DEFINE_BITS_JPEG2,
	TAG_DEFINE_SHAPE2,
	TAG_DEFINE_BUTTON_CXFORM,
	TAG_PROTECT,
	TAG_PLACE_OBJECT2 = 26,
	TAG_REMOVE_OBJECT2 = 28,
	TAG_DEFINE_SHAPE3 = 32,
	TAG_DEFINE_TEXT2,
	TAG_DEFINE_BUTTON2,
	TAG_DEFINE_BITS_JPEG3,
	TAG_DEFINE_BITS_LOSSLESS2,
	TAG_DEFINE_EDIT_TEXT,
	TAG_DEFINE_SPRITE = 39,
	TAG_FRAME_LABEL = 43,
	TAG_SOUND_STREAM_HEAD2 = 45,
	TAG_DEFINE_MORPH_SHAPE,
	TAG_DEFINE_FONT2 = 48,
	TAG_EXPORT_ASSETS = 56,
	TAG_IMPORT_ASSETS,
	TAG_ENABLE_DEBUGGER,
	TAG_DO_INIT_ACTION,
	TAG_DEFINE_VIDEO_STREAM,
	TAG_VIDEO_FRAME,
	TAG_DEFINE_FONT_INFO2,
	TAG_ENABLE_DEBUGGER2 = 64,
	TAG_SCRIPT_LIMITS,
	TAG_SET_TAB_INDEX,
	TAG_FILE_ATTRIBUTES = 69,
	TAG_PLACE_OBJECT3,
	TAG_IMPORT_ASSETS2,
	TAG_DEFINE_FONT_ALIGN_ZONES = 73,
	TAG_DEFINE_CSM_TEXT_SETTINGS,
	TAG_DEFINE_FONT3,
	TAG_SYMBOL_CLASS,
	TAG_METADATA,
	TAG_DEFINE_SCALING_GRID,
	TAG_DO_ABC = 82,
	TAG_DEFINE_SHAPE4,
	TAG_DEFINE_MORPH_SHAPE2,
	TAG_DEFINE_SCENE_AND_FRAME_LABEL_DATA = 86,
	TAG_DEFINE_BINARY_DATA,
	TAG_DEFINE_FONT_NAME = 88,
	TAG_DEFINE_START_SOUND2 = 89,
	TAG_DEFINE_BITS_JPEG4 = 90,
	TAG_DEFINE_FONT4 = 91
};

enum SWF_FILL_STYLES
{
	FILL_STYLE_SOLID = 0x00,
	FILL_STYLE_LINEAR_GRADIENT = 0x10,
	FILL_STYLE_RADIAL_GRADIENT = 0x12,
	FILL_STYLE_FOCAL_RADIAL_GRADIENT = 0x13,
	FILL_STYLE_REPEATING_BITMAP = 0x40,
	FILL_STYLE_CLIPPED_BITMAP = 0x41,
	FILL_STYLE_NON_SMOOTHED_CLIPPED_BITMAP = 0x43
};

struct SWF_RGBA
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
};

struct SWF_ARGB
{
	uint8_t alpha;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct SWF_RGB
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

class QSWFTYPESEXPORT_API SWF_RECT
{
public:
	SWF_RECT()
	{
		Xmin = 0;
		Xmax = 0;
		Ymin = 0;
		Ymax = 0;
		Nbits = 0;
	};
	~SWF_RECT(){};

	uint8_t Nbits;

	//	HACK: We're making the assumption that
	//		  the values we get from GetBits are
	//		  not larger than 4 bytes. While it's
	//		  possible to get larger than 4 byte values
	//		  we believe it's highly unlikely.

	int32_t Xmin;
	int32_t Xmax;
	int32_t Ymin;
	int32_t Ymax;
};

struct SWF_FILE_ATTRIBUTES
{
	bool useDirectBlit;
	bool useGpu;
	bool hasMetaData;
	bool useActionScript3;
	bool useNetwork;
};

struct SWF_DEFINE_SCENE_AND_FRAME_LABEL_DATA
{
	uint32_t sceneCount;
	std::vector<uint32_t> frameOffsets;
	std::vector<std::string> names;
	uint32_t frameLabelCount;
	std::vector<uint32_t> frameNumbers;
	std::vector<std::string> frameLabels;
};

struct SWF_FILL_STYLE_SOLID
{
	uint8_t fillStyleType;
	SWF_RGBA color;
};

struct SWF_LINE_STYLE
{
	uint16_t width;
	SWF_RGBA color;
};

struct SWF_FILL_STYLE_ARRAY
{
	uint8_t fillStyleCount;
	uint16_t fillStyleCountExtended;
	std::vector<SWF_FILL_STYLE_SOLID> fillStylesSolid;
};

struct SWF_LINE_STYLE_ARRAY
{
	uint8_t lineStyleCount;
	uint16_t lineStyleCountExtended;
	std::vector<SWF_LINE_STYLE> lineStyles;
};

struct SWF_SHAPE_WITH_STYLE
{
	SWF_FILL_STYLE_ARRAY* fillStyles;
	SWF_LINE_STYLE_ARRAY* lineStyles;
	uint8_t NumFillBits;
	uint8_t NumLineBits;
};

struct SWF_RECORD_HEADER
{
	uint16_t tagType;
	uint32_t tagLengthLong;
};

struct SWF_STYLE_CHANGE_RECORD
{
	bool typeFlag;
	bool stateNewStyles;
	bool stateLineStyle;
	bool stateFillStyle1;
	bool stateFillStyle0;
	bool stateMoveTo;
};

struct SWF_HEADER
{
	uint8_t	signature[3];
	uint8_t	version;
	uint32_t		fileLength;
	SWF_RECT*		rect;
	uint16_t		fps;
	uint16_t  numFrames;
};

#endif /*__SWF_TYPES_H_*/
