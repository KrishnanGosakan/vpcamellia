#include <emmintrin.h>
#include <tmmintrin.h>
#include <immintrin.h>

__m128i get_m128i_variable_from_uint8_array(uint8_t *inputArray);

__m128i key_schedule1(__m128i keyleft, __m128i keyright);

__m128i key_schedule2(__m128i ka, __m128i keyleft, __m128i keyright);

__m128i encrypt(__m128i plaintext, __m128i kl, __m128i kr, __m128i ka, __m128i kb, int keylength);
