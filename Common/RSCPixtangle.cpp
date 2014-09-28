// 
// RSC Pixtangle Class (RSCPixtangle.cpp) 
// By Jae H. Park 
//
// Created: 01.31.2005
// Revised: 06.11.2006, 07.07.2006
//
// RSCPixtangle class handles sub-regions of pre-determined size called pixtangle in screen framebuffer 
// for pixtangle comparison and manipulation
//
//

#include "stdafx.h"

RSCPixtangle::RSCPixtangle(void)
{
	m_pos = NULL;
	m_pos_ctr = 0;
	m_pixtangle_w = 0;
	m_pixtangle_h = 0;
	m_last_pixtangle_w = 0;
	m_last_pixtangle_h = 0;
	m_n_pixtangle_w = 0;
	m_n_pixtangle_h = 0;
	m_screen_w = 0;
	m_screen_h = 0;
	m_Bps = 0;
	m_Bpp = 0;

}
	
RSCPixtangle::RSCPixtangle(LONG screen_w, LONG screen_h, int Bps, int Bpp, int pixtangle_w, int pixtangle_h)
// Bps = Byte(s) per scanline, Bpp = Byte(s) per pixel
{
	m_pixtangle_w = pixtangle_w;
	m_pixtangle_h = pixtangle_h;
	m_screen_w = screen_w;
	m_screen_h = screen_h;
	m_Bps = Bps;
	m_Bpp = Bpp;

	// Calculate whether the size of pixtangle is the same for all pixtangles in screen buffer
	m_n_pixtangle_w = (m_screen_w / m_pixtangle_w);
	m_last_pixtangle_w = (m_screen_w - m_pixtangle_w * m_n_pixtangle_w);
	if(m_last_pixtangle_w > 0)
		m_n_pixtangle_w = m_n_pixtangle_w + 1;

	m_n_pixtangle_h = (m_screen_h / m_pixtangle_h);
	m_last_pixtangle_h = (m_screen_h - m_pixtangle_h * m_n_pixtangle_h);
	if(m_last_pixtangle_h > 0)
		m_n_pixtangle_h = m_n_pixtangle_h + 1;	

	m_pos = new RSC_PIXTANGLE_POS[m_n_pixtangle_w*m_n_pixtangle_h];
	m_pos_ctr = 0;
}

RSCPixtangle::RSCPixtangle(LONG screen_w, LONG screen_h, int Bps, int Bpp, int pixtangle_w, int pixtangle_h, bool posarray)
// Bps = Byte(s) per scanline, Bpp = Byte(s) per pixel
{
	m_pixtangle_w = pixtangle_w;
	m_pixtangle_h = pixtangle_h;
	m_screen_w = screen_w;
	m_screen_h = screen_h;
	m_Bps = Bps;
	m_Bpp = Bpp;

	// Calculate whether the size of pixtangle is the same for all pixtangles in screen buffer
	m_n_pixtangle_w = (m_screen_w / m_pixtangle_w);
	m_last_pixtangle_w = (m_screen_w - m_pixtangle_w * m_n_pixtangle_w);
	if(m_last_pixtangle_w > 0)
		m_n_pixtangle_w = m_n_pixtangle_w + 1;

	m_n_pixtangle_h = (m_screen_h / m_pixtangle_h);
	m_last_pixtangle_h = (m_screen_h - m_pixtangle_h * m_n_pixtangle_h);
	if(m_last_pixtangle_h > 0)
		m_n_pixtangle_h = m_n_pixtangle_h + 1;	

	if(posarray){
		m_pos = new RSC_PIXTANGLE_POS[m_n_pixtangle_w*m_n_pixtangle_h];
		m_pos_ctr = 0;
	}else{
		m_pos = NULL;
		m_pos_ctr = 0;
	}
}

RSCPixtangle::~RSCPixtangle(void)
{
	if(m_pos != NULL)
		delete[] m_pos;

}

//bool RSCInitPixtangle(LONG screen_w, LONG screen_h, int Bps, int Bpp, int pixtangle_w, int pixtangle_h);
bool RSCPixtangle::RSCCreateListPos(LONG screen_w, LONG screen_h, int Bps, int Bpp, int pixtangle_w, int pixtangle_h)
{
	m_pixtangle_w = pixtangle_w;
	m_pixtangle_h = pixtangle_h;
	m_screen_w = screen_w;
	m_screen_h = screen_h;
	m_Bps = Bps;
	m_Bpp = Bpp;

	// Calculate whether the size of pixtangle is the same for all pixtangles in screen buffer
	m_n_pixtangle_w = (m_screen_w / m_pixtangle_w);
	m_last_pixtangle_w = (m_screen_w - m_pixtangle_w * m_n_pixtangle_w);
	if(m_last_pixtangle_w > 0)
		m_n_pixtangle_w = m_n_pixtangle_w + 1;

	m_n_pixtangle_h = (m_screen_h / m_pixtangle_h);
	m_last_pixtangle_h = (m_screen_h - m_pixtangle_h * m_n_pixtangle_h);
	if(m_last_pixtangle_h > 0)
		m_n_pixtangle_h = m_n_pixtangle_h + 1;	

	if(m_last_pixtangle_w < 0) return false; // Possibly, incorrect parameters
	if(m_last_pixtangle_h < 0) return false; // Possibly, incorrect parameters 

	m_pos = new RSC_PIXTANGLE_POS[m_n_pixtangle_w*m_n_pixtangle_h];
	if(m_pos == NULL)
		return false;

	m_pos_ctr = 0;

	return true;
}

bool RSCPixtangle::RSCDestroyListPos(void)
{
	if(m_pos != NULL)
		delete[] m_pos;
	else
		return false;
	
	return true;
}

bool RSCPixtangle::RSCComparePixtangle32(BYTE *prev, BYTE *next, int sx, int sy, int sw, int sh)
{
	BYTE *ptr1, *ptr2;
	int nsx = m_Bpp*sx;
	int size = sw*sizeof(DWORD);
	
	
	for(int y=sy; y < sy+sh; y++){

		ptr1 = &prev[nsx+(m_Bps*((m_screen_h-1)-y))];
		ptr2 = &next[nsx+(m_Bps*((m_screen_h-1)-y))];

		if(memcmp(ptr1, ptr2, size) != 0){
			return false;
		}

	}
	return true;

}

bool RSCPixtangle::RSCComparePixtangle32(BYTE *prev, BYTE *next, int px, int py, int w, int h, int nx, int ny)
{
	BYTE *ptr1, *ptr2;
	int npx = m_Bpp*px;
	int nnx = m_Bpp*nx;
	int size = w*sizeof(DWORD);
	
	
	for(int y=0; y < h; y++){

		ptr1 = &prev[npx+(m_Bps*((m_screen_h-1)-(py+y)))];
		ptr2 = &next[nnx+(m_Bps*((m_screen_h-1)-(ny+y)))];

		if(memcmp(ptr1, ptr2, size) != 0){
			return false;
		}

	}
	return true;

}


bool RSCPixtangle::RSCGetListPixtangle32(BYTE *prev, BYTE *next)
{
	// Find a different Pixtangle and add it to the data structure	
	//
	int x, y, tx, ty, ctr, tctr;
	bool rle;

	if(m_pos_ctr != 0) return false; // you need to destroy old pos and create new pos structure
	if(m_pos == NULL) return false; // you need to create new pos structure
	
	m_pos_ctr = 0;

	ctr = 0;
	for(y = 0; y < m_n_pixtangle_h-1; y++){
		for(x = 0; x < m_n_pixtangle_w-1; x++){
			
			//if(!RSCComparePixtangle32(prev, next, x*m_pixtangle_w, y*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h)){
			if(!RSCComparePixtangle32(prev, next, x*m_pixtangle_w, y*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, x*m_pixtangle_w, y*m_pixtangle_h)){
				
				rle = true;
				tctr = 0;
				for(ty = 0; ty < m_n_pixtangle_h-1; ty++){
					for(tx = 0; tx < m_n_pixtangle_w-1; tx++){
						
						if(tctr < ctr){
							if(RSCComparePixtangle32(next, next, tx*m_pixtangle_w, ty*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, x*m_pixtangle_w, y*m_pixtangle_h)){
								// CopyRect Encoding
								m_pos[m_pos_ctr].type = 3;
								m_pos[m_pos_ctr].x = x*m_pixtangle_w;
								m_pos[m_pos_ctr].y = y*m_pixtangle_h;
								m_pos[m_pos_ctr].w = m_pixtangle_w;
								m_pos[m_pos_ctr].h = m_pixtangle_h;
								m_pos[m_pos_ctr].sx = tx*m_pixtangle_w;
								m_pos[m_pos_ctr].sy = ty*m_pixtangle_h;
								m_pos_ctr++;

								tctr = ctr;
								tx=(m_n_pixtangle_w-1);
								ty=m_n_pixtangle_h;

								rle = false;
							}

							tctr++;

						}else{
							//tctr = ctr;
							tx=(m_n_pixtangle_w-1);
							ty=m_n_pixtangle_h-1;
							//rle = true;
						}

					}
				}
			
				if(rle){
					// RLE Encoding
					m_pos[m_pos_ctr].type = 1;
					m_pos[m_pos_ctr].x = x*m_pixtangle_w;
					m_pos[m_pos_ctr].y = y*m_pixtangle_h;
					m_pos[m_pos_ctr].w = m_pixtangle_w;
					m_pos[m_pos_ctr].h = m_pixtangle_h;
					m_pos_ctr++;
				}

			}
		
			ctr++;
		}
	}
	// Last 2 vertical and horizontal lines of pixtangles
	//
	int x1 = (m_n_pixtangle_w-1)*m_pixtangle_w;
	if(m_last_pixtangle_w > 0){
		// last pixtangle's width is different from other pixtanles.
		for(int y = 0; y < m_n_pixtangle_h-1; y++){
			//if(!RSCComparePixtangle32(prev, next, x1, y*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h)){
			if(!RSCComparePixtangle32(prev, next, x1, y*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h, x1, y*m_pixtangle_h)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x1;
				m_pos[m_pos_ctr].y = y*m_pixtangle_h;
				m_pos[m_pos_ctr].w = m_last_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}else{
		for(int y = 0; y < m_n_pixtangle_h-1; y++){
			//if(!RSCComparePixtangle32(prev, next, x1, y*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h)){
			if(!RSCComparePixtangle32(prev, next, x1, y*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, x1, y*m_pixtangle_h)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x1;
				m_pos[m_pos_ctr].y = y*m_pixtangle_h;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}
	
	int y1 = (m_n_pixtangle_h-1)*m_pixtangle_h;
	if(m_last_pixtangle_h > 0){
		for(int x = 0; x < m_n_pixtangle_w-1; x++){
			//if(!RSCComparePixtangle32(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_last_pixtangle_h)){
			if(!RSCComparePixtangle32(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_last_pixtangle_h, x*m_pixtangle_w, y1)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x*m_pixtangle_w;
				m_pos[m_pos_ctr].y = y1;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_last_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}else{
		for(int x = 0; x < m_n_pixtangle_w-1; x++){
			//if(!RSCComparePixtangle32(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_pixtangle_h)){
			if(!RSCComparePixtangle32(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_pixtangle_h, x*m_pixtangle_w, y1)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x*m_pixtangle_w;
				m_pos[m_pos_ctr].y = y1;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}

	// Last pixtangle (right and bottom most pixtangle)
	if(m_last_pixtangle_w > 0 && m_last_pixtangle_h > 0){
		//if(!RSCComparePixtangle32(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_last_pixtangle_h)){				
		if(!RSCComparePixtangle32(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_last_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){				
			
			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_last_pixtangle_w;
			m_pos[m_pos_ctr].h = m_last_pixtangle_h;
			m_pos_ctr++;
		}
	}else if(m_last_pixtangle_w > 0){
		// m_last_pixtangle_h == 0 is implied.
		//if(!RSCComparePixtangle32(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h)){
		if(!RSCComparePixtangle32(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){
			
			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_last_pixtangle_w;
			m_pos[m_pos_ctr].h = m_pixtangle_h;
			m_pos_ctr++;
		}
	}else if(m_last_pixtangle_h > 0){
		// m_last_pixtangle_w == 0 is implied.
		//if(!RSCComparePixtangle32(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_last_pixtangle_h)){
		if(!RSCComparePixtangle32(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_last_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){
			
			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_pixtangle_w;
			m_pos[m_pos_ctr].h = m_last_pixtangle_h;
			m_pos_ctr++;
		}
	}else{
		// m_last_pixtangle_w == 0 && m_last_pixtangle_h == 0
		//if(!RSCComparePixtangle32(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h)){
		if(!RSCComparePixtangle32(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){
			
			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_pixtangle_w;
			m_pos[m_pos_ctr].h = m_pixtangle_h;
			m_pos_ctr++;
		}
	}

	return true;
}


/*
bool RSCPixtangle::RSCGetListPixtangle32(BYTE *prev, BYTE *next, LONG region_x, LONG region_y)
{
	// Find a different Pixtangle and add it to the data structure	
	//
	if(m_pos_ctr != 0) return false; // you need to destroy old pos and create new pos structure
	if(m_pos == NULL) return false; // you need to create new pos structure
	
	m_pos_ctr = 0;
	for(int y = 0; y < m_n_pixtangle_h-1; y++)
		for(int x = 0; x < m_n_pixtangle_w-1; x++){
			
			if(!RSCComparePixtangle32(prev, next, (x*m_pixtangle_w)+region_x, (y*m_pixtangle_h)+region_y, m_pixtangle_w, m_pixtangle_h)){
				// Add the pixtangle to ...

				m_pos[m_pos_ctr].x = (x*m_pixtangle_w)+region_x;
				m_pos[m_pos_ctr].y = (y*m_pixtangle_h)+region_y;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}

	// Last 2 vertical and horizontal lines of pixtangles
	//
	int x1 = (m_n_pixtangle_w-1)*m_pixtangle_w;
	if(m_last_pixtangle_w > 0){
		// last pixtangle's width is different from other pixtanles.
		for(int y = 0; y < m_n_pixtangle_h-1; y++){
			if(!RSCComparePixtangle32(prev, next, x1+region_x, (y*m_pixtangle_h)+region_y, m_last_pixtangle_w, m_pixtangle_h)){
				// Add the pixtangle to ...
				m_pos[m_pos_ctr].x = x1+region_x;
				m_pos[m_pos_ctr].y = (y*m_pixtangle_h)+region_y;
				m_pos[m_pos_ctr].w = m_last_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}else{
		for(int y = 0; y < m_n_pixtangle_h-1; y++){
			if(!RSCComparePixtangle32(prev, next, x1+region_x, (y*m_pixtangle_h)+region_y, m_pixtangle_w, m_pixtangle_h)){
				// Add the pixtangle to ...
				m_pos[m_pos_ctr].x = x1+region_x;
				m_pos[m_pos_ctr].y = (y*m_pixtangle_h)+region_y;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}
	
	int y1 = (m_n_pixtangle_h-1)*m_pixtangle_h;
	if(m_last_pixtangle_h > 0){
		for(int x = 0; x < m_n_pixtangle_w-1; x++){
			if(!RSCComparePixtangle32(prev, next, (x*m_pixtangle_w)+region_x, y1+region_y, m_pixtangle_w , m_last_pixtangle_h)){
				// Add the pixtangle to ...
				m_pos[m_pos_ctr].x = (x*m_pixtangle_w)+region_x;
				m_pos[m_pos_ctr].y = y1+region_y;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_last_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}else{
		for(int x = 0; x < m_n_pixtangle_w-1; x++){
			if(!RSCComparePixtangle32(prev, next, (x*m_pixtangle_w)+region_x, y1+region_y, m_pixtangle_w , m_pixtangle_h)){
				// Add the pixtangle to ...
				m_pos[m_pos_ctr].x = (x*m_pixtangle_w)+region_x;
				m_pos[m_pos_ctr].y = y1+region_y;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}

	// Last pixtangle (right and bottom most pixtangle)
	if(m_last_pixtangle_w > 0 && m_last_pixtangle_h > 0){
		if(!RSCComparePixtangle32(prev, next, ((m_n_pixtangle_w-1)*m_pixtangle_w)+region_x, ((m_n_pixtangle_h-1)*m_pixtangle_h)+region_y, m_last_pixtangle_w, m_last_pixtangle_h)){				
			m_pos[m_pos_ctr].x = ((m_n_pixtangle_w-1)*m_pixtangle_w)+region_x;
			m_pos[m_pos_ctr].y = ((m_n_pixtangle_h-1)*m_pixtangle_h)+region_y;
			m_pos[m_pos_ctr].w = m_last_pixtangle_w;
			m_pos[m_pos_ctr].h = m_last_pixtangle_h;
			m_pos_ctr++;
		}
	}else if(m_last_pixtangle_w > 0){
		// m_last_pixtangle_h == 0 is implied.
		if(!RSCComparePixtangle32(prev, next, ((m_n_pixtangle_w-1)*m_pixtangle_w)+region_x, ((m_n_pixtangle_h-1)*m_pixtangle_h)+region_y, m_last_pixtangle_w, m_pixtangle_h)){
			m_pos[m_pos_ctr].x = ((m_n_pixtangle_w-1)*m_pixtangle_w)+region_x;
			m_pos[m_pos_ctr].y = ((m_n_pixtangle_h-1)*m_pixtangle_h)+region_y;
			m_pos[m_pos_ctr].w = m_last_pixtangle_w;
			m_pos[m_pos_ctr].h = m_pixtangle_h;
			m_pos_ctr++;
		}
	}else if(m_last_pixtangle_h > 0){
		// m_last_pixtangle_w == 0 is implied.
		if(!RSCComparePixtangle32(prev, next, ((m_n_pixtangle_w-1)*m_pixtangle_w)+region_x, ((m_n_pixtangle_h-1)*m_pixtangle_h)+region_y, m_pixtangle_w, m_last_pixtangle_h)){
			m_pos[m_pos_ctr].x = ((m_n_pixtangle_w-1)*m_pixtangle_w)+region_x;
			m_pos[m_pos_ctr].y = ((m_n_pixtangle_h-1)*m_pixtangle_h)+region_y;
			m_pos[m_pos_ctr].w = m_pixtangle_w;
			m_pos[m_pos_ctr].h = m_last_pixtangle_h;
			m_pos_ctr++;
		}
	}else{
		// m_last_pixtangle_w == 0 && m_last_pixtangle_h == 0
		if(!RSCComparePixtangle32(prev, next, ((m_n_pixtangle_w-1)*m_pixtangle_w)+region_x, ((m_n_pixtangle_h-1)*m_pixtangle_h)+region_y, m_pixtangle_w, m_pixtangle_h)){
			m_pos[m_pos_ctr].x = ((m_n_pixtangle_w-1)*m_pixtangle_w)+region_x;
			m_pos[m_pos_ctr].y = ((m_n_pixtangle_h-1)*m_pixtangle_h)+region_y;
			m_pos[m_pos_ctr].w = m_pixtangle_w;
			m_pos[m_pos_ctr].h = m_pixtangle_h;
			m_pos_ctr++;
		}
	}

	return true;
}
*/

bool RSCPixtangle::RSCComparePixtangle16(BYTE *prev, BYTE *next, int sx, int sy, int sw, int sh)
{
	BYTE *ptr1, *ptr2;
	int nsx = m_Bpp*sx;
	int size = sw*sizeof(WORD);

	for(int y=sy; y < sy+sh; y++){
		
		ptr1 = &prev[nsx+(m_Bps*((m_screen_h-1)-y))];
		ptr2 = &next[nsx+(m_Bps*((m_screen_h-1)-y))];

		if(memcmp(ptr1, ptr2, size) != 0){
			return false;
		}	
	}

	return true;
}

bool RSCPixtangle::RSCComparePixtangle16(BYTE *prev, BYTE *next, int px, int py, int w, int h, int nx, int ny)
{
	BYTE *ptr1, *ptr2;
	int npx = m_Bpp*px;
	int nnx = m_Bpp*nx;
	int size = w*sizeof(WORD);

	for(int y=0; y < h; y++){
		
		ptr1 = &prev[npx+(m_Bps*((m_screen_h-1)-(py+y)))];
		ptr2 = &next[nnx+(m_Bps*((m_screen_h-1)-(ny+y)))];

		if(memcmp(ptr1, ptr2, size) != 0){
			return false;
		}	
	}

	return true;
}

bool RSCPixtangle::RSCGetListPixtangle16(BYTE *prev, BYTE *next)
{
	// Find a different Pixtangle and add it to the data structure	
	//
	int x, y, tx, ty, ctr, tctr;
	bool rle;

	if(m_pos_ctr != 0) return false; // you need to destroy old pos and create new pos structure
	if(m_pos == NULL) return false; // you need to create new pos structure
	
	m_pos_ctr = 0;

	ctr = 0;
	for(y = 0; y < m_n_pixtangle_h-1; y++){
		for(x = 0; x < m_n_pixtangle_w-1; x++){
			
			if(!RSCComparePixtangle16(prev, next, x*m_pixtangle_w, y*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, x*m_pixtangle_w, y*m_pixtangle_h)){

				rle = true;
				tctr = 0;
				for(ty = 0; ty < m_n_pixtangle_h-1; ty++){
					for(tx = 0; tx < m_n_pixtangle_w-1; tx++){
						
						if(tctr < ctr){
							if(RSCComparePixtangle16(next, next, tx*m_pixtangle_w, ty*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, x*m_pixtangle_w, y*m_pixtangle_h)){
								// CopyRect Encoding
								m_pos[m_pos_ctr].type = 3;
								m_pos[m_pos_ctr].x = x*m_pixtangle_w;
								m_pos[m_pos_ctr].y = y*m_pixtangle_h;
								m_pos[m_pos_ctr].w = m_pixtangle_w;
								m_pos[m_pos_ctr].h = m_pixtangle_h;
								m_pos[m_pos_ctr].sx = tx*m_pixtangle_w;
								m_pos[m_pos_ctr].sy = ty*m_pixtangle_h;
								m_pos_ctr++;

								tctr = ctr;
								tx=(m_n_pixtangle_w-1);
								ty=m_n_pixtangle_h;

								rle = false;
							}

							tctr++;

						}else{
							//tctr = ctr;
							tx=(m_n_pixtangle_w-1);
							ty=m_n_pixtangle_h-1;
							//rle = true;
						}

					}
				}
			
				if(rle){
					// RLE Encoding
					m_pos[m_pos_ctr].type = 1;
					m_pos[m_pos_ctr].x = x*m_pixtangle_w;
					m_pos[m_pos_ctr].y = y*m_pixtangle_h;
					m_pos[m_pos_ctr].w = m_pixtangle_w;
					m_pos[m_pos_ctr].h = m_pixtangle_h;
					m_pos_ctr++;
				}
			}						

		
			ctr++;
		}
	}
	// Last 2 vertical and horizontal lines of pixtangles
	//
	int x1 = (m_n_pixtangle_w-1)*m_pixtangle_w;
	if(m_last_pixtangle_w > 0){
		// last pixtangle's width is different from other pixtanles.
		for(int y = 0; y < m_n_pixtangle_h-1; y++){
			//if(!RSCComparePixtangle16(prev, next, x1, y*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h)){
			if(!RSCComparePixtangle16(prev, next, x1, y*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h, x1, y*m_pixtangle_h)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x1;
				m_pos[m_pos_ctr].y = y*m_pixtangle_h;
				m_pos[m_pos_ctr].w = m_last_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}else{
		for(int y = 0; y < m_n_pixtangle_h-1; y++){
			//if(!RSCComparePixtangle16(prev, next, x1, y*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h)){
			if(!RSCComparePixtangle16(prev, next, x1, y*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, x1, y*m_pixtangle_h)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x1;
				m_pos[m_pos_ctr].y = y*m_pixtangle_h;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}
	
	int y1 = (m_n_pixtangle_h-1)*m_pixtangle_h;
	if(m_last_pixtangle_h > 0){
		for(int x = 0; x < m_n_pixtangle_w-1; x++){
			//if(!RSCComparePixtangle16(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_last_pixtangle_h)){
			if(!RSCComparePixtangle16(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_last_pixtangle_h, x*m_pixtangle_w, y1)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x*m_pixtangle_w;
				m_pos[m_pos_ctr].y = y1;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_last_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}else{
		for(int x = 0; x < m_n_pixtangle_w-1; x++){
			//if(!RSCComparePixtangle16(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_pixtangle_h)){
			if(!RSCComparePixtangle16(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_pixtangle_h, x*m_pixtangle_w, y1)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x*m_pixtangle_w;
				m_pos[m_pos_ctr].y = y1;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;

			}
		}
	}


	// Last pixtangle (right and bottom most pixtangle)
	if(m_last_pixtangle_w > 0 && m_last_pixtangle_h > 0){
		//if(!RSCComparePixtangle16(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_last_pixtangle_h)){
		if(!RSCComparePixtangle16(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_last_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){
			
			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;	
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_last_pixtangle_w;
			m_pos[m_pos_ctr].h = m_last_pixtangle_h;
			m_pos_ctr++;

		}
	}else if(m_last_pixtangle_w > 0){
		// m_last_pixtangle_h == 0 is implied.
		//if(!RSCComparePixtangle16(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h)){
		if(!RSCComparePixtangle16(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){
			
			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_last_pixtangle_w;
			m_pos[m_pos_ctr].h = m_pixtangle_h;
			m_pos_ctr++;

		}

	}else if(m_last_pixtangle_h > 0){
		// m_last_pixtangle_w == 0 is implied.
		//if(!RSCComparePixtangle16(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_last_pixtangle_h)){
		if(!RSCComparePixtangle16(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_last_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){

			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_pixtangle_w;
			m_pos[m_pos_ctr].h = m_last_pixtangle_h;
			m_pos_ctr++;

		}
	}else{
		// m_last_pixtangle_w == 0 && m_last_pixtangle_h == 0
		//if(!RSCComparePixtangle16(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h)){
		if(!RSCComparePixtangle16(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){

			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_pixtangle_w;
			m_pos[m_pos_ctr].h = m_pixtangle_h;
			m_pos_ctr++;

		}
	}

	return true;
}

bool RSCPixtangle::RSCComparePixtangle8(BYTE *prev, BYTE *next, int sx, int sy, int sw, int sh)
{
	BYTE *ptr1, *ptr2;
	int size = sw*sizeof(BYTE);

	for(int y=sy; y < sy+sh; y++){

		ptr1 = &prev[sx+(m_Bps*((m_screen_h-1)-y))];
		ptr2 = &next[sx+(m_Bps*((m_screen_h-1)-y))];

		if(memcmp(ptr1, ptr2, size) != 0){
			return false;
		}	
	}

	return true;
}

bool RSCPixtangle::RSCComparePixtangle8(BYTE *prev, BYTE *next, int px, int py, int w, int h, int nx, int ny)
{
	BYTE *ptr1, *ptr2;
	int size = w*sizeof(BYTE);

	for(int y=0; y < h; y++){

		ptr1 = &prev[px+(m_Bps*((m_screen_h-1)-(py+y)))];
		ptr2 = &next[nx+(m_Bps*((m_screen_h-1)-(ny+y)))];

		if(memcmp(ptr1, ptr2, size) != 0){
			return false;
		}	
	}

	return true;
}

bool RSCPixtangle::RSCGetListPixtangle8(BYTE *prev, BYTE *next)
{
	// Find a different Pixtangle and add it to the data structure	
	//
	int x, y, tx, ty, ctr, tctr;
	bool rle;

	if(m_pos_ctr != 0) return false; // you need to destroy old pos and create new pos structure
	if(m_pos == NULL) return false; // you need to create new pos structure
	
	m_pos_ctr = 0;
	
	ctr = 0;
	for(y = 0; y < m_n_pixtangle_h-1; y++){
		for(x = 0; x < m_n_pixtangle_w-1; x++){
			
			if(!RSCComparePixtangle8(prev, next, x*m_pixtangle_w, y*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, x*m_pixtangle_w, y*m_pixtangle_h)){
				
				rle = true;
				tctr = 0;
				for(ty = 0; ty < m_n_pixtangle_h-1; ty++){
					for(tx = 0; tx < m_n_pixtangle_w-1; tx++){
						
						if(tctr < ctr){
							
							if(RSCComparePixtangle8(next, next, tx*m_pixtangle_w, ty*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, x*m_pixtangle_w, y*m_pixtangle_h)){
								// CopyRect Encoding
								m_pos[m_pos_ctr].type = 3;
								m_pos[m_pos_ctr].x = x*m_pixtangle_w;
								m_pos[m_pos_ctr].y = y*m_pixtangle_h;
								m_pos[m_pos_ctr].w = m_pixtangle_w;
								m_pos[m_pos_ctr].h = m_pixtangle_h;
								m_pos[m_pos_ctr].sx = tx*m_pixtangle_w;
								m_pos[m_pos_ctr].sy = ty*m_pixtangle_h;
								m_pos_ctr++;

								tctr = ctr;
								tx=(m_n_pixtangle_w-1);
								ty=m_n_pixtangle_h;
								
								rle = false;
							}

							tctr++;

						}else{
							//tctr = ctr;
							tx=(m_n_pixtangle_w-1);
							ty=m_n_pixtangle_h-1;
							//rle = true;
						}

					}
				}
			
				if(rle){
					// RLE Encoding
					m_pos[m_pos_ctr].type = 1;
					m_pos[m_pos_ctr].x = x*m_pixtangle_w;
					m_pos[m_pos_ctr].y = y*m_pixtangle_h;
					m_pos[m_pos_ctr].w = m_pixtangle_w;
					m_pos[m_pos_ctr].h = m_pixtangle_h;
					m_pos_ctr++;
				}

			}						
		
			ctr++;
		}
	}
	// Last 2 vertical and horizontal lines of pixtangles
	//
	int x1 = (m_n_pixtangle_w-1)*m_pixtangle_w;
	if(m_last_pixtangle_w > 0){
		// last pixtangle's width is different from other pixtanles.
		for(int y = 0; y < m_n_pixtangle_h-1; y++){
			//if(!RSCComparePixtangle8(prev, next, x1, y*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h)){
			if(!RSCComparePixtangle8(prev, next, x1, y*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h, x1, y*m_pixtangle_h)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x1;
				m_pos[m_pos_ctr].y = y*m_pixtangle_h;
				m_pos[m_pos_ctr].w = m_last_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}else{
		for(int y = 0; y < m_n_pixtangle_h-1; y++){
			//if(!RSCComparePixtangle8(prev, next, x1, y*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h)){
			if(!RSCComparePixtangle8(prev, next, x1, y*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, x1, y*m_pixtangle_h)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x1;
				m_pos[m_pos_ctr].y = y*m_pixtangle_h;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}
	
	int y1 = (m_n_pixtangle_h-1)*m_pixtangle_h;
	if(m_last_pixtangle_h > 0){
		for(int x = 0; x < m_n_pixtangle_w-1; x++){
			//if(!RSCComparePixtangle8(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_last_pixtangle_h)){
			if(!RSCComparePixtangle8(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_last_pixtangle_h, x*m_pixtangle_w, y1)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x*m_pixtangle_w;
				m_pos[m_pos_ctr].y = y1;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_last_pixtangle_h;
				m_pos_ctr++;
			}
		}
	}else{
		for(int x = 0; x < m_n_pixtangle_w-1; x++){
			//if(!RSCComparePixtangle8(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_pixtangle_h)){
			if(!RSCComparePixtangle8(prev, next, x*m_pixtangle_w, y1, m_pixtangle_w , m_pixtangle_h, x*m_pixtangle_w, y1)){
				
				// RLE Encoding
				m_pos[m_pos_ctr].type = 1;
				m_pos[m_pos_ctr].x = x*m_pixtangle_w;
				m_pos[m_pos_ctr].y = y1;
				m_pos[m_pos_ctr].w = m_pixtangle_w;
				m_pos[m_pos_ctr].h = m_pixtangle_h;
				m_pos_ctr++;

			}
		}
	}


	// Last pixtangle (right and bottom most pixtangle)
	if(m_last_pixtangle_w > 0 && m_last_pixtangle_h > 0){
		//if(!RSCComparePixtangle8(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_last_pixtangle_h)){
		if(!RSCComparePixtangle8(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_last_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){

			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_last_pixtangle_w;
			m_pos[m_pos_ctr].h = m_last_pixtangle_h;
			m_pos_ctr++;

		}
	}else if(m_last_pixtangle_w > 0){
		// m_last_pixtangle_h == 0 is implied.
		//if(!RSCComparePixtangle8(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h)){
		if(!RSCComparePixtangle8(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_last_pixtangle_w, m_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){
			
			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_last_pixtangle_w;
			m_pos[m_pos_ctr].h = m_pixtangle_h;
			m_pos_ctr++;

		}

	}else if(m_last_pixtangle_h > 0){
		// m_last_pixtangle_w == 0 is implied.
		//if(!RSCComparePixtangle8(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_last_pixtangle_h)){
		if(!RSCComparePixtangle8(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_last_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){
			
			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_pixtangle_w;
			m_pos[m_pos_ctr].h = m_last_pixtangle_h;
			m_pos_ctr++;

		}
	}else{
		// m_last_pixtangle_w == 0 && m_last_pixtangle_h == 0
		//if(!RSCComparePixtangle8(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h)){
		if(!RSCComparePixtangle8(prev, next, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h, m_pixtangle_w, m_pixtangle_h, (m_n_pixtangle_w-1)*m_pixtangle_w, (m_n_pixtangle_h-1)*m_pixtangle_h)){
			
			// RLE Encoding
			m_pos[m_pos_ctr].type = 1;
			m_pos[m_pos_ctr].x = (m_n_pixtangle_w-1)*m_pixtangle_w;
			m_pos[m_pos_ctr].y = (m_n_pixtangle_h-1)*m_pixtangle_h;
			m_pos[m_pos_ctr].w = m_pixtangle_w;
			m_pos[m_pos_ctr].h = m_pixtangle_h;
			m_pos_ctr++;

		}
	}

	return true;
}

void RSCPixtangle::RSCCopyPixtangle2Buffer(BYTE *scr, BYTE *buf, int sx, int sy, int sw, int sh)
{
	int ctr = 0;

	int nsx = m_Bpp * sx;
	int nsy = m_Bps * ((m_screen_h-1)-sy);
	int nw = m_Bpp * sw;
	int nh = nsy - (m_Bps*sh);
		
	for(int ny0 = nsy; ny0 > nh; ny0-=m_Bps){
		memcpy(buf, &scr[ny0+nsx], nw);
		buf = buf + nw;
		/*
		for(int nx0 = nsx; nx0 < (nsx+nw); nx0++){
			buf[ctr] = scr[ny0+nx0];
			ctr++;
		}
		*/
	}
}

void RSCPixtangle::RSCCopyBuffer2Pixtangle(BYTE *buf, BYTE *scr, int sx, int sy, int sw, int sh)
{
	int ctr=0;

	int nsx = m_Bpp * sx;
	int nsy = m_Bps * ((m_screen_h-1)-sy);
	int nw = m_Bpp * sw;
	int nh = nsy - (m_Bps*sh);
		
	for(int ny0 = nsy; ny0 > nh; ny0-=m_Bps){
		memcpy(&scr[ny0+nsx], buf, nw);
		buf = buf + nw;
		/*
		for(int nx0 = nsx; nx0 < (nsx+nw); nx0++){
			scr[ny0+nx0] = buf[ctr];
			ctr++;
		}
		*/
	}

}

bool RSCPixtangle::RSCConvertPixtangle_32to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps32, bps8;
	DWORD *scr_tmp;
	BYTE *scr_bytetmp;
	BYTE *dst_tmp;

	bps32 = screen_w*4;
	bps8 = screen_w;
	
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
			
			scr_tmp = (DWORD *) &scr[4*x + bps32*y];
			dst_tmp = (BYTE *) &dst[x + bps8*y];
		
			*dst_tmp = 0;
			scr_bytetmp = (BYTE *) scr_tmp;
			*dst_tmp = palette->m_rgb_index[(scr_bytetmp[2]>>3)][(scr_bytetmp[1]>>3)][(scr_bytetmp[0]>>3)];
			

		}
	}
	
	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_32to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps15;
	DWORD *scr_tmp;
	WORD *dst_tmp;
	
	bps32 = screen_w*4;
	bps15 = screen_w*2;
		
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[4*x + bps32*y];
			dst_tmp = (WORD *) &dst[2*x + bps15*y];
			
			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x0000f800) >> 6); // GREEN
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x00f80000) >> 9); //RED

		}
	}

	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_32to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps16;
	DWORD *scr_tmp;
	WORD *dst_tmp;

	bps32 = screen_w*4;
	bps16 = screen_w*2;
		
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[4*x + bps32*y];
			dst_tmp = (WORD *) &dst[2*x + bps16*y];

			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x0000fc00) >> 5); // GREEN // 0x0000fc00
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x00f80000) >> 8); //RED

		}
	}

	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_32to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps24;
	DWORD *scr_tmp;
	DWORD *dst_tmp;
	
	bps32 = screen_w*4;
	bps24 = screen_w*4;
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[4*x + bps32*y];
			dst_tmp = (DWORD *) &dst[3*x + bps24*y];
		
			dst_tmp[0] = scr_tmp[0];
			dst_tmp[1] = scr_tmp[1];
			dst_tmp[2] = scr_tmp[2];

		}
	}
	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_24to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps24, bps8;
	DWORD *scr_tmp;
	BYTE *scr_bytetmp;
	BYTE *dst_tmp;

	bps24 = screen_w*4;
	bps8 = screen_w;

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
			
			scr_tmp = (DWORD *) &scr[3*x + bps24*y];
			dst_tmp = (BYTE *) &dst[x+ bps8*y];
		
			*dst_tmp = 0;
			scr_bytetmp = (BYTE *) scr_tmp;
			*dst_tmp = palette->m_rgb_index[(scr_bytetmp[2]>>3)][(scr_bytetmp[1]>>3)][(scr_bytetmp[0]>>3)];
		}
	}
	
	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_24to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps24, bps15;
	DWORD *scr_tmp;
	WORD *dst_tmp;
	
	bps24 = screen_w*4;
	bps15 = screen_w*2;
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[3*x + bps24*y];
			dst_tmp = (WORD *) &dst[2*x + bps15*y];
			
			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x0000f800) >> 6); // GREEN
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x00f80000) >> 9); //RED

		}
	}

	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_24to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps24, bps16;
	DWORD *scr_tmp;
	WORD *dst_tmp;
	
	bps24 = screen_w*4;
	bps16 = screen_w*2;
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[3*x + bps24*y];
			dst_tmp = (WORD *) &dst[2*x + bps16*y];

			*dst_tmp = 0;
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x000000f8) >> 3); // BLUE
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x0000fc00) >> 5); // GREEN // 0x0000fc00
			*dst_tmp = *dst_tmp | (WORD) ((*scr_tmp & 0x00f80000) >> 8); //RED

		}
	}

	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_24to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps24;
	DWORD *scr_tmp;
	DWORD *dst_tmp;
	
	bps32 = screen_w*4;
	bps24 = screen_w*4;
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (DWORD *) &scr[3*x + bps24*y];
			dst_tmp = (DWORD *) &dst[4*x + bps32*y];
			
			dst_tmp[0] = scr_tmp[0];
			dst_tmp[1] = scr_tmp[1];
			dst_tmp[2] = scr_tmp[2];

		}
	}

	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_16to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps16, bps8;
	DWORD *scr_tmp;
	BYTE *dst_tmp;

	bps16 = screen_w*2;
	bps8 = screen_w;

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
			
			scr_tmp = (DWORD *) &scr[2*x + bps16*y];
			dst_tmp = (BYTE *) &dst[x + bps8*y];

			*dst_tmp = 0;
			*dst_tmp = palette->m_rgb_index[(BYTE)((*scr_tmp&0xF800)>>11)][(BYTE) ((*scr_tmp&0x07E0)>>6)][(BYTE) (*scr_tmp&0x001F)];
		}
	}
	
	return true;

}

bool RSCPixtangle::RSCConvertPixtangle_16to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps15, bps16;
	WORD *scr_tmp;
	WORD *dst_tmp;
	
	bps15 = screen_w*2;
	bps16 = screen_w*2;
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + bps16*y];
			dst_tmp = (WORD *) &scr[2*x + bps15*y];
			
			*dst_tmp = ((((*scr_tmp)&0xffc0)>>1)|((*scr_tmp)&0x001f));//*scr_tmp;

		}
	}

	return true;

}

bool RSCPixtangle::RSCConvertPixtangle_16to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps24, bps16;
	WORD *scr_tmp;
	BYTE *dst_tmp;
	
	bps24 = screen_w*4;
	bps16 = screen_w*2;
	
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + bps16*y];
			dst_tmp = &dst[3*x + bps24*y];
					
			dst_tmp[0] = (BYTE) ((*scr_tmp & 0x001F) << 3); // BLUE
			dst_tmp[1] = (BYTE) ((*scr_tmp & 0x07E0) >> 3); // GREEN
			dst_tmp[2] = (BYTE) ((*scr_tmp & 0xF800) >> 8); // RED

		}
	}
	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_16to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps16;
	WORD *scr_tmp;
	BYTE *dst_tmp;
	
	bps32 = screen_w*4;
	bps16 = screen_w*2;
	
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + bps16*y];
			dst_tmp = &dst[4*x + bps32*y];
					
			dst_tmp[0] = (BYTE) ((*scr_tmp & 0x001F) << 3); // BLUE
			dst_tmp[1] = (BYTE) ((*scr_tmp & 0x07E0) >> 3); // GREEN
			dst_tmp[2] = (BYTE) ((*scr_tmp & 0xF800) >> 8); // RED
			dst_tmp[3] = 0;

		}
	}
	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_15to8(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps15, bps8;
	DWORD *scr_tmp;
	BYTE *dst_tmp;

	bps15 = screen_w*2;
	bps8 = screen_w;

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
			
			scr_tmp = (DWORD *) &scr[2*x + bps15*y];
			dst_tmp = (BYTE *) &dst[x + bps8*y];

			*dst_tmp = 0;
			*dst_tmp = palette->m_rgb_index[(BYTE)((*scr_tmp&0x7C00)>>10)][(BYTE) ((*scr_tmp&0x03E0)>>5)][(BYTE) (*scr_tmp&0x001F)];
		}
	}

	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_15to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps15, bps16;
	WORD *scr_tmp;
	WORD *dst_tmp;
	
	bps15 = screen_w*2;
	bps16 = screen_w*2;
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + bps15*y];
			dst_tmp = (WORD *) &scr[2*x + bps16*y];

			*dst_tmp = ((((*scr_tmp)&0x7fe0)<<1)|((*scr_tmp)&0x001f));//*scr_tmp;
			
		}
	}
	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_15to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps24, bps15;
	WORD *scr_tmp;
	BYTE *dst_tmp;
	
	bps24 = screen_w*4;
	bps15 = screen_w*2;
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + bps15*y];
			dst_tmp = &dst[3*x + bps24*y];
					
			dst_tmp[0] = (BYTE) ((*scr_tmp & 0x001F) << 3); // BLUE
			dst_tmp[1] = (BYTE) ((*scr_tmp & 0x03E0) >> 2); // GREEN
			dst_tmp[2] = (BYTE) ((*scr_tmp & 0x7C00) >> 7); // RED
		}
	}
	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_15to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h)
{
	LONG bps32, bps15;
	WORD *scr_tmp;
	BYTE *dst_tmp;
	
	bps32 = screen_w*4;
	bps15 = screen_w*2;
			
	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){

			scr_tmp = (WORD *) &scr[2*x + bps15*y];
			dst_tmp = &dst[4*x + bps32*y];
					
			dst_tmp[0] = (BYTE) ((*scr_tmp & 0x001F) << 3); // BLUE
			dst_tmp[1] = (BYTE) ((*scr_tmp & 0x03E0) >> 2); // GREEN
			dst_tmp[2] = (BYTE) ((*scr_tmp & 0x7C00) >> 7); // RED
			dst_tmp[3] = 0;

		}
	}
	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_8to16(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps16, bps8;
	BYTE scr_tmp;
	WORD *dst_tmp;
	
	BYTE scr_tmp1[4];
	DWORD *scr_tmp2;

	bps16 = screen_w*2;
	bps8 = screen_w;

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + bps8*y];
			dst_tmp = (WORD *) &dst[2*x + bps16*y];
			
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

bool RSCPixtangle::RSCConvertPixtangle_8to15(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps15, bps8;
	BYTE scr_tmp;
	WORD *dst_tmp;
	
	BYTE scr_tmp1[4];
	DWORD *scr_tmp2;

	bps15 = screen_w*2;
	bps8 = screen_w;

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + bps8*y];
			dst_tmp = (WORD *) &dst[2*x + bps15*y];
			
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

bool RSCPixtangle::RSCConvertPixtangle_8to24(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps24, bps8;
	BYTE scr_tmp;
	BYTE *dst_tmp;

	bps24 = screen_w*4;
	bps8 = screen_w;

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + bps8*y];
			dst_tmp = &dst[3*x + bps24*y];

			dst_tmp[0] = palette->m_col_table[scr_tmp*4+2]; //(BYTE) (rgbquad[scr_tmp].rgbBlue);	// BLUE
			dst_tmp[1] = palette->m_col_table[scr_tmp*4+1]; //(BYTE) (rgbquad[scr_tmp].rgbGreen);	// GREEN
			dst_tmp[2] = palette->m_col_table[scr_tmp*4]; //(BYTE) (rgbquad[scr_tmp].rgbRed);		// RED
		}
	}

	return true;
}

bool RSCPixtangle::RSCConvertPixtangle_8to32(BYTE *scr, BYTE *dst, LONG screen_w, LONG screen_h, RSCPalette *palette)
{
	LONG bps32, bps8;
	BYTE scr_tmp;
	BYTE *dst_tmp;

	bps32 = screen_w*4;
	bps8 = screen_w;

	for(LONG y=0; y < screen_h; y++){
		for(LONG x=0; x < screen_w; x++){
		
			scr_tmp = scr[x + bps8*y];
			dst_tmp = &dst[4*x + bps32*y];
			
			dst_tmp[0] = palette->m_col_table[scr_tmp*4+2]; //(BYTE) (rgbquad[scr_tmp].rgbBlue);	// BLUE
			dst_tmp[1] = palette->m_col_table[scr_tmp*4+1]; //(BYTE) (rgbquad[scr_tmp].rgbGreen);	// GREEN
			dst_tmp[2] = palette->m_col_table[scr_tmp*4]; //(BYTE) (rgbquad[scr_tmp].rgbRed);		// RED
			dst_tmp[3] = 0;
		}
	}

	return true;
}

void RSCPixtangle::RSCCopyPixtangle2Pixtangle(BYTE *sscr, BYTE *dscr, int sx, int sy, int sw, int sh, int dx, int dy)
{

	int ctr = 0;

	int nsx = m_Bpp * sx;
	int nsy = m_Bps * ((m_screen_h-1)-sy);

	int nsw = m_Bpp * sw;
	int nsh = nsy - (m_Bps*sh);

	int ndx = m_Bpp * dx;
	int ndy = m_Bps * ((m_screen_h-1)-dy);

	int nsy0, ndy0;

	ndy0 = ndy;
	for(nsy0 = nsy; nsy0 > nsh; nsy0-=m_Bps){
		memcpy(&dscr[ndy0+ndx], &sscr[nsy0+nsx], nsw);
		ndy0-=m_Bps;
	}


}