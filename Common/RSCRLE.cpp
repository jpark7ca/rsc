// 
// RSC Run-Length Enconding Class (RSCRLE.cpp) 
// By Jae H. Park 
//
// Created: 02.01.2005
//
// RSCRLE class compresses and decompresses the PIXEL data(8-bit, 16-bit and 32-bit)
// of the screen framebuffer using RLE algorithm in order to tranfer (nearly real time) massive image data
// over the network.
//
#include "stdafx.h"

RSCRLE::RSCRLE(void)
{

}

RSCRLE::~RSCRLE(void)
{

}

bool RSCRLE::RSCCompress32(BYTE *org, BYTE *cmp, ULONG org_size, ULONG *cmp_size)
{
	DWORD *ptr1, *ptr2;
	BYTE *ptr3;
	ULONG n_pix=0;
	ULONG i=0;
	BYTE c_run; // maxium c_run = 256 
	ULONG s_cmp=0;

	// buffer size is smaller than one DWORD pixel
	if(org_size < 4) return false;

	// number of pixel from buffer size
	n_pix = org_size/4;
	if(org_size%4 != 0) return false; // buffer size is not even with DWORD pixel structure. buffer is incorrect

	c_run=0;
	ptr1 = (DWORD *) &org[(i*4)];
	// n_pix-1 indicates that one pixel or last pixel in pixel stream will not be addressed during while loop
	while(i<n_pix-1)
	{
		// next pixel
		ptr2 = (DWORD *) &org[(i*4)+4];
		// compare
		if((*ptr1 == *ptr2) && c_run < 255){    
			//256-1
			// current and next pixel value matches
			// increment the run counter
			c_run++;
		}else{
			// negative compression or same as original uncompressed pixel stream
			if(s_cmp+5 >= org_size) return false;
			// record run_count in buffer
			cmp[s_cmp] = c_run;
			// record pixel
			ptr3 = (BYTE *) ptr1;
			cmp[s_cmp+1] = ptr3[0];
			cmp[s_cmp+2] = ptr3[1];
			cmp[s_cmp+3] = ptr3[2];
			cmp[s_cmp+4] = ptr3[3];
			// reset ptr1
			ptr1 = ptr2;
			// reset run counter
			c_run = 0;
			// calculate the size of cmp
			s_cmp+=5;
		}
		i++;
	}

	//one pixel or last pixel in pixel stream
	if(s_cmp+5 >= org_size) return false;
	// record run_count in buffer
	cmp[s_cmp] = c_run;
	// record pixel
	ptr3 = (BYTE *) ptr1;
	cmp[s_cmp+1] = ptr3[0];
	cmp[s_cmp+2] = ptr3[1];
	cmp[s_cmp+3] = ptr3[2];
	cmp[s_cmp+4] = ptr3[3];
	s_cmp+=5;

	*cmp_size = s_cmp; 
	
	return true;
}

bool RSCRLE::RSCDecompress32(BYTE *cmp, BYTE *org, ULONG cmp_size)
{
	ULONG org_ctr=0;
	// number of run block
	ULONG n_run = cmp_size/5;
	if(cmp_size%5 != 0) return false; // buffer size can not be evenly divided by number of run blocks. buffer is incorrect

	for(ULONG i=0; i<n_run; i++){
		for(int j=0; j<((cmp[i*5])+1);j++){
			org[org_ctr]=cmp[i*5+1];
			org_ctr++;
			org[org_ctr]=cmp[i*5+2];
			org_ctr++;
			org[org_ctr]=cmp[i*5+3];
			org_ctr++;
			org[org_ctr]=cmp[i*5+4];
			org_ctr++;
		}
	}

	return true;
}

bool RSCRLE::RSCCompress16(BYTE *org, BYTE *cmp, ULONG org_size, ULONG *cmp_size)
{
	DWORD *ptr1, *ptr2;
	BYTE *ptr3;
	ULONG n_pix=0;
	ULONG i=0;
	BYTE c_run; // maxium c_run = 256 
	ULONG s_cmp=0;

	// buffer size is smaller than one WORD pixel
	if(org_size < 2) return false;

	// number of pixel from buffer size
	n_pix = org_size/2;
	if(org_size%2 != 0) return false; // buffer size is not even with WORD pixel structure. buffer is incorrect

	c_run=0;
	ptr1 = (DWORD *) &org[(i*2)];
	// n_pix-1 indicates that one pixel or last pixel in pixel stream will not be addressed during while loop
	while(i<n_pix-1)
	{
		// next pixel
		ptr2 = (DWORD *) &org[(i*2)+2];
		// compare
		if((*ptr1 == *ptr2) && c_run < 255){    //256-1
			// current and next pixel value matches
			// increment the run counter
			c_run++;
		}else{
			// negative compression or same as original uncompressed pixel stream
			if(s_cmp+3 >= org_size) return false;
			// record run_count in buffer
			cmp[s_cmp] = c_run;
			// record pixel
			ptr3 = (BYTE *) ptr1;
			cmp[s_cmp+1] = ptr3[0];
			cmp[s_cmp+2] = ptr3[1];
			// reset ptr1
			ptr1 = ptr2;
			// reset run counter
			c_run = 0;
			// calculate the size of cmp
			s_cmp+=3;
		}
		i++;
	}

	//one pixel or last pixel in pixel stream
	if(s_cmp+3 >= org_size) return false;
	// record run_count in buffer
	cmp[s_cmp] = c_run;
	// record pixel
	ptr3 = (BYTE *) ptr1;
	cmp[s_cmp+1] = ptr3[0];
	cmp[s_cmp+2] = ptr3[1];
	s_cmp+=3;

	*cmp_size = s_cmp; 
	
	return true;
}

bool RSCRLE::RSCDecompress16(BYTE *cmp, BYTE *org, ULONG cmp_size)
{
	ULONG org_ctr=0;
	// number of run block
	ULONG n_run = cmp_size/3;
	if(cmp_size%3 != 0) return false; // buffer size can not be evenly divided by number of run blocks. buffer is incorrect

	for(ULONG i=0; i<n_run; i++){
		for(int j=0; j<((cmp[i*3])+1);j++){
			org[org_ctr]=cmp[i*3+1];
			org_ctr++;
			org[org_ctr]=cmp[i*3+2];
			org_ctr++;
		}
	}

	return true;
}

bool RSCRLE::RSCCompress8(BYTE *org, BYTE *cmp, ULONG org_size, ULONG *cmp_size)
{
	BYTE *ptr1, *ptr2;
	BYTE *ptr3;
	ULONG i=0;
	BYTE c_run; // maxium c_run = 256 
	ULONG s_cmp=0;

	// buffer size is smaller than one BYTE pixel
	if(org_size < 1) return false;

	c_run=0;
	ptr1 = (BYTE *) &org[i];
	// org_size-1 indicates that one pixel or last pixel in pixel stream will not be addressed during while loop
	while(i<org_size-1)
	{
		// next pixel
		ptr2 = (BYTE *) &org[i+1];
		// compare
		if((*ptr1 == *ptr2) && c_run < 255){    //256-1
			// current and next pixel value matches
			// increment the run counter
			c_run++;
		}else{
			// negative compression or same as original uncompressed pixel stream
			if(s_cmp+2 >= org_size) return false;
			// record run_count in buffer
			cmp[s_cmp] = c_run;
			// record pixel
			ptr3 = (BYTE *) ptr1;
			cmp[s_cmp+1] = ptr3[0];
			// reset ptr1
			ptr1 = ptr2;
			// reset run counter
			c_run = 0;
			// calculate the size of cmp
			s_cmp+=2;
		}
		i++;
	}

	//one pixel or last pixel in pixel stream
	if(s_cmp+2 >= org_size) return false;
	// record run_count in buffer
	cmp[s_cmp] = c_run;
	// record pixel
	ptr3 = (BYTE *) ptr1;
	cmp[s_cmp+1] = ptr3[0];
	s_cmp+=2;

	*cmp_size = s_cmp; 
	
	return true;
}

bool RSCRLE::RSCDecompress8(BYTE *cmp, BYTE *org, ULONG cmp_size)
{
	ULONG org_ctr=0;
	// number of run block
	ULONG n_run = cmp_size/2;
	if(cmp_size%2 != 0) return false; // buffer size can not be evenly divided by number of run blocks. buffer is incorrect

	for(ULONG i=0; i<n_run; i++){
		for(int j=0; j<((cmp[i*2])+1);j++){
			org[org_ctr]=cmp[i*2+1];
			org_ctr++;
		}
	}

	return true;
}

ULONG RSCRLE::RSCGetBufferSize4Decompress(BYTE *cmp, ULONG cmp_size, ULONG run_length_in_byte)
// The buffer size can not be evenly divided by number of run blocks. 
// In this case, the function returns -1.
{
	ULONG size=0;
	ULONG n_run = cmp_size/run_length_in_byte;
	if(cmp_size%run_length_in_byte != 0) return -1;// buffer size can not be evenly divided by number of run blocks. buffer is incorrect
	
	for(ULONG i=0; i<n_run; i++){
		size = size + (cmp[i*run_length_in_byte]+1);
	}

	return size*(run_length_in_byte-1);

}
