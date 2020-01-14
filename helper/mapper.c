#include <stdio.h>
#include <stdint.h>

void MainFieldRep(uint8_t matCol[], uint8_t subFieldRep)
{
	int i,j,k;
	int v[8];
	k=0;
	for(i=7;i>=0;i--)
	{
		v[k++] = subFieldRep>>i & 1;
	}
	
	for(j=7;j>=0;j--)
	{
		int c[8];
		for(k=0;k<8;k++)
		{
			c[k] = matCol[k]>>j & 1;
		}
		
		/*for(k=0;k<8;k++)
		{
			printf("%d ",c[k]);
		}
		printf("\n");
		for(k=0;k<8;k++)
		{
			printf("%d ",v[k]);
		}
		printf("\n");*/
		
		int res = 0;
		for(i=0;i<8;i++)
		{
			res ^= c[i] & v[i];
		}
		
		printf("%d",res);
	}
}

int main()
{
	uint8_t matColumns[] = {0xe3, 0x14, 0x58, 0x64, 0xec, 0x2e, 0x73, 0x01};
	uint8_t subFieldElement = 0xe4;
	
	MainFieldRep(matColumns,subFieldElement);
	printf("\n");
}
