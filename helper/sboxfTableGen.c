#include <stdio.h>
#include <math.h>

int subFunFupper()
{
	int a1,a2,a3,a4;
	int i,j;
	int res = 0;
	for(j=0;j<16;j++)
	{
		i  = j;
		i  = i ^ 12;//xor with 0x0c
		a4 = i%2;
		i  = i/2;
		a3 = i%2;
		i  = i/2;
		a2 = i%2;
		i  = i/2;
		a1 = i%2;
		i  = i/2;
		int k;
		for(k=0;k<8;k++)
		{
			switch(k)
			{
				case 0:
					//a4
					res += pow(2,k)*a4;
					break;
				case 1:
					//a1
					res += pow(2,k)*a1;
					break;
				case 2:
					//a2
					res += pow(2,k)*a2;
					break;
				case 3:
					//a4
					res += pow(2,k)*a4;
					break;
				case 4:
					//a3
					res += pow(2,k)*a3;
					break;
				case 5:
					//a3
					res += pow(2,k)*a3;
					break;
				case 6:
					//a1
					res += pow(2,k)*a1;
					break;
				case 7:
					//a2
					res += pow(2,k)*a2;
					break;
			}
		}
		printf("0x%02x, ",res);
		res = 0;
	}
	printf("\n");
}

int subFunFlower()
{
	int a5,a6,a7,a8;
	int i,j;
	int res = 0;
	for(j=0;j<16;j++)
	{
		i  = j;
		i  = i ^ 5;//xor with 0x05
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
					//a8
					res += pow(2,k)*a8;
					break;
				case 2:
					//a5
					res += pow(2,k)*a5;
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
					//a8^a5
					res += pow(2,k)*(a8^a5);
					break;
				case 6:
					//a7
					res += pow(2,k)*a7;
					break;
				case 7:
					//a6
					res += pow(2,k)*a6;
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
	subFunFupper();
	printf("lower: ");
	subFunFlower();
}
