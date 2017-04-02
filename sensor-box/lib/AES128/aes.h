#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>

#ifndef CBC
  #define CBC 1
#endif

#ifndef ECB
  #define ECB 1
#endif



//#if defined(ECB) && ECB

void AES128_ECB_encrypt(uint8_t* input, const uint8_t* key, uint8_t *output);
void AES128_ECB_decrypt(uint8_t* input, const uint8_t* key, uint8_t *output);

//#endif // #if defined(ECB) && ECB


//#if defined(CBC) && CBC

void AES128_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
void AES128_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);

//#endif // #if defined(CBC) && CBC



#endif //_AES_H_
