// 
// RSC Miscellaneous Class (RSCMisc.cpp) 
// By Jae H. Park 
//
// Created: 12.27.2004
// Rev. 10.20.2005, 10.22.2005
//
// RSCMisc.cpp contains some useful function definition for other classes.
//
#include "stdafx.h"

RSCErrorHandling::RSCErrorHandling()
{
	m_error_code = 0;
	memset(m_error_string, NULL, 1024);
}

RSCErrorHandling::~RSCErrorHandling()
{
}

void RSCErrorHandling::RSCSetErrorCode(LONG error_code, const char *error_string)
{
	m_error_code = error_code;
	memset(m_error_string, NULL, 1024);
	strcpy((char *) m_error_string, error_string);
}

RSC_DIBPIXELTYPE RSCGetPixelTypeFromBitsPerPixel(UINT bpp, UINT red_b, UINT green_b, UINT blue_b)
{
	RSC_DIBPIXELTYPE pixel_type;
	
	switch(bpp)
	{
	case 1: 
		pixel_type = DIB_1BPP;
		break;
	case 2: 
		pixel_type = DIB_2BPP;
		break;
	case 4: 
		pixel_type = DIB_4BPP;
		break;
	case 8: 
		pixel_type = DIB_8BPP;
		break;
	case 24: 
		pixel_type = DIB_24RGB888;
		break;
	case 15:
		pixel_type = DIB_16RGB555;
	case 16: 
		if((red_b==5) && (green_b==6) && (blue_b==5)){
			pixel_type = DIB_16RGB565;
		}else{
			if((red_b==5) && (green_b==5) && (blue_b==5)){
				pixel_type = DIB_16RGB555;
			}else{
				pixel_type = DIB_16RGB555;
			}
		}
		break;
	case 32: 
		pixel_type = DIB_32RGB888;
		break;
	default: 
		pixel_type = DIB_NONE;
	}

	return pixel_type;

}

RSC_DIBPIXELTYPE RSCGetPixelType(HDC hdc)
{
	RSC_DIBPIXELTYPE pixel_type = DIB_NONE;
	RSC_DIBINFO dibinfo;
	DWORD *pbitfields;
	DWORD red;
	DWORD green;
	DWORD blue;
	HBITMAP hbmp;

	hbmp = CreateCompatibleBitmap(hdc, 1, 1);
	if(hbmp == NULL)
		pixel_type = DIB_NONE;

	memset(&dibinfo, 0, sizeof(dibinfo));
	
	dibinfo.header.biSize = sizeof(BITMAPINFOHEADER);

	GetDIBits(hdc, hbmp, 0, 1, NULL, (BITMAPINFO *) &dibinfo, DIB_RGB_COLORS);
	GetDIBits(hdc, hbmp, 0, 1, NULL, (BITMAPINFO *) &dibinfo, DIB_RGB_COLORS);	
	
	DeleteObject(hbmp);
	
	switch(dibinfo.header.biBitCount)
	{
	case 1: 
		pixel_type = DIB_1BPP;
		break;
	case 2: 
		pixel_type = DIB_2BPP;
		break;
	case 4: 
		pixel_type = DIB_4BPP;
		break;
	case 8: 
		pixel_type = DIB_8BPP;
		break;
	case 24: 
		pixel_type = DIB_24RGB888;
		break;
	case 16: 
		pixel_type = DIB_16RGB555;
		break;
	case 32: 
		pixel_type = DIB_32RGB888;
		break;
	default: 
		pixel_type = DIB_NONE;
	}

	if(dibinfo.header.biCompression == BI_BITFIELDS){
		pbitfields = (DWORD *) dibinfo.colors;
		red = pbitfields[0];
		green = pbitfields[1];
		blue = pbitfields[2];

		if((red==0x7C00) && (green==0x03E0) && (blue==0x001F)){
			pixel_type = DIB_16RGB555;
		}else if((red==0xF800) && (green==0x07E0) && (blue==0x001F)){
			pixel_type = DIB_16RGB565;
		}else if ((red==0xFF0000) && (green==0xFF00) && (blue==0x00FF)){
			pixel_type = DIB_32RGB888;
		}else
			pixel_type = DIB_NONE;
	}

	return pixel_type;
}

DWORD RSCGetCurrentDCbpp(void)
{
	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;

	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) !=0){
		 return dm.dmBitsPerPel;
	}

	return -1;
}

bool RegSetStringValue(DWORD option, HKEY rootkey, char *subkey, DWORD type, char *valuename, BYTE *data)
{
	LONG ret;
	HKEY keyHandler;
	DWORD datasize;

	ret = RegCreateKeyEx(rootkey, 
		subkey, 
		0, 
		NULL, 
		option,
		KEY_ALL_ACCESS, 
		NULL, 
		&keyHandler, 
		NULL);
	
	if(ret != ERROR_SUCCESS){
		// Unable to create or open the subkey
		return false;
	}

	if(type == REG_SZ || type == REG_EXPAND_SZ || type == REG_MULTI_SZ){
		datasize = (DWORD) strlen((char *) data)+1;
	}else{
		return false;
		/*
		if(type == REG_DWORD)
			datasize = sizeof(DWORD);
		else{
			if(type == REG_BINARY){
				datasize = sizeof(BYTE);
			}else{
				// Not supported
				return false;
			}
		}
		*/
	}
	
	ret  = RegSetValueEx(keyHandler, 
		valuename,
		0,
		type,
		data,
		datasize);

	if(ret != ERROR_SUCCESS){
		ret = RegCloseKey(keyHandler);
		
		if(ret != ERROR_SUCCESS)
			return false;
	}

	ret = RegCloseKey(keyHandler);

	if(ret != ERROR_SUCCESS)
		return false;

	return true;
}

bool RegGetStringValue(HKEY rootkey, char *subkey, char *valuename, BYTE *data, DWORD datasize)
{
	LONG ret;
	HKEY keyHandler;
	DWORD type;

	ret = RegOpenKeyEx(rootkey, 
		subkey, 
		0, 
		KEY_ALL_ACCESS, 
		&keyHandler);
	
	if(ret != ERROR_SUCCESS){
		// "Unable to create or open the subkey"
		return false;
	}
	
	ret  = RegQueryValueEx(keyHandler, 
		valuename,
		0,
		&type,
		data,
		&datasize);

	if(ret != ERROR_SUCCESS){
		ret = RegCloseKey(keyHandler);
		if(ret != ERROR_SUCCESS)
			return false;
	}

	if(type != REG_SZ && type != REG_EXPAND_SZ && type != REG_MULTI_SZ){
		
		ret = RegCloseKey(keyHandler);
		if(ret != ERROR_SUCCESS)
			return false;
		
		return false;
	}

	ret = RegCloseKey(keyHandler);
	if(ret != ERROR_SUCCESS)
		return false;

	return true;
}


bool RegSetDWORDValue(DWORD option, HKEY rootkey, char *subkey, char *valuename, DWORD dwdata)
{
	LONG ret;
	HKEY keyHandler;
	DWORD datasize;
	BYTE data[4];

	ret = RegCreateKeyEx(rootkey, 
		subkey, 
		0, 
		NULL, 
		option,
		KEY_ALL_ACCESS, 
		NULL, 
		&keyHandler, 
		NULL);
	
	if(ret != ERROR_SUCCESS){
		// "Unable to create or open the subkey"
		return false;
	}
	
	datasize = sizeof(DWORD);
	
	data[3] = (BYTE) ((dwdata & 0xFF000000) >> 24); 
	data[2] = (BYTE) ((dwdata & 0x00FF0000) >> 16);
	data[1] = (BYTE) ((dwdata & 0x0000FF00) >> 8);
	data[0] = (BYTE) (dwdata & 0x000000FF);

	ret  = RegSetValueEx(keyHandler, 
		valuename,
		0,
		REG_DWORD,
		data,
		datasize);

	if(ret != ERROR_SUCCESS){
		ret = RegCloseKey(keyHandler);
		if(ret != ERROR_SUCCESS)
			return false;
	}

	ret = RegCloseKey(keyHandler);
	if(ret != ERROR_SUCCESS)
		return false;

	return true;
}

bool RegGetDWORDValue(HKEY rootkey, char *subkey, char *valuename, DWORD *retdata)
{
	LONG ret;
	HKEY keyHandler;
	DWORD type, datasize;
	BYTE data[1024];

	datasize = 1024;

	ret = RegOpenKeyEx(rootkey, 
		subkey, 
		0, 
		KEY_ALL_ACCESS, 
		&keyHandler);
	
	if(ret != ERROR_SUCCESS){
		// "Unable to create or open the subkey"
		return false;
	}
	
	ret  = RegQueryValueEx(keyHandler, 
		valuename,
		0,
		&type,
		data,
		&datasize);

	if(ret != ERROR_SUCCESS){
		ret = RegCloseKey(keyHandler);
		if(ret != ERROR_SUCCESS)
			return false;
	}

	if(type != REG_DWORD){
		ret = RegCloseKey(keyHandler);
		if(ret != ERROR_SUCCESS)
			return false;
		
		return false;
	}

	*retdata = 0;
	*retdata = *retdata | data[3];
	*retdata = *retdata << 8;
	*retdata = *retdata | data[2];
	*retdata = *retdata << 8;
	*retdata = *retdata | data[1];
	*retdata = *retdata << 8;
	*retdata = *retdata | data[0];
	

	ret = RegCloseKey(keyHandler);
	if(ret != ERROR_SUCCESS)
		return false;

	return true;
}

bool RegSetBinValue(DWORD option, HKEY rootkey, char *subkey, char *valuename, BYTE *data, int datalen)
{
	LONG ret;
	HKEY keyHandler;

	ret = RegCreateKeyEx(rootkey, 
		subkey, 
		0, 
		NULL, 
		option,
		KEY_ALL_ACCESS, 
		NULL, 
		&keyHandler, 
		NULL);
	
	if(ret != ERROR_SUCCESS){
		// "Unable to create or open the subkey"
		return false;
	}
	
	ret  = RegSetValueEx(keyHandler, 
		valuename,
		0,
		REG_BINARY,
		data,
		datalen);

	if(ret != ERROR_SUCCESS){
		ret = RegCloseKey(keyHandler);
		if(ret != ERROR_SUCCESS)
			return false;
	}

	ret = RegCloseKey(keyHandler);
	if(ret != ERROR_SUCCESS)
		return false;

	return true;
}

bool RegGetBinValue(HKEY rootkey, char *subkey, char *valuename, BYTE *data,  DWORD datasize, int *datalen)
{
	LONG ret;
	HKEY keyHandler;
	DWORD type;

	ret = RegOpenKeyEx(rootkey, 
		subkey, 
		0, 
		KEY_ALL_ACCESS, 
		&keyHandler);
	
	if(ret != ERROR_SUCCESS){
		// "Unable to create or open the subkey"
		return false;
	}
	
	ret  = RegQueryValueEx(keyHandler, 
		valuename,
		0,
		&type,
		data,
		&datasize);

	if(ret != ERROR_SUCCESS){
		ret = RegCloseKey(keyHandler);
		if(ret != ERROR_SUCCESS)
			return false;
	}

	if(type != REG_BINARY){
		ret = RegCloseKey(keyHandler);
		if(ret != ERROR_SUCCESS)
			return false;
		
		return false;
	}

	*datalen = datasize;
	
	return true;
}
