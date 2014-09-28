// 
// RSC Region Class (RSCRegion.h) 
// By Jae H. Park 
//
// Created: 12.13.2004
// Rev. 12.27.2004, 12.28.2004, 12.29.2004, 01.29.2005, 02.09.2005, 09.11.2005
// 10.15.2005, 10.20.2005, 05.18.2006, 05.21.2006, 07.07.2006, 08.01.2006, 08.02.2006
//
// RSCRegion class handles the Device Dependent Bitmap and Device Independent Bitmap. 
// It captures the screen and store it in DDB format to be converted to DIB format.
// Once converted, the DIB may converted to lower color depth or treated in a compressed form
// in order to be transmitted via TCP/IP network.
//
// Note: RSCMisc.h, RSCPalette.h must be declared prior this class.
//

#pragma once

typedef struct RSC_REGIONPROP{
		LONG				region_x;	// the position of upper x within the entire screen
		LONG				region_y;	// the position of upper y within the entire screen
		LONG				region_w;	// the region width
		LONG				region_h;	// the region height
		UINT				bpp;		// bits per pixel
		UINT				Bps;		// Bytes per scanline
		RSC_DIBPIXELTYPE	pixel_type;

		// When the screen is scaled,
		//
		BOOL				isscaled;	// 1. flag for screen scale
		LONG				region_o_x;	// 2. if scaled, the original position of upper x
		LONG				region_o_y; // 3. if scaled, the original position of upper y
		LONG				region_o_w; // 4. if scaled, the scaled region width
		LONG				region_o_h; // 5. if scaled, the scaled region height
}RSC_REGIONPROP;

class RSCRegion{
public:
	RSCRegion(LONG region_x, LONG region_y, LONG region_w, LONG region_h);
	//
	// [Note] Scaling is disabled for sub-regional udpate request
	//
	//RSCRegion(LONG region_x, LONG region_y, LONG region_w, LONG region_h, BOOL isscaled);
	//
	RSCRegion(LONG region_w, LONG region_h, BOOL isscaled);
	RSCRegion(LONG region_w, LONG region_h);
	RSCRegion(BOOL isscaled);
	RSCRegion(void);
	~RSCRegion(void);

	void RSCResetRegionInfo(void);

	bool RSCResetRegion(void);
	bool RSCResetRegion(BOOL isscaled);
	bool RSCResetRegion(LONG region_w, LONG region_h);
	bool RSCResetRegion(LONG region_w, LONG region_h, BOOL isscaled);
	bool RSCResetRegion(LONG region_x, LONG region_y, LONG region_w, LONG region_h);
	//
	// [Note] Scaling is disabled for sub-regional udpate request
	//
	//bool RSCResetRegion(LONG region_x, LONG region_y, LONG region_w, LONG region_h, BOOL isscaled);
	//

	int RSCIsScreenChanged(void); // -1: error		0: false	1: true
	//int RSCIsRegionChanged(LONG region_w, LONG region_h); // -1: error		0: false	1: true
	//int RSCIsRegionChanged(LONG region_x, LONG region_y, LONG region_w, LONG region_h); // -1: error		0: false	1: true


	bool RSCCapture(void);
	bool RSCInitDIB(HPALETTE hpal, WORD nbitcount, DWORD ncompression);
	bool RSCDDB2DIB(UINT height, UINT hinfo_size, UINT color_tbl_identifier); 
	HBITMAP RSCDIB2DDB(BYTE *pdibdata, RSC_DIBINFO *dibheader, UINT color_type);
	HBITMAP RSCDIB2DDB(BYTE *pdibdata, RSC_DIBINFO *dibheader, RSCPalette *pal, UINT color_type);
	
	bool RSCConvertDIB_8to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RGBQUAD *rgbquad);
	bool RSCConvertDIB_8to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RGBQUAD *rgbquad);
	bool RSCConvertDIB_8to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RGBQUAD *rgbquad);
	bool RSCConvertDIB_8to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RGBQUAD *rgbquad);

	bool RSCConvertDIB_8to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertDIB_8to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertDIB_8to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertDIB_8to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);

	bool RSCConvertDIB_15to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertDIB_15to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertDIB_15to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertDIB_15to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);

	bool RSCConvertDIB_16to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertDIB_16to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertDIB_16to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertDIB_16to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);

	bool RSCConvertDIB_24to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertDIB_24to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertDIB_24to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertDIB_24to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);

	bool RSCConvertDIB_32to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertDIB_32to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertDIB_32to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertDIB_32to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);

public:
	UINT				m_dibhsize;			// ?DIB Header Size (int)
	HDC					m_hdc;				// ?Device Context for Full Screen Display (HDC)
	HDC					m_hdcmem;			// ?Device Context Compatible Memory(HDC)
	HBITMAP				m_hdcbmp;			// ?Bitmap Compatible DC (HBITMAP)
	RSC_REGIONPROP		m_regionprop;
	RSC_DIBPIXELTYPE	m_dib_pixel_type;
	BYTE				*m_pdib;
	BITMAP				m_ddbinfo;
	RSC_DIBINFO			m_dibinfo;
	RSCErrorHandling	m_error;
};
