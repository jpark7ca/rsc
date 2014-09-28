// 
// RSC Crypt Class (RSCCrypt.cpp) 
// By Jae H. Park 
//
// Created: 07.16.2006
//
// RSCCrypt class handles the encryption and decryption of string, byte stream data
//
//
#include "StdAfx.h"
#include "..\Common\d3des.h"

RSCCrypt::RSCCrypt(void)
{
	// Preset the default key value
	prekey[0]= 10;
	prekey[1]= 8;
	prekey[2]= 73;
	prekey[3]= 12;
	prekey[4]= 23;
	prekey[5]= 76;
	prekey[6]= 108;
	prekey[7]= 6;
	
	enc.text = NULL;
	enc.length = 0;
	enc.orglength = 0;
	enc.status = false;
}

RSCCrypt::RSCCrypt(unsigned char key[8])
{
	// Load user-defined key
	for(int i=0; i<8; i++)
		prekey[i] = key[i];

	enc.text = NULL;
	enc.length = 0;
	enc.orglength = 0;
	enc.status = false;
}

RSCCrypt::~RSCCrypt(void)
{
	if(enc.status){
		delete [] enc.text;
		enc.status = false;
	}

	//printf("Released \n");
}

bool RSCCrypt::RSCLoadEncrytped(unsigned char *enctext, long enclength)
{
	if(enc.status){
		return false;
	}

	//enc.text = (unsigned char *) malloc(enclength);
	enc.text = new unsigned char[enclength];
	
	if(enc.text == NULL){
		return false;
	}
	enc.length = enclength;
	enc.status = true;

	for(long i=0; i<enclength; i++)
		enc.text[i] = enctext[i];


	return true;
}

bool RSCCrypt::RSCLoadEncrytped(unsigned char *enctext, long enclength, long orglength)
{
	if(enc.status){
		return false;
	}

	enc.text = new unsigned char[enclength];

	if(enc.text == NULL){
		return false;
	}

	enc.length = enclength;
	enc.orglength = orglength;
	enc.status = true;

	for(long i=0; i<enclength; i++)
		enc.text[i] = enctext[i];

	return true;
}


void RSCCrypt::RSCLoadKey(unsigned char key[8])
{
	for(int i=0; i<8; i++)
		prekey[i] = key[i];
}

void RSCCrypt::RSCKeyGen(unsigned char *key, long length)
{
	unsigned int seed=0;

	seed += (unsigned int) time(0) + getpid() + getpid() * 987654;

	srand(seed);
	for(int i=0; i < length; i++){
		key[i] = (unsigned char) (rand() & 255);
	}
}

bool RSCCrypt::RSCEncryptString(char *plaintext)
{
	long i;
	long strlength;

	if(enc.status)
		return false;

	strlength = strlen(plaintext);
	if(strlength == 0)
		return false;

	enc.length  =  strlength + (8 - ((strlength%8) == 0 ? 8 : (strlength%8)));

	enc.text = new unsigned char[enc.length];
	
	if(enc.text == NULL){
		enc.length = 0;	
		enc.status = false;
		return false;
	}

	enc.status = true;

	for(i=0; i < enc.length; i++){

		if( i < strlength){
			enc.text[i] = plaintext[i];
		}else{
			enc.text[i] = 0;
		}
	}

	// Encrypting the string ...
	deskey(prekey, EN0);
	for(i=0; i < enc.length; i+=8){
		//deskey(prekey, EN0);
		des(&enc.text[i], &enc.text[i]);
	}

	return true;
}

bool RSCCrypt::RSCEncryptByteStream(unsigned char *plaintext, long length)
{
	long i;

	if(length == 0)
		return false;

	if(enc.status)
		return false;

	enc.orglength = length;
	enc.length  =  enc.orglength + (8 - ((enc.orglength%8) == 0 ? 8:(enc.orglength%8)));

	enc.text =  new unsigned char[enc.length];
	
	if(enc.text == NULL){
		enc.length = 0;	
		enc.orglength = 0;
		enc.status = false;
		return false;
	}

	enc.status = true;

	for(i=0; i < enc.length; i++){

		if( i < enc.orglength){
			enc.text[i] = plaintext[i];
		}else{
			enc.text[i] = 0;
		}
	}

	// Encrypting the string ...
	deskey(prekey, EN0);
	for(i=0; i < enc.length; i+=8){
		des(&enc.text[i], &enc.text[i]);
	}

	return true;
}


bool RSCCrypt::RSCEncryptString(char *plaintext, unsigned char key[8])
{
	long i;
	long strlength;

	if(enc.status)
		return false;

	strlength = strlen(plaintext);
	if(strlength == 0)
		return false;

	enc.length  =  strlength + (8 - ((strlength%8) == 0 ? 8 : (strlength%8)));

	enc.text =  new unsigned char[enc.length];
	
	if(enc.text == NULL){
		enc.length = 0;	
		enc.status = false;
		return false;
	}

	enc.status = true;

	for(i=0; i < enc.length; i++){

		if( i < strlength){
			enc.text[i] = plaintext[i];
		}else{
			enc.text[i] = 0;
		}
	}

	// Encrypting the string ...
	deskey(key, EN0);
	for(i=0; i < enc.length; i+=8){
		//deskey(prekey, EN0);
		des(&enc.text[i], &enc.text[i]);
	}

	return true;
}

bool RSCCrypt::RSCEncryptByteStream(unsigned char *plaintext, long length, unsigned char key[8])
{
	long i;

	if(length == 0)
		return false;

	if(enc.status)
		return false;

	enc.orglength = length;
	enc.length  =  enc.orglength + (8 - ((enc.orglength%8) == 0 ? 8:(enc.orglength%8)));

	enc.text = new unsigned char[enc.length];
	
	if(enc.text == NULL){
		enc.length = 0;	
		enc.orglength = 0;
		enc.status = false;
		return false;
	}

	enc.status = true;

	for(i=0; i < enc.length; i++){

		if( i < enc.orglength){
			enc.text[i] = plaintext[i];
		}else{
			enc.text[i] = 0;
		}
	}

	// Encrypting the string ...
	deskey(key, EN0);
	for(i=0; i < enc.length; i+=8){
		des(&enc.text[i], &enc.text[i]);
	}

	return true;
}



char * RSCCrypt::RSCDecryptString(void)
{
	long i;
	unsigned char *text =  new unsigned char[enc.length+1];;
	
	if(text == NULL)
		return NULL;

	if(!enc.status){
		return NULL;
	}

	deskey(prekey, DE1);
	for(i=0; i < enc.length; i+=8){
		//deskey(prekey, DE1);
		des(&enc.text[i], &text[i]);
	}

	text[enc.length] = 0;

	return (char *) text;
}

unsigned char * RSCCrypt::RSCDecryptByteStream(void)
{
	long i;
	unsigned char *text =  new unsigned char[enc.length];
	
	if(text == NULL)
		return NULL;

	if(!enc.status){
		return NULL;
	}

	deskey(prekey, DE1);
	for(i=0; i < enc.length; i+=8){
		//deskey(prekey, DE1);
		des(&enc.text[i], &text[i]);
	}

	return text;
}


char * RSCCrypt::RSCDecryptString(unsigned char key[8])
{
	long i;
	unsigned char *text =  new unsigned char[enc.length+1];
	
	if(text == NULL)
		return NULL;

	if(!enc.status){
		return NULL;
	}

	deskey(key, DE1);
	for(i=0; i < enc.length; i+=8){
		//deskey(prekey, DE1);
		des(&enc.text[i], &text[i]);
	}

	text[enc.length] = 0;

	return (char *) text;
}
unsigned char * RSCCrypt::RSCDecryptByteStream(unsigned char key[8])
{
	long i;
	unsigned char *text =  new unsigned char[enc.length];
	
	if(text == NULL)
		return NULL;

	if(!enc.status){
		return NULL;
	}

	deskey(key, DE1);
	for(i=0; i < enc.length; i+=8){
		//deskey(prekey, DE1);
		des(&enc.text[i], &text[i]);
	}

	return text;
}

void RSCCrypt::RSCFreeEncrypted(void)
{
	if(enc.status){
		delete [] enc.text;

		enc.text = NULL;
		enc.length = 0;
		enc.orglength = 0;
		enc.status = false;
	}
}

void RSCCrypt::RSCFreeMemory(void *str)
{
	if(str != NULL)
		delete [] str;
}