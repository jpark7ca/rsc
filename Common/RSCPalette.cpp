// 
// RSC Palette Class (RSCPalette.cpp) 
// By Jae H. Park 
//
// Created: 01.29.2005
// Rev. 10.05.2005, 10.13.2005
//
// RSCPalette class handles the Palette manipulation for Color Depth Reduction, 8-bit operation etc.
//
//

#include "stdafx.h"

RSCPalette::RSCPalette()
{
	m_palette = NULL;
}

RSCPalette::~RSCPalette()
{
	if(m_palette != NULL)
	{
		DeleteObject(m_palette);
	}

}

bool RSCPalette::SetRGBIndex(HDC hdc, RSC_PALETTETYPE type)
{

	switch(type){
		case RSC_PALETTE_GRAYSCALE:
			if(!CreateGrayscale()){
				return false;
			}	
			break;
		case RSC_PALETTE_HALFTONE:
			if(!CreateHalfTone(hdc)){
				return false;
			}
			break;
		case RSC_PALETTE_SYSTEM:
			if(!CreateSystem()){
				return false;
			}
			break;
		case RSC_PALETTE_STANDARD:
			if(!CreateSTDPalette()){
				return false;
			}
			break;

	}

	if(type == RSC_PALETTE_GRAYSCALE){
		for(int red = 0; red<32; red++)
			for(int green = 0; green<32; green++)
				for(int blue = 0; blue<32; blue++)
				{
					//m_rgb_index[red][green][blue] = GetNearestPaletteIndex(m_palette, PALETTERGB(red*8, green*8, blue*8)); 
					m_rgb_index[red][green][blue] = GetNearestPaletteIndex(m_palette, RGB(red*8, green*8, blue*8)); 
					
					//double lum = (red*8)*0.299 + (green*8)*0.587 + (blue*8)*0.114;
					//m_rgb_index[red][green][blue] = (BYTE)(lum*(256-1)/255+0.5);
				}
	
	}else{
		for(int red = 0; red<32; red++)
			for(int green = 0; green<32; green++)
				for(int blue = 0; blue<32; blue++)
				{
					//m_rgb_index[red][green][blue] = GetNearestPaletteIndex(m_palette, PALETTERGB(red*8, green*8, blue*8)); 
					m_rgb_index[red][green][blue] = GetNearestPaletteIndex(m_palette, RGB(red*8, green*8, blue*8)); 					
				}
	}

	return true;
}


bool RSCPalette::CreateSTDPalette(void)
{
	int pal_ctr = 0;

	if(m_palette != NULL)
		DeleteObject(m_palette);

	LOGPALETTE *plogpal = (LOGPALETTE *) new BYTE[sizeof(LOGPALETTE) + 255 * sizeof(PALETTEENTRY)];
	if(plogpal == NULL)
		return false;

	plogpal->palVersion = 0x0300;
	plogpal->palNumEntries = 256; //216; //256;

	for(int i=0; i<STD_COLORSIZE; i++)
		for(int j=0; j<STD_COLORSIZE; j++)
			for(int k=0; k<STD_COLORSIZE; k++)
			{
				PALETTEENTRY entry = {STD_PALETTE[k], STD_PALETTE[j], STD_PALETTE[i], 0};
				plogpal->palPalEntry[pal_ctr] = entry;
				pal_ctr++;
			}

	m_palette = CreatePalette(plogpal);

	if(m_palette == NULL)
		return false;

	delete [] (BYTE *) plogpal;

	return true;
}


bool RSCPalette::CreatePaletteFromColorTable(void)
{
	if(m_palette != NULL)
		DeleteObject(m_palette);

	LOGPALETTE *plogpal = (LOGPALETTE *) new BYTE[sizeof(LOGPALETTE) + 255 * sizeof(PALETTEENTRY)];
	if(plogpal == NULL)
		return false;

	plogpal->palVersion = 0x0300;
	plogpal->palNumEntries = 256;


	for(int i=0; i<256; i++)
	{
		PALETTEENTRY entry = {m_col_table[i*4], m_col_table[(i*4)+1], m_col_table[(i*4)+2], 0};
		plogpal->palPalEntry[i] = entry;
	}

	m_palette = CreatePalette(plogpal);

	if(m_palette == NULL)
		return false;

	delete [] (BYTE *) plogpal;

	return true;
}

bool RSCPalette::CreateGrayscale(void)
{
	//if(m_palette != NULL)
	//	return false;
	if(m_palette != NULL)
		DeleteObject(m_palette);

	LOGPALETTE *plogpal = (LOGPALETTE *) new BYTE[sizeof(LOGPALETTE) + 255 * sizeof(PALETTEENTRY)];
	if(plogpal == NULL)
		return false;

	plogpal->palVersion = 0x0300;
	plogpal->palNumEntries = 256;

	for(int i=0; i<256; i++)
	{
		PALETTEENTRY entry = {i, i, i, 0};
		plogpal->palPalEntry[i] = entry;
	}

	m_palette = CreatePalette(plogpal);

	if(m_palette == NULL)
		return false;

	delete [] (BYTE *) plogpal;

	return true;
}

bool RSCPalette::CreateHalfTone(HDC hdc)
{
	//if(m_palette != NULL)
	//	return false;
	if(m_palette != NULL)
		DeleteObject(m_palette);
	
	m_palette = CreateHalftonePalette(hdc);

	if(m_palette == NULL)
		return false;

	return true;
}

bool RSCPalette::CreateSystem(void)
{
	//if(m_palette != NULL)
	//	return false;
	if(m_palette != NULL)
		DeleteObject(m_palette);

	m_palette = (HPALETTE) GetStockObject(DEFAULT_PALETTE);
	
	if(m_palette == NULL)
		return false;

	return true;
}

bool RSCPalette::FillColorTable(RSC_PALETTETYPE type)
{
	switch(type){
		case RSC_PALETTE_GRAYSCALE:
		/*	for(int i=0; i<256; i++)
			{
				m_col_table[i*4] = i;
				m_col_table[i*4+1] = i;
				m_col_table[i*4+2] = i;
				m_col_table[i*4+3] = 0;
			}
			
			break;
		*/
		case RSC_PALETTE_HALFTONE:
		case RSC_PALETTE_SYSTEM:
			if(GetPaletteEntries(m_palette, 0, 256, (LPPALETTEENTRY) m_col_table) == 0){
				return false;
			}
			break;
		case RSC_PALETTE_STANDARD:
			if(GetPaletteEntries(m_palette, 0, 256, (LPPALETTEENTRY) m_col_table) == 0){
				return false;
			}
			break;
	}

	return true;
}

BYTE RSCPalette::GetPaletteIndex(BYTE val)
{
	UINT  retval = 0;
    UINT  pos = val / STD_PALETTE_DIF;
    
	if(0 == val % STD_PALETTE_DIF)
    {
        retval = val/STD_PALETTE_DIF;
    }
    else
    {
        if(abs(val - STD_PALETTE[pos]) > abs(val - STD_PALETTE[pos+1]))
        {
            retval = pos+1;
        }
        else
        {
            retval = pos;
        }
    }
    return (BYTE) retval;
}