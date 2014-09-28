// 
// RSC Region Class (RSCRegion.cpp) 
// By Jae H. Park 
//
// Created: 12.27.2004
// Rev. 12.28.2004, 12.29.2004, 01.05.2005, 01.06.2005, 01,07,2005
// 01.17.2005, 01.29.2005, 01.30.2005, 09.11.2005, 10.15.2005, 10.20.2005
// 05.18.2006, 07.07.2006, 08.01.2006, 08.02.2006, 08.03.2006
//
// RSCRegion class handles the Device Dependent Bitmap and Device Independent Bitmap. 
// It captures the screen and store it in DDB format to be converted to DIB format.
// Once converted, the DIB may converted to lower color depth or treated in a compressed form
// in order to be transmitted via TCP/IP network.
//
//

#include "stdafx.h"

RSCRegion::RSCRegion()
{
	// Reset error code
	m_error.m_error_code = 0;
	m_regionprop.isscaled = FALSE;

	//m_hdcmem = NULL;
	m_hdc = NULL;
	m_hdcmem = NULL;
	m_dibhsize = 0;
	m_pdib = NULL;

	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = 0;
	m_regionprop.region_y = 0;
	m_regionprop.region_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_h = GetDeviceCaps(m_hdc, VERTRES);

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}
	
	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return;
	}

}

RSCRegion::RSCRegion(BOOL isscaled)
{
	// Reset error code
	m_error.m_error_code = 0;
	m_regionprop.isscaled = isscaled;

	//m_hdcmem = NULL;
	m_hdc = NULL;
	m_hdcmem = NULL;
	m_dibhsize = 0;
	m_pdib = NULL;

	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = 0;
	m_regionprop.region_y = 0;
	m_regionprop.region_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_h = GetDeviceCaps(m_hdc, VERTRES);

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}
	
	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return;
	}

}

RSCRegion::RSCRegion(LONG region_w, LONG region_h)
{
	// Reset error code
	m_error.m_error_code = 0;
	m_regionprop.isscaled = FALSE;

	//m_hdcmem = NULL;
	m_hdc = NULL;
	m_hdcmem = NULL;
	m_dibhsize = 0;
	m_pdib = NULL;

	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = 0;
	m_regionprop.region_y = 0;
	m_regionprop.region_w = region_w;
	m_regionprop.region_h = region_h;

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);


	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}
	
	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return;
	}

}

RSCRegion::RSCRegion(LONG region_w, LONG region_h, BOOL isscaled)
{
	// Reset error code
	m_error.m_error_code = 0;
	m_regionprop.isscaled = isscaled;

	//m_hdcmem = NULL;
	m_hdc = NULL;
	m_hdcmem = NULL;
	m_dibhsize = 0;
	m_pdib = NULL;

	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = 0;
	m_regionprop.region_y = 0;
	m_regionprop.region_w = region_w;
	m_regionprop.region_h = region_h;

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}
	
	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return;
	}

}

RSCRegion::RSCRegion(LONG region_x, LONG region_y, LONG region_w, LONG region_h)
{
	// Reset error code
	m_error.m_error_code = 0;
	m_regionprop.isscaled = FALSE;

	//m_hdcmem = NULL;
	m_hdc = NULL;
	m_hdcmem = NULL;
	m_dibhsize = 0;
	m_pdib = NULL;

	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = region_x;
	m_regionprop.region_y = region_y;
	m_regionprop.region_w = region_w;
	m_regionprop.region_h = region_h;

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}

	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return;
	}

}

//
// [Note] Scaling is disabled for sub-regional udpate request
//
/*
RSCRegion::RSCRegion(LONG region_x, LONG region_y, LONG region_w, LONG region_h, BOOL isscaled)
{
	// Reset error code
	m_error.m_error_code = 0;
	m_regionprop.isscaled = isscaled;

	//m_hdcmem = NULL;
	m_hdc = NULL;
	m_hdcmem = NULL;
	m_dibhsize = 0;
	m_pdib = NULL;

	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = region_x;
	m_regionprop.region_y = region_y;
	m_regionprop.region_w = region_w;
	m_regionprop.region_h = region_h;

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}

	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return;
	}

}
*/


RSCRegion::~RSCRegion()
{
	if(m_hdc != NULL){
		ReleaseDC(NULL, m_hdc);
		m_hdc = NULL;
	}

	if(m_hdcmem != NULL){
		DeleteDC(m_hdcmem);
		m_hdcmem = NULL;
	}

	if(m_hdcbmp != NULL){
		DeleteObject(m_hdcbmp);
		m_hdcbmp = NULL;
	}

	if(m_pdib != NULL){
		delete [] m_pdib;
		m_pdib = NULL;
	}
}

int RSCRegion::RSCIsScreenChanged(void)
// -1: error		0: false (no - same)	1: true (yes - changed)
{
	LONG new_region_w, new_region_h;
	DEVMODE dm;
	UINT new_bpp, new_Bps;

	new_region_w = GetDeviceCaps(m_hdc, HORZRES);
	new_region_h = GetDeviceCaps(m_hdc, VERTRES);
	
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		new_bpp = dm.dmBitsPerPel;
	}
	
	if(new_bpp < 8 || new_bpp > 32){
		// Error: Incompatible Device Mode
		return -1;
	}
	new_Bps = RSCGetBPS(new_region_w, new_bpp);

	//if(m_regionprop.region_o_h == new_region_h && m_regionprop.region_o_w == new_region_w && m_regionprop.bpp == new_bpp && m_regionprop.Bps == new_Bps){
	if(m_regionprop.region_o_h == new_region_h && m_regionprop.region_o_w == new_region_w && m_regionprop.bpp == new_bpp){
		return 0;
	}

	return 1;
}

bool RSCRegion::RSCResetRegion(void)
{

	if(m_hdc != NULL){
		ReleaseDC(NULL, m_hdc);
		m_hdc = NULL;
	}

	if(m_hdcmem != NULL){
		DeleteDC(m_hdcmem);
		m_hdcmem = NULL;
	}

	if(m_hdcbmp != NULL){
		DeleteObject(m_hdcbmp);
		m_hdcbmp = NULL;
	}

	if(m_pdib != NULL){
		delete [] m_pdib;
		m_pdib = NULL;
	}

	m_dibhsize = 0;
	m_regionprop.isscaled = FALSE;

	//memset(m_regionprop,0,sizeof(m_regionprop));
	
	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return false;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = 0;
	m_regionprop.region_y = 0;
	m_regionprop.region_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_h = GetDeviceCaps(m_hdc, VERTRES);

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	m_regionprop.isscaled = FALSE;

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}
	
	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return false;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return false;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return false;
	}


	return true;
}

bool RSCRegion::RSCResetRegion(BOOL isscaled)
{

	if(m_hdc != NULL){
		ReleaseDC(NULL, m_hdc);
		m_hdc = NULL;
	}

	if(m_hdcmem != NULL){
		DeleteDC(m_hdcmem);
		m_hdcmem = NULL;
	}

	if(m_hdcbmp != NULL){
		DeleteObject(m_hdcbmp);
		m_hdcbmp = NULL;
	}

	if(m_pdib != NULL){
		delete [] m_pdib;
		m_pdib = NULL;
	}

	m_dibhsize = 0;
	m_regionprop.isscaled = isscaled;

	//memset(m_regionprop,0,sizeof(m_regionprop));
	
	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return false;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = 0;
	m_regionprop.region_y = 0;
	m_regionprop.region_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_h = GetDeviceCaps(m_hdc, VERTRES);

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	m_regionprop.isscaled = FALSE;

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}
	
	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return false;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return false;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return false;
	}


	return true;
}

bool RSCRegion::RSCResetRegion(LONG region_w, LONG region_h)
{

	if(m_hdc != NULL){
		ReleaseDC(NULL, m_hdc);
		m_hdc = NULL;
	}

	if(m_hdcmem != NULL){
		DeleteDC(m_hdcmem);
		m_hdcmem = NULL;
	}

	if(m_hdcbmp != NULL){
		DeleteObject(m_hdcbmp);
		m_hdcbmp = NULL;
	}

	if(m_pdib != NULL){
		delete [] m_pdib;
		m_pdib = NULL;
	}

	m_dibhsize = 0;
	m_regionprop.isscaled = FALSE;

	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return false;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = 0;
	m_regionprop.region_y = 0;
	m_regionprop.region_w = region_w;
	m_regionprop.region_h = region_h;

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}

	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return false;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return false;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return false;
	}

	return true;
}

bool RSCRegion::RSCResetRegion(LONG region_w, LONG region_h, BOOL isscaled)
{

	if(m_hdc != NULL){
		ReleaseDC(NULL, m_hdc);
		m_hdc = NULL;
	}

	if(m_hdcmem != NULL){
		DeleteDC(m_hdcmem);
		m_hdcmem = NULL;
	}

	if(m_hdcbmp != NULL){
		DeleteObject(m_hdcbmp);
		m_hdcbmp = NULL;
	}

	if(m_pdib != NULL){
		delete [] m_pdib;
		m_pdib = NULL;
	}

	m_dibhsize = 0;
	m_regionprop.isscaled = isscaled;

	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return false;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = 0;
	m_regionprop.region_y = 0;
	m_regionprop.region_w = region_w;
	m_regionprop.region_h = region_h;

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}

	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return false;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return false;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return false;
	}

	return true;
}

bool RSCRegion::RSCResetRegion(LONG region_x, LONG region_y, LONG region_w, LONG region_h)
{

	if(m_hdc != NULL){
		ReleaseDC(NULL, m_hdc);
		m_hdc = NULL;
	}

	if(m_hdcmem != NULL){
		DeleteDC(m_hdcmem);
		m_hdcmem = NULL;
	}

	if(m_hdcbmp != NULL){
		DeleteObject(m_hdcbmp);
		m_hdcbmp = NULL;
	}

	if(m_pdib != NULL){
		delete [] m_pdib;
		m_pdib = NULL;
	}

	m_dibhsize = 0;
	m_regionprop.isscaled = FALSE;

	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return false;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = region_x;
	m_regionprop.region_y = region_y;
	m_regionprop.region_w = region_w;
	m_regionprop.region_h = region_h;

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}

	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return false;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return false;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return false;
	}

	return true;
}

//
// [Note] Scaling is disabled for sub-regional udpate request
//
/*
bool RSCRegion::RSCResetRegion(LONG region_x, LONG region_y, LONG region_w, LONG region_h, BOOL isscaled)
{

	if(m_hdc != NULL){
		ReleaseDC(NULL, m_hdc);
		m_hdc = NULL;
	}

	if(m_hdcmem != NULL){
		DeleteDC(m_hdcmem);
		m_hdcmem = NULL;
	}

	if(m_hdcbmp != NULL){
		DeleteObject(m_hdcbmp);
		m_hdcbmp = NULL;
	}

	if(m_pdib != NULL){
		delete [] m_pdib;
		m_pdib = NULL;
	}

	m_dibhsize = 0;
	m_regionprop.isscaled = isscaled;

	// Create or Get DC of the entire screen
	//m_hdc = CreateDC("DISPLAY", NULL, NULL, NULL);  // The entire display card instead of current display context 	
	m_hdc = GetDC(NULL); 

	if (m_hdc == NULL){
		m_error.RSCSetErrorCode(1, "[RSCRegion] Error in GetDC");
		return false;
	}
	
	// Region Position relative to the entire screen
	m_regionprop.region_x = region_x;
	m_regionprop.region_y = region_y;
	m_regionprop.region_w = region_w;
	m_regionprop.region_h = region_h;

	m_dib_pixel_type  = RSCGetPixelType(m_hdc);

	m_regionprop.region_o_x = 0;
	m_regionprop.region_o_y = 0;
	m_regionprop.region_o_w = GetDeviceCaps(m_hdc, HORZRES);
	m_regionprop.region_o_h = GetDeviceCaps(m_hdc, VERTRES);

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		m_regionprop.bpp = dm.dmBitsPerPel;
	}

	if(m_regionprop.bpp < 8 || m_regionprop.bpp > 32){
		m_error.RSCSetErrorCode(10, "Incompatible Device Mode");
		return false;
	}

	m_regionprop.Bps = RSCGetBPS(m_regionprop.region_w, m_regionprop.bpp);

	m_hdcbmp = CreateCompatibleBitmap(m_hdc,
					m_regionprop.region_w, 
					m_regionprop.region_h); 

	if (m_hdcbmp == NULL){	
		m_error.RSCSetErrorCode(2, "[RSCRegion] Error in CreateCompatibleBitmap");
		return false;
	}

	m_hdcmem = CreateCompatibleDC(m_hdc);
	if (m_hdcmem == NULL){
		m_error.RSCSetErrorCode(3, "[RSCRegion] Error in CreateCompatibleDC");
		return false;
	}

	return true;
}
*/

void RSCRegion::RSCResetRegionInfo(void)
{
	m_dibhsize = 0;
	m_regionprop.region_x = 0;
	m_regionprop.region_y = 0;
	m_regionprop.region_w = 0;
	m_regionprop.region_h = 0;
	m_dib_pixel_type  = DIB_NONE;
	m_regionprop.bpp = 0;
	m_regionprop.Bps = 0;

}

bool RSCRegion::RSCCapture()
{
	// Reset error code
	m_error.m_error_code = 0;

	HGDIOBJ holdobj;

	holdobj = SelectObject(m_hdcmem, m_hdcbmp);
	if (!holdobj){ 
		m_error.RSCSetErrorCode(6, "[RSCRegion] Error in SelectObject");
		return false;
	}
	
	if(m_regionprop.isscaled == TRUE){
		// HALFTONE - smooth coloring
		SetStretchBltMode(m_hdcmem, HALFTONE);
		SetBrushOrgEx(m_hdcmem, 0, 0, NULL);
		//

		if (!StretchBlt(m_hdcmem, 
				0, 0, 
				m_regionprop.region_w,
				m_regionprop.region_h, 
				m_hdc, 
				0, 0,
				m_regionprop.region_o_w, 
				m_regionprop.region_o_h, 
				SRCCOPY)){
		
			m_error.RSCSetErrorCode(7, "Error in BitBlt operation");
			return false;
		}

	}else{
		if (!BitBlt(m_hdcmem, 
				0,0, 
				m_regionprop.region_w,
				m_regionprop.region_h, 
				m_hdc, 
				m_regionprop.region_x, 
				m_regionprop.region_y, 
				SRCCOPY)){
		
			m_error.RSCSetErrorCode(7, "Error in BitBlt operation");
			return false;
		}
	}

	SelectObject(m_hdcmem, holdobj);
	return true;
}

bool RSCRegion::RSCInitDIB(HPALETTE hpal, WORD nbitcount, DWORD ncompression)
{
	// Reset error code
	m_error.m_error_code = 0;

	HGDIOBJ hpalold;
	int infosize;
	int totalsize;
	
	// Retrieve DDB information
	if(GetObject(m_hdcbmp, sizeof(BITMAP), &m_ddbinfo) == 0){
		m_error.RSCSetErrorCode(4, "[RSCRegion] Failed to retrieve DDB information");
		return false;

	}

	// Fill in data in BITMAPINFOHEADER data structure
	memset(&m_dibinfo, 0, sizeof(m_dibinfo));

	m_dibinfo.header.biSize			=	sizeof(BITMAPINFOHEADER);
	m_dibinfo.header.biWidth		=	m_ddbinfo.bmWidth;
	m_dibinfo.header.biHeight		=	m_ddbinfo.bmHeight;
	m_dibinfo.header.biPlanes		=	1;
	m_dibinfo.header.biBitCount		=	nbitcount;
	m_dibinfo.header.biCompression	=	ncompression;


	if(hpal)
		hpalold	= SelectPalette(m_hdc, hpal, FALSE);
	else
		hpalold = NULL;

	// Query GDI for image size
	// The first parameter (m_hdc) specifies a reference-device context, 
	// whose palette will be used in the pixel array format conversion.
	// If the fifth parameter is NULL, the function passes the DIMENSIONS 
	// and FORMAT of the bitmap to the BITMAPINFO structure pointed to by m_dibinfo parameter. 
	
	GetDIBits(m_hdc, m_hdcbmp, 0, m_ddbinfo.bmHeight, NULL, (BITMAPINFO *) &m_dibinfo, DIB_RGB_COLORS);
	
	// Find out the size of DIB Information Header
	if(m_dibinfo.header.biBitCount <= 8)
		if(m_dibinfo.header.biClrUsed) // biClrUsed = number of Colors in the color table
			infosize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*m_dibinfo.header.biClrUsed;
		else
			infosize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*(1 <<m_dibinfo.header.biBitCount);
	else if(m_dibinfo.header.biCompression == BI_BITFIELDS)
		infosize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*(3+m_dibinfo.header.biClrUsed); // bit masks = 3 * double words (3 * 4)
	else
		infosize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*(m_dibinfo.header.biClrUsed);

	m_dibhsize = infosize;
	
	// Find out the totol size of DIB structure (DIB Information Header + Array of Pixels)


	if (m_dibinfo.header.biSizeImage)
		totalsize = infosize + (m_dibinfo.header.biSizeImage);
	else
		totalsize = infosize + ((m_dibinfo.header.biWidth * m_dibinfo.header.biBitCount + 31)/32*4 * m_dibinfo.header.biPlanes * abs(m_dibinfo.header.biHeight));
	
	if(m_pdib != NULL){
		// If m_pdib is not null, this routine release the previously allocated space for m_pdib
		delete [] m_pdib;
		m_pdib = NULL;
	}

	m_pdib = new BYTE[totalsize];

	if(m_pdib){
		memcpy(m_pdib, &m_dibinfo, infosize);
	}else{	
		m_error.RSCSetErrorCode(5, "[RSCRegion]  Failed to obtain the memory allocation for DIB");
		if(hpalold)
			SelectObject(m_hdc, hpalold);
		return false;
	}
	
	if(hpalold)
		SelectObject(m_hdc, hpalold);
	
	return true;

}

bool RSCRegion::RSCDDB2DIB(UINT height, UINT hinfo_size, UINT color_tbl_identifier)
// color_tbl_identifier could be either DIB_RGB_COLORS or DIB_PAL_COLORS
{
	// Reset error code
	m_error.m_error_code = 0;

	// color_tbl_identifier should always be DIB_RGB_COLORS. This stores the bits of the DIB 
	// with the BITMAPINFOHEADER structure and completes the color table.
	color_tbl_identifier = DIB_RGB_COLORS;

	if(m_pdib){
		if(height != GetDIBits(m_hdc, m_hdcbmp, 0, height, m_pdib + hinfo_size, (BITMAPINFO *) m_pdib, color_tbl_identifier)){
			m_error.RSCSetErrorCode(9, "Error in GetDIBits operation");
			return false;
		}
	}else{
		m_error.RSCSetErrorCode(8, "m_pdib is null");
		return false;
	}

	return true;
}

HBITMAP RSCRegion::RSCDIB2DDB(BYTE *pdibdata, RSC_DIBINFO *dibheader, UINT color_type)
// color_type could be either DIB_RGB_COLORS or DIB_PAL_COLORS
{
	// Reset error code
	m_error.m_error_code = 0;
	
	// color_type should always be DIB_RGB_COLORS
	color_type = DIB_RGB_COLORS;

	// Since the fifth parameter is NULL, the function passes the DIMENSIONS 
	// and FORMAT of the bitmap to the BITMAPINFO structure pointed to by m_dibinfo parameter. 
	if(GetDIBits(m_hdc, m_hdcbmp, 0, dibheader->header.biHeight, NULL, (BITMAPINFO *) dibheader, color_type) == 0){
		// If GetDIBits == 0, Then the function call fails.
		m_error.RSCSetErrorCode(11, "[RSCRegion] Error in GetDIBits - BITMAPINFO Retrieval");

	}

	return CreateDIBitmap(m_hdc, &(dibheader->header), 
							CBM_INIT, pdibdata, (BITMAPINFO *) dibheader,
							color_type);
}

HBITMAP RSCRegion::RSCDIB2DDB(BYTE *pdibdata, RSC_DIBINFO *dibheader, RSCPalette *pal, UINT color_type)
// color_type could be either DIB_RGB_COLORS or DIB_PAL_COLORS
{
	// Reset error code
	m_error.m_error_code = 0;
	
	// color_type should always be DIB_RGB_COLORS
	color_type = DIB_RGB_COLORS;

	// Since the fifth parameter is NULL, the function passes the DIMENSIONS 
	// and FORMAT of the bitmap to the BITMAPINFO structure pointed to by m_dibinfo parameter. 
	
	
	HBITMAP retbmp;

	if(m_regionprop.bpp == 8){ // 8 bits color handling code
		HPALETTE holdpal;
		
		holdpal = SelectPalette(m_hdc, pal->m_palette, FALSE);
		RealizePalette(m_hdc);
		
		retbmp = CreateDIBitmap(m_hdc, &(dibheader->header), 
							CBM_INIT, pdibdata, (BITMAPINFO *) dibheader,
							color_type);

		SelectPalette(m_hdc, holdpal, TRUE);

	}else{
		if(GetDIBits(m_hdc, m_hdcbmp, 0, dibheader->header.biHeight, NULL, (BITMAPINFO *) dibheader, color_type) == 0){
			// If GetDIBits == 0, Then the function call fails.
			m_error.RSCSetErrorCode(11, "[RSCRegion] Error in GetDIBits - BITMAPINFO Retrieval");

		}	

		retbmp = CreateDIBitmap(m_hdc, &(dibheader->header), 
							CBM_INIT, pdibdata, (BITMAPINFO *) dibheader,
							color_type);
	}
	
	return retbmp;
}


bool RSCRegion::RSCConvertDIB_8to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RGBQUAD *rgbquad)
{
	LONG bps16, bps8, aux16, aux8;
	BYTE scr_tmp;
	WORD *dst_tmp;
	
	BYTE scr_tmp1[4];
	DWORD *scr_tmp2;

	bps16 = RSCGetBPS(screen_w, 16);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps16 = screen_w*2;
	//bps8 = screen_w;

	aux16 = bps16*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + aux8 - bps8*y];
			dst_tmp = (WORD *) &dst[2*x + aux16 - bps16*y];

			//scr_tmp = scr[x+(bps8*((screen_h-1)-y) )];
			//dst_tmp = (WORD *) &dst[2*x+( bps16*((screen_h-1)-y) )];
			
			scr_tmp1[0] = (BYTE) (rgbquad[scr_tmp].rgbBlue);	// BLUE
			scr_tmp1[1] = (BYTE) (rgbquad[scr_tmp].rgbGreen);	// GREEN
			scr_tmp1[2] = (BYTE) (rgbquad[scr_tmp].rgbRed);		// RED
			scr_tmp1[3] = 0;
			scr_tmp2 = (DWORD *) &scr_tmp1[0];

			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x0000fc00) >> 5); // GREEN
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x00f80000) >> 8); //RED

		}
	}

	return true;
}	
bool RSCRegion::RSCConvertDIB_8to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RGBQUAD *rgbquad)
{
	LONG bps15, bps8, aux15, aux8;
	BYTE scr_tmp;
	WORD *dst_tmp;
	
	BYTE scr_tmp1[4];
	DWORD *scr_tmp2;

	bps15 = RSCGetBPS(screen_w, 15);
	bps8 = RSCGetBPS(screen_w, 8);
	
	// Debug
	//bps15 = screen_w*2;
	//bps8 = screen_w;

	aux15 = bps15*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x+ aux8 - bps8*y];
			dst_tmp = (WORD *) &dst[2*x + aux15 - bps15*y];

			//scr_tmp = scr[x+(bps8*((screen_h-1)-y) )];
			//dst_tmp = (WORD *) &dst[2*x+( bps15*((screen_h-1)-y) )];
			
			scr_tmp1[0] = (BYTE) (rgbquad[scr_tmp].rgbBlue);	// BLUE
			scr_tmp1[1] = (BYTE) (rgbquad[scr_tmp].rgbGreen);	// GREEN
			scr_tmp1[2] = (BYTE) (rgbquad[scr_tmp].rgbRed);		// RED
			scr_tmp1[3] = 0;
			scr_tmp2 = (DWORD *) &scr_tmp1[0];

			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x0000f800) >> 6); // GREEN
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x00f80000) >> 9); //RED

		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_8to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RGBQUAD *rgbquad)
{
	LONG bps24, bps8, aux24, aux8;
	BYTE scr_tmp;
	BYTE *dst_tmp;

	bps24 = RSCGetBPS(screen_w, 24);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps24 = screen_w*4;
	//bps8 = screen_w;

	aux24 = bps24*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + aux8 - bps8*y];
			dst_tmp = &dst[3*x + aux24 - bps24*y];

			//scr_tmp = scr[x+(bps8*((screen_h-1)-y) )];
			//dst_tmp = &dst[3*x+( bps24*((screen_h-1)-y) )];
			
			dst_tmp[0] = (BYTE) (rgbquad[scr_tmp].rgbBlue);		// BLUE
			dst_tmp[1] = (BYTE) (rgbquad[scr_tmp].rgbGreen);	// GREEN
			dst_tmp[2] = (BYTE) (rgbquad[scr_tmp].rgbRed);		// RED
		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_8to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RGBQUAD *rgbquad)
{
	LONG bps32, bps8, aux32, aux8;
	BYTE scr_tmp;
	BYTE *dst_tmp;

	bps32 = RSCGetBPS(screen_w, 32);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps32 = screen_w*4;
	//bps8 = screen_w;

	aux32 = bps32*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + aux8 - bps8*y];
			dst_tmp = &dst[4*x + aux32 - bps32*y];

			//scr_tmp = scr[x+(bps8*((screen_h-1)-y) )];
			//dst_tmp = &dst[4*x+( bps32*((screen_h-1)-y) )];
			
			dst_tmp[0] = (BYTE) (rgbquad[scr_tmp].rgbBlue);		// BLUE
			dst_tmp[1] = (BYTE) (rgbquad[scr_tmp].rgbGreen);	// GREEN
			dst_tmp[2] = (BYTE) (rgbquad[scr_tmp].rgbRed);		// RED
			dst_tmp[3] = 0;
		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_8to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps16, bps8, aux16, aux8;
	BYTE scr_tmp;
	WORD *dst_tmp;
	
	BYTE scr_tmp1[4];
	DWORD *scr_tmp2;

	bps16 = RSCGetBPS(screen_w, 16);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps16 = screen_w*2;
	//bps8 = screen_w;

	aux16 = bps16*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + aux8 - bps8*y];
			dst_tmp = (WORD *) &dst[2*x + aux16 - bps16*y];

			//scr_tmp = scr[x+(bps8*((screen_h-1)-y) )];
			//dst_tmp = (WORD *) &dst[2*x+( bps16*((screen_h-1)-y) )];
			
			scr_tmp1[0] = palette->m_col_table[scr_tmp*4+2];//(BYTE) (rgbquad[scr_tmp].rgbBlue);	// BLUE
			scr_tmp1[1] = palette->m_col_table[scr_tmp*4+1];//(BYTE) (rgbquad[scr_tmp].rgbGreen);	// GREEN
			scr_tmp1[2] = palette->m_col_table[scr_tmp*4];//(BYTE) (rgbquad[scr_tmp].rgbRed);		// RED
			scr_tmp1[3] = 0;
			scr_tmp2 = (DWORD *) &scr_tmp1[0];

			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x0000fc00) >> 5); // GREEN
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x00f80000) >> 8); //RED

		}
	}

	return true;
}	

bool RSCRegion::RSCConvertDIB_8to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps15, bps8, aux15, aux8;
	BYTE scr_tmp;
	WORD *dst_tmp;
	
	BYTE scr_tmp1[4];
	DWORD *scr_tmp2;

	bps15 = RSCGetBPS(screen_w, 15);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps15 = screen_w*2;
	//bps8 = screen_w;

	aux15 = bps15*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + aux8 - bps8*y];
			dst_tmp = (WORD *) &dst[2*x + aux15 - bps15*y];

			//scr_tmp = scr[x+(bps8*((screen_h-1)-y) )];
			//dst_tmp = (WORD *) &dst[2*x+( bps15*((screen_h-1)-y) )];
			
			scr_tmp1[0] = palette->m_col_table[scr_tmp*4+2]; //(BYTE) (rgbquad[scr_tmp].rgbBlue);	// BLUE
			scr_tmp1[1] = palette->m_col_table[scr_tmp*4+1]; //(BYTE) (rgbquad[scr_tmp].rgbGreen);	// GREEN
			scr_tmp1[2] = palette->m_col_table[scr_tmp*4]; //(BYTE) (rgbquad[scr_tmp].rgbRed);		// RED
			scr_tmp1[3] = 0;
			scr_tmp2 = (DWORD *) &scr_tmp1[0];

			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x0000f800) >> 6); // GREEN
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp2 & 0x00f80000) >> 9); //RED

		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_8to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps24, bps8, aux24, aux8;
	BYTE scr_tmp;
	BYTE *dst_tmp;

	bps24 = RSCGetBPS(screen_w, 24);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps24 = screen_w*4;
	//bps8 = screen_w;


	aux24 = bps24*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + aux8 - bps8*y];
			dst_tmp = &dst[3*x + aux24 - bps24*y];

			//scr_tmp = scr[x+(bps8*((screen_h-1)-y) )];
			//dst_tmp = &dst[3*x+( bps24*((screen_h-1)-y) )];
			
			dst_tmp[0] = palette->m_col_table[scr_tmp*4+2]; //(BYTE) (rgbquad[scr_tmp].rgbBlue);		// BLUE
			dst_tmp[1] = palette->m_col_table[scr_tmp*4+1]; //(BYTE) (rgbquad[scr_tmp].rgbGreen);	// GREEN
			dst_tmp[2] = palette->m_col_table[scr_tmp*4]; //(BYTE) (rgbquad[scr_tmp].rgbRed);		// RED
		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_8to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps32, bps8, aux32, aux8;
	BYTE scr_tmp;
	BYTE *dst_tmp;

	bps32 = RSCGetBPS(screen_w, 32);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps32 = screen_w*4;
	//bps8 = screen_w;

	aux32 = bps32*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + aux8 - bps8*y];
			dst_tmp = &dst[4*x + aux32 - bps32*y];

			//scr_tmp = scr[x+(bps8*((screen_h-1)-y) )];
			//dst_tmp = &dst[4*x+( bps32*((screen_h-1)-y) )];
			
			dst_tmp[0] = palette->m_col_table[scr_tmp*4+2]; //(BYTE) (rgbquad[scr_tmp].rgbBlue);	// BLUE
			dst_tmp[1] = palette->m_col_table[scr_tmp*4+1]; //(BYTE) (rgbquad[scr_tmp].rgbGreen);	// GREEN
			dst_tmp[2] = palette->m_col_table[scr_tmp*4]; //(BYTE) (rgbquad[scr_tmp].rgbRed);		// RED
			dst_tmp[3] = 0;
		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_15to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps15, bps8, aux15, aux8;
	DWORD *scr_tmp;
	BYTE *dst_tmp;

	bps15 = RSCGetBPS(screen_w, 15);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps15 = screen_w*2;
	//bps8 = screen_w;

	aux15 = bps15*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
			
			scr_tmp = (DWORD *) &scr[2*x + aux15 - bps15*y];
			dst_tmp = (BYTE *) &dst[x + aux8 - bps8*y];

			//scr_tmp = (DWORD *) &scr[2*x+( bps15*((screen_h-1)-y) )];
			//dst_tmp = (BYTE *) &dst[x+( bps8*((screen_h-1)-y) )];

			*dst_tmp = 0;
			*dst_tmp = palette->m_rgb_index[(BYTE)((*scr_tmp&0x7C00)>>10)][(BYTE) ((*scr_tmp&0x03E0)>>5)][(BYTE) (*scr_tmp&0x001F)];
		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_15to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps15, bps16, aux15, aux16;
	WORD *scr_tmp;
	WORD *dst_tmp;
	
	bps15 = RSCGetBPS(screen_w, 15);
	bps16 = RSCGetBPS(screen_w, 16);

	// Debug
	//bps15 = screen_w*2;
	//bps16 = screen_w*2;
			
	aux15 = bps15*(screen_h-1);
	aux16 = bps16*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + aux15 - bps15*y];
			dst_tmp = (WORD *) &scr[2*x + aux16 - bps16*y];

			//scr_tmp = (WORD *) &scr[(x<<1) + aux15 - bps15*y];
			//dst_tmp = (WORD *) &scr[(x<<1) + aux16 - bps16*y];
			
			//scr_tmp = (WORD *) &scr[2*x+( bps15*((screen_h-1)-y) )];
			//dst_tmp = (WORD *) &scr[2*x+( bps16*((screen_h-1)-y) )];

			*dst_tmp = ((((*scr_tmp)&0x7fe0)<<1)|((*scr_tmp)&0x001f));//*scr_tmp;
			
		}
	}
	return true;
}

bool RSCRegion::RSCConvertDIB_15to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps24, bps15, aux24, aux15;
	WORD *scr_tmp;
	BYTE *dst_tmp;
	
	bps24 = RSCGetBPS(screen_w, 24);
	bps15 = RSCGetBPS(screen_w, 15);
	
	// Debug
	//bps24 = screen_w*4;
	//bps15 = screen_w*2;


	aux24 = bps24*(screen_h-1);
	aux15 = bps15*(screen_h-1);
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + aux15 - bps15*y];
			dst_tmp = &dst[3*x + aux24 - bps24*y];

			//scr_tmp = (WORD *) &scr[2*x+( bps15*((screen_h-1)-y) )];
			//dst_tmp = &dst[3*x+( bps24*((screen_h-1)-y) )];
					
			dst_tmp[0] = (BYTE) ((*scr_tmp & 0x001F) << 3); // BLUE
			dst_tmp[1] = (BYTE) ((*scr_tmp & 0x03E0) >> 2); // GREEN
			dst_tmp[2] = (BYTE) ((*scr_tmp & 0x7C00) >> 7); // RED
		}
	}
	return true;
}

bool RSCRegion::RSCConvertDIB_15to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps15, aux32, aux15;
	WORD *scr_tmp;
	BYTE *dst_tmp;
	
	bps32 = RSCGetBPS(screen_w, 32);
	bps15 = RSCGetBPS(screen_w, 15);

	// Debug
	//bps32 = screen_w*4;
	//bps15 = screen_w*2;

	aux32 = bps32*(screen_h-1);
	aux15 = bps15*(screen_h-1);
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + aux15 - bps15*y];
			dst_tmp = &dst[4*x + aux32 - bps32*y];

			//scr_tmp = (WORD *) &scr[2*x+( bps15*((screen_h-1)-y) )];
			//dst_tmp = &dst[4*x+( bps32*((screen_h-1)-y) )];
					
			dst_tmp[0] = (BYTE) ((*scr_tmp & 0x001F) << 3); // BLUE
			dst_tmp[1] = (BYTE) ((*scr_tmp & 0x03E0) >> 2); // GREEN
			dst_tmp[2] = (BYTE) ((*scr_tmp & 0x7C00) >> 7); // RED
			dst_tmp[3] = 0;

		}
	}
	return true;
}

bool RSCRegion::RSCConvertDIB_16to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps16, bps8, aux16, aux8;
	DWORD *scr_tmp;
	BYTE *dst_tmp;

	bps16 = RSCGetBPS(screen_w, 16);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps16 = screen_w*2;
	//bps8 = screen_w;

	aux16 = bps16*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
			
			scr_tmp = (DWORD *) &scr[2*x + aux16 - bps16*y];
			dst_tmp = (BYTE *) &dst[x + aux8 - bps8*y];

			//scr_tmp = (DWORD *) &scr[2*x+( bps16*((screen_h-1)-y) )];
			//dst_tmp = (BYTE *) &dst[x+( bps8*((screen_h-1)-y) )];

			*dst_tmp = 0;
			*dst_tmp = palette->m_rgb_index[(BYTE)((*scr_tmp&0xF800)>>11)][(BYTE) ((*scr_tmp&0x07E0)>>6)][(BYTE) (*scr_tmp&0x001F)];
		}
	}
	
	return true;

}

bool RSCRegion::RSCConvertDIB_16to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps15, bps16, aux15, aux16;
	WORD *scr_tmp;
	WORD *dst_tmp;
	
	bps15 = RSCGetBPS(screen_w, 15);
	bps16 = RSCGetBPS(screen_w, 16);

	// Debug
	//bps15 = screen_w*2;
	//bps16 = screen_w*2;
			
	aux15 = bps15*(screen_h-1);
	aux16 = bps16*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + aux16 - bps16*y];
			dst_tmp = (WORD *) &scr[2*x + aux15 - bps15*y];

			//scr_tmp = (WORD *) &scr[2*x+( bps16*((screen_h-1)-y) )];
			//dst_tmp = (WORD *) &scr[2*x+( bps15*((screen_h-1)-y) )];
			
			*dst_tmp = ((((*scr_tmp)&0xffc0)>>1)|((*scr_tmp)&0x001f));//*scr_tmp;

		}
	}

	return true;

}

bool RSCRegion::RSCConvertDIB_16to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps24, bps16, aux24, aux16;
	WORD *scr_tmp;
	BYTE *dst_tmp;
	
	bps24 = RSCGetBPS(screen_w, 24);
	bps16 = RSCGetBPS(screen_w, 16);

	// Debug
	//bps24 = screen_w*4;
	//bps16 = screen_w*2;

	aux24 = bps24*(screen_h-1);
	aux16 = bps16*(screen_h-1);
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + aux16 - bps16*y];
			dst_tmp = &dst[3*x + aux24 - bps24*y];

			//scr_tmp = (WORD *) &scr[2*x+( bps16*((screen_h-1)-y) )];
			//dst_tmp = &dst[3*x+( bps24*((screen_h-1)-y) )];
					
			dst_tmp[0] = (BYTE) ((*scr_tmp & 0x001F) << 3); // BLUE
			dst_tmp[1] = (BYTE) ((*scr_tmp & 0x07E0) >> 3); // GREEN
			dst_tmp[2] = (BYTE) ((*scr_tmp & 0xF800) >> 8); // RED

		}
	}
	return true;
}

bool RSCRegion::RSCConvertDIB_16to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps16, aux32, aux16;
	WORD *scr_tmp;
	BYTE *dst_tmp;
	
	bps32 = RSCGetBPS(screen_w, 32);
	bps16 = RSCGetBPS(screen_w, 16);

	// Debug
	//bps32 = screen_w*4;
	//bps16 = screen_w*2;
	
	aux32 = bps32*(screen_h-1);
	aux16 = bps16*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + aux16 - bps16*y];
			dst_tmp = &dst[4*x + aux32 - bps32*y];

			//scr_tmp = (WORD *) &scr[2*x+( bps16*((screen_h-1)-y) )];
			//dst_tmp = &dst[4*x+( bps32*((screen_h-1)-y) )];
					
			dst_tmp[0] = (BYTE) ((*scr_tmp & 0x001F) << 3); // BLUE
			dst_tmp[1] = (BYTE) ((*scr_tmp & 0x07E0) >> 3); // GREEN
			dst_tmp[2] = (BYTE) ((*scr_tmp & 0xF800) >> 8); // RED
			dst_tmp[3] = 0;

		}
	}
	return true;
}

bool RSCRegion::RSCConvertDIB_24to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps24, bps8, aux24, aux8;
	DWORD *scr_tmp;
	BYTE *scr_bytetmp;
	BYTE *dst_tmp;

	bps24 = RSCGetBPS(screen_w, 24);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps24 = screen_w*4;
	//bps8 = screen_w;

	aux24 = bps24*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
			
			scr_tmp = (DWORD *) &scr[3*x + aux24 - bps24*y];
			dst_tmp = (BYTE *) &dst[x+ aux8 - bps8*y];

			//scr_tmp = (DWORD *) &scr[3*x+( bps24*((screen_h-1)-y) )];
			//dst_tmp = (BYTE *) &dst[x+( bps8*((screen_h-1)-y) )];
			
			*dst_tmp = 0;
			scr_bytetmp = (BYTE *) scr_tmp;
			*dst_tmp = palette->m_rgb_index[(scr_bytetmp[2]>>3)][(scr_bytetmp[1]>>3)][(scr_bytetmp[0]>>3)];
		}
	}
	
	return true;
}

bool RSCRegion::RSCConvertDIB_24to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps24, bps15, aux24, aux15;
	DWORD *scr_tmp;
	WORD *dst_tmp;
	
	bps24 = RSCGetBPS(screen_w, 24);
	bps15 = RSCGetBPS(screen_w, 15);

	// Debug
	//bps24 = screen_w*4;
	//bps15 = screen_w*2;
			
	aux24 = bps24*(screen_h-1);
	aux15 = bps15*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[3*x + aux24 - bps24*y];
			dst_tmp = (WORD *) &dst[2*x + aux15 - bps15*y];

			//scr_tmp = (DWORD *) &scr[3*x+( bps24*((screen_h-1)-y) )];
			//dst_tmp = (WORD *) &dst[2*x+( bps15*((screen_h-1)-y) )];
			
			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x0000f800) >> 6); // GREEN
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x00f80000) >> 9); //RED

		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_24to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps24, bps16, aux24, aux16;
	DWORD *scr_tmp;
	WORD *dst_tmp;
	
	bps24 = RSCGetBPS(screen_w, 24);
	bps16 = RSCGetBPS(screen_w, 16);

	// Debug
	//bps24 = screen_w*4;
	//bps16 = screen_w*2;
			
	aux24 = bps24*(screen_h-1);
	aux16 = bps16*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[3*x + aux24 - bps24*y];
			dst_tmp = (WORD *) &dst[2*x + aux16 - bps16*y];

			//scr_tmp = (DWORD *) &scr[3*x+( bps24*((screen_h-1)-y) )];
			//dst_tmp = (WORD *) &dst[2*x+( bps16*((screen_h-1)-y) )];
			
			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x0000fc00) >> 5); // GREEN // 0x0000fc00
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x00f80000) >> 8); //RED

		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_24to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps24, aux32, aux24;
	DWORD *scr_tmp;
	DWORD *dst_tmp;
	
	bps32 = RSCGetBPS(screen_w, 32);
	bps24 = RSCGetBPS(screen_w, 24);

	// Debug
	//bps32 = screen_w*4;
	//bps24 = screen_w*4;

	aux32 = bps32*(screen_h-1);
	aux24 = bps24*(screen_h-1);
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[3*x + aux24 - bps24*y];
			dst_tmp = (DWORD *) &dst[4*x + aux32 - bps32*y];

			//scr_tmp = (DWORD *) &scr[3*x+( bps24*((screen_h-1)-y) )];
			//dst_tmp = (DWORD *) &dst[4*x+( bps32*((screen_h-1)-y) )];
			
			dst_tmp[0] = scr_tmp[0];
			dst_tmp[1] = scr_tmp[1];
			dst_tmp[2] = scr_tmp[2];

		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_32to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps32, bps8, aux32, aux8;
	DWORD *scr_tmp;
	BYTE *scr_bytetmp;
	BYTE *dst_tmp;

	bps32 = RSCGetBPS(screen_w, 32);
	bps8 = RSCGetBPS(screen_w, 8);

	// Debug
	//bps32 = screen_w*4;
	//bps8 = screen_w;
	
	aux32 = bps32*(screen_h-1);
	aux8 = bps8*(screen_h-1);

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
			
			scr_tmp = (DWORD *) &scr[4*x + aux32 - bps32*y];
			dst_tmp = (BYTE *) &dst[x + aux8 - bps8*y];

			//scr_tmp = (DWORD *) &scr[4*x+( bps32*((screen_h-1)-y) )];
			//dst_tmp = (BYTE *) &dst[x+( bps8*((screen_h-1)-y) )];
			
			*dst_tmp = 0;
			scr_bytetmp = (BYTE *) scr_tmp;
			*dst_tmp = palette->m_rgb_index[(scr_bytetmp[2]>>3)][(scr_bytetmp[1]>>3)][(scr_bytetmp[0]>>3)];
			
			// Debug
			//scr_bytetmp = (BYTE *) scr_tmp;
			//TCHAR tmpbuf[1024];
			//wsprintf(tmpbuf, "R: %d, G: %d, B: %d",scr_bytetmp[2], scr_bytetmp[1], scr_bytetmp[0]);
			//MessageBox(NULL, tmpbuf, "Server Debug", MB_OK);

			// Debug - STD palette manipultion test ...
			//*dst_tmp = palette->GetPaletteIndex((((BYTE) scr_tmp[0])>>3))*36+palette->GetPaletteIndex((((BYTE) scr_tmp[1])>>3))*6+palette->GetPaletteIndex((((BYTE) scr_tmp[2])>>3));
			
			// Debug - Color testing ...
			//*dst_tmp = GetNearestPaletteIndex(palette->m_palette, RGB(0,255,0));

			// Debug - Color testing ...
			//*dst_tmp = palette->m_rgb_index[127][1][1];			
			
			// Debug -- Too Slow ...
			//*dst_tmp = GetNearestPaletteIndex(palette->m_palette, RGB(((BYTE) scr_tmp[2]), ((BYTE) scr_tmp[1]), ((BYTE) scr_tmp[0]))); 
				
			// Debug
			//tmpv = palette->GetPaletteIndex((BYTE) scr_tmp[0])*36+palette->GetPaletteIndex((BYTE) scr_tmp[1])*6+palette->GetPaletteIndex((BYTE) scr_tmp[2]);
			//*dst_tmp = tmpv;
		}
	}
	
	return true;
}

bool RSCRegion::RSCConvertDIB_32to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps15, aux32, aux15;
	DWORD *scr_tmp;
	WORD *dst_tmp;
	
	bps32 = RSCGetBPS(screen_w, 32);
	bps15 = RSCGetBPS(screen_w, 15);

	// Debug
	//bps32 = screen_w*4;
	//bps15 = screen_w*2;

	aux32 = bps32*(screen_h-1);
	aux15 = bps15*(screen_h-1);
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[4*x + aux32 - bps32*y];
			dst_tmp = (WORD *) &dst[2*x + aux15 - bps15*y];

			//scr_tmp = (DWORD *) &scr[4*x+( bps32*((screen_h-1)-y) )];
			//dst_tmp = (WORD *) &dst[2*x+( bps15*((screen_h-1)-y) )];
			
			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x0000f800) >> 6); // GREEN
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x00f80000) >> 9); //RED

		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_32to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps16, aux32, aux16;
	DWORD *scr_tmp;
	WORD *dst_tmp;
	
	bps32 = RSCGetBPS(screen_w, 32);
	bps16 = RSCGetBPS(screen_w, 16);

	// Debug
	//bps32 = screen_w*4;
	//bps16 = screen_w*2;

	aux32 = bps32*(screen_h-1);
	aux16 = bps16*(screen_h-1);
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[4*x + aux32 - bps32*y];
			dst_tmp = (WORD *) &dst[2*x + aux16 - bps16*y];

			//scr_tmp = (DWORD *) &scr[4*x+( bps32*((screen_h-1)-y) )];
			//dst_tmp = (WORD *) &dst[2*x+( bps16*((screen_h-1)-y) )];
			
			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x0000fc00) >> 5); // GREEN // 0x0000fc00
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x00f80000) >> 8); //RED

		}
	}

	return true;
}

bool RSCRegion::RSCConvertDIB_32to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps24, aux32, aux24;
	DWORD *scr_tmp;
	DWORD *dst_tmp;
	
	bps32 = RSCGetBPS(screen_w, 32);
	bps24 = RSCGetBPS(screen_w, 24);

	// Debug
	//bps32 = screen_w*4;
	//bps24 = screen_w*4;

	aux32 = bps32*(screen_h-1);
	aux24 = bps24*(screen_h-1);
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[4*x + aux32 - bps32*y];
			dst_tmp = (DWORD *) &dst[3*x + aux24 - bps24*y];

			//scr_tmp = (DWORD *) &scr[4*x+( bps32*((screen_h-1)-y) )];
			//dst_tmp = (DWORD *) &dst[3*x+( bps24*((screen_h-1)-y) )];
			
			dst_tmp[0] = scr_tmp[0];
			dst_tmp[1] = scr_tmp[1];
			dst_tmp[2] = scr_tmp[2];

		}
	}
	return true;
}


//
// Deprecated functions 
//
//
/*
int RSCRegion::RSCIsRegionChanged(LONG new_region_w, LONG new_region_h)
// -1: error		0: false (no - same)	1: true (yes - changed)
{
	DEVMODE dm;
	UINT new_bpp, new_Bps;

	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		new_bpp = dm.dmBitsPerPel;
	}
	
	if(new_bpp < 8 || new_bpp > 32){
		// Error: Incompatible Device Mode
		return -1;
	}

	new_Bps = RSCGetBPS(new_region_w, new_bpp);

	if(m_regionprop.region_h == new_region_h && m_regionprop.region_w == new_region_w && m_regionprop.bpp == new_bpp && m_regionprop.Bps == new_Bps){
		return 0;
	}

	return 1;
}


int RSCRegion::RSCIsRegionChanged(LONG new_region_x, LONG new_region_y, LONG new_region_w, LONG new_region_h)
// -1: error		0: false (no - same)	1: true (yes - changed)
{
	DEVMODE dm;
	UINT new_bpp, new_Bps;

	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		new_bpp = dm.dmBitsPerPel;
	}
	
	if(new_bpp < 8 || new_bpp > 32){
		// Error: Incompatible Device Mode
		return -1;
	}

	new_Bps = RSCGetBPS(new_region_w, new_bpp);

	if(m_regionprop.region_h == new_region_h && m_regionprop.region_x == new_region_x && m_regionprop.region_w == new_region_w && m_regionprop.region_y == new_region_y && m_regionprop.bpp == new_bpp && m_regionprop.Bps == new_Bps){
		return 0;
	}

	return 1;
}

*/
//
//
//
