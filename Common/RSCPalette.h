// 
// RSC Palette Class (RSCPalette.h) 
// By Jae H. Park 
//
// Created: 01.28.2005
// Rev. 10.05.2005, 10.13.2005
//
//
// RSCPalette class handles the Palette manipulation for Color Depth Reduction, 8-bit operation etc.
//
// Note: RSCMisc.h must be declared prior this class.
//

#pragma once

const BYTE STD_PALETTE[] = {0,51,102,153,204,255};
const BYTE STD_COLORSIZE = 6;
const BYTE STD_PALETTE_DIF = 51; 

typedef enum RSC_PALETTETYPE{
	RSC_PALETTE_HALFTONE,
	RSC_PALETTE_GRAYSCALE,
	RSC_PALETTE_SYSTEM,
	RSC_PALETTE_STANDARD
}RSC_PALETTETYPE;

class RSCPalette{
public:
	RSCPalette(void);
	~RSCPalette(void);
	bool SetRGBIndex(HDC hdc, RSC_PALETTETYPE type);
	bool CreateSTDPalette(void);
	bool FillColorTable(RSC_PALETTETYPE type);
	bool CreatePaletteFromColorTable(void);
	BYTE GetPaletteIndex(BYTE val);


private:
	bool CreateHalfTone(HDC hdc);
	bool CreateGrayscale(void);
	bool CreateSystem(void);

public:
	HPALETTE m_palette;
	BYTE m_col_table[1024];
	BYTE m_rgb_index[32][32][32];

};