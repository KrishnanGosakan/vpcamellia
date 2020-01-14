#include <stdio.h>
#include <stdint.h>

uint8_t ffmul(uint8_t a, uint8_t b, uint8_t irredPoly)
{
	uint8_t aa = a, bb = b, r = 0, t;
	while (aa != 0)
	{
		if ((aa & 1) != 0)
			r = r ^ bb;
		t = bb & 0x80;
		bb = bb << 1;
		if (t != 0)
			bb = bb ^ irredPoly;
		aa = aa >> 1; 
	}
	return r;
}

uint8_t gf2p4powerreducer(int a)
{
	uint8_t retVal = 0x00;
	switch(a)
	{
		case 0:
			retVal = 0x01;
			break;
		case 1:
			retVal = 0x02;
			break;
		case 2:
			retVal = 0x04;
			break;
		case 3:
			retVal = 0x08;
			break;
		case 4:
			retVal = 0x03;
			break;
		case 5:
			retVal = 0x06;
			break;
		case 6:
			retVal = 0x0c;
			break;
	}
	return retVal;
}

uint8_t gf2p4_ffmul(uint8_t n1, uint8_t n2)
{
	uint8_t res = 0;
	int b1,b2;
	int i,j;
	for(i=0;i<4;i++)
	{
		b1 = n1>>i & 1;
		for(j=0;j<4;j++)
		{
			b2 = n2>>j & 1;
			if(b1==1 && b2==1)
				res ^= gf2p4powerreducer(i+j);
		}
	}
	return res;
}

uint8_t findInv(uint8_t x, uint8_t irredPoly)
{
	uint8_t i;
	
	for(i=1;i<256;i++)
	{
		if(ffmul(x,i,irredPoly)==1)
			break;
	}
	
	return i;
}

uint8_t reduce(uint8_t exp, uint8_t irredPoly)
{
	int times = exp / 8;
	int rem = exp % 8;
	if(rem>0)
		rem = 1 << rem;
	
	int k;
	uint8_t res = irredPoly;
	for(k=0;k<times;k++)
	{
		res = ffmul(res,res,irredPoly);
	}
	res = ffmul(res,rem,irredPoly);
	return res;
}

uint8_t power(uint8_t base, int exp, int irredPoly)
{
	int i;
	uint8_t res = base;
	for(i=0;i<exp-1;i++)
	{
		res = ffmul(res,base,irredPoly);
	}
	return res;
}

int isRootofPxCamellia(uint8_t r,uint8_t poly)
{
	uint8_t res;
	res = power(r,8,poly) ^ power(r,6,poly) ^ power(r,5,poly) ^ power(r,3,poly) ^ 1;
	return res == 0;
}

int isRootofPxAES(uint8_t r,uint8_t poly)
{
	uint8_t res;
	res = power(r,8,poly) ^ power(r,4,poly) ^ power(r,3,poly) ^ r ^ 1;
	return res == 0;
}

uint8_t MainFieldRep(uint8_t matCol[], uint8_t subFieldRep)
{
	uint8_t mainFieldEl = 0x00;
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

		mainFieldEl |= res<<j;
	}

	return mainFieldEl;
}

int main()
{
	int i,j,k;
	uint8_t x,xdash;
	/*
	tested for (2,3) and (4,5) - not possible
	(10,11) and (68,69) all possibilities available
	(10,11) same as original i,j
	*/
	int res[256];
	x = 0x2f;
	xdash = 0x06;
	for(i=0;i<16;i++)
	{
		for(j=0;j<16;j++)
		{
			int index = (i^j)*16 ^ j;
			//printf("%02x %02x\n",index,ffmul(i,x,0x69)^ffmul(j,xdash,0x69));
			res[index] = ffmul(i,x,0x69)^ffmul(j,xdash,0x69);
		}
	}
	
	for(i=0;i<16;i++)
	{
		for(j=0;j<16;j++)
		{
			//printf("%02x ",res[i*16+j]);
		}
		//printf("\n");
	}
	
	//printf("%02x\n",ffmul(0x04,0x2f,0x69));
	
	/*printf("%02x\n",ffmul(0x07,0x68,0x69)^0x04);
	printf("%02x\n",ffmul(0x03,x,0x69)^ffmul(0x04,xdash,0x69));*/
	
	uint8_t lambda;
	//lambda = beta^2 + beta
	lambda = ffmul(0x68,0x68,0x69) ^ 0x68;
	
	//printf("%02x\n",lambda);
	
	uint8_t a = 0x75;
	uint8_t inva = power(a,254,0x69);
	//printf("%02x\n",inva);
	//printf("%02x\n",ffmul(a,inva,0x69));
	
	uint8_t isPrimEle[257];
	for(i=0;i<257;i++)
		isPrimEle[i]=0;
	
	int primEl = 0;
	for(i=1;i<256;i++)
	{
		int states[256],index;
		for(j=0;j<256;j++)
			states[j]=0;
		
		for(j=1;j<256;j++)
		{
			index = power(i,j,0x1b);
			states[index]++;
			if(states[index]>1)
				break;
		}
		if(states[index]==1)
		{
			//printf("%02x ",power(i,17,0x69));
			//if(power(i,17,0x69)<0x10)
				//printf("%02x %02x %02x\n",i,power(i,16,0x69)^1,power(i,17,0x69));
			//primEl++;
			isPrimEle[i]=1;
		}
	}
	//printf("\n");
	//printf("%d\n",primEl);
	
	/*int cnt=0;
	for(i=1;i<256;i++)
	{
		if(i^power(i,16,0x69)!=1)
			cnt++;
	}
	printf("%d\n",cnt);*/
	
	/*x = 0x68;
	xdash = x ^ 1;
	int cnt = 0;
	int res[256];
	for(i=0;i<256;i++)
		res[i]=0;
	for(i=0;i<16;i++)
	{
		for(j=0;j<16;j++)
		{
			int index = (i^j)*16 ^ j;
			//res[index] = ffmul(i,x,0x69)^ffmul(j,xdash,0x69);
			res[ffmul(i,x,0x69)^ffmul(j,xdash,0x69)] = index;
		}
	}
	
	for(i=0;i<16;i++)
	{
		for(j=0;j<16;j++)
		{
			printf("%02x ",res[i*16+j]);
		}
		printf("\n");
	}*/
	
	/*int i,j;
	uint8_t root = 0x13;
	printf("%02x\n",power(root,1,0x69));
	printf("%02x\n",power(root,2,0x69));
	printf("%02x\n",power(root,4,0x69));
	printf("%02x\n",power(root,8,0x69));
	printf("%02x\n",power(root,16,0x69));
	printf("%02x\n",power(root,32,0x69));
	printf("%02x\n",power(root,64,0x69));*/
	
	/*uint8_t camelliaIrredPol = 0x69;
	uint8_t beta = 0x04;
	uint8_t alpha = power(beta,238,camelliaIrredPol);
	
	int cnt = 0;
	
	for(i=1;i<256;i++)
	{
		int res = power(i,2,camelliaIrredPol) ^ i ^ alpha;
		if(res == 0)
		{
			printf("%-2x\n",i);
			cnt++;
		}
	}
	printf("%d\n",cnt);*/
	
	/*for(i=0;i<256;i++)
	{
		if(isRootofPxAES(i,0x1b))
			printf("%02x %02x\n",i,power(i,2,0x1b)^power(i,3,0x1b)^power(i,4,0x1b)^power(i,6,0x1b));
	}*/
	
	/*for(i=0;i<256;i++)
	{
		if(isRootofPxCamellia(i,0x69))
			printf("%02x %02x\n",i,power(i,238,0x69));
	}*/
	
	
	uint8_t beta,alpha,irred;

	beta = 0x02;
	irred = 0x69;

	alpha = power(beta,6,irred) ^ power(beta,5,irred) ^ power(beta,3,irred) ^ power(beta,2,irred);

	uint8_t matColums[8];

	matColums[7] = 0x01;
	matColums[6] = beta;
	matColums[5] = power(beta,2,irred);
	matColums[4] = power(beta,3,irred);
	matColums[3] = power(beta,4,irred);
	matColums[2] = power(beta,5,irred);
	matColums[1] = power(beta,6,irred);
	matColums[0] = power(beta,7,irred);

	//for(i=0;i<8;i++)
		//printf("%02x ",matColums[i]);
	//printf("\n");
	
	for(i=0;i<16;i++)
	{
		for(j=0;j<16;j++)
		{
			printf("%02x ",gf2p4_ffmul(i,j));
		}
		printf("\n");
	}
	
	//printf("%02x\n%02x\n",gf2p4_ffmul(0x07,0x0b),gf2p4_ffmul(0x0e,0x0b));

	//uint8_t subFieldElement = 0x75;

	//printf("%02x\n",MainFieldRep(matColums,subFieldElement));
	
	
	//printf("%02x\n",power(0x03,16,0x1b)^0x03);
	//printf("%02x\n",power(0x03,17,0x1b));
	
	/*int cnt=0;
	for(i=1;i<256;i++)
	{
		if(power(i,17,0x69)<0x0a && (i^power(i,16,0x69))<0x0a)
		{
			printf("%02x %02x %02x\n",i,power(i,16,0x69)^i,power(i,17,0x69));
			cnt++;
		}
	}*/
	//printf("%d\n",cnt);
	
	/*printf("%02x\n",power(0x03,2,0x69));
	printf("%02x\n",power(0x03,4,0x69));
	printf("%02x\n",power(0x03,8,0x69));
	printf("%02x\n",power(0x03,16,0x69));
	printf("%02x\n",power(0x03,32,0x69));
	printf("%02x\n",power(0x03,64,0x69));*/
	
	
	/*int cnt=0;
	for(i=1;i<256;i++)
	{
		if(isPrimEle[i] == 1)
		{
			uint8_t py = power(i,2,0x69) ^ ffmul(i,0x07,0x69) ^ 0x01;
			if(py == 0)
			{
				printf("%02x\n",i);
				cnt++;
			}
		}
	}
	printf("%d\n",cnt);*/
}
