// 
// RSC Miscellaneous Class (RSCMisc.h) 
// By Jae H. Park 
//
// Created: 12.27.2004
// Rev. 10.20.2005, 10.22.2005
//
// RSCMisc.h contains the complex or custom type declaration for other classes.
//

#define RSC_UBYTE	unsigned char
#define RSC_UWORD	unsigned short	
#define RSC_UDWORD	unsigned long
#define RSC_BYTE	char
#define RSC_WORD	short
#define RSC_DWORD	long

typedef enum RSC_DIBPIXELTYPE
{
	DIB_NONE,				// None 
	DIB_1BPP,				// 2 color image, palette
	DIB_2BPP,				// 4 color image, palette
	DIB_4BPP,				// 16 color image, palette
	//DIB_4BPPRLE,			// 16 color image, palette, RLE compression
	DIB_8BPP,				// 256 color image, palette
	//DIB_8BPPRLE,			// 256 color image, palette, REL compression
	DIB_15RGB555,
	DIB_16RGB555,			// 15 bit RGB color image, 5-5-5
	DIB_16RGB565,			// 16 bit RGB color image, 5-6-5, 1 bit unused
	DIB_24RGB888,			// 24 bit RGB color image, 8-8-8
	DIB_32RGB888,			// 24 bit RGB color image, 0-8-8-8,	most significant 8 bit unused
	DIB_32RGBA888,			// 32 bit RGB color image, a-8-8-8,	most significant 8 bit alpha
	//DIB_16RGBbitfields,	// 16 bit RGB color image, nonstandard bit masks
	//DIB_32RGBbitfields,	// 32 bit RGB color image, nonstandard bit masks
	//DIB_JPEG,				// embedded JPEG image
	//DIB_PNG				// embedded PNG image
} RSC_DIBPIXELTYPE;

typedef struct RSC_DIBINFO{
	BITMAPINFOHEADER	header;
	RGBQUAD				colors[256+3];
}RSC_DIBINFO;

LONG inline RSCGetBPS(LONG width, LONG bpp)
// Return the total number of byte per scanline
{
	return (width * bpp + 31) /32 * 4;
}

class RSCErrorHandling{
public:
	RSCErrorHandling();
	~RSCErrorHandling(void);
	void RSCSetErrorCode(LONG error_code, const char *error_string);

public:
	LONG m_error_code;
	BYTE m_error_string[1024];
};

// Pixel/BPP/BPS Handling Functions
//
RSC_DIBPIXELTYPE RSCGetPixelType(HDC hdc);
RSC_DIBPIXELTYPE RSCGetPixelTypeFromBitsPerPixel(UINT bpp, UINT red_b, UINT green_b, UINT blue_b);
DWORD RSCGetCurrentDCbpp(void);

//Registry Handling Function
//
bool RegSetStringValue(DWORD option, HKEY rootkey, char *subkey, DWORD type, char *valuename, BYTE *data);
bool RegGetStringValue(HKEY rootkey, char *subkey, char *valuename, BYTE *data, DWORD datasize);
bool RegSetDWORDValue(DWORD option, HKEY rootkey, char *subkey, char *valuename, DWORD dwdata);
bool RegGetDWORDValue(HKEY rootkey, char *subkey, char *valuename, DWORD *retdata);
bool RegSetBinValue(DWORD option, HKEY rootkey, char *subkey, char *valuename, BYTE *data, int datalen);
bool RegGetBinValue(HKEY rootkey, char *subkey, char *valuename, BYTE *data,  DWORD datasize, int *datalen);



int inline RSCGetRedBit(RSC_DIBPIXELTYPE pixel_type)
{
	switch(pixel_type)
	{
		case DIB_15RGB555:
		case DIB_16RGB555:
		case DIB_16RGB565:
			return 5;
			break;
		case DIB_32RGB888:
		case DIB_32RGBA888:
			return 8;
			break;
	}

	return -1;
}

int inline RSCGetGreenBit(RSC_DIBPIXELTYPE pixel_type)
{
		switch(pixel_type)
	{
		case DIB_15RGB555:
		case DIB_16RGB555:
		case DIB_16RGB565:
			return 5;
			break;
		case DIB_32RGB888:
		case DIB_32RGBA888:
			return 8;
			break;
	}

	return -1;
}

int inline RSCGetBlueBit(RSC_DIBPIXELTYPE pixel_type)
{
		switch(pixel_type)
	{
		case DIB_15RGB555:
		case DIB_16RGB555:
		case DIB_16RGB565:
			return 5;
			break;
		case DIB_32RGB888:
		case DIB_32RGBA888:
			return 8;
			break;
	}

	return -1;
}

