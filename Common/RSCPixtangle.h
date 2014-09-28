// 
// RSC Pixtangle Class (RSCPixtangle.h) 
// By Jae H. Park 
//
// Created: 01.31.2005
// Rev. 03.18.2005, 07.07.2006, 10.02.2006
//
// RSCPixtangle class handles sub-regions of pre-determined size called pixtangle in screen framebuffer 
// for pixtangle comparison and manipulation
//
// Note: RSCMisc.h must be declared prior this class.
//

#pragma once

typedef struct RSC_PIXTANGLE_POS{
		// actual pixel position on the screen
		LONG x;
		LONG y; 
		LONG w;
		LONG h;
		LONG sx;
		LONG sy;
		RSC_BYTE type;  // 0 - No Encoding, 1 - RLE Encoding
						// 2 - Copy Rectangle Encoding from previous screen
						// 3 - Copy Rectangel Encoding from same screen
}RSC_PIXTANGLE_POS;

class RSCPixtangle {
public:
	RSCPixtangle(void);
	// Bps = Byte(s) per scanline, Bpp = Byte(s) per pixel
	RSCPixtangle(LONG screen_w, LONG screen_h, int Bps, int Bpp, int pixtangle_w, int pixtangle_h);
	RSCPixtangle(LONG screen_w, LONG screen_h, int Bps, int Bpp, int pixtangle_w, int pixtangle_h, bool posarray);

	~RSCPixtangle(void);

	//bool RSCInitPixtangle(LONG screen_w, LONG screen_h, int Bps, int Bpp, int pixtangle_w, int pixtangle_h);
	bool RSCCreateListPos(LONG screen_w, LONG screen_h, int Bps, int Bpp, int pixtangle_w, int pixtangle_h);
	bool RSCDestroyListPos(void);

	bool RSCComparePixtangle32(BYTE *prev, BYTE *next, int sx, int sy, int sw, int sh);
	bool RSCComparePixtangle32(BYTE *prev, BYTE *next, int px, int py, int w, int h, int nx, int ny);
	bool RSCGetListPixtangle32(BYTE *prev, BYTE *next);
	//bool RSCGetListPixtangle32(BYTE *prev, BYTE *next, LONG region_x, LONG region_y);

	bool RSCComparePixtangle16(BYTE *prev, BYTE *next, int sx, int sy, int sw, int sh);
	bool RSCComparePixtangle16(BYTE *prev, BYTE *next, int px, int py, int w, int h, int nx, int ny);
	bool RSCGetListPixtangle16(BYTE *prev, BYTE *next);

	bool RSCComparePixtangle8(BYTE *prev, BYTE *next, int sx, int sy, int sw, int sh);
	bool RSCComparePixtangle8(BYTE *prev, BYTE *next, int px, int py, int w, int h, int nx, int ny);
	bool RSCGetListPixtangle8(BYTE *prev, BYTE *next);

	void RSCCopyPixtangle2Buffer(BYTE *scr, BYTE *buf, int sx, int sy, int sw, int sh);
	void RSCCopyBuffer2Pixtangle(BYTE *buf, BYTE *scr, int sx, int sy, int sw, int sh);
	void RSCCopyPixtangle2Pixtangle(BYTE *sscr, BYTE *dscr, int sx, int sy, int sw, int sh, int dx, int dy);


	bool RSCConvertPixtangle_8to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertPixtangle_8to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertPixtangle_8to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	//
	bool RSCConvertPixtangle_8to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);

	bool RSCConvertPixtangle_15to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertPixtangle_15to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertPixtangle_15to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertPixtangle_15to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);

	bool RSCConvertPixtangle_16to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertPixtangle_16to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertPixtangle_16to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertPixtangle_16to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);

	bool RSCConvertPixtangle_24to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertPixtangle_24to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertPixtangle_24to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertPixtangle_24to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);

	//
	bool RSCConvertPixtangle_32to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette);
	bool RSCConvertPixtangle_32to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertPixtangle_32to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);
	bool RSCConvertPixtangle_32to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h);

public:
	RSC_PIXTANGLE_POS *m_pos;
	int m_pos_ctr;
	LONG m_screen_w;
	LONG m_screen_h;
	int m_pixtangle_w;
	int m_pixtangle_h;
	int m_last_pixtangle_w;
	int m_last_pixtangle_h;
	int m_n_pixtangle_w;
	int m_n_pixtangle_h;
	int m_Bps; // Byte(s) per scanline
	int m_Bpp; // Byte(s) per pixel
};