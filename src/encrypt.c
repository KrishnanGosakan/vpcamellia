#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <emmintrin.h>
#include <tmmintrin.h>
#include <immintrin.h>

__m128i get_m128i_variable_from_uint8_array(uint8_t *inputArray)
{
	__m128i res;
	int i;
	int64_t l64, r64;
	l64 = 0;
	r64 = 0;
	
	for(i=0;i<16;i++)
	{
		if(i<8)
			l64 = l64 << 8 | inputArray[i];
		else
			r64 = r64 << 8 | inputArray[i];
	}
	
	res = _mm_set_epi64x (r64, l64);
	
	return res;
}

void inverseGF2P4(__m128i input, __m128i *lOut, __m128i *rOut)
{
	//input is of form a+bt
	uint8_t lowersplitterscalar[] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	__m128i lowersplitter = _mm_loadu_si128((const __m128i*)lowersplitterscalar);
	__m128i a = _mm_and_si128 (input, lowersplitter);

	uint8_t uppersplitterscalar[] = {0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	__m128i uppersplitter = _mm_loadu_si128((const __m128i*)uppersplitterscalar);
	__m128i b = _mm_and_si128 (input, uppersplitter);
	b = _mm_srli_epi32 (b, 4);
	
	//find a+b
	//let's call it x
	__m128i x = _mm_xor_si128 (a, b);
	
	//tables to be used below
	__m128i gf2p4Inverse, lambdaInverse, lambdaMul;
	uint8_t inv[] = {0x80, 0x01, 0x09, 0x0e, 0x0d, 0x0b, 0x07, 0x06, 0x0f, 0x02, 0x0c, 0x05, 0x0a, 0x04, 0x03, 0x08};
	uint8_t lambMul[] = {0x00, 0x09, 0x01, 0x08, 0x02, 0x0b, 0x03, 0x0a, 0x04, 0x0d, 0x05, 0x0c, 0x06, 0x0f, 0x07, 0x0e};
	uint8_t lambInv[] = {0x80, 0x02, 0x01, 0x0f, 0x09, 0x05, 0x0e, 0x0c, 0x0d, 0x04, 0x0b, 0x0a, 0x07, 0x08, 0x06, 0x03};
	gf2p4Inverse = _mm_loadu_si128((const __m128i *)inv);
	lambdaInverse = _mm_loadu_si128((const __m128i *)lambInv);
	lambdaMul = _mm_loadu_si128((const __m128i *)lambMul);
	
	//find inverse of a
	__m128i aInv = _mm_shuffle_epi8(gf2p4Inverse, a);
		
	//find inverse of a+b
	__m128i xInv = _mm_shuffle_epi8(gf2p4Inverse, x);
	
	//find inverse of lambda*b
	__m128i blInv = _mm_shuffle_epi8(lambdaInverse, b);
	
	//find aInv + blInv
	aInv = _mm_xor_si128(aInv, blInv);
	
	//find xInv + blInv
	xInv = _mm_xor_si128(xInv, blInv);
	
	//find inverse(aInv + blInv)
	aInv = _mm_shuffle_epi8(gf2p4Inverse, aInv);
	
	//find inverse(xInv + blInv)
	xInv = _mm_shuffle_epi8(gf2p4Inverse, xInv);
	
	//find inverse(aInv+blInv)+x
	aInv = _mm_xor_si128(aInv, x);
	
	//find inverse(xInv+blInv)+a
	xInv = _mm_xor_si128(xInv, a);
	
	//find I1
	__m128i I1 = _mm_shuffle_epi8(gf2p4Inverse, aInv);
	
	//find I2
	__m128i I2 = _mm_shuffle_epi8(gf2p4Inverse, xInv);
	
	//let's assume c+dt be the inverse of input
	//then d=I1+I2
	//and c=I2+(I1+I2)lambda
	//__m128i d = _mm_xor_si128(I1, I2);
	*lOut = _mm_xor_si128(I1, I2);
	
	I1 = _mm_shuffle_epi8(lambdaMul, *lOut);
	
	//__m128i c = _mm_xor_si128(I1, I2);
	*rOut = _mm_xor_si128(I1, I2);
}

__m128i s(__m128i x)
{
	//pre s-box computaion
	//1.shuffle for left rotating s4
	uint8_t s4shufmaskscalar[] = {0x00, 0x80, 0x02, 0x03, 0x80, 0x05, 0x06, 0x07, 0x01, 0x80, 0x80, 0x80, 0x04, 0x80, 0x80, 0x80};
	__m128i s4shufmask = _mm_loadu_si128((const __m128i*)s4shufmaskscalar);
	x = _mm_shuffle_epi8(x, s4shufmask);
	
	//2.left rotate s4 - shift left by 1 and right by 7
	__m128i s4rotleftamt = _mm_setr_epi32(0x00, 0x00, 0x01, 0x01);
	__m128i lr = _mm_sllv_epi32(x, s4rotleftamt);
	__m128i s4rotrightamt = _mm_setr_epi32(0x00, 0x00, 0x07, 0x07);
	__m128i rr = _mm_srlv_epi32(x, s4rotrightamt);
	
	//3.pxor the above
	__m128i s4rotres = _mm_xor_si128 (lr, rr);
	
	//4.purify x
	uint8_t xpurshufmaskscalar0[] = {0x00, 0x80, 0x02, 0x03, 0x80, 0x05, 0x06, 0x07, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i xpurshufmask = _mm_loadu_si128((const __m128i*)xpurshufmaskscalar0);
	x = _mm_shuffle_epi8(x, xpurshufmask);
	
	//5.reshuffle s4 to original position
	uint8_t s4revshufmaskscalar[] = {0x80, 0x08, 0x80, 0x80, 0x0c, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i s4revshufmask = _mm_loadu_si128((const __m128i*)s4revshufmaskscalar);
	__m128i s4reordered = _mm_shuffle_epi8(s4rotres, s4revshufmask);
	x = _mm_xor_si128 (s4reordered, x);
	
	//can merge this into f function tables
	//6.pxor with 0xc5
	//uint8_t presboxc5xorscalar[] = {0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0xc5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//__m128i presboxc5xor = _mm_loadu_si128((const __m128i*)presboxc5xorscalar);
	//x = _mm_xor_si128 (x, presboxc5xor);
	
	//s-box computation
	//1.split each byte into nibbles
	uint8_t lowersplitterscalar[] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	__m128i lowersplitter = _mm_loadu_si128((const __m128i*)lowersplitterscalar);
	__m128i lowernibble = _mm_and_si128 (x, lowersplitter);
	uint8_t uppersplitterscalar[] = {0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	__m128i uppersplitter = _mm_loadu_si128((const __m128i*)uppersplitterscalar);
	__m128i uppernibble = _mm_and_si128 (x, uppersplitter);
	uppernibble = _mm_srli_epi32 (uppernibble, 4);
	
	//2.compute f functions
	uint8_t fshufupperscalar[] = {0xc6, 0xcf, 0xf6, 0xff, 0x42, 0x4b, 0x72, 0x7b, 0x84, 0x8d, 0xb4, 0xbd, 0x00, 0x09, 0x30, 0x39};
	__m128i fshufuppersrc = _mm_loadu_si128((const __m128i*)fshufupperscalar);
	uint8_t fshuflowerscalar[] = {0xb3, 0x81, 0xfb, 0xc9, 0x32, 0x00, 0x7a, 0x48, 0x97, 0xa5, 0xdf, 0xed, 0x16, 0x24, 0x5e, 0x6c};
	__m128i fshuflowerersrc = _mm_loadu_si128((const __m128i*)fshuflowerscalar);
	__m128i flower = _mm_shuffle_epi8(fshuflowerersrc, lowernibble);
	__m128i fupper = _mm_shuffle_epi8(fshufuppersrc, uppernibble);
	__m128i fout = _mm_xor_si128 (flower, fupper);
	
	//3.compute g function
	inverseGF2P4 (fout, &uppernibble, &lowernibble);
	
	//4.again split each byte into nibbles
	//lowernibble = _mm_and_si128 (fout, lowersplitter);
	//uppernibble = _mm_and_si128 (fout, uppersplitter);
	//uppernibble = _mm_srli_epi32 (uppernibble, 4);
	
	//5.compute h function
	uint8_t hshufupperscalar[] = {0x6e, 0x4e, 0x67, 0x47, 0xbe, 0x9e, 0xb7, 0x97, 0x68, 0x48, 0x61, 0x41, 0xb8, 0x98, 0xb1, 0x91};
	__m128i hshufuppersrc = _mm_loadu_si128((const __m128i*)hshufupperscalar);
	uint8_t hshuflowerscalar[] = {0x00, 0x14, 0x28, 0x3c, 0xc1, 0xd5, 0xe9, 0xfd, 0x82, 0x96, 0xaa, 0xbe, 0x43, 0x57, 0x6b, 0x7f};
	__m128i hshuflowersrc = _mm_loadu_si128((const __m128i*)hshuflowerscalar);
	__m128i hlower = _mm_shuffle_epi8(hshuflowersrc, lowernibble);
	__m128i hupper = _mm_shuffle_epi8(hshufuppersrc, uppernibble);
	x = _mm_xor_si128 (hlower, hupper);
	
	//can merge this into h function tables
	//6.xor 0x6e with s-box result
	//uint8_t postsbox6exorscalar[] = {0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//__m128i postsbox6exor = _mm_loadu_si128((const __m128i*)postsbox6exorscalar);
	//x = _mm_xor_si128 (hout, postsbox6exor);
	
	//post s-box computation
	//1.left rotate s2 by 1
	//1.1.shuffle for left rotating s2
	uint8_t s2shufmaskscalar[] = {0x00, 0x01, 0x02, 0x80, 0x04, 0x05, 0x80, 0x07, 0x03, 0x80, 0x80, 0x80, 0x06, 0x80, 0x80, 0x80};
	__m128i s2shufmask = _mm_loadu_si128((const __m128i*)s2shufmaskscalar);
	x = _mm_shuffle_epi8(x, s2shufmask);
	
	//1.2.left rotate s2 - shift left by 1 and right by 7
	__m128i s2rotleftamt = _mm_setr_epi32(0x00, 0x00, 0x01, 0x01);
	lr = _mm_sllv_epi32(x, s2rotleftamt);
	__m128i s2rotrightamt = _mm_setr_epi32(0x00, 0x00, 0x07, 0x07);
	rr = _mm_srlv_epi32(x, s2rotrightamt);
	
	//1.3.pxor the above
	__m128i s2rotres = _mm_xor_si128 (lr, rr);
	
	//1.4.purify x
	uint8_t xpurshufmaskscalar1[] = {0x00, 0x01, 0x02, 0x80, 0x04, 0x05, 0x80, 0x07, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	xpurshufmask = _mm_loadu_si128((const __m128i*)xpurshufmaskscalar1);
	x = _mm_shuffle_epi8(x, xpurshufmask);
	
	//1.5.reshuffle s2 to original position
	uint8_t s2revshufmaskscalar[] = {0x80, 0x80, 0x80, 0x08, 0x80, 0x80, 0x0c, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i s2revshufmask = _mm_loadu_si128((const __m128i*)s2revshufmaskscalar);
	__m128i s2reordered = _mm_shuffle_epi8(s2rotres, s2revshufmask);
	x = _mm_xor_si128 (s2reordered, x);
	
	//2.right rotate s3 by 1
	//2.1.shuffle for left rotating s3
	uint8_t s3shufmaskscalar[] = {0x00, 0x01, 0x80, 0x03, 0x04, 0x80, 0x06, 0x07, 0x02, 0x80, 0x80, 0x80, 0x05, 0x80, 0x80, 0x80};
	__m128i s3shufmask = _mm_loadu_si128((const __m128i*)s3shufmaskscalar);
	x = _mm_shuffle_epi8(x, s3shufmask);
	
	//2.2.left rotate s3 - shift right by 1 and left by 7
	__m128i s3rotleftamt = _mm_setr_epi32(0x00, 0x00, 0x07, 0x07);
	lr = _mm_sllv_epi32(x, s3rotleftamt);
	__m128i s3rotrightamt = _mm_setr_epi32(0x00, 0x00, 0x01, 0x01);
	rr = _mm_srlv_epi32(x, s3rotrightamt);
	
	//2.3.pxor the above
	__m128i s3rotres = _mm_xor_si128 (lr, rr);
	
	//2.4.purify x
	uint8_t xpurshufmaskscalar2[] = {0x00, 0x01, 0x80, 0x03, 0x04, 0x80, 0x06, 0x07, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	xpurshufmask = _mm_loadu_si128((const __m128i*)xpurshufmaskscalar2);
	x = _mm_shuffle_epi8(x, xpurshufmask);
	
	//2.5.reshuffle s3 to original position
	uint8_t s3revshufmaskscalar[] = {0x80, 0x80, 0x08, 0x80, 0x80, 0x0c, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i s3revshufmask = _mm_loadu_si128((const __m128i*)s3revshufmaskscalar);
	__m128i s3reordered = _mm_shuffle_epi8(s3rotres, s3revshufmask);
	x = _mm_xor_si128 (s3reordered, x);
	
	return x;
}

__m128i p(__m128i z)
{
	__m128i result = _mm_setzero_si128();
	__m128i temp;
	
	uint8_t p1shufmaskscalar[] = {0x07, 0x07, 0x07, 0x06, 0x07, 0x06, 0x05, 0x07, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i p1shufmask = get_m128i_variable_from_uint8_array (p1shufmaskscalar);
	temp = _mm_shuffle_epi8 (z, p1shufmask);
	result = _mm_xor_si128 (result, temp);
	
	uint8_t p2shufmaskscalar[] = {0x05, 0x06, 0x06, 0x05, 0x06, 0x05, 0x04, 0x04, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i p2shufmask = get_m128i_variable_from_uint8_array (p2shufmaskscalar);
	temp = _mm_shuffle_epi8 (z, p2shufmask);
	result = _mm_xor_si128 (result, temp);
	
	uint8_t p3shufmaskscalar[] = {0x04, 0x04, 0x05, 0x04, 0x02, 0x03, 0x03, 0x03, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i p3shufmask = get_m128i_variable_from_uint8_array (p3shufmaskscalar);
	temp = _mm_shuffle_epi8 (z, p3shufmask);
	result = _mm_xor_si128 (result, temp);
	
	uint8_t p4shufmaskscalar[] = {0x02, 0x03, 0x03, 0x03, 0x01, 0x01, 0x02, 0x02, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i p4shufmask = get_m128i_variable_from_uint8_array (p4shufmaskscalar);
	temp = _mm_shuffle_epi8 (z, p4shufmask);
	result = _mm_xor_si128 (result, temp);
	
	uint8_t p5shufmaskscalar[] = {0x01, 0x01, 0x02, 0x02, 0x00, 0x00, 0x00, 0x01, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i p5shufmask = get_m128i_variable_from_uint8_array (p5shufmaskscalar);
	temp = _mm_shuffle_epi8 (z, p5shufmask);
	result = _mm_xor_si128 (result, temp);
	
	uint8_t p6shufmaskscalar[] = {0x00, 0x00, 0x00, 0x01, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i p6shufmask = get_m128i_variable_from_uint8_array (p6shufmaskscalar);
	temp = _mm_shuffle_epi8 (z, p6shufmask);
	result = _mm_xor_si128 (result, temp);
	
	return result;
}

__m128i f(__m128i l,__m128i k)
{
	__m128i res;
	
	//initial xor
	__m128i x = _mm_xor_si128 (l, k); //x = l ^ k
	
	//apply s function on x
	__m128i sres = s(x);
	
	//apply p function on s(x)
	res = p(sres);
	
	return res;
}

__m128i flandflinverse(__m128i ciphertext, __m128i flkey)
{
	__m128i res, tempkey;
	
	uint8_t flinitialshufflescalar[] = {0x80, 0x80, 0x80, 0x80, 0x07, 0x06, 0x05, 0x04, 0x0b, 0x0a, 0x09, 0x08, 0x80, 0x80, 0x80, 0x80};
	__m128i flinitialshuffle = get_m128i_variable_from_uint8_array (flinitialshufflescalar);
	
	uint8_t flfinalshufflescalar[] = {0x03, 0x02, 0x01, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x0f, 0x0e, 0x0d, 0x0c};
	__m128i flfinalshuffle = get_m128i_variable_from_uint8_array (flfinalshufflescalar);
	
	uint8_t kl1l32extracterscalar[] = {0x80, 0x80, 0x80, 0x80, 0x07, 0x06, 0x05, 0x04, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i kl1l32extracter = get_m128i_variable_from_uint8_array (kl1l32extracterscalar);
		
	uint8_t kl2r32extracterscalar[] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x0b, 0x0a, 0x09, 0x08, 0x80, 0x80, 0x80, 0x80};
	__m128i kl2r32extracter = get_m128i_variable_from_uint8_array (kl2r32extracterscalar);
	
	uint8_t kl1r32extracterscalar[] = {0x03, 0x02, 0x01, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i kl1r32extracter = get_m128i_variable_from_uint8_array (kl1r32extracterscalar);
	
	uint8_t kl2l32extracterscalar[] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x0f, 0x0e, 0x0d, 0x0c};
	__m128i kl2l32extracter = get_m128i_variable_from_uint8_array (kl2l32extracterscalar);
	
	uint8_t selectiveffarrayscalar1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00};
	__m128i selectiveffarray1 = get_m128i_variable_from_uint8_array (selectiveffarrayscalar1);
	
	uint8_t selectiveffarrayscalar2[] = {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	__m128i selectiveffarray2 = get_m128i_variable_from_uint8_array (selectiveffarrayscalar2);
	
	uint8_t rotpurifierscalar1[] = {0x07, 0x06, 0x05, 0x04, 0x80, 0x80, 0x80, 0x80, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08};
	__m128i rotpurifier1 = get_m128i_variable_from_uint8_array (rotpurifierscalar1);
	
	uint8_t rotpurifierscalar2[] = {0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x0f, 0x0e, 0x0d, 0x0c, 0x80, 0x80, 0x80, 0x80};
	__m128i rotpurifier2 = get_m128i_variable_from_uint8_array (rotpurifierscalar2);
	
	__m128i flrotleftamtfd = _mm_setr_epi32(0x00, 0x00, 0x01, 0x00);
	__m128i flrotrightamtfd = _mm_setr_epi32(0x00, 0x00, 0x1f, 0x00);
	
	__m128i flrotleftamtsd = _mm_setr_epi32(0x01, 0x00, 0x00, 0x00);
	__m128i flrotrightamtsd = _mm_setr_epi32(0x1f, 0x00, 0x00, 0x00);
	
	//1.shuffle ciphertext
	res = _mm_shuffle_epi8 (ciphertext, flinitialshuffle);
	
	//2.extract kl1l32 and AND with res
	tempkey = _mm_shuffle_epi8 (flkey, kl1l32extracter);
	tempkey = _mm_or_si128 (tempkey, selectiveffarray1);
	res = _mm_and_si128 (res, tempkey);
	
	//3.extract kl2r32 and OR with res
	tempkey = _mm_shuffle_epi8 (flkey, kl2r32extracter);
	res = _mm_or_si128 (res, tempkey);
	
	//4.left rotate by 1, the second doubleword of res
	//print128_num(flrotleftamtsd);
	//print128_num(flrotrightamtsd);
	__m128i kl1l32leftrot = _mm_sllv_epi32 (res, flrotleftamtsd);
	__m128i kl1l32rightrot = _mm_srlv_epi32 (res, flrotrightamtsd);
	__m128i kl1l32rotres = _mm_xor_si128 (kl1l32leftrot, kl1l32rightrot);
	res = _mm_shuffle_epi8 (res, rotpurifier1);
	res = _mm_xor_si128 (res, kl1l32rotres);
	
	//4.res = res ^ ciphertext
	ciphertext = _mm_xor_si128 (res, ciphertext);
	
	//5.shuffle res again
	res = _mm_shuffle_epi8 (ciphertext, flfinalshuffle);
	
	//6.extract kl1r32 and OR with res
	tempkey = _mm_shuffle_epi8 (flkey, kl1r32extracter);
	res = _mm_or_si128 (res, tempkey);
	
	//7.extract kl2l32 and AND wih res
	tempkey = _mm_shuffle_epi8 (flkey, kl2l32extracter);
	tempkey = _mm_or_si128 (tempkey, selectiveffarray2);
	res = _mm_and_si128 (res, tempkey);
	
	//8.left rotate by 1, the fourth doubleword of res
	__m128i kl2l32leftrot = _mm_sllv_epi32 (res, flrotleftamtfd);
	__m128i kl2l32rightrot = _mm_srlv_epi32 (res, flrotrightamtfd);
	__m128i kl2l32rotres = _mm_xor_si128 (kl2l32leftrot, kl2l32rightrot);
	res = _mm_shuffle_epi8 (res, rotpurifier2);
	res = _mm_xor_si128 (res, kl2l32rotres);
	
	//9. res = res ^ ciphertext
	res = _mm_xor_si128 (res, ciphertext);
	
	return res;
}

__m128i key_schedule1(__m128i keyleft, __m128i keyright)
{
	__m128i ka;
	__m128i ksconstant[4];
	
	uint8_t ksconstant1scalar[] = {0xa0, 0x9e, 0x66, 0x7f, 0x3b, 0xcc, 0x90, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t ksconstant2scalar[] = {0xb6, 0x7a, 0xe8, 0x58, 0x4c, 0xaa, 0x73, 0xb2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t ksconstant3scalar[] = {0xc6, 0xef, 0x37, 0x2f, 0xe9, 0x4f, 0x82, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t ksconstant4scalar[] = {0x54, 0xff, 0x53, 0xa5, 0xf1, 0xd3, 0x6f, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	ksconstant[0] = get_m128i_variable_from_uint8_array (ksconstant1scalar);
	ksconstant[1] = get_m128i_variable_from_uint8_array (ksconstant2scalar);
	ksconstant[2] = get_m128i_variable_from_uint8_array (ksconstant3scalar);
	ksconstant[3] = get_m128i_variable_from_uint8_array (ksconstant4scalar);
	
	//initial xor
	ka = _mm_xor_si128 (keyleft, keyright); //ka = kl ^ kr
	
	uint8_t kaleftextracterscalar[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i kaleftextracter = _mm_loadu_si128((const __m128i*)kaleftextracterscalar);
	
	uint8_t xresshufflescalar[] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	__m128i xresshuffle = _mm_loadu_si128((const __m128i*)xresshufflescalar);
	
	uint8_t kaquadexchangescalar[] = {0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	__m128i kaquadexchange = _mm_loadu_si128((const __m128i*)kaquadexchangescalar);
	
	//store result of f function in variable x
	__m128i x;
	
	int i;
	for (i=0; i<2; i++)
	{
		//1.extract left 64 bits of ka
		x = _mm_shuffle_epi8 (ka, kaleftextracter);
		
		//2.apply f function on x and ksconstant[i]
		x = f (x, ksconstant[i]);
		
		//3.move f result to lower half of x
		x = _mm_shuffle_epi8 (x, xresshuffle);
		
		//4.ka = x ^ ka
		ka = _mm_xor_si128 (x, ka);
		
		//5.shuffle quadwords of ka
		ka = _mm_shuffle_epi8 (ka, kaquadexchange);
	}
	
	ka = _mm_xor_si128 (keyleft, ka); //ka = ka ^ kl
	
	for (i=2; i<4; i++)
	{
		//1.extract left 64 bits of ka
		x = _mm_shuffle_epi8 (ka, kaleftextracter);

		//2.apply f function on x and ksconstant[i]
		x = f (x, ksconstant[i]);
		
		//3.move f result to lower half of x
		x = _mm_shuffle_epi8 (x, xresshuffle);
		
		//4.ka = x ^ ka
		ka = _mm_xor_si128 (x, ka);
		
		//5.shuffle quadwords of ka
		ka = _mm_shuffle_epi8 (ka, kaquadexchange);
	}
	
	return ka;
}

__m128i key_schedule2(__m128i ka, __m128i keyleft, __m128i keyright)
{
	__m128i kb;
	__m128i ksconstant[2];
	
	uint8_t ksconstant1scalar[] = {0x1d, 0x2d, 0x68, 0xde, 0xfa, 0x27, 0xe5, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t ksconstant2scalar[] = {0xfd, 0xc1, 0xe6, 0xb3, 0xc2, 0x88, 0x56, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	ksconstant[0] = _mm_loadu_si128((const __m128i*)ksconstant1scalar);
	ksconstant[1] = _mm_loadu_si128((const __m128i*)ksconstant2scalar);
	
	//initial xor
	kb = _mm_xor_si128 (ka, keyright); //kb = ka ^ kr
	
	uint8_t kaleftextracterscalar[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	__m128i kaleftextracter = _mm_loadu_si128((const __m128i*)kaleftextracterscalar);
	
	uint8_t xresshufflescalar[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07};
	__m128i xresshuffle = _mm_loadu_si128((const __m128i*)xresshufflescalar);
	
	uint8_t kbquadexchangescalar[] = {0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07};
	__m128i kbquadexchange = _mm_loadu_si128((const __m128i*)kbquadexchangescalar);
	
	//store result of f function in variable x
	__m128i x;
	
	int i;
	for (i=0; i<2; i++)
	{
		//1.extract left 64 bits of kb
		x = _mm_shuffle_epi8 (kb, kaleftextracter);
		
		//2.apply f function on x and ksconstant[i]
		x = f (x, ksconstant[i]);
		
		//3.move f result to lower half of x
		x = _mm_shuffle_epi8 (x, xresshuffle);
		
		//4.kb = x ^ kb
		kb = _mm_xor_si128 (x, kb);
		
		//5.shuffle quadwords of kb
		kb = _mm_shuffle_epi8 (kb, kbquadexchange);
	}
	
	return kb;
}

__m128i doleftrotation(__m128i target, uint8_t amt)
{
	uint8_t targetrotatescalar[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t quadwordswaperscalar[] = {0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	__m128i quadwordswaper = _mm_loadu_si128((const __m128i*)quadwordswaperscalar);
	__m128i targetrotate, targetleftrotate;
	__int64_t l, r;
	__m64 rotAmt;
	
	if (amt == 64)
	{
		//swap quadwords of target
		return _mm_shuffle_epi8 (target, quadwordswaper);
	}
	else if (amt < 64)
	{
		l = amt;
		r = amt;
		targetrotate = _mm_set_epi64x (l, r);
		targetleftrotate = _mm_sllv_epi64 (target, targetrotate);
		
		l = 64 - amt;
		r = 64 - amt;
		targetrotate = _mm_set_epi64x (l, r);
		target = _mm_srlv_epi64 (target, targetrotate);
		
		target = _mm_shuffle_epi8 (target, quadwordswaper);
		
		return _mm_xor_si128 (target, targetleftrotate);
	}
	else
	{
		l = 128 - amt;
		r = 128 - amt;
		targetrotate = _mm_set_epi64x (l, r);
		targetleftrotate = _mm_srlv_epi64 (target, targetrotate);
		
		l = amt - 64;
		r = amt - 64;
		targetrotate = _mm_set_epi64x (l, r);
		target = _mm_sllv_epi64 (target, targetrotate);
		target = _mm_shuffle_epi8 (target, quadwordswaper);
		
		return _mm_xor_si128 (target, targetleftrotate);
	}
}

__m128i getroundkey(int keyid, __m128i kl, __m128i kr, __m128i ka, __m128i kb, int keylength)
{
	__m128i roundkey;
	
	uint8_t leftextracterscalar[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	uint8_t rightextracterscalar[] = {0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	
	__m128i leftextracter = _mm_loadu_si128((const __m128i*)leftextracterscalar);
	__m128i rightextracter = _mm_loadu_si128((const __m128i*)rightextracterscalar);
	
	if(keylength == 128)
	{
		switch(keyid)
		{
			case 1:
				roundkey = doleftrotation (ka, 0);
				roundkey = _mm_shuffle_epi8 (roundkey, leftextracter);
				break;
			case 2:
				roundkey = doleftrotation (ka, 0);
				roundkey = _mm_shuffle_epi8 (roundkey, rightextracter);
				break;
			case 3:
				roundkey = doleftrotation (kl, 15);
				roundkey = _mm_shuffle_epi8 (roundkey, leftextracter);
				break;
			case 4:
				roundkey = doleftrotation (kl, 15);
				roundkey = _mm_shuffle_epi8 (roundkey, rightextracter);
				break;
			case 5:
				roundkey = doleftrotation (ka, 15);
				roundkey = _mm_shuffle_epi8 (roundkey, leftextracter);
				break;
			case 6:
				roundkey = doleftrotation (ka, 15);
				roundkey = _mm_shuffle_epi8 (roundkey, rightextracter);
				break;
			case 7:
				roundkey = doleftrotation (kl, 45);
				roundkey = _mm_shuffle_epi8 (roundkey, leftextracter);
				break;
			case 8:
				roundkey = doleftrotation (kl, 45);
				roundkey = _mm_shuffle_epi8 (roundkey, rightextracter);
				break;
			case 9:
				roundkey = doleftrotation (ka, 45);
				roundkey = _mm_shuffle_epi8 (roundkey, leftextracter);
				break;
			case 10:
				roundkey = doleftrotation (kl, 60);
				roundkey = _mm_shuffle_epi8 (roundkey, rightextracter);
				break;
			case 11:
				roundkey = doleftrotation (ka, 60);
				roundkey = _mm_shuffle_epi8 (roundkey, leftextracter);
				break;
			case 12:
				roundkey = doleftrotation (ka, 60);
				roundkey = _mm_shuffle_epi8 (roundkey, rightextracter);
				break;
			case 13:
				roundkey = doleftrotation (kl, 94);
				roundkey = _mm_shuffle_epi8 (roundkey, leftextracter);
				break;
			case 14:
				roundkey = doleftrotation (kl, 94);
				roundkey = _mm_shuffle_epi8 (roundkey, rightextracter);
				break;
			case 15:
				roundkey = doleftrotation (ka, 94);
				roundkey = _mm_shuffle_epi8 (roundkey, leftextracter);
				break;
			case 16:
				roundkey = doleftrotation (ka, 94);
				roundkey = _mm_shuffle_epi8 (roundkey, rightextracter);
				break;
			case 17:
				roundkey = doleftrotation (kl, 111);
				roundkey = _mm_shuffle_epi8 (roundkey, leftextracter);
				break;
			case 18:
				roundkey = doleftrotation (kl, 111);
				roundkey = _mm_shuffle_epi8 (roundkey, rightextracter);
				break;
		}
	}
	else
	{
		
	}
	
	return roundkey;
}

__m128i getflkey(int keyid, __m128i kl, __m128i kr, __m128i ka, __m128i kb, int keylength)
{
	__m128i flkey;
	
	if( keylength == 128 )
	{
		switch (keyid)
		{
			case 1:
				flkey = doleftrotation (ka, 30);
				break;
			case 2:
				flkey = doleftrotation (kl, 77);
				break;
		}
	}
	else
	{
		switch (keyid)
		{
			case 1:
				flkey = doleftrotation (kr, 30);
				break;
			case 2:
				flkey = doleftrotation (kl, 60);
				break;
			case 3:
				flkey = doleftrotation (ka, 77);
				break;
		}
	}
	
	return flkey;
}

__m128i getpostwhiteningkey(__m128i kl, __m128i kr, __m128i ka, __m128i kb, int keylength)
{
	__m128i postwhiteningkey;
	
	if( keylength == 128 )
	{
		postwhiteningkey = doleftrotation (ka, 111);
	}
	else
	{
		postwhiteningkey = doleftrotation (kb, 111);
	}

	return postwhiteningkey;
}

__m128i encrypt(__m128i plaintext, __m128i kl, __m128i kr, __m128i ka, __m128i kb, int keylength)
{
	__m128i ciphertext, roundkey, lefthalf, flkey, postwhiteningkey;
	int i;
	
	uint8_t ctleftextracterscalar[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	__m128i ctleftextracter = _mm_loadu_si128((const __m128i*)ctleftextracterscalar);
	
	uint8_t lhquadexchangescalar[] = {0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	__m128i lhquadexchange = _mm_loadu_si128((const __m128i*)lhquadexchangescalar);
	
	//1.prewhitening
	ciphertext = _mm_xor_si128 (plaintext, kl); //ciphertext = pt ^ kl
	
	//2.first six rounds of encryption
	for(i=1;i<=6;i++)
	{
		//2.1 obtain round key
		roundkey = getroundkey (i, kl, kr, ka, kb, keylength);
		
		//2.2 extract left 64 bits of cipher text
		lefthalf = _mm_shuffle_epi8 (ciphertext, ctleftextracter);
		
		//2.3 apply f function on lefthalf and roundkey
		lefthalf = f (lefthalf, roundkey);
		
		//2.4 swap quadwords of lefthalf
		lefthalf = _mm_shuffle_epi8 (lefthalf, lhquadexchange);
		
		//2.5 ciphertext = ciphertext ^ lefthalf
		ciphertext = _mm_xor_si128 (ciphertext, lefthalf);
		
		//2.6 swap quadwords of ciphertext
		ciphertext = _mm_shuffle_epi8 (ciphertext, lhquadexchange);
	}
	
	//3.apply fl and fl^-1 on ciphertext
	//3.1 obtain fl key
	flkey = getflkey (1, kl, kr, ka, kb, keylength);
	
	//3.2 apply function on ciphertext and flkey
	ciphertext = flandflinverse (ciphertext, flkey);
	
	//4.second six rounds of encryption
	for(i=7;i<=12;i++)
	{
		//4.1 obtain round key
		roundkey = getroundkey (i, kl, kr, ka, kb, keylength);
		
		//4.2 extract left 64 bits of cipher text
		lefthalf = _mm_shuffle_epi8 (ciphertext, ctleftextracter);
		
		//4.3 apply f function on lefthalf and roundkey
		lefthalf = f (lefthalf, roundkey);
		
		//4.4 swap quadwords of lefthalf
		lefthalf = _mm_shuffle_epi8 (lefthalf, lhquadexchange);
		
		//4.5 ciphertext = ciphertext ^ lefthalf
		ciphertext = _mm_xor_si128 (ciphertext, lefthalf);
		
		//4.6 swap quadwords of ciphertext
		ciphertext = _mm_shuffle_epi8 (ciphertext, lhquadexchange);
	}
	
	//5.apply fl and fl^-1 on ciphertext
	//5.1 obtain fl key
	flkey = getflkey (2, kl, kr, ka, kb, keylength);
	
	//5.2 apply function on ciphertext and flkey
	ciphertext = flandflinverse (ciphertext, flkey);
	
	//6.third six rounds of encryption
	for(i=13;i<=18;i++)
	{
		//6.1 obtain round key
		roundkey = getroundkey (i, kl, kr, ka, kb, keylength);
		
		//6.2 extract left 64 bits of cipher text
		lefthalf = _mm_shuffle_epi8 (ciphertext, ctleftextracter);
		
		//6.3 apply f function on lefthalf and roundkey
		lefthalf = f (lefthalf, roundkey);
		
		//6.4 swap quadwords of lefthalf
		lefthalf = _mm_shuffle_epi8 (lefthalf, lhquadexchange);
		
		//6.5 ciphertext = ciphertext ^ lefthalf
		ciphertext = _mm_xor_si128 (ciphertext, lefthalf);
		
		//6.6 swap quadwords of ciphertext
		ciphertext = _mm_shuffle_epi8 (ciphertext, lhquadexchange);
	}
	
	if( keylength > 128 )
	{
		//7.apply fl and fl^-1 on ciphertext
		//7.1 obtain fl key
		flkey = getflkey (3, kl, kr, ka, kb, keylength);
		
		//7.2 apply function on ciphertext and flkey
		ciphertext = flandflinverse (ciphertext, flkey);
		
		//8.fourth six rounds of encryption
		for(i=19;i<=24;i++)
		{
			//8.1 obtain round key
			roundkey = getroundkey (i, kl, kr, ka, kb, keylength);
			
			//8.2 extract left 64 bits of cipher text
			lefthalf = _mm_shuffle_epi8 (ciphertext, ctleftextracter);
			
			//8.3 apply f function on lefthalf and roundkey
			lefthalf = f (lefthalf, roundkey);
			
			//8.4 swap quadwords of lefthalf
			lefthalf = _mm_shuffle_epi8 (lefthalf, lhquadexchange);
			
			//8.5 ciphertext = ciphertext ^ lefthalf
			ciphertext = _mm_xor_si128 (ciphertext, lefthalf);
			
			//8.6 swap quadwords of ciphertext
			ciphertext = _mm_shuffle_epi8 (ciphertext, lhquadexchange);
		}
	}
	
	//9.swap quadwords of ciphertext, one last time
	ciphertext = _mm_shuffle_epi8 (ciphertext, lhquadexchange);
	
	//10.postwhitening
	//7.1 obtain fl key
	postwhiteningkey = getpostwhiteningkey (kl, kr, ka, kb, keylength);
	
	//7.2 apply function on ciphertext and flkey
	ciphertext = _mm_xor_si128 (ciphertext, postwhiteningkey);
	
	return ciphertext;
}
