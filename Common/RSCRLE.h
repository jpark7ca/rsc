// 
// RSC Run-Length Enconding Class (RSCRLE.h) 
// By Jae H. Park 
//
// Created: 02.01.2005
//
// RSCRLE class compresses and decompresses the PIXEL data(8-bit, 16-bit and 32-bit)
// of the screen framebuffer using RLE algorithm in order to tranfer (nearly real time) massive image data
// over the network.
//
// Note: Maximum run is 256 (1 byte)
// 

#pragma once

class RSCRLE
{
public:
	RSCRLE(void);
	~RSCRLE(void);

	bool RSCCompress32(BYTE *org, BYTE *cmp, ULONG org_size, ULONG *cmp_size);
	bool RSCDecompress32(BYTE *cmp, BYTE *org, ULONG cmp_size);
	bool RSCCompress16(BYTE *org, BYTE *cmp, ULONG org_size, ULONG *cmp_size);
	bool RSCDecompress16(BYTE *cmp, BYTE *org, ULONG cmp_size);
	bool RSCCompress8(BYTE *org, BYTE *cmp, ULONG org_size, ULONG *cmp_size);
	bool RSCDecompress8(BYTE *cmp, BYTE *org, ULONG cmp_size);

	ULONG RSCGetBufferSize4Decompress(BYTE *cmp, ULONG cmp_size, ULONG run_length_in_byte);
	// i.e., 4-bytes pixel = 5 bytes run length 
};