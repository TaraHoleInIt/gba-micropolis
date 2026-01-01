#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>

static int held = 0;
static int pressed = 0;
static int up = 0;

void inputUpdateVBlank( void ) {
    scanKeys( );

    held = keysHeld( );
    pressed = keysDown( );
    up = keysUp( );
}

bool inputIsHeld( int key ) {
    return ( held & key ) != 0;
}

bool inputIsDown( int key ) {
    return ( pressed & key ) != 0;
}

bool inputIsUp( int key ) {
    return ( up & key ) != 0;
}

uint32_t inputHeld( void ) {
    return held;
}

uint32_t inputDown( void ) {
    return pressed;
}

uint32_t inputUp( void ) {
    return up;
}
