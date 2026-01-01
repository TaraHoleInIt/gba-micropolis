#ifndef _INPUT_H_
#define _INPUT_H_

void inputUpdateVBlank( void );
 
bool inputIsHeld( int key );
bool inputIsDown( int key );
bool inputIsUp( int key );

uint32_t inputHeld( void );
uint32_t inputDown( void );
uint32_t inputUp( void );

#endif
