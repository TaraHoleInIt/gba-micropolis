#ifndef _SAVE_H_
#define _SAVE_H_

bool saveInit( void );

void sramRead( int offset, uint8_t* dest, int length );
void sramWrite( int offset, const uint8_t* src, int length );

#endif
