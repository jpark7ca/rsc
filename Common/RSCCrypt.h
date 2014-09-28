// 
// RSC Crypt Class (RSCCrypt.h) 
// By Jae H. Park 
//
// Created: 07.16.2006
//
// RSCCrypt class handles the encryption and decryption of string, byte stream data
//
// ANSI C/C++ 
//
// unsigned char refers to BYTE (8bits) type
//

#pragma once

#define MAXKEYLENGTH 8

typedef struct RSC_ENCRYPTED{
	unsigned char *text;
	long length;
	long orglength;
	bool status;
}RSC_ENCRYPTED;

class RSCCrypt
{
public:
	RSCCrypt(void);
	RSCCrypt(unsigned char key[8]);
	~RSCCrypt(void);

	bool RSCLoadEncrytped(unsigned char *enctext, long enclength);
	bool RSCLoadEncrytped(unsigned char *enctext, long enclength, long orglength);
	void RSCLoadKey(unsigned char key[8]);
	
	bool RSCEncryptString(char *plaintext);
	bool RSCEncryptString(char *plaintext, unsigned char key[8]);
	bool RSCEncryptByteStream(unsigned char *plaintext, long length);
	bool RSCEncryptByteStream(unsigned char *plaintext, long length, unsigned char key[8]);
	char *RSCDecryptString(void);
	char *RSCDecryptString(unsigned char key[8]);
	unsigned char *RSCDecryptByteStream(void);
	unsigned char *RSCDecryptByteStream(unsigned char key[8]);

	void RSCFreeEncrypted(void);

	static void RSCKeyGen(unsigned char *key, long length);
	static void RSCFreeMemory(void *str);

private:
	unsigned char prekey[8];

public:
	RSC_ENCRYPTED enc;

};
