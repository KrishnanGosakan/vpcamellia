#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "encrypt.h"
#include "modes.h"

int main(int argc, char *argv[])
{
	int i;
	
	bool encordec = false;
	int keylength;
	int mode;
	char *plainText, *key, *cipherText;
	
	keylength = 0;
	
	mode = invalid_mode;
	
	plainText  = NULL;
	key		   = NULL;
	cipherText = NULL;
	
	for(i=1;i<argc;i++)
	{
		if(strcmp(argv[i], "-encrypt") == 0)
		{
			encordec = true;
		}
		else if(strcmp(argv[i], "-keylength") == 0)
		{
			i++;
			if(strcmp(argv[i], "128") == 0)
				keylength = 128;
			else if(strcmp(argv[i], "192") == 0)
				keylength = 192;
			else if(strcmp(argv[i], "256") == 0)
				keylength = 256;
		}
		else if(strcmp(argv[i], "-mode") == 0)
		{
			i++;
			if(strcmp(argv[i], "ecb") == 0)
			{
				mode = ecb_mode;
			}
			else if(strcmp(argv[i], "cbc") == 0)
			{
				mode = cbc_mode;
			}
			else if(strcmp(argv[i], "ctr") == 0)
			{
				mode = ctr_mode;
			}
			else if(strcmp(argv[i], "cfb") == 0)
			{
				mode = cfb_mode;
			}
		}
		else if(strcmp(argv[i], "-plaintext") == 0)
		{
			i++;
			plainText = strdup(argv[i]);
		}
		else if(strcmp(argv[i], "-key") == 0)
		{
			i++;
			key = strdup(argv[i]);
		}
	}
	
	if(mode == invalid_mode || keylength == 0 || plainText == NULL || key == NULL)
	{
		printf("Missing essential parameters. So quitting\n");
	}
	else
	{
		switch(mode)
		{
			case ecb_mode:
				if(encordec)
				{
					if(keylength == 128)
						cipherText = ecb_128_encrypt (plainText, key);
					else if(keylength == 192)
						cipherText = ecb_192_encrypt (plainText, key);
				}
				break;
			case cbc_mode:
				break;
			case ctr_mode:
				break;
			case cfb_mode:
				break;
		}
	}
	
	return 0;
}
