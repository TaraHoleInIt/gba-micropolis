#ifndef _TEXT_AND_DEBUG_H_
#define _TEXT_AND_DEBUG_H_

void textAndDebugInit( void );

void mgbaPrintf( const char* format, ... );
void textPrintf( const char* format, ... );
void textPrintfCenter( int y, const char* format, ... );

#endif
