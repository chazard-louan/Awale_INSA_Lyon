//
// Created by louan on 21/10/2026.
//

#ifndef AWALE_CONSTANTS_H
#define AWALE_CONSTANTS_H
unsigned int USERNAME_SIZE = 61;
unsigned int BUFFER_SIZE = 1024;
unsigned int MAX_PLAYER_ONLINE = 100;

inline unsigned int next_pow2(unsigned int size)
{
  unsigned int log2 = 0;
  while (size != 0){size = size >> 1; log2++;}
  return 1 << log2;
};



#endif //AWALE_CONSTANTS_H
