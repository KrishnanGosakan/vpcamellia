#define invalid_mode -1
#define ecb_mode 1
#define cbc_mode 2
#define ctr_mode 3
#define cfb_mode 4

void print128_num(__m128i var);
__m128i get_m128i_variable_from_chararray(char *input);

char* ecb_128_encrypt(char *plainText, char *key);
char* ecb_192_encrypt(char *plainText, char *key);
char* ecb_256_encrypt(char *plainText, char *key);
