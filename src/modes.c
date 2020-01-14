#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "encrypt.h"

void print128_num(__m128i var)
{
	int64_t e0, e1;
	e0 = _mm_extract_epi64 (var, 0);
	e1 = _mm_extract_epi64 (var, 1);
    printf("%016"PRIx64"%016"PRIx64"\n", e0, e1);
}

uint8_t getHexFromChar(char a)
{
	switch(a)
	{
		case '0':
			return 0x00;
			break;
		case '1':
			return 0x01;
			break;
		case '2':
			return 0x02;
			break;
		case '3':
			return 0x03;
			break;
		case '4':
			return 0x04;
			break;
		case '5':
			return 0x05;
			break;
		case '6':
			return 0x06;
			break;
		case '7':
			return 0x07;
			break;
		case '8':
			return 0x08;
			break;
		case '9':
			return 0x09;
			break;
		case 'a':
			return 0x0a;
			break;
		case 'b':
			return 0x0b;
			break;
		case 'c':
			return 0x0c;
			break;
		case 'd':
			return 0x0d;
			break;
		case 'e':
			return 0x0e;
			break;
		case 'f':
			return 0x0f;
			break;
	}
}

__m128i get_m128i_variable_from_chararray(char *input)
{
	__m128i var;
	
	uint8_t singleblock[16];
	int i;
	int j=0;
    for(i=0;i<16;i++)
    {
    	singleblock[i] = getHexFromChar(input[j]) * 16 + getHexFromChar(input[j+1]);
    	j=j+2;
    }
    
    var = get_m128i_variable_from_uint8_array(singleblock);

	return var;
}

char *get_chararray_from_m128i_variable(__m128i var)
{
	char *res = (char *)malloc(16);
	
	int64_t e0, e1;
	e0 = _mm_extract_epi64 (var, 0);
	e1 = _mm_extract_epi64 (var, 1);
    sprintf(res,"%016"PRIx64"%016"PRIx64"\n", e0, e1);
	
	return res;
}

char* ecb_128_encrypt(char *plaintextstring, char *keystring)
{
	__m128i plainText, key;
	__m128i keyright = _mm_setzero_si128();
	__m128i ka, kb = _mm_setzero_si128();
	__m128i ct;
	
	char *cipherText;
	
	int keylength = 128;
	key       = get_m128i_variable_from_chararray(keystring);
	ka = key_schedule1 (key, keyright);
	
	plainText = get_m128i_variable_from_chararray(plaintextstring);
	
    ct = encrypt (plainText, key, keyright, ka, kb, keylength);
    
    print128_num (ct);

	return cipherText;
}

char* ecb_192_encrypt(char *plainText, char *key)
{
	char *cipherText;
	
	return cipherText;
}

char* ecb_256_encrypt(char *plainText, char *key)
{
	char *cipherText;
	
	return cipherText;
}
