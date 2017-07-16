#include "mbed.h"
#include "util.h"



int char2int(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  return -1;
}

// This function assumes src an array of 12 characters of
// [0-9a-fA-F] and target to be of length 6 bytes.
void str2mac(const char * src, uint8 * target)
{
  //converting the MAC to the white list format (big endian)
  for(int i=0; i<6; i++)
    *(target + 5 - i) = (char2int(src[i * 2])<<4) + char2int(src[i * 2 + 1]);
}

void print_mac(uint8 * mac)
{
  DBG("%02X%02X%02X%02X%02X%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
}

void phex(uint8_t* str) {
    unsigned char i;
    for(i = 0; i < 16; ++i)
        DBG("%.2x", str[i]);
    DBG("\n");
}

int average_array(int nums[40]){
  int sum = 0;
  for(int i=0; i<40; i++){
    sum+=nums[i];
  }
  return sum/40;
}
