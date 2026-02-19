#include <gba_base.h>
#include <fat.h>
#include <string.h>

#include "w_micropolis.h"
#include "save.h"

bool saveInit( void ) {
    return true;
}

void sramRead( int offset, uint8_t* dest, int length ) {
    volatile const uint8_t* sram = ( volatile const uint8_t* ) SRAM;

    assert( ( offset + length ) <= 32768 );

    sram+= offset;

    while ( length-- )
        *dest++ = *sram++;
}

void sramWrite( int offset, const uint8_t* src, int length ) {
    volatile uint8_t* sram = ( volatile uint8_t* ) SRAM;

    assert( ( offset + length ) <= 32768 );

    sram+= offset;

    while ( length-- )
        *sram++ = *src++;
}
