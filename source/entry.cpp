#include <gba_console.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include "w_micropolis.h"

#include "timer.h"
#include "text_and_debug.h"
#include "save.h"

#include "Game.h"

int gettimeofday( struct timeval* tv, void* tzp );
uint32_t generateEntropy( void );
void irqVBlankPreGame( void );
void irqVBlankGame( void );
void processEvents( uint32_t tickNow );

IWRAM_DATA Micropolis* sim = nullptr;

volatile uint32_t frameCount = 0;

static volatile int needsRedraw = 0;

IWorldRenderer* renderer = nullptr;

static uint32_t seed = 0;

static volatile int gameReady = 0;

int gettimeofday( struct timeval* tv, void* tzp ) {
	uint32_t timeNow = timerMillis( );

	tv->tv_sec = ( timeNow / 1000 ) + seed;
	tv->tv_usec = timeNow * 1000;

	return 0;
}

void irqVBlankPreGame( void ) {
	//inputUpdateVBlank( );
	frameCount++;
}

void irqVBlankGame( void ) {
	game->vblank( );
	frameCount++;
}

#if 0
uint32_t generateEntropy( void ) {
	int y = ( ( SCREEN_HEIGHT / 8 ) / 2 ) - 2;
	uint32_t tickNow = 0;
	uint32_t result = 0;
	uint32_t lastHeld = 0;
	uint32_t held = 0;

	textPrintfCenter( y, "Generating entropy..." );
	textPrintfCenter( y + 1, "Mash buttons for a while." );
	textPrintfCenter( y + 2, "Press START to finish." );

	do {
		VBlankIntrWait( );
		scanKeys( );

		tickNow = timerMillis( );
		held = keysHeld( );

		if ( held != lastHeld ) {
			result |= tickNow;
			result <<= 8;
			result |= held;
			result <<= 8;

			lastHeld = held;
		}
	} while ( ! ( held & KEY_START) );

	textClearScreen( );

	return result;
}
#endif

#define REG_WAITCNT ( *( volatile u16* ) 0x04000204 )

int main( void ) {
	//REG_WAITCNT = ( 1 << 14 ) | 2;

	irqInit();
	irqSet( IRQ_VBLANK, irqVBlankPreGame );
	irqEnable( IRQ_VBLANK );

	textAndDebugInit( );
	timerInit( );

	game = new Game( );
	assert( game != nullptr );

	irqDisable( IRQ_VBLANK );
		irqSet( IRQ_VBLANK, irqVBlankGame );
	irqEnable( IRQ_VBLANK );

	while ( true ) {
		VBlankIntrWait( );
		game->runFrame( );
		// scanKeys( );

		// down = keysDown( );
		// held = keysHeld( );
		// up = keysUp( );

		// game.tick( timerMillis( ), down, held, up );
	}

	//seed = generateEntropy( );

	// sim = new Micropolis( );
	// assert( sim != nullptr );

	// cursor.init( sim );
	// setRenderer( &rendererMCGA );

	// sim->resourceDir = "rom:/";

	// //sim->generateSomeCity( 0xAABBCCDD );
	// //sim->loadScenario( SC_TOKYO );
	// //sim->loadScenario( SC_DETROIT, detroit_bin, detroit_bin_size );
	// sim->loadScenario( SC_TOKYO, tokyo_bin, tokyo_bin_size );
	// sim->setSpeed( 1 );
	// sim->setPasses( 1 );
	// sim->simTick( );
	// sim->simUpdate( );

	// irqDisable( IRQ_VBLANK );
	// 	irqSet( IRQ_VBLANK, irqVBlankGame );
	// irqEnable( IRQ_VBLANK );

	// gameReady = 1;

	// while ( true ) {
	// 	tickNow = timerMillis( );

	// 	processEvents( tickNow );
	
	// 	if ( tickNow >= nextSimTick ) {
	// 		sim->simTick( );

	// 		nextSimTick = tickNow + 100;
	// 		needsRedraw = 1;
	// 	}

	// 	if ( tickNow >= nextAnimationTime ) {
	// 		nextAnimationTime = tickNow + 200;

	// 		renderer->getViewport( left, right, top, bottom );

	// 		left>>= 3;
	// 		right>>= 3;
	// 		top>>= 3;
	// 		bottom >>= 3;

	// 		sim->animateTiles( left, top, left + ( SCREEN_WIDTH / 8 ), top + ( SCREEN_HEIGHT / 8 ) );
	// 		needsRedraw = 1;
	// 	}

	// 	VBlankIntrWait( );
	// }
}

// void processEvents( uint32_t tickNow ) {
// 	static uint32_t nextScrollTime = 0;
// 	static uint32_t nextPlaceTime = 0;
// 	int dx = 0;
// 	int dy = 0;
//     int down = 0;
//     int held = 0;
//     int up = 0;
// 	int cursorX = 0;
// 	int cursorY = 0;
// 	int scrollX = 0;
// 	int scrollY = 0;
	
// 	scanKeys( );

// 	down = keysDown( );
// 	held = keysHeld( );
// 	up = keysUp( );

// 	if ( ( held & KEY_SELECT ) && ( down & KEY_START ) ) {
// 		// Switch renderer
// 		if ( renderer == &rendererMCGA )
// 			setRenderer( &rendererTandy );
// 		else
// 			setRenderer( &rendererMCGA );

// 		needsRedraw = 1;
// 	}

// 	if ( tickNow >= nextScrollTime ) {
// 		dx = ( held & KEY_LEFT ) ? -1 : 0;
// 		dx = ( held & KEY_RIGHT ) ? 1 : dx;

// 		dy = ( held & KEY_UP ) ? -1 : 0;
// 		dy = ( held & KEY_DOWN ) ? 1 : dy;

// 		cursor.moveBy( dx, dy );

// 		cursorX = ( cursor.getX( ) * 8 ) + 4;
// 		cursorY = ( cursor.getY( ) * 8 ) + 4;

// 		scrollX = cursorX - ( SCREEN_WIDTH / 2 );
// 		scrollY = cursorY - ( SCREEN_HEIGHT / 2 );

// 		renderer->scrollTo( scrollX, scrollY );

// 		nextScrollTime = tickNow + 100;
// 		needsRedraw = 1;
// 	}

// 	if ( down & KEY_R )
// 		cursor.nextTool( );

// 	if ( down & KEY_L )
// 		cursor.prevTool( );

// 	if ( tickNow >= nextPlaceTime ) {
// 		if ( held & KEY_A ) {
// 			cursor.doTool( );
// 		}	

// 		nextPlaceTime = tickNow + 100;
// 	}
// }
