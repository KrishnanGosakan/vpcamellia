#include <stdio.h>
#include <stdint.h>
#include <math.h>

void subFunHupper()
{
	uint8_t a1,a2,a3,a4;
	uint8_t i,j;
	uint8_t res = 0;
	for(j=0;j<16;j++)
	{
		i  = j;
		a4 = i%2;
		i  = i/2;
		a3 = i%2;
		i  = i/2;
		a2 = i%2;
		i  = i/2;
		a1 = i%2;
		i  = i/2;
		uint8_t k;
		for(k=0;k<8;k++)
		{
			switch(k)
			{
				case 0:
					//a3
					res += pow(2,k)*a3;
					break;
				case 1:
					//a1
					res += pow(2,k)*a1;
					break;
				case 2:
					//a1
					res += pow(2,k)*a1;
					break;
				case 3:
					//a3
					res += pow(2,k)*a3;
					break;
				case 4:
					//a2
					res += pow(2,k)*a2;
					break;
				case 5:
					//a4
					res += pow(2,k)*a4;
					break;
				case 6:
					//a2
					res += pow(2,k)*a2;
					break;
				case 7:
					//a2
					res += pow(2,k)*a2;
					break;
			}
		}
		res = res ^ 0x6e;
		printf("0x%02x, ",res);
		res = 0;
	}
	printf("\n");
}

void subFunHlower()
{
	int a5,a6,a7,a8;
	int i,j;
	int res = 0;
	for(j=0;j<16;j++)
	{
		i  = j;
		a8 = i%2;
		i  = i/2;
		a7 = i%2;
		i  = i/2;
		a6 = i%2;
		i  = i/2;
		a5 = i%2;
		i  = i/2;
		int k;
		for(k=0;k<8;k++)
		{
			switch(k)
			{
				case 0:
					//a6
					res += pow(2,k)*a6;
					break;
				case 1:
					//a5
					res += pow(2,k)*a5;
					break;
				case 2:
					//a8
					res += pow(2,k)*a8;
					break;
				case 3:
					//a7
					res += pow(2,k)*a7;
					break;
				case 4:
					//a8
					res += pow(2,k)*a8;
					break;
				case 5:
					//a7
					res += pow(2,k)*a7;
					break;
				case 6:
					//a6
					res += pow(2,k)*a6;
					break;
				case 7:
					//a5^a6
					res += pow(2,k)*(a5^a6);
					break;
			}
		}
		printf("0x%02x, ",res);
		res = 0;
	}
	printf("\n");
}

int main()
{
	printf("upper: ");
	subFunHupper();
	printf("lower: ");
	subFunHlower();
}
